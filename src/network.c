/*
*  C Implementation: network.c
*
*       Description: Contains all the network realted functions , for LAN-based play in Tux,of Math Command.
*
*
* Author: Akash Gangil, David Bruce, and the TuxMath team, (C) 2009
* Developers list: <tuxmath-devel@lists.sourceforge.net>
*
* Copyright: See COPYING file that comes with this distribution.  (Briefly, GNU GPL).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 

#include "SDL_net.h"
#include "transtruct.h"
//#include "testclient.h"


TCPsocket sd;           /* Server socket descriptor */
SDLNet_SocketSet set;

//MC_FlashCard flash;    //current question
/*int quit = 0;

int Make_Flashcard(char *buf, MC_FlashCard* fc);
int LAN_AnsweredCorrectly(MC_FlashCard* fc);
int playgame(void);
void server_pinged(void);*/

/* Local function prototypes: */
int say_to_server(char *statement);
int evaluate(char *statement);


int LAN_Setup(char *host, int port)
{
  IPaddress ip;           /* Server address */
//  int len;
//  char buf[NET_BUF_LEN];     // for network messages from server
  char buffer[NET_BUF_LEN];  // for command-line input
//  char *check1;
  char name[NAME_SIZE]="Player 1";

  if (SDLNet_Init() < 0)
  {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    return 0;
  } 
 
  /* Resolve the host we are connecting to */
  if (SDLNet_ResolveHost(&ip, host, port) < 0)
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

  if(SDLNet_TCP_AddSocket(set, sd) == -1)
  {
    printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
    // perhaps you need to restart the set and make it bigger...
  }

  return 1;
}


void LAN_Cleanup(void)
{
  if(sd)
  {
    SDLNet_TCP_Close(sd);
    sd = NULL;
  }

  if(set)
  {
    SDLNet_FreeSocketSet(set);
    set = NULL;
  }
  SDLNet_Quit();
}



int LAN_SetName(char* name)
{
  char buf[NET_BUF_LEN];

  if(!name)
    return 0;

  snprintf(buf, NET_BUF_LEN, 
                  "%s\t%s",
                  "SET_NAME",
                  name);

  return say_to_server(buf);
}


/* This function prints the 'msg' part of the buffer (i.e. everything */
/* after the first '\t') to stdout.                                   */
int player_msg_recvd(char* buf)
{
  char* p;
  if(buf == NULL)
    return 0;
  p = strchr(buf, '\t');
  if(p)
  { 
    p++;
    printf("%s\n", p);
    return 1;
  }
  else
    return 0;
}



int say_to_server(char* statement)
{
  char buffer[NET_BUF_LEN];

  if(!statement)
    return 0;

  snprintf(buffer, NET_BUF_LEN, 
                  "%s\n",
                  statement);
  if (SDLNet_TCP_Send(sd, (void *)buffer, NET_BUF_LEN) < NET_BUF_LEN)
  {
    fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    return 0;
  }

  return 1;
}


int check_messages(char buf[NET_BUF_LEN])
{ 
  int x = 0, numready;
  
  //This is supposed to check to see if there is activity:
  numready = SDLNet_CheckSockets(set, 0);
  if(numready == -1)
  {
    printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
    //most of the time this is a system error, where perror might help you.
    perror("SDLNet_CheckSockets");
  }
  else if(numready > 0)
  {
#ifdef LAN_DEBUG
//  printf("There are %d sockets with activity!\n", numready);
#endif
   // check all sockets with SDLNet_SocketReady and handle the active ones.
    if(SDLNet_SocketReady(sd))
    {
      buf[0] = '\0';
      x = SDLNet_TCP_Recv(sd, buf, NET_BUF_LEN);
      if( x <= 0)
      {
        fprintf(stderr, "In play_game(), SDLNet_TCP_Recv() failed!\n");
        exit(EXIT_FAILURE);
      }
      return 0;
    }
  }
  return 1;
}


/* Here we get the next message from the server if one is available. */
/* We return 1 if a message received, 0 if no activity, -1 on errors */
/* or if connection is lost:                                         */
int LAN_NextMsg(char* buf)
{ 
  int numready = 0;

  /* Make sure we have place to put message: */
  if(buf == NULL)
  {
    printf("get_next_msg() passed NULL buffer\n");
    return -1;
  }
  
  //Check to see if there is socket activity:
  numready = SDLNet_CheckSockets(set, 0);
  if(numready == -1)
  {
    printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
    //most of the time this is a system error, where perror might help you.
    perror("SDLNet_CheckSockets");
    return -1;
  }
  else if(numready > 0)
  {
   // check with SDLNet_SocketReady():
    if(SDLNet_SocketReady(sd))
    {
      buf[0] = '\0';
      
      if(SDLNet_TCP_Recv(sd, buf, NET_BUF_LEN) > 0)
      {
        //Success - message is now in buffer
        return 1;
      }
      else
      {
        fprintf(stderr, "In get_next_msg(), SDLNet_TCP_Recv() failed!\n");
        SDLNet_TCP_DelSocket(set, sd);
        if(sd != NULL)
          SDLNet_TCP_Close(sd);
        sd = NULL;
        return -1;
      }
    }
    else
    {
      fprintf(stderr, "In get_next_msg(), socket set reported active but no activity found\n");
      SDLNet_TCP_DelSocket(set, sd);
      if(sd != NULL)
        SDLNet_TCP_Close(sd);
      sd = NULL;
      return -1;
    }
  }
  // No socket activity - just return 0:
  return 0;
}




int Make_Flashcard(char* buf, MC_FlashCard* fc)
{
  int i = 0,tab = 0, s = 0;
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


int LAN_StartGame(void)
{
  char buffer[NET_BUF_LEN];
  snprintf(buffer, NET_BUF_LEN, 
                  "%s\n",
                  "START_GAME");
  if (SDLNet_TCP_Send(sd, (void *)buffer, NET_BUF_LEN) < NET_BUF_LEN)
  {
    fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    return 0;
  }
#ifdef LAN_DEBUG
  printf("Sent the game notification %s\n",buffer);
#endif
  return 1;
}


int LAN_AnsweredCorrectly(MC_FlashCard* fc)
{
  char buffer[NET_BUF_LEN];

  snprintf(buffer, NET_BUF_LEN, 
                  "%s %d\n",
                  "CORRECT_ANSWER",
                  fc->question_id);
  if (SDLNet_TCP_Send(sd, (void *)buffer, NET_BUF_LEN) < NET_BUF_LEN)
  {
    fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
  return 1;
}
    




/*This mainly is a network version of all the MathCards Functions
  MC_* that have integer as their return value*/
/* Looks to me like it just sends "statement".  Again, when we send a  */
/* message, we can't assume when we are going to get a reply.          */
int evaluate(char statement[20])
{
  int ans,x;
  char command[NET_BUF_LEN];
  int len;
  char buffer[NET_BUF_LEN];
  char buf[NET_BUF_LEN];

   snprintf(buffer, NET_BUF_LEN, 
                  "%s\n",
                  statement);
   len = strlen(buffer) + 1;
   if (SDLNet_TCP_Send(sd, (void *)buffer, NET_BUF_LEN) < NET_BUF_LEN)
   {
     fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
     exit(EXIT_FAILURE);
   }
  
//   x = SDLNet_TCP_Recv(sd, buf, NET_BUF_LEN);
//  if( x <= 0)
//  {
//    fprintf(stderr, "In play_game(), SDLNet_TCP_Recv() failed!\n");
//    exit(EXIT_FAILURE);
//  }
//  player_msg_recvd(buf,command);
//  ans=atoi(command);

  return 0;
}






