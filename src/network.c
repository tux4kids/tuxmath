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


int setup_net(char *host, int port)
{
  IPaddress ip;           /* Server address */
  int sockets_used;
//  int len;
//  char buf[NET_BUF_LEN];     // for network messages from server
  char buffer[NET_BUF_LEN];  // for command-line input
//  char *check1;
  char name[NAME_SIZE]="Player 1";

  if (SDLNet_Init() < 0)
  {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
  /* Resolve the host we are connecting to */
  if (SDLNet_ResolveHost(&ip, host, port) < 0)
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

  /* We create a socket set so we can check for activity: */
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

 snprintf(buffer, NET_BUF_LEN, 
                       "%s\n",
                       name);
 
  if (SDLNet_TCP_Send(sd, (void *)buffer, NET_BUF_LEN) < NET_BUF_LEN)
  {
   fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
   exit(EXIT_FAILURE);
  }
 
  return 1;

}

int player_msg_recvd(char* buf,char* p)
{
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


int say_to_server(char statement[20])
{
  int len;
  char buffer[NET_BUF_LEN];

   snprintf(buffer, NET_BUF_LEN, 
                  "%s\n",
                  statement);
   len = strlen(buffer) + 1;
   if (SDLNet_TCP_Send(sd, (void *)buffer, NET_BUF_LEN) < NET_BUF_LEN)
   {
     fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
     exit(EXIT_FAILURE);
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
    

void cleanup_client(void)
{
  
  if (set != NULL)
  {
    SDLNet_FreeSocketSet(set);    //releasing the memory of the client socket set
    set = NULL;                   //this helps us remember that this set is not allocated
  } 

  if(sd != NULL)
  {
    SDLNet_TCP_Close(sd);
    sd = NULL;
  }
}


/*This mainly is a network version of all the MathCards Functions
  MC_* that have integer as their return value*/
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



/*The Ping system is not yet used */
void server_pinged(void)
{ 
  int len;
  char buffer[NET_BUF_LEN];

  snprintf(buffer, NET_BUF_LEN, 
                  "%s \n",
                  "PING_BACK");
  len = strlen(buffer) + 1;
  if (SDLNet_TCP_Send(sd, (void *)buffer, NET_BUF_LEN) < NET_BUF_LEN)
  {
    fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
#ifdef LAN_DEBUG
//   printf("Buffer sent is %s\n",buffer);
#endif
 
}



