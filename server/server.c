/*
*  C Implementation: server.c
*
*       Description: Server program for LAN-based play in Tux,of Math Command.
*
*
* Author: Akash Gangil, David Bruce, and the TuxMath team, (C) 2009
* Developers list: <tuxmath-devel@lists.sourceforge.net>
*
* Copyright: See COPYING file that comes with this distribution.  (Briefly, GNU GPL).
*
* NOTE: This file was initially based on example code from The Game Programming Wiki
* (http://gpwiki.org), in a tutorial covered by the GNU Free Documentation License 1.2.
* No invariant sections were indicated, and no separate license for the example code
* was listed. The author was also not listed. AFAICT,this scenario allows incorporation of
* derivative works into a GPLv2+ project like TuxMath - David Bruce 
*/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include "server.h" 
#include "transtruct.h"
#include "mathcards.h"


#define MAX_CLIENTS 16



/* Local function prototypes: */
int setup_server(void);
void cleanup_server(void);
void update_clients(void);
void test_connections(void);
void ping_client(int i);
int check_messages(void);
int handle_client_game_msg(int i,char *buffer);
void handle_client_nongame_msg(int i,char *buffer);
int find_vacant_client(void);
void remove_client(int i);
void game_msg_correct_answer(int i, int id);
void game_msg_quit(int i);
void game_msg_exit(int i);
void start_game(int i);
int SendQuestion(MC_FlashCard flash, TCPsocket client_sock);
int SendMessage(int message, int ques_id,char *name, TCPsocket client_sock);
int player_msg(int i, char* msg);
void broadcast_msg(char* msg);
void throttle(int loop_msec);
void game_msg_next_question(void);
void game_msg_total_questions_left(int i);

/* "Local globals" for server.c:   */
TCPsocket server_sock = NULL; /* Socket descriptor for server            */
IPaddress ip;
SDLNet_SocketSet client_set = NULL,temp_sock=NULL,temp_set=NULL;
static client_type client[MAX_CLIENTS];
static int num_clients = 0;
static int numready = 0;
static int game_in_progress = 0;
static int quit = 0;
static int frame = 0;
MC_FlashCard flash;




int main(int argc, char **argv)
{ 
  printf("Started tuxmathserver, waiting for client to connect:\n>\n");

  /*     ---------------- Setup: ---------------------------   */
  if (!setup_server())
  {
    fprintf(stderr, "setup_server() failed - exiting.\n");
    cleanup_server();
    exit(EXIT_FAILURE);
  }
 
  /*    ------------- Main server loop:  ------------------   */
  while (!quit)
  {
//    frame++;
//    /* See if our existing clients are really still there. For */
//    /* performance reasons, we don't do this on every loop:    */
//    if(frame%1000 == 0)
//      test_connections();

    /* Now we check to see if anyone is trying to connect. */
    update_clients();
    /* Check for any pending messages from clients already connected: */
    check_messages();
    /* Limit frame rate to keep from eating all CPU: */
    /* NOTE almost certainly could make this longer wtihout noticably */
    /* affecting performance, but even throttling to 1 msec/loop cuts */
    /* CPU from 100% to ~2% on my desktop - DSB                       */
    throttle(1);  //min loop time 1 msec
  }
   
  /*   -----  Free resources before exiting: -------    */
  cleanup_server();

  return EXIT_SUCCESS;
}


/*********************************************************************/
/*  "Private" (to server.c) functions                                */
/*********************************************************************/


// setup_server() - all the things needed to get server running:
int setup_server(void)
{
  int i = 0;

  for(i = 0; i < MAX_CLIENTS; i++)
  {
    client[i].game_ready = 0;   /* waiting for user to OK game start */
    client[i].name[0] = '\0';   /* no nicknames yet                  */
    client[i].sock = NULL;      /* sockets start out unconnected     */
  }

  //this sets up mathcards with hard-coded defaults - no settings
  //read from config file here:
  if (!MC_Initialize())
  {
    fprintf(stderr, "Could not initialize MathCards\n");
    return 0;
  }

      
  if (SDLNet_Init() < 0)
  {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    return 0;
  }
 
  /* Resolving the host using NULL make network interface to listen */
  if (SDLNet_ResolveHost(&ip, NULL, DEFAULT_PORT) < 0)
  {
    fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return 0;
  }
 
  /* Open a connection with the IP provided (listen on the host's port) */
  if (!(server_sock = SDLNet_TCP_Open(&ip)))
  {
    fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    return 0;
  }

  client_set = SDLNet_AllocSocketSet(MAX_CLIENTS);
  if(!client_set)
  {
    printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
    return 0;
  }
 return 1;
}



