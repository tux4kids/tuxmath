/*

        testclient.h

        Description: As of now it conatinsthe enum, which identifies
        the network commands , as they are added(WORK IN PROGRESS).

        Author: David Bruce ,Akash Gangil and the TuxMath team, (C) 2009

        Copyright: See COPYING file that comes with this distribution (briefly, GNU GPL version 2 or later)

*/

/*I really doubt the existence of this file */


#ifndef TESTCLIENT_H
#define TESTCLIENT_H


enum {
  GAME_NOT_STARTED,
  GAME_IN_PROGRESS,
  GAME_OVER_WON,
  GAME_OVER_LOST,
  GAME_OVER_OTHER,
  GAME_OVER_ESCAPE,
  GAME_OVER_WINDOW_CLOSE,
  GAME_OVER_CHEATER,
  GAME_OVER_ERROR
};


enum {
  SEND_QUESTION
};

#endif
