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
* derivative works into a GPLv2+ project like TuxMath.  FWIW, virtually none of
* the tutorial code is still present here - David Bruce 
*/

/* This must come before #ifdef HAVE_LIBSDL_NET to get "config.h" */
#include "globals.h"

#ifdef HAVE_LIBSDL_NET

#include "server.h" 
#include "transtruct.h"
#include "mathcards.h"
#include "throttle.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/types.h>  
#include <unistd.h>

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif


#define MAX_ARGS 16


/*  -----------  Local function prototypes:   ------------  */

// setup and cleanup:
int setup_server(void);
void cleanup_server(void);
void server_handle_command_args(int argc, char* argv[]);
void* run_server_local_args(void);

// top level functions in main loop:
void check_UDP(void);
void update_clients(void);
int server_check_messages(void);

// client management utilities:
int find_vacant_client(void);
void remove_client(int i);
void check_game_clients(void);

// message reception:
int handle_client_game_msg(int i, char* buffer);
void handle_client_nongame_msg(int i, char* buffer);
int msg_set_name(int i, char* buf);
void start_game(void);
void game_msg_correct_answer(int i, char* inbuf);
void game_msg_wrong_answer(int i, char* inbuf);
void game_msg_quit(int i);
void game_msg_exit(int i);
int calc_score(int difficulty, float t);

//message sending:
int add_question(MC_FlashCard* fc);
int remove_question(int id);
int send_counter_updates(void);
int send_score_updates(void);
//int SendQuestion(MC_FlashCard flash, TCPsocket client_sock);
int SendMessage(int message, int ques_id, char* name, TCPsocket client_sock);
int player_msg(int i, char* msg);
void broadcast_msg(char* msg);
int transmit(int i, char* msg);
int transmit_all(char* msg);

// For non-blocking input:
int read_stdin_nonblock(char* buf, size_t max_length);


// not really deprecated but not done in response to 
// client message --needs better name:
void game_msg_next_question(void);



/*  ------------   "Local globals" for server.c: ----------  */
char server_name[NAME_SIZE];  /* User-visible name for server selection  */
int need_server_name = 1;
UDPsocket udpsock = NULL;     /* Used to listen for client's server autodetection           */
TCPsocket server_sock = NULL; /* Socket descriptor for server to accept client TCP sockets. */
IPaddress ip;
SDLNet_SocketSet client_set = NULL, temp_set = NULL;
static client_type client[MAX_CLIENTS];
static int num_clients = 0;
static int game_in_progress = 0;
static int server_running = 0;
static int quit = 0;
MC_FlashCard flash;
int local_argc;
char* local_argv[MAX_ARGS];








/* The previous contents of main() are wrapped into this function to   */
/* allow the server to be run as a function in a process or thread     */
/* within another program.  main() is now in a separate file,          */
/* servermain.c, that consists solely of a call to RunServer().        */

/* FIXME this isn't thread-safe - we need to return gracefully if we     */
/* find that the server is already running, instead of calling cleanup() */
/* and crashing the program. Some of the setup and cleanup will have to  */
/* be called from main() rather than from here.                          */
int RunServer(int argc, char* argv[])
{ 
  Uint32 timer = 0;

  printf("Started tuxmathserver, waiting for client to connect:\n>\n");

  server_handle_command_args(argc, argv);

  /*     ---------------- Setup: ---------------------------   */
  if (!setup_server())
  {
    fprintf(stderr, "setup_server() failed - exiting.\n");
    cleanup_server();
    return EXIT_FAILURE;
  }

  server_running = 1;

  printf("Waiting for clients to connect:\n>");
  fflush(stdout);

 /*    ------------- Main server loop:  ------------------   */
  while (!quit)
  {
    /* Respond to any clients pinging us to find the server: */
    check_UDP();
    /* Now we check to see if anyone is trying to connect. */
    update_clients();
    /* Check for any pending messages from clients already connected: */
    server_check_messages();

    /* Limit frame rate to keep from eating all CPU: */
    /* NOTE almost certainly could make this longer wtihout noticably */
    /* affecting performance, but even throttling to 1 msec/loop cuts */
    /* CPU from 100% to ~2% on my desktop - DSB                       */
    Throttle(5, &timer);  //min loop time 5 msec
  }

  server_running = 0;
   
  /*   -----  Free resources before exiting: -------    */
  cleanup_server();

  return EXIT_SUCCESS;
}

