/* lessons.c

   manage bundled lesson config files for tuxmath

   Copyright 2007, 2008, 2009, 2010, 2011.
Author: David Bruce, Tim Holy, Brendan Luchen.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

lessons.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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
//for strtok()
#include <string.h>
#include "lessons.h"
//for basename(), if available
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

// extern unsigned char **lesson_list_titles;
// extern unsigned char **lesson_list_filenames;
int* lesson_list_goldstars = NULL;
// extern int num_lessons;

/* local function prototypes: */
static int filename_comp(const char* s1, const char* s2);

/* Reads the file pointed to by the arg and sets */
/* lesson_list_goldstars* accordingly:           */
int read_goldstars_fp(FILE* fp)
{
    char buf[PATH_MAX];
    char* token;
    const char delimiters[] = "\t\n\r"; /* this will keep newline chars out of string */
    int i;

    DEBUGMSG(debug_lessons, "Entering read_goldstars_fp()\n");

    /* get out if file pointer invalid: */
    if(!fp)
    {
        fprintf(stderr, "In read_goldstars_fp(), file pointer invalid!\n");
        return 0;
    }

    if (num_lessons <= 0)
    {
        perror("no lessons - returning");
        num_lessons = 0;
        return 0;
    }



    /* make sure we start at beginning: */
    rewind(fp);

    /* read in a line at a time: */
    while (fgets (buf, PATH_MAX, fp))
    { 
        /* Ignore comment lines: */
        if ((buf[0] == ';') || (buf[0] == '#'))
        {
            continue;
        }

        /* Split up line with strtok()to get needed values -    */ 
        /* for now, each line just contains a lesson file name, */
        /* but eventually there may be more fields (e.g date, % correct) */
        token = strtok(buf, delimiters);
        if (!token)
            continue;

        /* Now set "goldstar" to 1 if we find a matching lesson: */
        for (i = 0; i < num_lessons; i++)
        {
            /* compare basenames only, not entire path (see below): */
            if (0 == filename_comp(token, lesson_list_filenames[i]))
            {
                lesson_list_goldstars[i] = 1;
                break; //should not have to worry about duplicates
            }
        }
    }
    return 1;
}


/* Write lessons gold star list to the provided FILE* in format  */
/* compatible with read_goldstars_fp () above.            */

void write_goldstars_fp(FILE* fp)
{
    int i = 0;

    DEBUGMSG(debug_lessons, "Entering write_goldstars_fp()\n");

    /* get out if file pointer invalid: */
    if(!fp)
    {
        fprintf(stderr, "In write_goldstars_fp(), file pointer invalid!\n");
        return;
    }

    /* make sure we start at beginning: */
    rewind(fp);

    for (i = 0; i < num_lessons; i++)
    {
        DEBUGMSG(debug_lessons, "i = %d\nfilename = %s\ngoldstar = %d\n",
                i, lesson_list_filenames[i],
                lesson_list_goldstars[i]);

        if(lesson_list_goldstars[i] == 1)
        {
            fprintf(fp, "%s\n", lesson_list_filenames[i]);
        }
    }
    return;
}


/* Perform a strcasecmp() on two path strings, stripping away all the */
/* dirs in the path and just comparing the filenames themselves:      */
/* FIXME: basename() may not be available on all platforms.           */
/* If not available, just compare the full paths. Consider including  */
/* our own implementation at some point. Note that the docs say       */
/* basename() takes a const char*, but the actual header is char*,    */
/* hence the casts to reassure the compiler.                          */
static int filename_comp(const char* s1, const char* s2)
{
#ifdef HAVE_BASENAME
    return strcasecmp(basename((char*)s1), basename((char*)s2));
#else
    return strcasecmp(s1, s2);
#endif
}

