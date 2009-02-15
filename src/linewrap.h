#ifndef LINEWRAP_H
#define LINEWRAP_H

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
extern int linewrap(const char *input, char **str_list, int width, int max_lines, int max_width);

/* This takes a NULL-terminated array of strings and performs
   translation and linewrapping, outputting another NULL-terminated
   array. */
extern void linewrap_list(const char *input[], char *str_list[], int width, int max_lines, int max_width);


/* Storage for linewrapping */
#define MAX_LINES 40
#define MAX_LINEWIDTH 40
extern char *wrapped_lines[];

/* Initialization and cleanup of storage */
void linewrap_initialize();
void linewrap_cleanup();

#endif
