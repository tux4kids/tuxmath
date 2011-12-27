/*
   server.h:

   Headers related to the tuxmathserver program (or thread) for the 
   multiplayer LAN version of tuxmath

   Copyright 2009, 2010, 2011.
Authors: David Bruce, Akash Gangil
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org


server.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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




#ifndef TM_SERVER_H
#define TM_SERVER_H 

#include "config.h"

#ifdef HAVE_LIBSDL_NET

#include "SDL_net.h"

#define NAME_SIZE 50
#define DEFAULT_SERVER_NAME "TuxMath LAN Server"
#define SERVER_NAME_TIMEOUT 30000
#define DEFAULT_PORT 4779

typedef struct client_type {
    int game_ready;   //game_ready = 1 means client has said OK to start
    char name[NAME_SIZE];
    int score;
    TCPsocket sock;
}client_type;



/* Ways to run the server - all accept command-line style arguments: */

/* 1. Type "tuxmathserver" at command line to run as standalone program. */

/* From within Tuxmath: */

#ifdef HAVE_PTHREAD_H
/* 2. Using POSIX threads library (RECOMMENDED if pthreads available on your system): */
int RunServer_pthread(int argc, char* argv[]);
#endif

/* 3. As a standalone program using system() - same as "tuxmathserver" at console:    */
int RunServer_prog(int argc, char* argv[]);

/* TODO 4. Using old-school Unix fork() call: */
int RunServer_fork(int argc, char* argv[]);

/* 2, 3, and 4 all return immediately, with the server running in a separate thread or process.  But if you don't mind waiting... */
/* 5. Plain "blocking" function call, leaving scheduling issues up to you: */
int RunServer(int argc, char **argv);

/* Find out if server is already running within this program: */
int OurServerRunning(void);
/* Find out if another program (perhaps another tuxmath server program)
 * is using the desired port: */
int PortAvailable(Uint16 port);
/* Find out if game is already in progress: */
int SrvrGameInProgress(void);
/* Stop Server */
void StopServer(void);
/* Stop currently running game: */
void StopSrvrGame(int thread_id_no);

#endif

#endif
