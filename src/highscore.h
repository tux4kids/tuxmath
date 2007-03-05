//
// C Interface: highscore
//
// Description: 
//
//
// Author: David Bruce <dbruce@tampabay.rr.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
// (Briefly, GNU GPL version 2 or greater).
//

#ifndef HIGHSCORE_H
#define HIGHSCORE_H


#include "tuxmath.h"

enum { 
  CADET_HIGH_SCORE,
  SCOUT_HIGH_SCORE,
  RANGER_HIGH_SCORE,
  ACE_HIGH_SCORE,
  NUM_HIGH_SCORE_LEVELS
};
int check_score_place(int diff_level, int new_score);
int insert_score(char* playername, int diff_level, int new_score);
void initialize_scores(void);
void print_high_scores(FILE* fp);

#endif
