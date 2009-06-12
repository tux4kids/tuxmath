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

#define MC_USE_NEWARC


#ifndef MC_USE_NEWARC
/* struct for individual "flashcard" */
typedef struct MC_FlashCard {
  int num1;
  int num2;
  int num3;
  int operation;
  int format;
  char formula_string[MC_FORMULA_LEN];
  char answer_string[MC_ANSWER_LEN];
} MC_FlashCard;
#else
/* experimental struct for a more generalized flashcard */
typedef struct _MC_FlashCard {
  char* formula_string;
  char* answer_string;
  int answer;
  int difficulty;
} MC_FlashCard;
#endif


#endif
