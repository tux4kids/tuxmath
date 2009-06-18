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



  /* Send messages */
  quit = 0;
  while (!quit)
  { 
    //Get user input from command line and send it to server: 
    printf("Welcome to the Tux Math Test Client!\n");
    printf("Type:\n"
             "'game' to start math game;\n"
             "'exit' to end client leaving server running;\n"
             "'quit' to end both client and server\n>\n"); 
    scanf("%s", buffer);

    //Figure out if we are trying to quit:
    if(  (strcmp(buffer, "exit") == 0)
      || (strcmp(buffer, "quit") == 0))
    {
      quit = 1;
      len = strlen(buffer) + 1;
      if (SDLNet_TCP_Send(sd, (void *)buffer, len) < len)
      {
        fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
      }
    }
    else if (strcmp(buffer, "game") == 0)
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
  }
 
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
  int x, i = 0;
  int end = 0;
  int have_question = 0;
  int len = 0;
  char buf[NET_BUF_LEN];

#ifdef LAN_DEBUG
  printf("Entering playgame()\n");
#endif

  //Tell server to start new game:
  snprintf(buf, NET_BUF_LEN, "%s\n", "a");
  if (SDLNet_TCP_Send(sd, (void *)buf, NET_BUF_LEN) < NET_BUF_LEN)
  {
    fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }

  //Ask for first question:
  snprintf(buf, NET_BUF_LEN, "%s\n", "b");
  if (SDLNet_TCP_Send(sd, (void *)buf, NET_BUF_LEN) < NET_BUF_LEN)
  {
    fprintf(stderr, "failed on b: SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }

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
        printf("There are %d sockets with activity!\n", numready);
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
          if(strcmp(command, "SEND_QUESTION") == 0)
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
    printf("No active sockets within timeout interval\n");
#endif

    //Now we check for any user responses
    while(have_question && !end)
    { 
      printf("Question is: %s\n", flash.formula_string);
      printf("Enter answer:\n>");
      fgets(buf, sizeof(buf), stdin);
      printf("buf is %s\n", buf);
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

          //and ask it to send us the next one:
          snprintf(buf, NET_BUF_LEN, "%s\n", "b");

#ifdef LAN_DEBUG
          printf("requesting next question, buf: %s", buf);
#endif
          if (SDLNet_TCP_Send(sd, (void *)buf, NET_BUF_LEN) < NET_BUF_LEN)
          {
            fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
            exit(EXIT_FAILURE);
          }
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


