/*
*  C Implementation: server.c
*
*       Description: Test client program for LAN-based play in Tux,of Math Command.
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
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 

#include "SDL_net.h"
#include "transtruct.h"
#include "mathcards.h"
#include "testclient.h"
#include "../src/throttle.h"
#include "../src/network.h"

/* Local (to testclient.c) "globals": */
TCPsocket sd;           /* Server socket descriptor */
SDLNet_SocketSet set;
IPaddress ip;           /* Server address */
int len = 0;
int sockets_used = 0;
int quit = 0;
MC_FlashCard flash;    //current question
int have_question = 0;

/* Local function prototypes: */
int setup_client(int argc, char **argv);
int playgame(void);
int game_check_msgs(void);
int read_stdin_nonblock(char* buf, size_t max_length);



int main(int argc, char **argv)
{
  char buf[NET_BUF_LEN];     // for network messages from server
  char buffer[NET_BUF_LEN];  // for command-line input

  /* Connect to server, create socket set, get player nickname, etc: */
  if(!LAN_Setup(argv[1], DEFAULT_PORT))
  {
    printf("setup_client() failed - exiting.\n");
    exit(EXIT_FAILURE);
  }

  /* Now we are connected - get nickname from player: */
  {
    char name[NAME_SIZE];
    char* p;

    printf("Please enter your name:\n>\n");
    fgets(buffer, NAME_SIZE, stdin);
    p = strchr(buf, '\t');
    if(p)
      *p = '\0';
    strncpy(name, buffer, NAME_SIZE);
    /* If no nickname received, use default: */
    if(strlen(name) == 1)
      strcpy(name, "Anonymous Coward");
  
//  printf("name is %s, length %d\n", name, strlen(name));
//  printf("buffer is %s, length %d\n", buffer, strlen(buffer));

    snprintf(buffer, NET_BUF_LEN, "%s", name);
    LAN_SetName(name);
  }

  printf("Welcome to the Tux Math Test Client!\n");

  /* Send messages */
  quit = 0;
  while(!quit)
  { 
    //Get user input from command line and send it to server: 
    /*now display the options*/
    printf("Type:\n"
             "'game' to start math game;\n"
             "'exit' to end client leaving server running;\n"
             "'quit' to end both client and server\n>\n"); 
    char* check;
    check = fgets(buffer, NET_BUF_LEN, stdin);

    //Figure out if we are trying to quit:
    if(  (strncmp(buffer, "exit", 4) == 0)
      || (strncmp(buffer, "quit", 4) == 0))
    {
      quit = 1;
      len = strlen(buffer) + 1;
      if (SDLNet_TCP_Send(sd, (void *)buffer, len) < len)
      {
        fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
      }
    }
    else if (strncmp(buffer, "game", 4) == 0)
    {
      playgame();
      printf("Math game finished.\n");
    }
    else
    {
      printf("Command not recognized. Type:\n"
             "'game' to start math game;\n"
             "'exit' to end client leaving server running;\n"
             "'quit' to end both client and server\n\n>\n");
    }
    //Limit loop to once per 10 msec so we don't eat all CPU
    Throttle(10);
  }
 
  LAN_Cleanup();
 
  return EXIT_SUCCESS;
}


/* Establish networking and identify player to server: */
int setup_client(int argc, char **argv)
{
  char* check1 = NULL;
  char name[NAME_SIZE];
  char buffer[NET_BUF_LEN];  // for command-line input


  /* Simple parameter checking */
  if (argc < 2)
  {
    fprintf(stderr, "Usage: %s host\n", argv[0]);
    return 0;
  }

  if (SDLNet_Init() < 0)
  {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    return 0;
  }
 
  /* Resolve the host we are connecting to */
  if (SDLNet_ResolveHost(&ip, argv[1], DEFAULT_PORT) < 0)
  {
    fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return 0;
  }
 
  /* Open a connection with the IP provided (listen on the host's port) */
  if (!(sd = SDLNet_TCP_Open(&ip)))
  {
    fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    return 0;
  }

  /* We create a socket set so we can check for activity: */
  set = SDLNet_AllocSocketSet(1);
  if(!set)
  {
    printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
    return 0;
  }

  sockets_used = SDLNet_TCP_AddSocket(set, sd);
  if(sockets_used == -1)
  {
    printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
    // perhaps you need to restart the set and make it bigger...
    return 0;
  }
  /* Now we are connected. Take in nickname and send to server. */


  if (SDLNet_TCP_Send(sd, (void*)buffer, NET_BUF_LEN) < NET_BUF_LEN)
  {
    fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    return 0;
  }

#ifdef LAN_DEBUG
  printf("Sent the name of the player %s\n",check1);
#endif

  return 1;
}



