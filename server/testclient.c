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
 
#include "SDL_net.h"
#include "transtruct.h"
#include "mathcards.h"
#include "testclient.h"


TCPsocket sd;           /* Server socket descriptor */
SDLNet_SocketSet set;

MC_FlashCard flash;    //current question
int Make_Flashcard(char *buf, MC_FlashCard* fc);
int LAN_AnsweredCorrectly(MC_FlashCard* fc);
int playgame(void);


int main(int argc, char **argv)
{
  IPaddress ip;           /* Server address */
  int quit, len, sockets_used;
  char buf[NET_BUF_LEN];     // for network messages from server
  char buffer[NET_BUF_LEN];  // for command-line input

      /* first just take in the name */
      char *check1;
      char name[NAME_SIZE];
      printf("Enter your Name.\n");
      check1=fgets(buffer,NET_BUF_LEN,stdin);
      strncpy(name,check1,strlen(check1));
      snprintf(buffer, NET_BUF_LEN, 
                       "%s",
                       name);
      len = strlen(buffer) + 1;

  /* Simple parameter checking */
  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s host port\n", argv[0]);
    exit(EXIT_FAILURE);
  }
 
  if (SDLNet_Init() < 0)
  {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
  /* Resolve the host we are connecting to */
  if (SDLNet_ResolveHost(&ip, argv[1], atoi(argv[2])) < 0)
  {
    fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
  /* Open a connection with the IP provided (listen on the host's port) */
  if (!(sd = SDLNet_TCP_Open(&ip)))
  {
    fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }

  // Create a socket set to handle up to 16 sockets
  // NOTE 16 taken from example - almost certainly don't need that many
  set = SDLNet_AllocSocketSet(1);
  if(!set) {
    printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }

  sockets_used = SDLNet_TCP_AddSocket(set, sd);
  if(sockets_used == -1) {
    printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
    // perhaps you need to restart the set and make it bigger...
  }

  if (SDLNet_TCP_Send(sd, (void *)buffer, NET_BUF_LEN) < NET_BUF_LEN)
  {
   fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
   exit(EXIT_FAILURE);
  }
#ifdef LAN_DEBUG
  printf("Sent the name of the player %s\n",check1);
#endif


  /* Send messages */
  quit = 0;
  do
  { 
    //Get user input from command line and send it to server: 
  /*now display the options*/
    printf("Welcome to the Tux Math Test Client!\n");
    printf("Type:\n"
             "'game' to start math game;\n"
             "'exit' to end client leaving server running;\n"
             "'quit' to end both client and server\n>\n"); 
    char *check;
    check=fgets(buffer,NET_BUF_LEN,stdin);
   //Figure out if we are trying to quit:
    if(  (strncmp(buffer, "exit",4) == 0)
      || (strncmp(buffer, "quit",4) == 0))
    {
      quit = 1;
      len = strlen(buffer) + 1;
      if (SDLNet_TCP_Send(sd, (void *)buffer, len) < len)
      {
        fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
      }
    }
    else if (strncmp(buffer, "game",4) == 0)
    {
      printf("Starting math game:\n");
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
  }while(!quit);
 
  SDLNet_TCP_Close(sd);
  SDLNet_FreeSocketSet(set);
  set=NULL; //this helps us remember that this set is not allocated

  SDLNet_Quit();
 
  return EXIT_SUCCESS;
}



int LAN_AnsweredCorrectly(MC_FlashCard* fc)
{
  int len;
  char buffer[NET_BUF_LEN];

  snprintf(buffer, NET_BUF_LEN, 
                  "%s %d\n",
                  "CORRECT_ANSWER",
                  fc->question_id);
  len = strlen(buffer) + 1;
  if (SDLNet_TCP_Send(sd, (void *)buffer, NET_BUF_LEN) < NET_BUF_LEN)
  {
    fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
  return 1;
}
                
 


int Make_Flashcard(char* buf, MC_FlashCard* fc)
{
  int i = 0, j, tab = 0, s = 0;
  char formula[MC_FORMULA_LEN];
  sscanf (buf,"%*s%d%d%d%s",
              &fc->question_id,
              &fc->difficulty,
              &fc->answer,
              fc->answer_string); /* can't formula_string in sscanf in here cause it includes spaces*/
 
  /*doing all this cause sscanf will break on encountering space in formula_string*/
  /* NOTE changed to index notation so we keep within NET_BUF_LEN */
  while(buf[i]!='\n' && i < NET_BUF_LEN)
  {
    if(buf[i]=='\t')
      tab++; 
    i++;
    if(tab == 5)
      break;
  }

  while((buf[i] != '\n') 
    && (s < MC_FORMULA_LEN - 1)) //Must leave room for terminating null
  {
    formula[s] = buf[i] ;
    i++;
    s++;
  }
  formula[s]='\0';
  strcpy(fc->formula_string, formula); 

#ifdef LAN_DEBUG
  printf ("card is:\n");
  printf("QUESTION_ID       :      %d\n",fc->question_id);
  printf("FORMULA_STRING    :      %s\n",fc->formula_string);
  printf("ANSWER STRING     :      %s\n",fc->answer_string);
  printf("ANSWER            :      %d\n",fc->answer);
  printf("DIFFICULTY        :      %d\n",fc->difficulty);  
#endif

return 1;
} 

int playgame(void)
{
  int numready;
  int command_type;
  int ans = 0;
  int x=0, i = 0;
  int end = 0;
  int have_question = 0;
  int len = 0;
  char buf[NET_BUF_LEN];
  char buffer[NET_BUF_LEN];
  char ch;

#ifdef LAN_DEBUG
  printf("Entering playgame()\n");
#endif

 
   snprintf(buffer, NET_BUF_LEN, 
                  "%s\n",
                  "start");
   len = strlen(buffer) + 1;
   if (SDLNet_TCP_Send(sd, (void *)buffer, NET_BUF_LEN) < NET_BUF_LEN)
   {
     fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
     exit(EXIT_FAILURE);
   }
 #ifdef LAN_DEBUG
   printf("Sent the game notification %s\n",buffer);
 #endif
 

  if( SDLNet_TCP_Recv(sd, buf, sizeof(buf)))
  {
   if(strncmp(buf,"Sorry",5) == 0)
   {
     printf("Sorry , the game has already been started.......=(\n");
     SDLNet_TCP_Close(sd);
     SDLNet_FreeSocketSet(set);
     set=NULL; //this helps us remember that this set is not allocated
     SDLNet_Quit();
     exit(5);
   }
   else if(strncmp(buf,"Success",7)==0)
   {
    printf("You Are In game.....Waiting for other players to be ready...\n");
   }
  }
#ifdef LAN_DEBUG
  printf("Received %s\n",buf);
#endif



  //Begin game loop:
  while (!end)
  {
    //First we check for any responses from server:
    //NOTE keep looping until SDLNet_CheckSockets() detects no activity.
    numready = 1;
    while(numready > 0)
    {
     
      char command[NET_BUF_LEN];
      int i = 0;

      //This is supposed to check to see if there is activity:
      numready = SDLNet_CheckSockets(set, 0);
      if(numready == -1)
      {
        printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
        //most of the time this is a system error, where perror might help you.
        perror("SDLNet_CheckSockets");
      }
      else
      {
#ifdef LAN_DEBUG
//        printf("There are %d sockets with activity!\n", numready);
#endif
        // check all sockets with SDLNet_SocketReady and handle the active ones.
        if(SDLNet_SocketReady(sd))
        {
          buf[0] = '\0';
          x = SDLNet_TCP_Recv(sd, buf, sizeof(buf));
          /* Copy the command name out of the tab-delimited buffer: */
          for (i = 0;
               buf[i] != '\0' && buf[i] != '\t' && i < NET_BUF_LEN;
               i++)
          {
            command[i] = buf[i];
          }

          command[i] = '\0';

#ifdef LAN_DEBUG
          printf("buf is %s\n", buf);
          printf("command is %s\n", command);
#endif
          /* Now we process the buffer according to the command: */
          if(strncmp(command, "SEND_QUESTION",13) == 0)
          {
            if(Make_Flashcard(buf, &flash))  /* function call to parse buffer into MC_FlashCard */
              have_question = 1; 
            else
              printf("Unable to parse buffer into FlashCard\n");
          }
        }
      }
    } // End of loop for checking server activity

#ifdef LAN_DEBUG
    printf(".\n");
#endif

    //Now we check for any user responses
    while(have_question && !end)
    { 
      char *check1;
      printf("Question is: %s\n", flash.formula_string);
      printf("Enter answer:\n>");
      check1=fgets(buf,NET_BUF_LEN,stdin);
#ifdef LAN_DEBUG
      printf("check is %s\n",check1);
      printf("buf is %s\n", buf);
#endif
      if ((strncmp(buf, "quit", 4) == 0)
        ||(strncmp(buf, "exit", 4) == 0)
	||(strncmp(buf, "q", 1) == 0))
      {
        end = 1;
      }
      else
      {
        /*NOTE atoi() will return zero for any string that is not
	a valid int, not just '0' - should not be a big deal for
	our test program - DSB */
        ans = atoi(buf);
        if(ans == flash.answer)
        {  
          have_question = 0;
          //Tell server we answered it right:
          if(!LAN_AnsweredCorrectly(&flash))
          {
            printf("Unable to communicate the same to server\n");
            exit(EXIT_FAILURE);
          }


#ifdef LAN_DEBUG
          printf("requesting next question, buf: %s", buf);
#endif
        }
        else  //incorrect answer:
          printf("Sorry, incorrect. Try again!\n");
      }  //isint() returned false:
    }
  } //End of game loop 
#ifdef LAN_DEBUG
  printf("Leaving playgame()\n");
#endif
}


