/*

        server.h

        Description: As of now it conatins the enum, which identifies
        the network commands , as they are added(WORK IN PROGRESS).

        Author: David Bruce, Akash Gangil and the TuxMath team, (C) 2009

        Copyright: See COPYING file that comes with this distribution (briefly, GNU GPL version 2 or later)

*/



#ifndef SERVER_H
#define SERVER_H

struct client
{
 TCPsocket csd;
 static int flag=0;                 //flag=1 , if it has been alloted to a client, and 0 otherwise
}client[16];


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
