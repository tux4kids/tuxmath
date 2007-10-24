/*
*  C Implementation: lessons
*
* Description: 
*
*
* Author: David Bruce <dbruce@tampabay.rr.com>, (C) 2007
*
* Copyright: See COPYING file that comes with this distribution
*
*/
#include "lessons.h"

// extern unsigned char **lesson_list_titles;
// extern unsigned char **lesson_list_filenames;
int* lesson_list_goldstars = NULL;
// extern int num_lessons;

/* local function prototypes: */
static int filename_comp(const unsigned char* s1, const unsigned char* s2);

/* Reads the file pointed to by the arg and sets */
/* lesson_list_goldstars* accordingly:           */
int read_goldstars_fp(FILE* fp)
{
  char buf[PATH_MAX];
  char* token;
  const char delimiters[] = "\t\n\r"; /* this will keep newline chars out of string */
  int i;

#ifdef TUXMATH_DEBUG
  printf("\nEntering read_goldstars_fp()\n");
#endif

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

  /* Allocate storage for lesson list */

  /* prevent memory leak in case we called this already and */
  /* free the list:                                         */
  if(lesson_list_goldstars)
  {
    free(lesson_list_goldstars);
    lesson_list_goldstars = NULL;
  }

  lesson_list_goldstars = (int*)malloc(num_lessons*sizeof(int));
  if (!lesson_list_goldstars)
  {
    perror("unable to allocate memory for gold star list");
    return 0;
  }
  for (i = 0; i < num_lessons; i++)
  {
    lesson_list_goldstars[i] = 0;
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


/* Write lessons gold star list provided FILE* in format  */
/* compatible with read_goldstars_fp () above.            */

void write_goldstars_fp(FILE* fp)
{
  int i = 0;

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "\nEntering write_goldstars_fp()\n");
#endif

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
#ifdef TUXMATH_DEBUG
    printf("i = %d\nfilename = %s\ngoldstar = %d\n",
           i, lesson_list_filenames[i],
           lesson_list_goldstars[i]);
#endif

    if(lesson_list_goldstars[i] == 1)
    {
      fprintf(fp, "%s\n", lesson_list_filenames[i]);
    }
  }
  return;
}

/* Perform a strcasecmp() on two path strings, stripping away all the */
/* dirs in the path and just comparing the filenames themselves:      */
static int filename_comp(const unsigned char* s1, const unsigned char* s2)
{
  char* f1 = basename(s1);
  char* f2 = basename(s2);
  return strcasecmp(f1, f2);
}