/* If we can't use pthreads, we use this function   */
/* to launch the server as a separate program using */
/* the C library system() call                      */
int RunServer_prog(int argc, char* argv[])
{
  char buf[256];
  int i;

  /* Construct command-line argument string from argc and argv:   */
  /* NOTE this is not safe from buffer overflow - do              */
  /* not use with user-supplied arguments.                        */
  snprintf(buf, 256, "tuxmathserver ");
  for(i = 1; i < argc; i++)
  {
    strncat(buf, argv[i], 256);
    strncat(buf, " ", 256);
  }
  /* Add '&' to make it non-blocking: */
  strncat(buf, "&", 256);

  return system(buf);
}


int RunServer_pthread(int argc, char* argv[])
{
  pthread_t server_thread;
  int i;

  /* We can only pass a single arg into the new thread, but it shares  */
  /* the same address space, so we save argc and argv locally instead: */
  local_argc = argc;
  for(i = 0; i < argc && i < MAX_ARGS; i++)
  {
    local_argv[i] = argv[i];
  }

  f(pthread_create(&server_thread, NULL, run_server_local_args, NULL))
  {
    printf("Error creating thread\n");
    return -1;
  }
  return 0;
}


void* run_server_local_args(void)
{

  RunServer(local_argc, local_argv);
  pthread_exit(NULL);
  return NULL;
}

/*********************************************************************/
/*  "Private" (to server.c) functions                                */
/*********************************************************************/


/*  ----- Setup and Cleanup:  ------------------- */


// setup_server() - all the things needed to get server running:
int setup_server(void)
{
  Uint32 timer = 0;

  //Initialize SDL and SDL_net:
  if(SDL_Init(0) == -1)
  {
    printf("SDL_Init: %s\n", SDL_GetError());
    return 0;;
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

  //this sets up our mathcards "library" with hard-coded defaults - no
  //settings read from config file here as of yet:
  if (!MC_Initialize())
  {
    fprintf(stderr, "Could not initialize MathCards\n");
    return 0;
  }

  /* Get server name: */
  /* We use default name after 30 sec timeout if no name entered. */
  /* FIXME we should save this to disc so it doesn't */
  /* have to be entered every time.                  */
  if(need_server_name)
  {
    Uint32 timeout = SDL_GetTicks() + SERVER_NAME_TIMEOUT;
    int name_recvd = 0;
    server_name[0] = '\0';
    fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) | O_NONBLOCK);

    printf("Enter the SERVER's NAME: \n>");
    fflush(stdout);

    while(!name_recvd && (SDL_GetTicks() < timeout))
    {
      if(read_stdin_nonblock(server_name, NAME_SIZE))
        name_recvd = 1;
      Throttle(10, &timer);
    }
    if(!name_recvd)
      printf("No name entered within timeout, will use default: %s\n",
             DEFAULT_SERVER_NAME);
  
    /* If no nickname received, use default: */
    if(strlen(server_name) == 0)
      strncpy(server_name, DEFAULT_SERVER_NAME, NAME_SIZE);
  }


  // Zero out our client list:
  {
    int i = 0;
    for(i = 0; i < MAX_CLIENTS; i++)
    {
      client[i].game_ready = 0;   /* waiting for user to OK game start */
      client[i].name[0] = '\0';   /* no nicknames yet                  */
      client[i].sock = NULL;      /* sockets start out unconnected     */
      client[i].score = 0;
    }
  }


  //Now open a UDP socket to listen for clients broadcasting to find the server:
  udpsock = SDLNet_UDP_Open(DEFAULT_PORT);
  if(!udpsock)
  {
    printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
    return 0;
  }

  // Indicates success:
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

  SDLNet_Quit();

  /* Clean up mathcards heap memory */
  MC_EndGame();
}


