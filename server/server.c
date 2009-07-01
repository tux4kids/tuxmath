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


// NOTE: here are some pseudo-code thoughts about structuring the server program - DSB:
// 
// int main()
// {
//   //All the stuff before starting the main loop:
//   if (!setup_server())
//   {
//     //print error and exit...
//   }
// 
//   while (!done)
//   {
//     //See if any clients trying to connect:
//     //If we aren't playing a game yet, we take their name and add them to the client set,
//     //if we have room.
//     //If a game is in progress, we just tell them "sorry, call back later"
//     check_for_new_clients();
//     
//     //Now check all the connected clients to see if we have any messages from them:
//     check_client_messages();
// 
//   }
// 
//   //Free resources, call MC_EndGame(), and so forth:
//   server_cleanup();
// }
// 
// 
// //This function will use SDLNet_CheckSockets() and SDLNet_SocketReady()
// //Some of the messages will only be meaningful between games, and others will
// //only be meaningful during games;
// void check_client_messages(void)
// {
//   for (go through client set and handle active ones)
//   {
//     get_buffer_from_client();
//     get_title_out_of_buffer();
// 
//     //Handle all the 'non-game'messages:
//     if (strcmp(title, NON_GAME_FOO) == 0)
//     {
//       //do something (e.g. disconnect)
//     }
//     else if (strcmp(title, NON_GAME_BAR) == 0)
//     {
//       //do something else (e.g. start a new game)
//     }
// 
//     //Now handle gameplay-related messages:
//     if (strcmp(title, GAME_FOO) == 0)
//     {
//       //do something (e.g. client answered correctly)
//     }
//     else if (strcmp(title, GAME_BAR) == 0)
//     {
//       //do something else (e.g. client quits game)
//     }
// 
//   }
// }







#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include "server.h" 
#include "transtruct.h"
#include "mathcards.h"


#define MAX_CLIENTS 16

TCPsocket server_sock = NULL; /* Socket descriptor for server            */
TCPsocket temp_sock = NULL;        /* Just used when client can't be accepted */
IPaddress ip;
SDLNet_SocketSet client_set = NULL;
static client_type client[MAX_CLIENTS];
static int num_clients = 0;
static int sockets_used = 0;
static int numready = 0;
static int game_in_progress = 0;
static int quit = 0; 
MC_FlashCard flash;

/* Local function prototypes: */
int setup_server(void);
void cleanup_server(void);
int update_clients(void);
int check_messages(void);
int handle_client_game_msg(int i,char *buffer);
void handle_client_nongame_msg(int i,char *buffer);
int find_vacant_client(void);
void remove_client(int i);
void game_msg_correct_answer(int i, int id);
void game_msg_quit(int i);
void game_msg_exit(int i);
void start_game(int i);
void ping_client(int i);
int SendQuestion(MC_FlashCard flash, TCPsocket client_sock);
int SendMessage(int message, int z, TCPsocket client_sock);

