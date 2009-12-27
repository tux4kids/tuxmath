//
// C Interface: lessons
//
// Description: Code for reading and parsing the lessons directory,
//              as well as keeping track of the player's progress
//
//
// Author: David Bruce <davidstuartbruce@gmail.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
// (Briefly, GNU GPL version 2 or greater).
//
#ifndef LESSONS_H
#define LESSONS_H

#include "globals.h"

int read_goldstars_fp(FILE* fp);
void write_goldstars_fp(FILE* fp);

#endif
