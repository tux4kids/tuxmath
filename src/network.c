/*
   
   network.c

   Contains all the network-related functions for
   LAN-based play in "Tux, of Math Command".
   
   Copyright 2009, 2010.
   Authors: Akash Gangil, David Bruce.
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org

network.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.  */




/* Must have this first for the #ifdef HAVE_LIBSDL_NET to work */
#include "globals.h"

#ifdef HAVE_LIBSDL_NET

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 

#include "mathcards.h"
#include "transtruct.h"
#include "network.h"
#include "server.h"


TCPsocket sd;           /* Server socket descriptor */
SDLNet_SocketSet set;
IPaddress serv_ip;
ServerEntry servers[MAX_SERVERS];
static int connected_server = -1;

/* Local function prototypes: */
int say_to_server(char *statement);
int evaluate(char *statement);
int add_to_server_list(UDPpacket* pkt);


int LAN_DetectServers(void)
{
  UDPsocket udpsock = NULL;  
  UDPpacket* out;
  UDPpacket* in;
  IPaddress bcast_ip;
  int sent = 0;
  int done = 0;
  int attempts = 0;
  int num_servers = 0;
  int i = 0;
  Uint32 timer = 0;
  //zero out old server list
  for(i = 0; i < MAX_SERVERS; i++)
    servers[i].ip.host = 0;

  /* Docs say we are supposed to call SDL_Init() before SDLNet_Init(): */
  if(SDL_Init(0) == -1)
  {
    DEBUGMSG(debug_lan, "SDL_Init: %s\n", SDL_GetError());
    return 0;;
  }

  /* Initialize SDL_net */
  if (SDLNet_Init() < 0)
  {
    DEBUGMSG(debug_lan, "SDLNet_Init: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }

  //NOTE we can't open a UDP socket on the same port if both client
  //and server are running on the same machine, so for now we let
  //it be auto-assigned:
  udpsock = SDLNet_UDP_Open(0);
  if(!udpsock)
  {
    DEBUGMSG(debug_lan, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
    return 0;
  }
  
  out = SDLNet_AllocPacket(NET_BUF_LEN);
  in = SDLNet_AllocPacket(NET_BUF_LEN);

  SDLNet_ResolveHost(&bcast_ip, "255.255.255.255", DEFAULT_PORT);
  out->address.host = bcast_ip.host;
  sprintf(out->data, "TUXMATH_CLIENT");
  out->address.port = bcast_ip.port;
  out->len = strlen("TUXMATH_CLIENT") + 1;

  //Here we will need to send every few seconds until we hear back from server
  //and get its ip address:  IPaddress bcast_ip;
  DEBUGMSG(debug_lan, "\nAutodetecting TuxMath servers:\n");
  DEBUGMSG(debug_lan, "out->address.host = %d\tout->address.port = %d\n", out->address.host, out->address.port);


  while(!done)
  {
    DEBUGMSG(debug_lan, "Sending message: %s\n", (char*)out->data);

    sent = SDLNet_UDP_Send(udpsock, -1, out);
    if(!sent)
    {
      DEBUGMSG(debug_lan, "broadcast failed - network inaccessible.\nTrying localhost (for testing)\n");
      SDLNet_ResolveHost(&bcast_ip, "localhost", DEFAULT_PORT);
      out->address.host = bcast_ip.host;
    }
    SDL_Delay(50);  //give server chance to answer

    while(SDLNet_UDP_Recv(udpsock, in))
    {
      if(strncmp((char*)in->data, "TUXMATH_SERVER", strlen("TUXMATH_SERVER")) == 0)
      {
        done = 1;
        //add to list, checking for duplicates
        num_servers = add_to_server_list(in);
      }
    }

    DEBUGCODE(debug_lan)
    {
      print_server_list();
    }

    //Make sure we always scan at least 0.5 but not more than 2 seconds:
    T4K_Throttle(100, &timer); //repeat 10x per second
    attempts++;
    if(attempts < 5)
      done = 0;
    if(attempts > 20)
      done = 1;
  }

  DEBUGMSG(debug_lan, "done\n\n");

  SDLNet_FreePacket(out); 
  SDLNet_FreePacket(in); 
  SDLNet_UDP_Close(udpsock); 
  print_server_list();
  return num_servers;
}


char* LAN_ServerName(int i)
{
  if(i < 0 || i > MAX_SERVERS)
    return NULL;
  if(servers[i].ip.host != 0)
    return servers[i].name;
  else
    return NULL; 
}

char* LAN_ConnectedServerName(void)
{
   return servers[connected_server].name;
}

char* LAN_ConnectedServerLesson(void)

{
   return servers[connected_server].lesson;
}


//For the simple case where a single server is found, i is 
//always 0. Otherwise the player has to review the choices
//via LAN_ServerName(i) to get the index 
int LAN_AutoSetup(int i)
{
  if(i < 0 || i > MAX_SERVERS)
    return 0;

  /* Open a connection based on autodetection routine: */
  if (!(sd = SDLNet_TCP_Open(&servers[i].ip)))
  {
    DEBUGMSG(debug_lan, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    return 0;
  }

  /* We create a socket set so we can check for activity: */
  set = SDLNet_AllocSocketSet(1);
  if(!set)
  {
    DEBUGMSG(debug_lan, "SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
    return 0;
  }

  if(SDLNet_TCP_AddSocket(set, sd) == -1)
  {
    DEBUGMSG(debug_lan, "SDLNet_AddSocket: %s\n", SDLNet_GetError());
    // perhaps you need to restart the set and make it bigger...
  }

  // Success - record the index for future reference:
  connected_server = i;
  return 1;
}



// int LAN_Setup(char *host, int port)
// {
//   IPaddress ip;           /* Server address */
// 
//   if(SDL_Init(0)==-1)
//   {
//     printf("SDL_Init: %s\n", SDL_GetError());
//     return 0;;
//   }
// 
//   if (SDLNet_Init() < 0)
//   {
//     fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
//     return 0;
//   } 
// 
//    /* Resolve the host we are connecting to */
//   if (SDLNet_ResolveHost(&ip, host, port) < 0)
//   {
//     fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
//     return 0;
//   }
//  
//   /* Open a connection with the IP provided (listen on the host's port) */
//   if (!(sd = SDLNet_TCP_Open(&ip)))
//   {
//     fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
//     return 0;
//   }
// 
//   /* We create a socket set so we can check for activity: */
//   set = SDLNet_AllocSocketSet(1);
//   if(!set)
//   {
//     printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
//     return 0;
//   }
// 
//   if(SDLNet_TCP_AddSocket(set, sd) == -1)
//   {
//     printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
//     // perhaps you need to restart the set and make it bigger...
//   }
// 
// 
//   return 1;
// }

/* NOTE - now we call SDLNet_Quit() in cleanup for overall program
 * so we don't clobber the server if it is still running in a thread
 * when a LAN game finishes.
 */
void LAN_Cleanup(void)
{
  char buf[256];
  
  DEBUGMSG(debug_lan|debug_game, "Enter LAN_cleanup():\n");

  //Empty the queue of any leftover messages:
//  while(LAN_NextMsg(buf)) {} //do nothing with the messages

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

  DEBUGMSG(debug_lan|debug_game, "Leave LAN_cleanup():\n");
}



int LAN_SetName(char* name)
{
  char buf[NET_BUF_LEN];
  if(!name)
    return 0;
  snprintf(buf, NET_BUF_LEN, "%s\t%s", "SET_NAME", name);
  return say_to_server(buf);
}






/* Appears a return value of 0 means message received, 1 means no socket activity */
int check_messages(char buf[NET_BUF_LEN])
{ 
  int numready;
  
  //This is supposed to check to see if there is activity:
  numready = SDLNet_CheckSockets(set, 0);
  if(numready == -1)
  {
    DEBUGMSG(debug_lan, "SDLNet_CheckSockets: %s\n", SDLNet_GetError());
    //most of the time this is a system error, where perror might help you.
    perror("SDLNet_CheckSockets");
    return -1;
  }
  else if(numready > 0)
  {
    // check socket with SDLNet_SocketReady and handle if active:
    if(SDLNet_SocketReady(sd))
    {
      buf[0] = '\0';
      if(SDLNet_TCP_Recv(sd, buf, NET_BUF_LEN) <= 0)
      {
        DEBUGMSG(debug_lan, "In check_messages(), SDLNet_TCP_Recv() failed!\n");
	strncpy(buf, "NETWORK_ERROR", NET_BUF_LEN);
        return -1;
      }
      DEBUGMSG(debug_lan, "In check_messages(), received buf: %s\n", buf);
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

  DEBUGMSG(debug_lan, "Enter LAN_NextMsg():\n");

  /* Make sure we have place to put message: */
  if(buf == NULL)
  {
    DEBUGMSG(debug_lan, "get_next_msg() passed NULL buffer\n");
    DEBUGMSG(debug_lan, "Leave LAN_NextMsg():\n");
    return -1;
  }
  
  //Check to see if there is socket activity:
  numready = SDLNet_CheckSockets(set, 0);
  if(numready == -1)
  {
    DEBUGMSG(debug_lan, "SDLNet_CheckSockets: %s\n", SDLNet_GetError());
    //most of the time this is a system error, where perror might help you.
    perror("SDLNet_CheckSockets");
    DEBUGMSG(debug_lan, "Leave LAN_NextMsg():\n");
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
        DEBUGMSG(debug_lan, "Leave LAN_NextMsg():\n");
        return 1;
      }
      else
      {
        DEBUGMSG(debug_lan, "In get_next_msg(), SDLNet_TCP_Recv() failed!\n");
        SDLNet_TCP_DelSocket(set, sd);
        if(sd != NULL)
          SDLNet_TCP_Close(sd);
        sd = NULL;
        DEBUGMSG(debug_lan, "Leave LAN_NextMsg():\n");
        return -1;
      }
    }
    else
    {
      DEBUGMSG(debug_lan, "In get_next_msg(), socket set reported active but no activity found\n");
      SDLNet_TCP_DelSocket(set, sd);
      if(sd != NULL)
        SDLNet_TCP_Close(sd);
      sd = NULL;
      DEBUGMSG(debug_lan, "Leave LAN_NextMsg():\n");
      return -1;
    }
  }
  // No socket activity - just return 0:
  DEBUGMSG(debug_lan, "Leave LAN_NextMsg():\n");
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

  DEBUGMSG(debug_lan, "In Make_Flashcard, new card is:\n");
  DEBUGCODE(debug_lan) print_card(*fc); 

return 1;
} 


int LAN_StartGame(void)
{
  char buffer[NET_BUF_LEN];
  snprintf(buffer, NET_BUF_LEN, "%s", "START_GAME");
  return say_to_server(buffer);
}


int LAN_RequestIndex(void)
{
  char buffer[NET_BUF_LEN];
  snprintf(buffer, NET_BUF_LEN, "%s", "REQUEST_INDEX");
  return say_to_server(buffer);
}


int LAN_AnsweredCorrectly(int id, float t)
{
  char buffer[NET_BUF_LEN];
  snprintf(buffer, NET_BUF_LEN, "%s\t%d\t%f", "CORRECT_ANSWER", id, t);
  return say_to_server(buffer);
}


int LAN_NotAnsweredCorrectly(int id)
{
  char buffer[NET_BUF_LEN];
  snprintf(buffer, NET_BUF_LEN, "%s\t%d", "WRONG_ANSWER", id);
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
    DEBUGMSG(debug_lan, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
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
    // this may have copied the next field as well, so we
    // find the delimiter and terminate the string there:
    p = strchr(servers[i].name, '\t');
    if(p)
      *p = '\0';
    // now we go to the second '\t' (note the use of "strrchr()"
    // rather than "strchr()") to get the lesson name:
    p = strrchr(pkt->data, '\t');
    p++;
    if(p)
      strncpy(servers[i].lesson, p, LESSON_TITLE_LENGTH);

    i++;
  }

  return i;  //i should be the number of items in the list
}

void print_server_list(void)
{
  int i = 0;
  fprintf(stderr, "Detected servers:\n");
  while(i < MAX_SERVERS && servers[i].ip.host != 0)
  {
    fprintf(stderr, "SERVER NUMBER %d: %s\n", i, servers[i].name);
    i++;
  }
}



#endif
