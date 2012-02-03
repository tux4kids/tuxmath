/*
   network.h:

   Provides routines for various networking functions to be used
   in the LAN multiplayer game.

   Copyright 2009, 2010, 2011.
Authors: David Bruce, Akash Gangil
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org


network.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/




#ifndef NETWORK_H
#define NETWORK_H

#include "config.h"

#ifdef HAVE_LIBSDL_NET

#include "transtruct.h"
#include "SDL_net.h"


typedef struct {
    IPaddress ip;            /* 32-bit IPv4 host address */
    char name[NAME_SIZE];
    char lesson[LESSON_TITLE_LENGTH];
}ServerEntry;

/* Keep information on other connected players for on-screen display: */
typedef struct lan_player_type {
    bool connected;
    char name[NAME_SIZE];
    bool mine;  
    bool ready; 
    int score;  
} lan_player_type;

/* Networking setup and cleanup: */
int LAN_DetectServers(void);
int LAN_AutoSetup(int i);
char* LAN_ServerName(int i);
char* LAN_ConnectedServerName(void);
char* LAN_ConnectedServerLesson(void);
void print_server_list(void);

void LAN_Cleanup(void);
int LAN_SetName(char* name);
int LAN_SetReady(bool ready);
int LAN_RequestIndex(void);
/* Network replacement functions for mathcards "API": */
/* These functions are how the client tells things to the server: */
int LAN_AnsweredCorrectly(int id, float t);
int LAN_NotAnsweredCorrectly(int id);
int LAN_LeaveGame(void);
/* These functions return info about currently connected players */
int LAN_NumPlayers(void);
char* LAN_PlayerName(int i);
bool LAN_PlayerMine(int i);
bool LAN_PlayerReady(int i);
bool LAN_PlayerConnected(int i);
int LAN_PlayerScore(int i);
int LAN_MyIndex(void);
/* This is how the client receives messages from the server: */
int LAN_NextMsg(char* buf);




#endif // HAVE_LIBSDL_NET

#endif // NETWORK_H
