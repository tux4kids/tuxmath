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
#include "mathcards.c"

TCPsocket sd;           /* Socket descriptor */
MC_FlashCard flash;

int main(int argc, char **argv)
{
  IPaddress ip;           /* Server address */
  int quit, len;
  char buffer[512];  // for command-line input
  char buf[512];     // for network messages from server
  MC_FlashCard* fc;
  int x, i = 0;

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
 
  /* Send messages */
  quit = 0;
  while (!quit)
  {
    //Get user input from command line and send it to server: 
    printf("Write something:\n>");
    scanf("%s", buffer);

    if(strcmp(buffer, "exit") == 0)
      quit = 1;
    if(strcmp(buffer, "quit") == 0)
      quit = 1;

    len = strlen(buffer) + 1;
    if (SDLNet_TCP_Send(sd, (void *)buffer, len) < len)
    {
      fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
      exit(EXIT_FAILURE);
    }

    //Now we check for any responses from server:
    //FIXME have to make this not block so we can keep checking until all the messages are gone
    //do
    {
      char command[NET_BUF_LEN];
      int i = 0;
      x = SDLNet_TCP_Recv(sd, buf, sizeof(buf));

      /* Copy the command name out of the tab-delimited buffer: */
      for (i = 0; buf[i] != '\t' && i < NET_BUF_LEN; i++)
        command[i] = buf[i];
      command[i] = '\0';

      /* Now we process the buffer according to the command: */
      if(strcmp(command, "") == 0)


      
    }// while (x > 0);

  }
 
  SDLNet_TCP_Close(sd);
  SDLNet_Quit();
 
  return EXIT_SUCCESS;
}


//function to receive a flashcard(question) by the client
int RecvQuestion(void)
{
  char ch[5];
  int x, i = 0;

  x = SDLNet_TCP_Recv(sd, &(flash.question_id), sizeof(flash.question_id));
  printf("no:(1):::QUESTION_ID::::Received %d bytes\n",x);
 
  x = SDLNet_TCP_Recv(sd, &(flash.difficulty), sizeof(flash.difficulty));
  printf("no:(2):::DIFFICULTY::::Received %d bytes\n",x);
 
  x = SDLNet_TCP_Recv(sd, &(flash.answer), sizeof(flash.answer));
  printf("no:(3):::ANSWER::::Received %d bytes\n",x);

  do{
    x = SDLNet_TCP_Recv(sd, &ch[i], 1);      
    printf("<<<SUB-PACKET%d>>>no:(4):::ANSWER_STRING::::Received %d bytes\n", i, x);
    i++;
  }while(ch[i-1]!='\0');

  strncpy(flash.answer_string, ch, i + 1);

  x = SDLNet_TCP_Recv(sd, flash.formula_string, 13);

  printf("no:(5):::FORMULA_STRING::::Received %d bytes\n",x);
  printf("RECEIVED >>\n");
  printf("QUESTION_ID    >>          %d\n",flash.question_id);  
  printf("FORMULA_STRING >>          %s\n",flash.formula_string);  
  printf("ANSWER_STRING  >>          %s\n",flash.answer_string);  
  printf("ANSWER         >>          %d\n",flash.answer);  
  printf("DIFFICULTY     >>          %d\n",flash.difficulty);  
       
  return 1;
}



 