/* Handle any arguments passed from command line */
void server_handle_command_args(int argc, char* argv[])
{
  int i;

  for (i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
    {
      /* Display help message: */
      printf("\n");
      cleanup_server();
      exit(0);
    }
    else if (strcmp(argv[i], "--debug-lan") == 0)
    {
      debug_status |= debug_lan;
    }

    else if (strcmp(argv[i], "--copyright") == 0 ||
             strcmp(argv[i], "-c") == 0)
    {
      printf(
        "\n\"Tux, of Math Command Server\" version " VERSION ", Copyright (C) 2009,\n"
        "David Bruce, Akash Gangil, and the Tux4Kids Project.\n"
        "This program is free software; you can redistribute it and/or\n"
        "modify it under the terms of the GNU General Public License\n"
        "as published by the Free Software Foundation.  See COPYING.txt\n"
        "\n"
        "This program is distributed in the hope that it will be useful,\n"
        "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
        "\n");

      cleanup_server();
      exit(0);
    }
    else if (strcmp(argv[i], "--usage") == 0 ||
             strcmp(argv[i], "-u") == 0)
    {
      /* Display (happy) usage: */

//      usage(0, argv[0]);
    }
    else if ((strcmp(argv[i], "--name") == 0 || strcmp(argv[i], "-n") == 0)
           && (i + 1 < argc))
    {
      strncpy(server_name, argv[i + 1], NAME_SIZE);
      need_server_name = 0;
    }
  }
}


// ----------- Top level functions in main loop ---------------:

//check_UDP() is the server side of the client-server autodetection system.
//When a client wants to connect, it sends a UDP broadcast to the local
//network on this port, and the server sends a response.
void check_UDP(void)
{
  int recvd = 0;
  UDPpacket* in = SDLNet_AllocPacket(NET_BUF_LEN);
  recvd = SDLNet_UDP_Recv(udpsock, in);

  // See if packet contains identifying string:
  if(strncmp((char*)in->data, "TUXMATH_CLIENT", strlen("TUXMATH_CLIENT")) == 0)
  {
    UDPpacket* out;
    int sent = 0;
    char buf[NET_BUF_LEN];
    // Send "I am here" reply so client knows where to connect socket,
    // with configurable identifying string so user can distinguish 
    // between multiple servers on same network (e.g. "Mrs. Adams' Class");
    out = SDLNet_AllocPacket(NET_BUF_LEN); 
    snprintf(buf, NET_BUF_LEN, "%s\t%s", "TUXMATH_SERVER", server_name);
    snprintf(out->data, NET_BUF_LEN, "%s", buf);
    out->len = strlen(buf) + 1;
    out->address.host = in->address.host;
    out->address.port = in->address.port;

    sent = SDLNet_UDP_Send(udpsock, -1, out);

    SDLNet_FreePacket(out); 
  }
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
             "%s\t%s",
             "PLAYER_MSG",
             "Sorry, already have maximum number of clients connected");
    SDLNet_TCP_Send(temp_sock, buffer, NET_BUF_LEN);
    //hang up:
    SDLNet_TCP_Close(temp_sock);
    temp_sock = NULL;

    DEBUGMSG(debug_lan, "update_clients() - no vacant slot found\n");

    return;   // Leave num_clients unchanged
  }

  //If everyone is disconnected, game no longer in progress:
  check_game_clients(); 

  // If game already started, send our regrets:
  if(game_in_progress)
  {
    snprintf(buffer, NET_BUF_LEN, 
             "%s",
             "GAME_IN_PROGRESS");
    SDLNet_TCP_Send(temp_sock, buffer, NET_BUF_LEN);
    //hang up:
    SDLNet_TCP_Close(temp_sock);
    temp_sock = NULL;

    DEBUGMSG(debug_lan, "update_clients() - game already started\n");

    return;   // Leave num_clients unchanged
  }

  // If we get to here, we have room for the new connection and the
  // game is not in progress, so we connect:
  DEBUGMSG(debug_lan, "creating connection for client[%d].sock:\n", slot);

  client[slot].sock = temp_sock;

  /* Add client socket to set: */
  sockets_used = SDLNet_TCP_AddSocket(client_set, client[slot].sock);
  if(sockets_used == -1) //No way this should happen
  {
    printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
    cleanup_server();
    exit(EXIT_FAILURE);
  }

  /* At this point num_clients can be updated: */
  num_clients = sockets_used;

  /* Now we can communicate with the client using client[i].sock socket */
  /* serv_sock will remain opened waiting other connections */
    

  /* Get the remote address */
  DEBUGCODE(debug_lan)
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

  return;
}