int game_check_msgs(void)
{
  char buf[NET_BUF_LEN];
  int status = 1;
  while(1)
  {
    buf[0] = '\0';
    status = LAN_NextMsg(buf);
    if (status == -1)  //Fatal error
    {
      printf("Error - get_next_msg() returned -1\n");
      return -1;
    }

    if (status == 0)  //Fatal error
    {
      //No messages
      return 0;
    }

    /* Now we process the buffer according to the command: */
    if(strncmp(buf, "SEND_QUESTION", strlen("SEND_QUESTION")) == 0)
    {
      /* function call to parse buffer and receive question */
      if(Make_Flashcard(buf, &flash))
      {
        have_question = 1; 
        printf("The question is: %s\n>\n", flash.formula_string);
      }
      else
        printf("Unable to parse buffer into FlashCard\n");
    }
    else if(strncmp(buf, "SEND_MESSAGE", strlen("SEND_MESSAGE")) == 0)
    {
      printf("%s\n", buf);
    }
    else if(strncmp(buf, "PLAYER_MSG", strlen("PLAYER_MSG")) == 0)
    {
      player_msg_recvd(buf);
    }
    else 
    {
      printf("game_check_msgs() - unrecognized message: %s\n", buf);
    }
  }

  return 1;
}

int playgame(void)
{
  int numready;
  int command_type;
  int ans = 0;
  int x=0, i = 0;
  int end = 0;
  int len = 0;
  char buf[NET_BUF_LEN];
  char buffer[NET_BUF_LEN];
  char ch;

  /* Set stdin to be non-blocking: */
  /* FIXME we might need to turn this back to blocking when we leave playgame() */
  fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) | O_NONBLOCK);

  printf("\nStarting Tux, of the Math Command Line ;-)\n");
  printf("Waiting for other players to be ready...\n\n");


 
  snprintf(buffer, NET_BUF_LEN, 
                  "%s\n",
                  "START_GAME");
  len = strlen(buffer) + 1;
  if (SDLNet_TCP_Send(sd, (void *)buffer, NET_BUF_LEN) < NET_BUF_LEN)
  {
    fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
#ifdef LAN_DEBUG
  printf("Sent the game notification %s\n",buffer);
 #endif
 

  //Begin game loop:
  while (!end)
  {

    //Check our network messages, bailing out for fatal errors:
    if (game_check_msgs() == -1)
      return -1;
    
    //Now we check for any user responses

    //This function returns 1 and updates buf with input from
    //stdin if input is present.
    //If no input, it returns 0 without blocking or waiting
    if(read_stdin_nonblock(buf, NET_BUF_LEN))
    {
      if ((strncmp(buf, "quit", 4) == 0)
        ||(strncmp(buf, "exit", 4) == 0)
        ||(strncmp(buf, "q", 1) == 0))
      {
        quit = 1;  //So we exit loop in main()
        end = 1;   //Exit our loop in playgame()
      }
      else if(strncmp(buf,"PLAYER_MSG", strlen("PLAYER_MSG")) == 0)
      {
        player_msg_recvd(buf);
      } 
      else
      {
        /*NOTE atoi() will return zero for any string that is not
        a valid int, not just '0' - should not be a big deal for
        our test program - DSB */
        ans = atoi(buf);
        if(have_question && (ans == flash.answer))
        {  
          have_question = 0;
          printf("%s is correct!\nRequesting next question...\n>\n", buf);

          //Tell server we answered it right:
          if(!LAN_AnsweredCorrectly(&flash))
          {
            printf("Unable to communicate the same to server\n");
            exit(EXIT_FAILURE);
          }
        }
        else  //we got input, but not the correct answer:
          printf("Sorry, %s is incorrect. Try again!\n>\n", buf);
      }  //input wasn't any of our keywords
    } // Input was received 

    Throttle(10);  //so don't eat all CPU
  } //End of game loop 
#ifdef LAN_DEBUG
  printf("Leaving playgame()\n");
#endif
}




//Here we read up to max_length bytes from stdin into the buffer.
//The first '\n' in the buffer, if present, is replaced with a
//null terminator.
//returns 0 if no data ready, 1 if at least one byte read.

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

