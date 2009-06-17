/*

        server.h

        Description: As of now it conatinsthe enum, which identifies
        the network commands , as they are added(WORK IN PROGRESS).

        Author: David Bruce ,Akash Gangil and the TuxMath team, (C) 2009

        Copyright: See COPYING file that comes with this distribution (briefly, GNU GPL version 2 or later)

*/



#ifndef SERVER_H
#define SERVER_H


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
