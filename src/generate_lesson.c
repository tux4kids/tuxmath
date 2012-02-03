/* generate_lesson.c

   A simple standalone program to help test the creation of valid 
   question lists by mathcards.

   Copyright 2008, 2009,2010, 2011.
Author: Tim Holy.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org


generate_lesson.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "globals.h"
#include "options.h"
#include "mathcards.h"
#include "fileops.h"

/* Compile this with the following statement:

   gcc -lm -o generate_lesson -DDATA_PREFIX=\"/usr/local/share/tuxmath\" generate_lesson.c mathcards.c options.c fileops.c lesson.c

Usage: generate_lesson configfile1 configfile2 ...

*/

/* Declarations needed for the auxillary functions */
char **lesson_list_titles = NULL;
char **lesson_list_filenames = NULL;
int num_lessons = 0;

int read_high_scores_fp(FILE* fp)
{
    /* This is a stub to let things compile */
    return 1;
}

void initialize_scores(void)
{
    /* This is a stub to let things compile */
}  

int main(int argc,char *argv[])
{
    int i;
    MC_MathGame game;

    /* Initialize MathCards backend for math questions: */
    if (!MC_Initialize(&game))
    {
        fprintf(stderr, "\nUnable to initialize MathCards\n");
        fprintf(stderr, "\nUnable to initialize MathCards\n");
        exit(1);
    }

    /* initialize game_options struct with defaults DSB */
    if (!Opts_Initialize())
    {
        fprintf(stderr, "\nUnable to initialize game_options\n");
        exit(1);
    }

    /* This next bit allows multiple config files to be read in sequence, since
       this is something that happens in the ordinary course of events
       in tuxmath itself. */
    for (i = 1; i < argc; i++) {
        fprintf(stderr, "Reading %s\n",argv[i]);
        read_named_config_file(&game, argv[i]);
    }
    fprintf(stderr, "All done reading!\n");

    MC_StartGame(&game);
    MC_PrintQuestionList(&game, stdout);
    return 0;
}