int main(int argc, char **argv)
{ 
  int h;
  int quit2;
  char buffer[NET_BUF_LEN];
  char buf[NET_BUF_LEN];
  int command_type = -1, j;
  static int i = 0;
  static int initialize = 0;
  int id,x;

  printf("Started tuxmathserver, waiting for client to connect:\n>\n");


  /*     ---------------- Setup: ---------------------------   */
  if (!setup_server())
  {
    fprintf(stderr, "setup_server() failed - exiting.\n");
    exit(EXIT_FAILURE);
  }
 
  /*    ------------- Main server loop:  ------------------   */
  while (!quit)
  {
    /* Now we check to see if anyone is trying to connect. */
    num_clients = update_clients();
    /* Check for any pending messages from clients already connected: */
    check_messages();
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
  int j;
  /* Close the client socket(s) */
  
  for(j = 0; j < MAX_CLIENTS; j++)
  {
    if(client[j].sock != NULL)
    {
      SDLNet_TCP_Close(client[j].sock);    //close all the client sockets one by one
      client[j].sock = NULL;               // So we don't segfault in case cleanup()
    }                                      // somehow gets called more than once.
  } 

  if (client_set != NULL)
  {
    SDLNet_FreeSocketSet(client_set);              //releasing the memory of the client socket set
    client_set = NULL; //this helps us remember that this set is not allocated
  } 

  if(server_sock != NULL)
  {
    SDLNet_TCP_Close(server_sock);
    server_sock = NULL;
  }
  SDLNet_Quit();

 /* Clean up mathcards heap memory */
  MC_EndGame();
}





//update_clients() sees if anyone is trying to connect, and connects if a slot
//is open and the game is not in progress. It returns the number of connected clients.
//FIXME we need to be able to test to see if the clients are really still connected
int update_clients(void)
{
  int slot = 0;
  int x = 0;
  char buffer[NET_BUF_LEN];

  /* See if we have a pending connection: */
  temp_sock = SDLNet_TCP_Accept(server_sock);
  if (!temp_sock)  /* No one waiting to join - do nothing */
  {
    return num_clients;
  }

  // See if any slots are available:
  slot = find_vacant_client();
  if (slot == -1) /* No vacancies: */
  {
    snprintf(buffer, NET_BUF_LEN, 
             "%s\n",
             "Sorry, already have maximum number of clients connected\n");
    x = SDLNet_TCP_Send(temp_sock, buffer, NET_BUF_LEN);
    //hang up:
    SDLNet_TCP_Close(temp_sock);
    temp_sock = NULL;
#ifdef LAN_DEBUG
    printf("buffer sent:::: %d bytes\n", x);
    printf("buffer is: %s\n", buffer);
#endif
    return num_clients;
  }     

  // If game already started, send our regrets:
  if(game_in_progress)
  {
    snprintf(buffer, NET_BUF_LEN, 
             "%s\n",
             "Sorry the game has started...... =(\n");
    x = SDLNet_TCP_Send(temp_sock, buffer, NET_BUF_LEN);
    //hang up:
    SDLNet_TCP_Close(temp_sock);
    temp_sock = NULL;
#ifdef LAN_DEBUG
    printf("buffer sent:::: %d bytes\n", x);
    printf("buffer is: %s\n", buffer);
#endif
    return num_clients;
  }

  // If we get to here, we have room for the new connection and the
  // game is not in progress, so we connect:

#ifdef LAN_DEBUG
  printf("creating connection for client[%d].sock:\n", slot);
#endif

  client[slot].sock = temp_sock;

  /* We are receiving the client's name that was entered: */
  if( SDLNet_TCP_Recv(client[slot].sock, buffer, NET_BUF_LEN) > 0)
  {
    strncpy(client[slot].name, buffer, NAME_SIZE);
    num_clients = sockets_used;
    printf(" JOINED  :::   %s\n", client[slot].name);
    printf("slot %d  is %d\n",slot,client[slot].game_ready); 

    /* Add client socket to set: */
    sockets_used = SDLNet_TCP_AddSocket(client_set, client[slot].sock);
    if(sockets_used == -1) //No way this should happen
    {
      printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
      remove_client(slot);
      return num_clients;
    }
  }
  else 
  {
    printf("SDLNet_TCP_Recv() failed");
    remove_client(slot);
  }
  /* Now we can communicate with the client using client[i].sock socket
  /* serv_sock will remain opened waiting other connections */
    

#ifdef LAN_DEBUG
  /* Get the remote address */
  {
    IPaddress* client_ip = NULL;
    client_ip = SDLNet_TCP_GetPeerAddress(client[slot].sock);
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

  return num_clients;
}



// check_messages() is where we look at the client socket set to see which 
// have sent us messages. This function is used in each server loop whether
// or not a math game is in progress (although we expect different messages
// during a game from those encountered outside of a game)

int check_messages(void)
{
  int i = 0;
  int actives = 0;
  int ready_found = 0;
  char buffer[NET_BUF_LEN];

  /* Check the client socket set for activity: */
  actives = SDLNet_CheckSockets(client_set, 0);
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
    if(actives != ready_found)
    {
      printf("Warning: SDLNet_CheckSockets() reported %d active sockets,\n"
             "but only %d detected by SDLNet_SocketReady()\n", actives, ready_found);
      /* We can investigate further - maybe ping all the sockets, etc. */

    }
  }
}



void handle_client_nongame_msg(int i,char *buffer)
{
  if(strncmp(buffer, "START_GAME", 10) == 0)
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

  if(strncmp(command, "CORRECT_ANSWER", 14) == 0)
  {
    game_msg_correct_answer(i,id);
  }                            

  else if(strncmp(command, "exit",4) == 0) /* Terminate this connection */
  {
    game_msg_exit(i);
  }

  else if(strncmp(command, "quit",4) == 0) /* Quit the program */
  {
    game_msg_quit(i);
    return(1);
  }

  return(0);
}


void game_msg_correct_answer(int i,int id)
{
  int n; 
  printf("question id %d was answered correctly by %s",id,client[i].name);             
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
                  
  for(n = 0; n < num_clients && client[n].sock; n++)
  {
    if(!SendQuestion(flash,client[n].sock))
    {
      printf("Unable to send Question\n");
    }
  } 

}

void ping_client(int i)
{
  char buf[NET_BUF_LEN];
  char msg[NET_BUF_LEN];
  int x;

  sprintf(msg,"%s", "PING\n");

  snprintf(buf, NET_BUF_LEN, "%s\t%s\n", "SEND_MESSAGE", msg);
  x = SDLNet_TCP_Send(client[i].sock, buf, NET_BUF_LEN);

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
                "%s\n",
                "Success");  //FIXME what did we succeed at? This is basically a sort of handshaking signal , although it is not much needed here , but since we have a blocking recv call in the client for the case of game_in_progress , so no client join , therefore it is in accordance with that SDL_NetRecv()

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
  printf("We have %d players.......\n",sockets_used);
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
int SendMessage(int message, int z,TCPsocket client_sock)         
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
      sprintf(msg,"%s %d %s", "Question ID:",
              z, "was answered correctly by the client\n");
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


//Commented-out copy of old check_messages() to keep ping work from 
//being lost

// int check_messages(void)
// {
//   int i = 0;
//   int actives = 0;
//   int msg_found = 0;
//   char buffer[NET_BUF_LEN];
// 
//   /* Check the client socket set for activity: */
//   actives = SDLNet_CheckSockets(client_set, 0);
//   if(actives == -1)
//   {
//     printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
//     //most of the time this is a system error, where perror might help you.
//     perror("SDLNet_CheckSockets");
//   }
// 
//   else if(actives) 
//   {
// #ifdef LAN_DEBUG
//     printf("There are %d sockets with activity\n", actives);
// #endif
// 
//     // check all sockets with SDLNet_SocketReady and handle the active ones.
//     // NOTE we have to check all the slots in the set because
//     // the set will become discontinuous if someone disconnects
//     for(i = 0; i < MAX_CLIENTS; i++)
//     {
//       if((client[i].sock != NULL)
//         && (SDLNet_SocketReady(client[i].sock))) 
//       {
// #ifdef LAN_DEBUG
//         printf("client socket %d is ready\n", i);
// #endif
//         if (SDLNet_TCP_Recv(client[i].sock, buffer, NET_BUF_LEN) > 0)
//         {
// #ifdef LAN_DEBUG
//           printf("buffer received from client %d is: %s\n", i, buffer);
// #endif
//           msg_found++;
// 
//           /* Here we pass the client number and the message buffer */
//           /* to a suitable function for further action:                */
//            if(game_in_progress==1)
//            {
//             if(handle_client_game_msg(i, buffer))
//             return(1);
//            }
//            if(game_in_progress==0)
//            handle_client_nongame_msg(i,buffer);
// 
//         }
//       }
//     }  // end of for() loop - all client sockets checked
//     // Make sure all the active sockets reported by SDLNet_CheckSockets()
//     // are accounted for:
//     if(actives == 0)
//     {
//       printf("Warning: SDLNet_CheckSockets() reported %d active sockets,\n"
//              "but only %d messages received.\n", actives, msg_found);
//       /* We can investigate further - maybe ping all the sockets, etc. */
//       for(i = 0; i < MAX_CLIENTS; i++)
//       {
//         ping_client(i);
//       }
//     
//    
//       /* Check the client socket set for activity: */
//       actives = SDLNet_CheckSockets(client_set, 5000);
//       if(actives == 0)
//       {
//         printf("No clients , All clients have disconnected...=(\n");
//       }
// 
//       else if(actives == -1)
//       {
//         printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
//         //most of the time this is a system error, where perror might help you.
//         perror("SDLNet_CheckSockets");
//       }
// 
//       else if(actives) 
//       {
// #ifdef LAN_DEBUG
//        printf("There are %d sockets with activity\n", actives);
// #endif
// 
//        // check all sockets with SDLNet_SocketReady and handle the active ones.
//        // NOTE we have to check all the slots in the set because
//        // the set will become discontinuous if someone disconnects
//        for(i = 0; i < MAX_CLIENTS; i++)
//        {
//          if((client[i].sock != NULL)
//             && (SDLNet_SocketReady(client[i].sock))) 
//          {
// #ifdef LAN_DEBUG
//            printf("client socket %d is ready\n", i);
// #endif
//            if (SDLNet_TCP_Recv(client[i].sock, buffer, NET_BUF_LEN) > 0)
//            {
// #ifdef LAN_DEBUG
//              printf("buffer received from client %d is: %s\n", i, buffer);
// #endif
//              if(strncmp(buffer,"PING_BACK",9))
//              printf("%s is connected =) \n",client[i].name);       
//            }
//          }
//        }
//       }
//     }
//   }
// }
