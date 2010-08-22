/* linewrap.c
  
   convenience API for libgettextpo's linebreak routine

   Copyright 2009, 2010.
   Author: Tim Holy
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org

linewrap.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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



#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//#include <locale.h>
#include "gettext.h"

#include "../linebreak/linebreak.h"
#include "linewrap.h"
#include "globals.h"

static char wrapped_lines0[MAX_LINES][MAX_LINEWIDTH];  // for internal storage
char wrapped_lines[MAX_LINES][MAX_LINEWIDTH]; // publicly available!



int linewrap(const char *input, char str_list[MAX_LINES][MAX_LINEWIDTH],
             int width, int max_lines, int max_width)
{
  int length = strlen (input);
//  char *breaks = malloc (length);
  char breaks[MAX_LINES * MAX_LINEWIDTH];

  int i;
  int listIndex;
  int strIndex;

  if (length > MAX_LINES * MAX_LINEWIDTH)
  {
    fprintf(stderr, "warning: in linewrap(), length of input %d exceeds maximum %d, truncating\n",
            length, MAX_LINES * MAX_LINEWIDTH);
    length = MAX_LINES * MAX_LINEWIDTH;
  }

  // Generate the positions with line breaks
  //mbs_width_linebreaks (input, length, width, 0, 0, NULL, locale_charset (), breaks);
  mbs_width_linebreaks (input, length, width, 0, 0, NULL, "UTF-8", breaks);

  // Load characters into the string list. "breaks" holds "true"
  // values at the first character of the next line, not at the space
  // between words.
  listIndex = 0;
  for (strIndex = 0, i = 0; i < length; strIndex++, i++)
  {
    if (breaks[i] == UC_BREAK_POSSIBLE
     || breaks[i] == UC_BREAK_MANDATORY)
    {
      str_list[listIndex][strIndex] = '\0';  // terminate the previous string
      strIndex = 0;                          // start the next line
      listIndex++;
      if (listIndex >= max_lines)
	break;
    }
    if (strIndex < max_width)
      str_list[listIndex][strIndex] = input[i];
  }
  str_list[listIndex][strIndex] = '\0';

//  free(breaks);

  // Return the number of lines
  if (listIndex < max_lines)
    return listIndex + 1;
  else
    return max_lines;
}

void linewrap_list(const char input[MAX_LINES][MAX_LINEWIDTH],
                   char str_list[MAX_LINES][MAX_LINEWIDTH],
                   int width, int max_lines, int max_width)
{
  int inputIndex;
  int outputIndex;
  int intermedIndex;
  int n_lines;

  outputIndex = 0;
  for (inputIndex = 0; strlen(input[inputIndex]) > 0 && outputIndex < max_lines-1; inputIndex++)
  {
    DEBUGMSG(debug_text_and_intl,"inputIndex = %d, outputIndex = %d, String: %s\n",
	   inputIndex, outputIndex, input[inputIndex]);
      
    /* Handle blank strings */
    if (strcmp(input[inputIndex], " ") == 0)
      {
	strcpy(str_list[outputIndex++]," ");
	DEBUGMSG(debug_text_and_intl,"Blank (%d)\n",inputIndex);
	continue;
      }
    
    /* Handle real strings */
    DEBUGMSG(debug_text_and_intl,"Not blank. Translated: %s\n",gettext(input[inputIndex]));
    n_lines = linewrap(gettext(input[inputIndex]), wrapped_lines0, width, max_lines, max_width);
    DEBUGMSG(debug_text_and_intl,"Wrapped to %d lines.\n", n_lines);

    for (intermedIndex = 0;
         intermedIndex < n_lines && outputIndex < max_lines-1;
         intermedIndex++, outputIndex++)
    {
      DEBUGMSG(debug_text_and_intl,"intermedIndex %d, outputIndex %d, string %s\n",intermedIndex,outputIndex, wrapped_lines0[intermedIndex]);
      strncpy(str_list[outputIndex], wrapped_lines0[intermedIndex], max_width);
    }
  }

  DEBUGMSG(debug_text_and_intl,"All done (outputIndex = %d)\n",outputIndex);

  for (; outputIndex < max_lines; outputIndex++)
  {
    //DEBUGMSG(debug_text_and_intl,"  blanking %d\n", outputIndex);
    str_list[outputIndex][0] = '\0';
  }
  DEBUGMSG(debug_text_and_intl,"All done.\n");
}