//Free resources, closing sockets, call MC_EndGame(), and so forth:
void cleanup_server(void)
{
  int i;
  /* Close the client socket(s) */
  
  for(i = 0; i < MAX_CLIENTS; i++)
  {
    if(client[i].sock != NULL)
    {
      SDLNet_TCP_Close(client[i].sock);    //close all the client sockets one by one
      client[i].sock = NULL;               // So we don't segfault in case cleanup()
    }                                      // somehow gets called more than once.
  } 

  if (client_set != NULL)
  {
    SDLNet_FreeSocketSet(client_set);    //releasing the memory of the client socket set
    client_set = NULL;                   //this helps us remember that this set is not allocated
  } 

  if(server_sock != NULL)
  {
    SDLNet_TCP_Close(server_sock);
    server_sock = NULL;
  }

  

 /* Clean up mathcards heap memory */
  MC_EndGame();
}




//update_clients() sees if anyone is trying to connect, and connects if a slot
//is open and the game is not in progress. The purpose is to make sure our
//client set accurately reflects the current state.
void update_clients(void)
{
  TCPsocket temp_sock = NULL;        /* Just used when client can't be accepted */
  int slot = 0;
  int sockets_used = 0;
  char buffer[NET_BUF_LEN];


  /* See if we have a pending connection: */
  temp_sock = SDLNet_TCP_Accept(server_sock);
  if (!temp_sock)  /* No one waiting to join - do nothing */
  {
    return;   // Leave num_clients unchanged
  }

  // See if any slots are available:
  slot = find_vacant_client();
  if (slot == -1) /* No vacancies: */
  {
    snprintf(buffer, NET_BUF_LEN, 
             "%s\n",
             "Sorry, already have maximum number of clients connected\n");
    SDLNet_TCP_Send(temp_sock, buffer, NET_BUF_LEN);
    //hang up:
    SDLNet_TCP_Close(temp_sock);
    temp_sock = NULL;
#ifdef LAN_DEBUG
    printf("buffer sent:\n");
    printf("buffer is: %s\n", buffer);
#endif
    return;   // Leave num_clients unchanged
  }     

  // If game already started, send our regrets:
  if(game_in_progress)
  {
    snprintf(buffer, NET_BUF_LEN, 
             "%s\n",
             "Sorry the game has started...... =(\n");
    SDLNet_TCP_Send(temp_sock, buffer, NET_BUF_LEN);
    //hang up:
    SDLNet_TCP_Close(temp_sock);
    temp_sock = NULL;
#ifdef LAN_DEBUG
    printf("buffer sent\n");
    printf("buffer is: %s\n", buffer);
#endif
    return;   // Leave num_clients unchanged
  }

  // If we get to here, we have room for the new connection and the
  // game is not in progress, so we connect:

#ifdef LAN_DEBUG
  printf("creating connection for client[%d].sock:\n", slot);
#endif

  client[slot].sock = temp_sock;

  /* We are receiving the client's name that was entered:       */
  /* NOTE this will block if the client doesn't send anything,  */
  /* making an easy target for a DOS attack - DSB               */
  if( SDLNet_TCP_Recv(client[slot].sock, buffer, NET_BUF_LEN) > 0)
  {
    strncpy(client[slot].name, buffer, NAME_SIZE);
    printf(" JOINED  :::   %s\n", client[slot].name);
    //Announcement for all clients:
    snprintf(buffer, NET_BUF_LEN, 
          "Player %s has connected to the server\n",
           client[slot].name);             
    broadcast_msg(buffer);


    /* Add client socket to set: */
    sockets_used = SDLNet_TCP_AddSocket(client_set, client[slot].sock);
    if(sockets_used == -1) //No way this should happen
    {
      printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
      cleanup_server();
      exit(EXIT_FAILURE);
    }
  }
  else   //This means the socket didn't connect successfully
  {
    printf("SDLNet_TCP_Recv() failed");
    remove_client(slot);
    return;   // Leave num_clients unchanged
  }

  /* At this point num_clients can be updated: */
  num_clients = sockets_used;

  /* Now we can communicate with the client using client[i].sock socket
  /* serv_sock will remain opened waiting other connections */
    

#ifdef LAN_DEBUG
  /* Get the remote address */
  {
    IPaddress* client_ip = NULL;
    client_ip = SDLNet_TCP_GetPeerAddress(client[slot].sock);

    printf("num_clients = %d\n", num_clients);
    if (client_ip != NULL)
    /* Print the address, converting in the host format */
    {
      printf("Client connected\n>\n");
      printf("Client: IP = %x, Port = %d\n",
             SDLNet_Read32(&client_ip->host),
             SDLNet_Read16(&client_ip->port));
    }
    else
      fprintf(stderr, "SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError());
  }
#endif

  return;
}



// check_messages() is where we look at the client socket set to see which 
// have sent us messages. This function is used in each server loop whether
// or not a math game is in progress (although we expect different messages
// during a game from those encountered outside of a game)

int check_messages(void)
{
  int i = 0, c = 0;
  int actives = 0;
  int ready_found = 0;
  char buffer[NET_BUF_LEN];


  //NOTE Does this belong here? Seems to have more to do wth client connections.
  if(game_in_progress==1)
  {
    for(i = 0; i < MAX_CLIENTS; i++)
    {
      if(client[i].sock != NULL)
      {
        c = 1;
        break;
      }
    }
    if(c == 0)
    {
      printf("All the clients have been disconnected....\n");
      //We just need to clean up to start a new math game
      cleanup_server();
      setup_server();
      game_in_progress = 0;
      return 0;
    }
  }



  /* Check the client socket set for activity: */
  actives = SDLNet_CheckSockets(client_set, 0);
//  printf("in check_messages(), actives = %d\n", actives);
  if(actives == -1)
  {
    printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
    //most of the time this is a system error, where perror might help you.
    perror("SDLNet_CheckSockets");
  }

  else if(actives) 
  {
#ifdef LAN_DEBUG
    printf("There are %d sockets with activity\n", actives);
#endif

    // check all sockets with SDLNet_SocketReady and handle the active ones.
    // NOTE we have to check all the slots in the set because
    // the set will become discontinuous if someone disconnects
    for(i = 0; i < MAX_CLIENTS; i++)
    {
      if((client[i].sock != NULL)
        && (SDLNet_SocketReady(client[i].sock))) 
      { 
        ready_found++;

#ifdef LAN_DEBUG
        printf("client socket %d is ready\n", i);
#endif
        if (SDLNet_TCP_Recv(client[i].sock, buffer, NET_BUF_LEN) > 0)
        {
#ifdef LAN_DEBUG
          printf("buffer received from client %d is: %s\n", i, buffer);
#endif

          /* Here we pass the client number and the message buffer */
          /* to a suitable function for further action:                */
          if(game_in_progress)
            handle_client_game_msg(i, buffer);
          else
            handle_client_nongame_msg(i, buffer);
        }
        else  // Socket activity but cannot receive - client invalid
        {
          printf("Client %d active but receive failed - apparently disconnected\n>\n", i);
          remove_client(i);
        }
      }
    }  // end of for() loop - all client sockets checked
    // Make sure all the active sockets reported by SDLNet_CheckSockets()
    // are accounted for:

    if(actives != ready_found )
    {
      printf("Warning: SDLNet_CheckSockets() reported %d active sockets,\n"
             "but only %d detected by SDLNet_SocketReady()\n", actives, ready_found);
      //Presently, this just runs ping_client() on all the sockets:
      test_connections();
    }
  } 
}



void handle_client_nongame_msg(int i,char *buffer)
{
  if(strncmp(buffer, "START_GAME", strlen("START_GAME")) == 0)
  {
    start_game(i);
  }
}


int handle_client_game_msg(int i , char *buffer)
{
  int id;
  char command[NET_BUF_LEN];

#ifdef LAN_DEBUG  
  printf("Buffer received from client: %s\n", buffer);
#endif

  sscanf (buffer,"%s %d\n",
                  command,
                  &id);


  if(strncmp(command, "CORRECT_ANSWER", strlen("CORRECT_ANSWER")) == 0)
  {
    game_msg_correct_answer(i,id);
  }                            

  else if(strncmp(command, "TOTAL_QUESTIONS_LEFT",strlen("TOTAL_QUESTIONS_LEFT")) == 0) /* Send Total Questions left */
  {
    game_msg_total_questions_left(i);
  }


  else if(strncmp(command, "MISSION_ACCOMPLISHED",strlen("MISSION_ACCOMPLISHED")) == 0) /* Check to see if mission is over*/
  {
    game_msg_mission_accomplished(i);
  }

  else if(strncmp(command, "NEXT_QUESTION",strlen("NEXT_QUESTION")) == 0) /* Send Next Question */
  {
    game_msg_next_question();
  }

  else if(strncmp(command, "exit",strlen("exit")) == 0) /* Terminate this connection */
  {
    game_msg_exit(i);
  }

  else if(strncmp(command, "quit",strlen("quit")) == 0) /* Quit the program */
  {
    game_msg_quit(i);
    return(1);
  }
  else
  {
    printf("command %s not recognized\n", command);
  }
  return(0);
}


void game_msg_total_questions_left(int i)
{
 int total_questions_left;
 char ch[10];
 total_questions_left=MC_TotalQuestionsLeft();
 snprintf(ch,10,"%d",total_questions_left); 
 player_msg(i,ch);
}

void game_msg_mission_accomplished(int i)
{
 int total_questions_left;
 char ch[10];
 total_questions_left=MC_MissionAccomplished();
 snprintf(ch,10,"%d",total_questions_left); 
 player_msg(i,ch);
}

void game_msg_correct_answer(int i, int id)
{
  int n;
  char buf[NET_BUF_LEN];
printf("game_msg_correct_answer() called\n");
  //Announcement for server and all clients:
  snprintf(buf, NET_BUF_LEN, 
          "question id %d was answered correctly by %s\n",
          id, client[i].name);             
  printf("%s", buf);
  broadcast_msg(buf);

  //Tell mathcards so lists get updated:
  MC_AnsweredCorrectly_id(id);

}

void game_msg_next_question(void)
{

  int n;
  if (!MC_NextQuestion(&flash))
  { 
    /* no more questions available */
    printf("MC_NextQuestion() returned NULL - no questions available\n");
  }
  else
  {                                     
#ifdef LAN_DEBUG
    printf("WILL SEND >>\n");  
    printf("QUESTION_ID       :      %d\n", flash.question_id);
    printf("FORMULA_STRING    :      %s\n", flash.formula_string);
    printf("ANSWER STRING     :      %s\n", flash.answer_string);
    printf("ANSWER            :      %d\n",flash.answer);
    printf("DIFFICULTY        :      %d\n",flash.difficulty);
#endif
  }
                  
  for(n = 0; n < MAX_CLIENTS && client[n].sock; n++)
  {
#ifdef LAN_DEBUG
    printf("About to send next question to client[%d]\n", n);
#endif
    if(!SendQuestion(flash, client[n].sock))
    {
      printf("Unable to send Question\n");
    }
  } 
}

// Go through and test all the current connections, removing
// any clients that fail to respond:
void test_connections(void)
{
  int i = 0;

  for (i = 0; i < MAX_CLIENTS; i++)
    ping_client(i);
}


// This is supposed to be a way to test and see if each client
// is really connected.
// FIXME I think we need to put in a SDLNet_TCP_Recv() to see
// if we get a reply, now that the client is modified to send back
// PING_BACK.  I am worried, however, that we could have a problem
// with intercepting messages not related to the ping testing - DSB

void ping_client(int i)
{
  char buf[NET_BUF_LEN];
  char msg[NET_BUF_LEN];
  int x;

  if(i < 0 || i > MAX_CLIENTS)
  {
    printf("ping_client() - invalid index argument\n");
    return;
  }
  
  if(client[i].sock == NULL)
  {
    return;
  }
  
//  sprintf(msg,"%s", "PING\n");
//  snprintf(buf, NET_BUF_LEN, "%s\t%s\n", "SEND_MESSAGE", msg);
  snprintf(buf, NET_BUF_LEN, "%s\n", "PING");
  x = SDLNet_TCP_Send(client[i].sock, buf, NET_BUF_LEN);
  if(x < NET_BUF_LEN)
  {
   printf("The client %s is disconnected\n",client[i].name);
   remove_client(i);
  }
//#ifdef LAN_DEBUG
  printf("buf is: %s\n", buf);
  printf("SendMessage() - buf sent:::: %d bytes\n", x);
//#endif
}


void game_msg_exit(int i)
{
  printf("LEFT the GAME : %s",client[i].name);
  remove_client(i);
}

//FIXME don't think we want to allow players to shut down the server
void game_msg_quit(int i)
{
  printf("Server has been shut down by %s\n",client[i].name); 
  cleanup_server();
  exit(9);                           // '9' means exit ;)  (just taken an arbitary no:)
}


void start_game(int i)
{
  char buf[NET_BUF_LEN];
  char buffer[NET_BUF_LEN];
  int x,j;
  game_in_progress = 1;  //setting the game_in_progress flag to '1'
  snprintf(buf, NET_BUF_LEN,
                "Player %s added for next math game\n",
                client[i].name);

  x = SDLNet_TCP_Send(client[i].sock, buf, sizeof(buf));
  client[i].game_ready = 1; // Means this player is ready to start game

  /*FIXME this will only work if the clients are in a contiguous series starting */
  /* at client[0].                                                             */
  /* Basically , when the clients are allocated sockets , they are allocated contiguos
     locations  starting from client[0] , so I dont think this will fail anytime.*/

  /*This loop sees that the game starts only when all the players are ready */
  for(j = 0; j < num_clients; j++)
  {
    if(client[j].game_ready != 1)
    {
      if (SDLNet_TCP_Recv(client[j].sock, buffer, NET_BUF_LEN) > 0)
      {
        if(strncmp(buffer, "START_GAME", 10) == 0)
        {
          client[j].game_ready = 1;
          snprintf(buf, NET_BUF_LEN, 
                "%s\n",
                "Success");
          x = SDLNet_TCP_Send(client[j].sock, buf, NET_BUF_LEN);
        }
      }
    }
  }

  /* If no players join the game (should not happen) */
  if(num_clients == 0)
  {
    printf("There were no players........=(\n");
    cleanup_server();
    exit(1);
  }

#ifdef LAN_DEBUG
  printf("We have %d players.......\n", num_clients);
#endif


  //Start a new math game as far as mathcards is concerned:
  if (!MC_StartGame())
  {
    fprintf(stderr, "\nMC_StartGame() failed!");
    exit(1);
  }

  game_in_progress = 1;

  if (!MC_NextQuestion(&flash))
  { 
    /* no more questions available */
    printf("MC_NextQuestion() returned NULL - no questions available\n");
    exit(1);
  }
  else
  {                                     
#ifdef LAN_DEBUG
    printf("WILL SEND >>\n");  
    printf("QUESTION_ID       :      %d\n", flash.question_id);
    printf("FORMULA_STRING    :      %s\n", flash.formula_string);
    printf("ANSWER STRING     :      %s\n", flash.answer_string);
    printf("ANSWER            :      %d\n",flash.answer);
    printf("DIFFICULTY        :      %d\n",flash.difficulty);
#endif
  }

  for(j = 0; j < num_clients; j++)
  {
    if(!SendQuestion(flash, client[j].sock))
    {
      printf("Unable to send Question to %s\n", client[j].name);
    }
  } 
}


//Returns the index of the first vacant client, or -1 if all clients full
int find_vacant_client(void)
{
  int i = 0;
  while (client[i].sock && i < MAX_CLIENTS)
    i++;
  if (i == MAX_CLIENTS)
  {
    fprintf(stderr, "All clients checked, none vacant\n");
    i = -1;
  }
  return i;
}


void remove_client(int i)
{
  printf("Removing client[%d] - name: %s\n>\n", i, client[i].name);

  SDLNet_TCP_DelSocket(client_set,client[i].sock);

  if(client[i].sock != NULL)
    SDLNet_TCP_Close(client[i].sock);

  client[i].sock = NULL;  
  client[i].game_ready = 0;
  client[i].name[0] = '\0';
}


//function to send a flashcard(question) from the server to the client
int SendQuestion(MC_FlashCard flash,TCPsocket client_sock)
{
  int x;

  char buf[NET_BUF_LEN];
  snprintf(buf, NET_BUF_LEN, 
                "%s\t%d\t%d\t%d\t%s\t%s\n",
                "SEND_QUESTION",
                flash.question_id,
                flash.difficulty,
                flash.answer,
                flash.answer_string,
                flash.formula_string);
  x = SDLNet_TCP_Send(client_sock, buf, sizeof(buf));

#ifdef LAN_DEBUG
  printf("SendQuestion() - buf sent:::: %d bytes\n", x);
  printf("buf is: %s\n", buf);
#endif

  if (x == 0)
    return 0;
  return 1;
}


/*Function to send any messages to the client be it any warnings
  or anything the client is made to be informed*/
int SendMessage(int message, int ques_id,char *name,TCPsocket client_sock)         
{
 int x, len;
 char buf[NET_BUF_LEN];
 char msg[100];  

 /* Create appropriate message: */
  switch(message)
  {
    case NO_QUESTION_LIST:
      sprintf(msg,"%s", "Please! first setup the question list by typing <a>\n");
      break;
    case ANSWER_CORRECT:
      sprintf(msg,"%s %d %s %s", "Question ID:",
              ques_id, "was answered correctly by the client",name);
      break;
   case LIST_SET_UP:
      sprintf(msg,"%s", "Question list was successfully setup\n");
      break;
   default :
     fprintf(stderr, "SendMessage() - unrecognized message type\n");
     return 0;
  }
  //transmit:
  snprintf(buf, NET_BUF_LEN, "%s\t%s\n", "SEND_MESSAGE", msg);
  x = SDLNet_TCP_Send(client_sock, buf, NET_BUF_LEN);

#ifdef LAN_DEBUG
  printf("buf is: %s\n", buf);
  printf("SendMessage() - buf sent:::: %d bytes\n", x);
#endif

  return 1;
}



/* Sends a string to be displayed to player: */
/* NOTE similar in concept to SendMessage(), but I think that */
/* SendMessage() is too complicated -DSB                      */
int player_msg(int i, char* msg)
{
  char buf[NET_BUF_LEN];

  //Validate arguments;
  if(i < 0 || i > MAX_CLIENTS)
  {
#ifdef LAN_DEBUG
    printf("player_msg() - invalid index argument\n");
#endif
  return 0;
  }
  
  if(!client[i].sock)
  {
#ifdef LAN_DEBUG
    printf("player_msg() - client socket is NULL\n");
#endif
  return 0;
  }
  
  if(!msg)
  {
#ifdef LAN_DEBUG
    printf("player_msg() - msg argument is NULL\n");
#endif
  return 0;
  }

  //transmit:
  snprintf(buf, NET_BUF_LEN, "%s\t%s\n", "PLAYER_MSG", msg);
  if(SDLNet_TCP_Send(client[i].sock, buf, NET_BUF_LEN) < NET_BUF_LEN)
  {
    printf("The client %s is disconnected\n", client[i].name);
    remove_client(i);
    return 0;
  }
  //Success:
  return 1;
}

/* Send the message to all clients: */
void broadcast_msg(char* msg)
{
  int i = 0;
  if (!msg)
    return;
  for(i = 0; i < MAX_CLIENTS; i++)
    player_msg(i, msg);   
}


/* Simple function that returns a minimum of 'loop_msec' */
/* milliseconds after it returned the previous time it   */
/* was called.                                           */
void throttle(int loop_msec)
{
  static Uint32 now_t, last_t; //These will be zero first time through
  int wait_t;

  //Target loop time must be between 0 and 100 msec:
  if(loop_msec < 0)
    loop_msec = 0;
  if(loop_msec > 100)
    loop_msec = 100;

  if (now_t == 0)  //For sane behavior first time through:
    last_t = SDL_GetTicks();
  else
    last_t = now_t;
  now_t = SDL_GetTicks();
  wait_t = (last_t + loop_msec) - now_t;

  //Avoid problem if we somehow wrap past uint32 size
  if(wait_t < 0)
    wait_t = 0;
  if(wait_t > loop_msec)
    wait_t = loop_msec;

  SDL_Delay(wait_t);
}
