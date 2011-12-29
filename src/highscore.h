/*
   highscore.h: Contains headers for Tux Math's high score table.

   Copyright 2007, 2008, 2009, 2010, 2011.
Authors: David Bruce, Tim Holy.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

highscore.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.  */




#ifndef HIGHSCORE_H
#define HIGHSCORE_H


#include "globals.h"


void DisplayHighScores(int level);
void HighScoreNameEntry(char* pl_name);
void NameEntry(char* pl_name, const char* s1, const char* s2, const char* s3);

int check_score_place(int diff_level, int new_score);
int insert_score(char* playername, int diff_level, int new_score);
void initialize_scores(void);
void print_high_scores(FILE* fp);
int read_high_scores_fp(FILE* fp);
/* Note: for writing, use append_high_score in fileops.c */

int HS_Score(int diff_level, int place);
char* HS_Name(int diff_level, int place);
#endif
