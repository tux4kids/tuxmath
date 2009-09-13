/*

        server.h

        Description: As of now it conatins the enum, which identifies
        the network commands , as they are added(WORK IN PROGRESS).

        Author: David Bruce, Akash Gangil and the TuxMath team, (C) 2009

        Copyright: See COPYING file that comes with this distribution (briefly, GNU GPL version 2 or later)

*/

#ifndef SERVER_H
#define SERVER_H == 1)

#include "SDL_net.h"


#define NAME_SIZE 50
#define DEFAULT_SERVER_NAME "TuxMath LAN Server"
#define SERVER_NAME_TIMEOUT 30000

typedef struct client_type {
  int game_ready;                 //game_ready = 1 , if client has said OK to start, and 0 otherwise
  char name[NAME_SIZE];
  TCPsocket sock;
}client_type;

  
 
/*enum for commands coming from the client side*/
// enum {
//   EXIT,
//   QUIT,
//   CORRECT_ANSWER,
//   NOT_ANSWERED_CORRECTLY,
//   NEXT_QUESTION,
//   TOTAL_QUESTIONS_LEFT
// };


/*enum for messages for SendMessage*/
enum {
  ANSWER_CORRECT,
  LIST_SET_UP,
  NO_QUESTION_LIST
};


/* Ways to run the server - all accept command-line style arguments: */

/* 1. Type "tuxmathserver" at command line tp rim as standalone program.              */

/* 2. Using POSIX threads library (RECOMMENDED if phtreads available on your system): */
int RunServer_pthreads(int argc, char* argv[]);
/* 3. As a standalone program using system() - same as "tuxmathserver" at console:    */
int RunServer_prog(int argc, char* argv[]);
/* 4. Using old-school Unix fork() call:                                              */
int RunServer_fork(int argc, char* argv[]);

/* 2, 3, and 4 all return immediately, with the server running in a separate thread   */
/* or process.  But if you don't mind waiting...                                      */
/* 5. Plain "blocking" function call, leaving scheduling issues up to you:            */ 
int RunServer(int argc, char **argv);
#endif
