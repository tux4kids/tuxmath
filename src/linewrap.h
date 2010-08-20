/*
   linewrap.h: Contains headers for using GNU liblinebreak within TuxMath
 
   Copyright 2009, 2010.
   Author: Tim Holy.
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org

linewrap.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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




#ifndef LINEWRAP_H
#define LINEWRAP_H


/* Storage for linewrapping */
#define MAX_LINES 128
#define MAX_LINEWIDTH 256

extern char wrapped_lines[MAX_LINES][MAX_LINEWIDTH];

/* linewrap takes an input string (can be in essentially arbitrary
   encoding) and loads it into an array of strings, each corresponding
   to one line of output text.  Arguments:

     input: a null-terminated input string
     str_list: a PRE-ALLOCATED array of character pointers. This must be
       at least of size str_list[max_lines][max_width]
     width: the desired number of characters per line. Note that words
       with more characters than "width" are not hypenated, so it's
       possible to get a line that is longer than "width."
     max_lines and max_width: memory-safety parameters for str_list
       (see above)

   On output, linewrap returns the number of lines used to format the
   string.
*/
extern int linewrap(const char* input, char str_list[MAX_LINES][MAX_LINEWIDTH], int width, int max_lines, int max_width);

/* This takes a NULL-terminated array of strings and performs
   translation and linewrapping, outputting another NULL-terminated
   array. */
extern void linewrap_list(const char input[MAX_LINES][MAX_LINEWIDTH], char str_list[MAX_LINES][MAX_LINEWIDTH], int width, int max_lines, int max_width);

#endif
