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


#include "globals.h"


void DisplayHighScores(int level);
void HighScoreNameEntry(char* pl_name);
void NameEntry(char* pl_name, const char* heading, const char* sub);
void Standby(const char* heading, const char* sub);
int detecting_servers(const char* heading, const char* sub);
void Ready(const char* heading);

int check_score_place(int diff_level, int new_score);
int insert_score(char* playername, int diff_level, int new_score);
void initialize_scores(void);
void print_high_scores(FILE* fp);
int read_high_scores_fp(FILE* fp);
/* Note: for writing, use append_high_score in fileops.c */

int HS_Score(int diff_level, int place);
char* HS_Name(int diff_level, int place);
#endif
