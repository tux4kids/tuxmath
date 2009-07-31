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

#include "transtruct.h"
#include "network.h"
#include "throttle.h"
//#include "testclient.h"


TCPsocket sd;           /* Server socket descriptor */
SDLNet_SocketSet set;
IPaddress serv_ip;
ServerEntry servers[MAX_SERVERS];

/* Local function prototypes: */
int say_to_server(char *statement);
int evaluate(char *statement);
int add_to_server_list(UDPpacket* pkt);
void print_server_list(void);

int LAN_DetectServers(void)
{
  UDPsocket udpsock = NULL;  
  UDPpacket* out;
  UDPpacket* in;
  IPaddress bcast_ip;
  int sent = 0;
  int done = 0;
  int seconds = 0;
  int num_servers = 0;
  int i = 0;

  //zero out old server list
  for(i = 0; i < MAX_SERVERS; i++)
    servers[i].ip.host = 0;

  /* Docs say we are supposed to call SDL_Init() before SDLNet_Init(): */
  if(SDL_Init(0)==-1)
  {
    printf("SDL_Init: %s\n", SDL_GetError());
    return 0;;
  }

  /* Initialize SDL_net */
  if (SDLNet_Init() < 0)
  {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }

  //NOTE we can't open a UDP socket on the same port if both client
  //and server are running on the same machine, so for now we let
  //it be auto-assigned:
  udpsock = SDLNet_UDP_Open(0);
  if(!udpsock)
  {
    printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
    return 0;
  }
  
  out = SDLNet_AllocPacket(NET_BUF_LEN);
  in = SDLNet_AllocPacket(NET_BUF_LEN);

  SDLNet_ResolveHost(&bcast_ip, "255.255.255.255", DEFAULT_PORT);
  out->address.host = bcast_ip.host;
  sprintf(out->data, "TUXMATH_CLIENT");
  out->address.port = bcast_ip.port;
  out->len = strlen("TUXMATH_CLIENT") + 1;

  //Here we will need to send every few sec onds until we hear back from server
  //and get its ip address:  IPaddress bcast_ip;
  printf("\nAutodetecting TuxMath servers:");
  fflush(stdout);
  while(!done)
  {
    printf(".");
    fflush(stdout);

    sent = SDLNet_UDP_Send(udpsock, -1, out);
    if(!sent)
    {
      printf("broadcast failed - network inaccessible.\nTrying localhost (for testing)\n");
      SDLNet_ResolveHost(&bcast_ip, "localhost", DEFAULT_PORT);
      out->address.host = bcast_ip.host;
    }
    SDL_Delay(250);  //give server chance to answer

    while(SDLNet_UDP_Recv(udpsock, in))
    {
      if(strncmp((char*)in->data, "TUXMATH_SERVER", strlen("TUXMATH_SERVER")) == 0)
      {
        done = 1;
        //add to list, checking for duplicates
        num_servers = add_to_server_list(in);
      }
    }
    //Make sure we always scan at least five but not more than ten seconds:
    seconds++;
    if(seconds < 5)
      done = 0;
    if(seconds > 10)
      done = 1;
    Throttle(1000); //repeat once per second
  }

  printf("done\n\n");


  SDLNet_FreePacket(out); 
  SDLNet_FreePacket(in); 
  print_server_list();
  return num_servers;
}


char* LAN_ServerName(int i)
{
  if(servers[i].ip.host != 0)
    return servers[i].name;
  else
    return NULL; 
}



//For the simple case where a single server is found, i is 
//always 0. Otherwise the player has to review the choices
//via LAN_ServerName(i) to get the index 
int LAN_AutoSetup(int i)
{
  /* Open a connection based on autodetection routine: */
  if (!(sd = SDLNet_TCP_Open(&servers[i].ip)))
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



int LAN_Setup(char *host, int port)
{
  IPaddress ip;           /* Server address */

  if(SDL_Init(0)==-1)
  {
    printf("SDL_Init: %s\n", SDL_GetError());
    return 0;;
  }

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
  snprintf(buf, NET_BUF_LEN, "%s\t%s", "SET_NAME", name);
  return say_to_server(buf);
}






// int LAN_NextQuestion(void)
// {
//   char buf[NET_BUF_LEN];
// 
//   snprintf(buf, NET_BUF_LEN, 
//                   "%s",
//                   "NEXT_QUESTION");
//   return say_to_server(buf);
// }


/* Appears a return value of 0 means message received, 1 means no socket activity */
int check_messages(char buf[NET_BUF_LEN])
{ 
  int numready;
  
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
   // check socket with SDLNet_SocketReady and handle if active:
    if(SDLNet_SocketReady(sd))
    {
      buf[0] = '\0';
      if(SDLNet_TCP_Recv(sd, buf, NET_BUF_LEN) <= 0)
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
  snprintf(buffer, NET_BUF_LEN, "%s", "START_GAME");
  return say_to_server(buffer);
}


int LAN_AnsweredCorrectly(MC_FlashCard* fc)
{
  char buffer[NET_BUF_LEN];
  snprintf(buffer, NET_BUF_LEN, "%s\t%d", "CORRECT_ANSWER", fc->question_id);
  return say_to_server(buffer);
}


int LAN_NotAnsweredCorrectly(MC_FlashCard* fc)
{
  char buffer[NET_BUF_LEN];
  snprintf(buffer, NET_BUF_LEN, "%s\t%d", "WRONG_ANSWER", fc->question_id);
  return say_to_server(buffer);
}


/* This tells the server we are quitting the current math game, but */
/* not disconnecting our socket:                                    */
int LAN_LeaveGame(void)
{
  char buf[NET_BUF_LEN];
  snprintf(buf, NET_BUF_LEN, "%s", "LEAVE_GAME");
  return say_to_server(buf);
}



/*private to network.c functions*/

int say_to_server(char* statement)
{
  char buffer[NET_BUF_LEN];

  if(!statement)
    return 0;

  snprintf(buffer, NET_BUF_LEN, 
                  "%s",
                  statement);
  if (SDLNet_TCP_Send(sd, (void *)buffer, NET_BUF_LEN) < NET_BUF_LEN)
  {
    fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    return 0;
  }

  return 1;
}

//add name to list, checking for duplicates:
int add_to_server_list(UDPpacket* pkt)
{
  int i = 0;
  int already_in = 0;
  char* p = NULL;

  if(!pkt)
    return 0;
 
  //first see if it is already in list:
  while((i < MAX_SERVERS)
      && (servers[i].ip.host != 0))
  {
    if(pkt->address.host == servers[i].ip.host)
      already_in = 1;
    i++;
  }

  //Copy it in unless it's already there, or we are out of room:
  if(!already_in && i < MAX_SERVERS)
  {
    servers[i].ip.host = pkt->address.host;
    servers[i].ip.port = pkt->address.port;
    // not using sscanf() because server_name could contain whitespace:
    p = strchr(pkt->data, '\t');
    p++;
    if(p)
      strncpy(servers[i].name, p, NAME_SIZE);

    i++;
  }

  return i;  //i should be the number of items in the list
}

void print_server_list(void)
{
  int i = 0;
  printf("Detected servers:\n");
  while(i < MAX_SERVERS && servers[i].ip.host != 0)
  {
    printf("SERVER NUMBER %d: %s\n", i, servers[i].name);
    i++;
  }
}



