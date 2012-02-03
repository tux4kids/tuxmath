/*

   network.c

   Contains all the network-related functions for
   LAN-based play in "Tux, of Math Command".

   Copyright 2009, 2010, 2011.
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
static int my_index = -1;

/* Keep track of other connected players: */
lan_player_type lan_player_info[MAX_CLIENTS];

/* Local function prototypes: */
int say_to_server(char *statement);
int evaluate(char *statement);
int add_to_server_list(UDPpacket* pkt);
void intercept(char* buf);
int socket_index_recvd(char* buf);
int connected_players_recvd(char* buf);
int parse_player_info_msg(char* buf);
int lan_player_left_recvd(char* buf);

int LAN_DetectServers(void)
{
    UDPsocket udpsock = NULL;  
    UDPpacket* out;
    UDPpacket* out_local;
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

    /* Init player info array for peer clients: */
    for(i = 0; i < MAX_CLIENTS; i++)
    {
        lan_player_info[i].connected = 0;
        strncpy(lan_player_info[i].name, _("Await player name"), NAME_SIZE);
        lan_player_info[i].score = -1;
        lan_player_info[i].mine = 0;
        lan_player_info[i].ready = 0;
    }

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
    out_local = SDLNet_AllocPacket(NET_BUF_LEN);
    in = SDLNet_AllocPacket(NET_BUF_LEN);

    //Prepare packets for broadcast and (for testing) for localhost:
    SDLNet_ResolveHost(&bcast_ip, "255.255.255.255", DEFAULT_PORT);
    out->address.host = bcast_ip.host;
    sprintf(out->data, "TUXMATH_CLIENT");
    out->address.port = bcast_ip.port;
    out->len = strlen("TUXMATH_CLIENT") + 1;

    SDLNet_ResolveHost(&bcast_ip, "255.255.255.255", DEFAULT_PORT);
    out_local->address.host = bcast_ip.host;
    sprintf(out_local->data, "TUXMATH_CLIENT");
    out_local->address.port = bcast_ip.port;
    out_local->len = strlen("TUXMATH_CLIENT") + 1;


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
            sent = SDLNet_UDP_Send(udpsock, -1, out_local);
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

        DEBUGCODE(debug_lan) print_server_list();

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
    SDLNet_FreePacket(out_local); 
    SDLNet_FreePacket(in); 
    SDLNet_UDP_Close(udpsock); 
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



/* NOTE - now we call SDLNet_Quit() in cleanup for overall program
 * so we don't clobber the server if it is still running in a thread
 * when a LAN game finishes.
 */
void LAN_Cleanup(void)
{
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



/* Here we get the next message from the server if one is available. */
/* We return 1 if a message received, 0 if no activity, -1 on errors */
/* or if connection is lost.  Also, some messages are handled within */
/* the network.c system instead of being passed to the rest of the   */
/* program.                                                          */
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
    else  //Make sure we start off with "empty" buffer
        buf[0] = '\0';

    //Check to see if there is socket activity:
    numready = SDLNet_CheckSockets(set, 0);
    if(numready == -1)
    {
        DEBUGMSG(debug_lan, "In LAN_NextMsg(), SDLNet_CheckSockets: %s\n", SDLNet_GetError());
        //most of the time this is a system error, where perror might help you.
        perror("In LAN_NextMsg(), SDLNet_CheckSockets");
        strncpy(buf, "NETWORK_ERROR", NET_BUF_LEN);
        DEBUGMSG(debug_lan, "Leave LAN_NextMsg():\n");
        return -1;
    }
    else if(numready > 0)
    {
        // check with SDLNet_SocketReady():
        if(SDLNet_SocketReady(sd))
        {
            if(SDLNet_TCP_Recv(sd, buf, NET_BUF_LEN) > 0)
            {
                //Success - message is now in buffer
                //We take care of some housekeeping messages internally
                //(e.g. player info) to hide complexity from rest of program;
                //In this case, buf gets replaced with "LAN_INTERCEPTED"
                intercept(buf);
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
                strncpy(buf, "NETWORK_ERROR", NET_BUF_LEN);
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
            strncpy(buf, "NETWORK_ERROR", NET_BUF_LEN);
            DEBUGMSG(debug_lan, "Leave LAN_NextMsg():\n");
            return -1;
        }
    }
    // No socket activity - just return 0:
    DEBUGMSG(debug_lan, "Leave LAN_NextMsg():\n");
    return 0;
}


int LAN_SetReady(bool ready)
{
    char buffer[NET_BUF_LEN];
    if(ready)
        snprintf(buffer, NET_BUF_LEN, "%s", "PLAYER_READY");
    else
        snprintf(buffer, NET_BUF_LEN, "%s", "PLAYER_NOT_READY");
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

/* -------------------------------------------------------------------------   */
/* Functions to tell rest of TuxMath about status of other connected players:  */

int LAN_NumPlayers(void)
{
    int num = 0;
    int i = 0;
    for(i = 0; i < MAX_CLIENTS; i++)
        if(lan_player_info[i].connected)
            num++;
    return num;
}

char* LAN_PlayerName(int i)
{
    if(i < 0 || i >= MAX_CLIENTS)
    {
        fprintf(stderr, "Warning - invalid index %d passed to LAN_PlayerName()\n", i);
        return NULL;
    }  

    return lan_player_info[i].name;
}

bool LAN_PlayerMine(int i)
{
    if(i < 0 || i >= MAX_CLIENTS)
    {
        fprintf(stderr, "Warning - invalid index %d passed to LAN_PlayerMine()\n", i);
        return false;
    }  
    return lan_player_info[i].mine;
}

bool LAN_PlayerReady(int i)
{
    if(i < 0 || i >= MAX_CLIENTS)
    {
        fprintf(stderr, "Warning - invalid index %d passed to LAN_PlayerReady()\n", i);
        return false;
    }  
    return lan_player_info[i].ready;
}

bool LAN_PlayerConnected(int i)
{
    if(i < 0 || i >= MAX_CLIENTS)
    {
        fprintf(stderr, "Warning - invalid index %d passed to LAN_PlayerConnected()\n", i);
        return false;
    }  
    return lan_player_info[i].connected;
}

int LAN_PlayerScore(int i)
{
    if(i < 0 || i >= MAX_CLIENTS)
    {
        fprintf(stderr, "Warning - invalid index %d passed to LAN_PlayerScore()\n", i);
        return -1;
    }  
    return lan_player_info[i].score;
}

int LAN_MyIndex(void)
{
    return my_index;
}




/*private to network.c functions*/

int say_to_server(char* statement)
{
    char buffer[NET_BUF_LEN];

    if(!statement)
        return 0;

    snprintf(buffer, NET_BUF_LEN, "%s", statement);
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
        p = strchr((const char*)pkt->data, '\t');
        p++;
        if(p)
            strncpy(servers[i].name, p, NAME_SIZE);
        // this may have copied the next field as well, so we
        // find the delimiter and terminate the string there:
        p = strchr(servers[i].name, '\t');
        if(p)
            *p = '\0';
        // we also don't want a newline char at the end:
        p = strchr(servers[i].name, '\n');
        if(p)
            *p = '\0';
        // now we go to the second '\t' (note the use of "strrchr()"
        // rather than "strchr()") to get the lesson name:
        p = strrchr((const char*)pkt->data, '\t');
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

/* Some of the server messages are handled within network.c, such
 * as those which maintain the state of the list of connected clients.
 * We handle those here before passing 'buf' to the game itself.
 */
void intercept(char* buf)
{
    if(!buf)
        return;

    if(strncmp(buf, "SOCKET_INDEX", strlen("SOCKET_INDEX")) == 0)
    {
        my_index = socket_index_recvd(buf);
        snprintf(buf, NET_BUF_LEN, "%s", "LAN_INTERCEPTED");
    }
    else if(strncmp(buf, "CONNECTED_PLAYERS", strlen("CONNECTED_PLAYERS")) == 0)
    {
        connected_players_recvd(buf);
        snprintf(buf, NET_BUF_LEN, "%s", "LAN_INTERCEPTED");
    }
    else if(strncmp(buf, "UPDATE_PLAYER_INFO", strlen("UPDATE_PLAYER_INFO")) == 0)
    {
        parse_player_info_msg(buf);
        snprintf(buf, NET_BUF_LEN, "%s", "LAN_INTERCEPTED");
    }
    else if(strncmp(buf, "PLAYER_LEFT", strlen("PLAYER_LEFT")) == 0)
    {
        lan_player_left_recvd(buf); //for this one buf is modified but sent
    }
    /* Otherwise we leave 'buf' unchanged to be handled elsewhere */
}


int socket_index_recvd(char* buf)
{
    int i = 0;
    int index = -1;
    char* p = NULL;

    if(!buf)
        return -1;

    p = strchr(buf, '\t');
    if(!p)
        return -1;
    p++;
    index = atoi(p);

    DEBUGMSG(debug_lan, "socket_index_recvd(): index = %d\n", index);

    if(index < 0 || index > MAX_CLIENTS)
    {
        fprintf(stderr, "socket_index_recvd() - illegal value: %d\n", index);
        return -1;
    }
    for(i = 0; i < MAX_CLIENTS; i++)
    {
        if(i == index)
            lan_player_info[i].mine = 1;
        else
            lan_player_info[i].mine = 0;
    }     
    return index; 
}


int parse_player_info_msg(char* buf)
{
    int i = 0;
    char* p = NULL;

    if(buf == NULL)
        return 0;
    // get i:
    p = strchr(buf, '\t');
    if(!p)
        return 0;
    p++;
    i = atoi(p);
    lan_player_info[i].connected = 1;

    // get ready:
    p = strchr(p, '\t');
    if(!p)
        return 0;
    p++;
    lan_player_info[i].ready = atoi(p);

    //get name:
    p = strchr(p, '\t');
    if(!p)
        return 0;
    p++;
    strncpy(lan_player_info[i].name, p, NAME_SIZE);
    //This has most likely copied the score field as well, so replace the
    //tab delimiter with a null to terminate the string:
    {
        char* p2 = strchr(lan_player_info[i].name, '\t');
        if (p2)
            *p2 = '\0';
    }

    //Now get score:
    p = strchr(p, '\t');
    p++;
    if(p)
        lan_player_info[i].score = atoi(p);

    DEBUGMSG(debug_lan, "update_score_recvd() - buf is: %s\n", buf);
    DEBUGMSG(debug_lan, "i is: %d\tname is: %s\tscore is: %d\n", 
            i, lan_player_info[i].name, lan_player_info[i].score);

    return 1;
}



int lan_player_left_recvd(char* buf)
{
    int i;
    if(!buf)
        return 0;
    i = atoi(buf + strlen("PLAYER_LEFT\t"));
    //rewrite buf to contain name itself for "downstream" rather than index,
    //because we are about to clobber name in lan_player_info[]
    snprintf(buf, NET_BUF_LEN, "%s\t%s", "PLAYER_LEFT", LAN_PlayerName(i));
    strncpy(lan_player_info[i].name, _("Await player name"), NAME_SIZE);
    lan_player_info[i].score = -1;
    lan_player_info[i].ready = false;
    lan_player_info[i].connected = false;
    return 1;
}


/* Here we have been told how many LAN players are still         */
/* in the game. This should always be followed by a series       */
/* of UPDATE_PLAYER_INFO messages, each with the name and score  */
/* of a player. We clear out the array to get rid of anyone      */
/* who has disconnected.                                         */
/* FIXME do we really need this message anymore? - DSB
*/
int connected_players_recvd(char* buf)
{
    int n = 0;
    int i = 0;
    char* p = NULL;

    if(!buf)
        return 0;

    p = strchr(buf, '\t');
    if(!p)
        return 0;
    p++;
    n = atoi(p);

    DEBUGMSG(debug_lan, "connected_players_recvd() for n = %d\n", n);

    if(n < 0 || n > MAX_CLIENTS)
    {
        fprintf(stderr, "connected_players_recvd() - illegal value: %d\n", n);
        return -1;
    }

    /* Reset array - we should be getting new values in immediately */
    /* following messages.                                          */
    for(i = 0; i < MAX_CLIENTS; i++)
    {
        strncpy(lan_player_info[i].name, _("Await player name"), NAME_SIZE);
        lan_player_info[i].score = -1;
        lan_player_info[i].connected = false;
        lan_player_info[i].ready = false;
    }
    return n;
}
#endif
