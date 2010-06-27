/************************************************************
 *  factoroids.h                                             *
 *                                                          *
 *  Description: Contains headers for the fractor and       *
 *               fraction activities.                       *
 *                                                          *
 *  Autor:       Jesus M. Mager H. (fongog@gmail.com) 2008  *
 *  Copyright:   GPL v3 or later                            *
 *                                                          *
 *  TuxMath                                                 *
 *  Part of "Tux4Kids" Project                              *
 *  http://tux4kids.alioth.debian.org/                      *
 ************************************************************/

#ifndef FACTOROIDS_H
#define FACTOROIDS_H

// Used in titleecreen.c

int factors(void);
int fractions(void);

#endif

#ifdef SCHOOLMODE
void factoroids_schoolmode(int);
#endif