/*
   network.h:

   Provides routines for various networking functions to be used
   in the LAN multiplayer game.
 
   Copyright 2009, 2010.
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
}ServerEntry;


/* Networking setup and cleanup: */
int LAN_DetectServers(void);
int LAN_AutoSetup(int i);
char* LAN_ServerName(int i);
char* LAN_ConnectedServerName(void);
void print_server_list(void);

//int LAN_Setup(char* host, int port);
void LAN_Cleanup(void);
int LAN_SetName(char* name);

/* Network replacement functions for mathcards "API": */
/* These functions are how the client tells things to the server: */
int LAN_StartGame(void);
int LAN_AnsweredCorrectly(int id, float t);
int LAN_NotAnsweredCorrectly(int id);
int LAN_LeaveGame(void);
/* This is how the client receives messages from the server: */
int LAN_NextMsg(char* buf);

/* NOTE probably won't have this in multiplayer - new quests determined by server */
//int LAN_NextQuestion(void);



/* FIXME appears this one is basically the same as LAN_NextMsg() */
int check_messages(char *);
/* FIXME this should be local to network.c */
int Make_Flashcard(char* buf, MC_FlashCard* fc);

#endif // HAVE_LIBSDL_NET

#endif // NETWORK_H
