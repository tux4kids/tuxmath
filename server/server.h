/*

        server.h

        Description: As of now it conatins the enum, which identifies
        the network commands , as they are added(WORK IN PROGRESS).

        Author: David Bruce, Akash Gangil and the TuxMath team, (C) 2009

        Copyright: See COPYING file that comes with this distribution (briefly, GNU GPL version 2 or later)

*/


#include "SDL_net.h"
#define NAME_SIZE 50

#ifndef SERVER_H
#define SERVER_H
typedef struct client_type {
  int game_ready;                 //game_ready = 1 , if client has said OK to start, and 0 otherwise
  char name[NAME_SIZE];
  TCPsocket sock;
}client_type;

  
 
/*enum for commands coming from the client side*/
enum {
  NEW_GAME,
  SEND_A_QUESTION,
  LIST_NOT_SETUP,
  CORRECT_ANSWER
};


/*enum for messages for SendMessage*/
enum {
  ANSWER_CORRECT,
  LIST_SET_UP,
  NO_QUESTION_LIST
};

#endif