// check_messages() is where we look at the client socket set to see which 
// have sent us messages. This function is used in each server loop whether
// or not a math game is in progress (although we expect different messages
// during a game from those encountered outside of a game)

int server_check_messages(void)
{
  int actives = 0, i = 0;
  int ready_found = 0;
  char buffer[NET_BUF_LEN];


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
    DEBUGMSG(debug_lan, "There are %d sockets with activity\n", actives);

    // check all sockets with SDLNet_SocketReady and handle the active ones.
    // NOTE we have to check all the slots in the set because
    // the set will become discontinuous if someone disconnects
    // NOTE this will only pick up the first message for each socket each time
    // check_messages() called - probably OK if we just get it next time through.
    for(i = 0; i < MAX_CLIENTS; i++)
    {
      if((client[i].sock != NULL)
        && (SDLNet_SocketReady(client[i].sock))) 
      { 
        ready_found++;

        DEBUGMSG(debug_lan, "client socket %d is ready\n", i);

        if (SDLNet_TCP_Recv(client[i].sock, buffer, NET_BUF_LEN) > 0)
        {
          DEBUGMSG(debug_lan, "buffer received from client %d is: %s\n", i, buffer);

          /* Here we pass the client number and the message buffer */
          /* to a suitable function for further action:                */
          if(game_in_progress)
          {
            handle_client_game_msg(i, buffer);
          }
          else
          {
            handle_client_nongame_msg(i, buffer);
          }
          // See if game is ended because everyone has left:
          check_game_clients(); 
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

    if(actives > ready_found)
    {
      printf("Warning: SDLNet_CheckSockets() reported %d active sockets,\n"
             "but only %d detected by SDLNet_SocketReady()\n", actives, ready_found);
      //Presently, this just runs ping_client() on all the sockets:
      //test_connections();
    }
  } 
  return 1;
}




// client management utilities:

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

  SDLNet_TCP_DelSocket(client_set, client[i].sock);

  if(client[i].sock != NULL)
    SDLNet_TCP_Close(client[i].sock);

  client[i].sock = NULL;  
  client[i].game_ready = 0;
  client[i].name[0] = '\0';
}


// check_game_clients() reviews the game_ready flags of all the connected
// clients to determine if a new game is started, or if an old game needs
// to be ended because all the players have left.  If it finds both "playing"
// and "nonplaying clients", it leaves game_in_progress unchanged.

// TODO this is not very sophisticated, and only supports one game at a time.
// We may want to make this extensible to multiple simultaneous games, perhaps
// with each game in its own thread with its own socket set and mathcards instance.
// FIXME we need to do more than just toggle game_in_progress - should have
// start_game() and end_game() functions that make sure mathcards is 
// properly set up or cleaned up.
void check_game_clients(void)
{
  int i = 0;

  //If the game is already started, we leave it running as long as at least
  //one client is both connected and willing to play:
  if(game_in_progress)
  {
    int someone_still_playing = 0;
    for(i = 0; i < MAX_CLIENTS; i++)
    {
      if((client[i].sock != NULL)
       && client[i].game_ready)
      {
        someone_still_playing = 1;
        break;
      }
    }

    if(!someone_still_playing)
    {
      printf("All the clients have left the game, setting game_in_progress = 0.\n");
      game_in_progress = 0;
    }
  }
  //If the game hasn't started yet, we only start it 
  //if all connected clients are ready:
  //FIXME should add a timeout so the game eventually starts without
  //those who don't answer
  else
  {
    int someone_connected = 0;
    int someone_not_ready = 0;
    for(i = 0; i < MAX_CLIENTS; i++)
    {
      if(client[i].sock != NULL)
      { 
        someone_connected = 1;
        if (!client[i].game_ready)
        {
          someone_not_ready = 1;
        }
      }
    }
    if(someone_connected && !someone_not_ready)
      start_game(); 
  }
}



void handle_client_nongame_msg(int i, char* buffer)
{
  char buf[NET_BUF_LEN];

  if(strncmp(buffer, "START_GAME", strlen("START_GAME")) == 0)
  {
    snprintf(buf, NET_BUF_LEN,
                "Player %s ready to start math game",
                client[i].name);
    broadcast_msg(buf);
    client[i].game_ready = 1;
    //This will call start_game() if all the other clients are ready:
    check_game_clients();
  }
  else if(strncmp(buffer, "SET_NAME", strlen("SET_NAME")) == 0)
  {
    msg_set_name(i, buffer);
  }
}


int handle_client_game_msg(int i , char* buffer)
{
  DEBUGMSG(debug_lan, "Buffer received from client: %s\n", buffer);

  if(strncmp(buffer, "CORRECT_ANSWER", strlen("CORRECT_ANSWER")) == 0)
  {
    game_msg_correct_answer(i, buffer);
  }                            

  else if(strncmp(buffer, "WRONG_ANSWER",strlen("WRONG_ANSWER")) == 0) /* Player answered the question incorrectly , meaning comet crashed into a city or an igloo */
  {
    game_msg_wrong_answer(i, buffer);
  }

  else if(strncmp(buffer, "LEAVE_GAME", strlen("LEAVE_GAME")) == 0) 
  {
    client[i].game_ready = 0;  /* Player quitting game but not disconnecting */
  }

  else if(strncmp(buffer, "exit",strlen("exit")) == 0) /* Terminate this connection */
  {
    game_msg_exit(i);
  }

  else if(strncmp(buffer, "quit",strlen("quit")) == 0) /* Quit the program */
  {
    game_msg_quit(i);
    return(1);
  }
  else
  {
    printf("command %s not recognized\n", buffer);
  }
  return(0);
}



int msg_set_name(int i, char* buf)
{
  char* p;

  if(buf == NULL)
    return 0;

  p = strchr(buf, '\t');
  if(p)
  { 
    p++;
    strncpy(client[i].name, p, NAME_SIZE);
    return 1;
  }
  else
    return 0;
}



void game_msg_correct_answer(int i, char* inbuf)
{
  char outbuf[NET_BUF_LEN];
  char* p = NULL;
  int id = -1;
  float t = -1;
  int points = 0;

  if(!inbuf)
    return;

  //parse inbuf to get question id:
  p = strchr(inbuf, '\t');
  if(!p)
    return; 
  p++;
  id = atoi(p);
  //Now get time player took to answer:
  p = strchr(p, '\t');
  if(!p)
    t = -1;
  else
  {
    p++;
    t = atof(p);
  }

  //Tell mathcards so lists get updated:
  points = MC_AnsweredCorrectly(id, t);
  if(!points)
    return;
  //If we get to here, the id was successfully parsed out of inbuf
  //and the corresponding question was found.
  client[i].score += points;

  //Announcement for server and all clients:
  snprintf(outbuf, NET_BUF_LEN, 
          "question id %d was answered in %f seconds for %d points by %s",
          id, t, points, client[i].name);             
  broadcast_msg(outbuf);

  DEBUGMSG(debug_lan, "game_msg_correct_answer(): %s\n", outbuf);

  //Tell all players to remove that question:
  remove_question(id);
  //send the next question to everyone:
  game_msg_next_question();
  //and update the game counters:
  send_counter_updates();
  //and the scores:
  send_score_updates();
}


void game_msg_wrong_answer(int i, char* inbuf)
{
  char outbuf[NET_BUF_LEN];
  char* p;
  int id;

  if(!inbuf)
    return;

  //parse inbuf to get question id:
  p = strchr(inbuf, '\t');
  if(!p)
    return; 
  p++;
  id = atoi(p);

  //Tell mathcards so lists get updated:
  if(!MC_NotAnsweredCorrectly(id))
    return;
  //If we get to here, the id was successfully parsed out of inbuf
  //and the corresponding question was found.

  //Announcement for server and all clients:
  snprintf(outbuf, NET_BUF_LEN, 
          "question id %d was missed by %s\n",
          id, client[i].name);             
  broadcast_msg(outbuf);
  //Tell all players to remove that question:
  remove_question(id);
  //send the next question to everyone:
  game_msg_next_question();
  //and update the game counters:
  send_counter_updates();
}



void game_msg_next_question(void)
{
  MC_FlashCard flash;

  /* Get next question from MathCards: */
  if (!MC_NextQuestion(&flash))
  { 
    /* no more questions available */
    printf("MC_NextQuestion() returned NULL - no questions available\n");
    return;
  }

  DEBUGMSG(debug_lan, "In game_msg_next_question(), about to send:\n");
  DEBUGCODE(debug_lan) print_card(flash); 

  /* Send it to all the clients: */ 
  add_question(&flash);
}





void game_msg_exit(int i)
{
  printf("LEFT the GAME : %s",client[i].name);
  remove_client(i);
}



//FIXME don't think we want to allow players to shut down the server
void game_msg_quit(int i)
{
  printf("Server has been shut down by %s\n", client[i].name); 
  cleanup_server();
  exit(9);                           // '9' means exit ;)  (just taken an arbitary no:)
}


/* Now this gets called to actually start the game once all the players */
/* have indicated that they are ready:                                  */
void start_game(void)
{
  char buf[NET_BUF_LEN];
  int j;


  /* NOTE this should no longer be needed - doing the same thing earlier    */
  /*This loop sees that the game starts only when all the players are ready */
  /* i.e. if someone is connected but not ready, we return.                 */
  for(j = 0; j < MAX_CLIENTS; j++)
  {
    // Only check sockets that aren't null:
    if((client[j].game_ready != 1)
    && (client[j].sock != NULL))
    {
      printf("Warning - start_game() entered when someone not ready\n");
      return;      
    }
  }


 /***********************Will be modified**************/
  //Tell everyone we are starting and count who's really in:
  num_clients = 0;
  snprintf(buf, NET_BUF_LEN, 
          "%s\n",
          "GO_TO_GAME");          
  for(j = 0; j < MAX_CLIENTS; j++)
  {
    if((client[j].game_ready == 1)
    && (client[j].sock != NULL))
    {
      if(SDLNet_TCP_Send(client[j].sock, buf, NET_BUF_LEN) == NET_BUF_LEN)
        num_clients++;
      else
      {
        printf("in start_game() - failed to send to client %d, removing\n", j);
        remove_client(j);
      }
    }
  }
 /*****************************************************/


  /* If no players join the game (should not happen) */
  if(num_clients == 0)
  {
    printf("There were no players........=(\n");
    return;
  }

  DEBUGMSG(debug_lan, "We have %d players.......\n", num_clients);

  game_in_progress = 1;  //setting the game_in_progress flag to '1'
  //Start a new math game as far as mathcards is concerned:
  if (!MC_StartGame())
  {
    fprintf(stderr, "\nMC_StartGame() failed!");
    return;
  }

  game_in_progress = 1;

  // Zero out scores:
  for(j = 0; j < MAX_CLIENTS; j++)
    client[j].score = 0;

  /* Send enough questions to fill the initial comet slots (currently 10) */
  for(j = 0; j < QUEST_QUEUE_SIZE; j++)
  {
    if (!MC_NextQuestion(&flash))
    { 
      /* no more questions available */
      printf("MC_NextQuestion() returned NULL - no questions available\n");
      return;
    }

    DEBUGMSG(debug_lan, "In start_game(), about to send:\n");
    DEBUGCODE(debug_lan) print_card(flash); 

    //Send to all clients with add_question();
    add_question(&flash);
  }
  //Send all the clients the counter totals:
  send_counter_updates();
  send_score_updates();
}



//More centralized function to update the clients of the number of 
//questions remaining, whether the mission has been accomplished,
//and so forth:
int send_counter_updates(void)
{
  int total_questions;

  //If game won, tell everyone:
  if(MC_MissionAccomplished())
  {
    char buf[NET_BUF_LEN];
    snprintf(buf, NET_BUF_LEN, "%s", "MISSION_ACCOMPLISHED");
    transmit_all(buf);
  }

  //Tell everyone how many questions left:
  total_questions = MC_TotalQuestionsLeft();
  {
    char buf[NET_BUF_LEN];
    snprintf(buf, NET_BUF_LEN, "%s\t%d", "TOTAL_QUESTIONS", total_questions);
    transmit_all(buf);
  }
  return 1;
}


int send_score_updates(void)
{
  int i = 0;

  /* Count how many players are active and send number to clients: */
  {
    int connected_players = 0;
    char buf[NET_BUF_LEN];
    for(i = 0; i < MAX_CLIENTS; i++)
      if((client[i].game_ready == 1) && (client[i].sock != NULL))
        connected_players++;

    snprintf(buf, NET_BUF_LEN, "%s\t%d", "CONNECTED_PLAYERS",
             connected_players);
    transmit_all(buf);
  }

  /* Now send out all the names and scores: */
  for(i = 0; i < MAX_CLIENTS; i++)
  {
    if((client[i].game_ready == 1)
    && (client[i].sock != NULL))
    {
      char buf[NET_BUF_LEN];
      snprintf(buf, NET_BUF_LEN, "%s\t%d\t%s\t%d", "UPDATE_SCORE",
               i,
               client[i].name,
               client[i].score);
      transmit_all(buf);
    }
  }

  return 1;
}


/* Sends a new question to all clients: */
int add_question(MC_FlashCard* fc)
{
  char buf[NET_BUF_LEN];

  if(!fc)
    return 0;

  snprintf(buf, NET_BUF_LEN,"%s\t%d\t%d\t%d\t%s\t%s\n",
                "ADD_QUESTION",
                fc->question_id,
                fc->difficulty,
                fc->answer,
                fc->answer_string,
                fc->formula_string);
  transmit_all(buf);
  return 1;
}

/* Tells all clients to remove a specific question: */
int remove_question(int id)
{
  char buf[NET_BUF_LEN];
  snprintf(buf, NET_BUF_LEN, "%s\t%d", "REMOVE_QUESTION", id);
  transmit_all(buf);
  return 1;
}


/* Sends a string for the client to display to player: */
int player_msg(int i, char* msg)
{
  char buf[NET_BUF_LEN];
  if(!msg)
  {
    DEBUGMSG(debug_lan, "player_msg() - msg argument is NULL\n");
    return 0;
  }

  /* Add header: */
  snprintf(buf, NET_BUF_LEN, "%s\t%s", "PLAYER_MSG", msg);
  //NOTE transmit() validates index and socket
  return transmit(i, buf);
}

/* Send a player message to all clients: */
void broadcast_msg(char* msg)
{
  int i = 0;
  if (!msg)
    return;
  for(i = 0; i < MAX_CLIENTS; i++)
    player_msg(i, msg);
}

/* Send string to client. String should already have its header */ 
int transmit(int i, char* msg)
{
  char buf[NET_BUF_LEN];

  //Validate arguments;
  if(i < 0 || i > MAX_CLIENTS)
  {
    DEBUGMSG(debug_lan,"transmit() - invalid index argument\n");
    return 0;
  }

  if(!msg)
  {
    DEBUGMSG(debug_lan, "transmit() - msg argument is NULL\n");
    return 0;
  }
  
  if(!client[i].sock)
  {
    return 0;
  }
  
  //NOTE SDLNet's Send() keeps sending until the requested length is
  //sent, so it really is an error if we send less thatn NET_BUF_LEN
  snprintf(buf, NET_BUF_LEN, "%s", msg);
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
int transmit_all(char* msg)
{
  int i = 0;
  if (!msg)
    return 0;

  for(i = 0; i < MAX_CLIENTS; i++)
    transmit(i, msg);

  return 1;
}



//Here we read up to max_length bytes from stdin into the buffer.
//The first '\n' in the buffer, if present, is replaced with a
//null terminator.
//returns 0 if no data ready, 1 if at least one byte read.
//NOTE for this to work we must first set stdin to O_NONBLOCK with:
//  fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) | O_NONBLOCK);

int read_stdin_nonblock(char* buf, size_t max_length)
{
  int bytes_read = 0;
  char* term = NULL;
  buf[0] = '\0';

  bytes_read = fread (buf, 1, max_length, stdin);
  term = strchr(buf, '\n');
  if (term)
    *term = '\0';
     
  if(bytes_read > 0)
    bytes_read = 1;
  else
    bytes_read = 0;
      
  return bytes_read;
}



int ServerRunning(void)
{
  return server_running;
}

#endif
