/*

        transtruct.h

        Description: contains headers for the data structures
        that would be transferred between the server and the client
        during the multiplayer LAN game.

        Author: David Bruce ,Akash Gangil and the TuxMath team, (C) 2009

        Copyright: See COPYING file that comes with this distribution (briefly, GNU GPL version 2 or later)

*/
#ifndef TRANSTRUCT_H
#define TRANSTRUCT_H

#define LAN_DEBUG
#define NET_BUF_LEN 512
#define DEFAULT_PORT 4779
#define NAME_SIZE 50
#define MAX_SERVERS 50

#define MC_USE_NEWARC
#define MC_FORMULA_LEN 40
#define MC_ANSWER_LEN 5

#define QUEST_QUEUE_SIZE 10


typedef struct _MC_FlashCard {
  char formula_string[MC_FORMULA_LEN];
  char answer_string[MC_ANSWER_LEN];
  int question_id;
  int answer;
  int difficulty;
} MC_FlashCard;


#endif
