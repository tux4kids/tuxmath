/*
*  C Implementation: highscore.c
*
* Description: Implementation of high score tables for tuxmath.
*
*
* Author: David Bruce <dbruce@tampabay.rr.com>, (C) 2007
*
* Copyright: See COPYING file that comes with this distribution
* (Briefly, GNU GPL version 2 or greater).
*/
#include <string.h>

#include "highscore.h"
#include "tuxmath.h"

typedef struct high_score_entry {
  int score;
  char name[HIGH_SCORE_NAME_LENGTH];
} high_score_entry;


high_score_entry high_scores[NUM_HIGH_SCORE_LEVELS][HIGH_SCORES_SAVED];

/* Local function prototypes: */

/* Zero-out the array before use: */
void initialize_scores(void)
{
  int i, j;
  for (i = 0; i < NUM_HIGH_SCORE_LEVELS; i++)
  {
    for (j = 0; j < HIGH_SCORES_SAVED; j++)
    {
      high_scores[i][j].score = 0;
      strcpy(high_scores[i][j].name, "");
    }
  }
}

/* Test to see where a new score ranks on the list.      */
/* The return value is the ordinary language place (e.g. */
/* 1 for the top of the list), rather than the index     */
/* itself (e.g. 0 for the top of the list - sorry RMS!)  */
int check_score_place(int diff_level, int new_score)
{
  int i = 0;

  /* Make sure diff_level is valid: */
  if (diff_level < 0
   || diff_level > ACE_HIGH_SCORE)
  {
    fprintf(stderr, "In insert_score(), diff_level invalid!\n");
    return 0;
  }

  /* Find correct place in list: */
  for (i = 0; i < HIGH_SCORES_SAVED; i++)
  {
    if (new_score > high_scores[diff_level][i].score)
      break;
  }

  if (HIGH_SCORES_SAVED == i) /* i.e. reached end of list */
  {
    return 0;
  }  
  else
    return (i + 1);
}

/* Put a new high score entry into the table for the corresponding */
/* difficulty level - returns 1 if successful.                     */ 
int insert_score(char* playername, int diff_level, int new_score)
{
  int i = 0;
  int insert_place;

  insert_place = check_score_place(diff_level, new_score);

  if (!insert_place) /* Score didn't make the top 10 */
  {
    return 0;
  }
  else  /* Subtract one to get index instead of common-language */
        /* list position.                                       */
  { 
    insert_place--;
  }

  /* Move lower entries down: */
  for (i = HIGH_SCORES_SAVED - 1; i > insert_place; i--)
  {
    high_scores[diff_level][i].score =
            high_scores[diff_level][i - 1].score;
    strncpy(high_scores[diff_level][i].name,
            high_scores[diff_level][i - 1].name,
            HIGH_SCORE_NAME_LENGTH);
  }

  /* Now put in new entry: */
  high_scores[diff_level][insert_place].score = new_score;
  strncpy(high_scores[diff_level][insert_place].name,
          playername,
          HIGH_SCORE_NAME_LENGTH);
  return 1;
}


void print_high_scores(FILE* fp)
{
  int i, j;

  fprintf(fp, "\nHigh Scores:\n");

  for (i = 0; i < NUM_HIGH_SCORE_LEVELS; i++)
  {
    switch(i)
    {    
      case CADET_HIGH_SCORE:
      {
        fprintf(fp, "\nSpace Cadet:\n");
        break;
      }
      case SCOUT_HIGH_SCORE:
      {
        fprintf(fp, "\nScout:\n");
        break;
      }
      case RANGER_HIGH_SCORE:
      {
        fprintf(fp, "\nRanger:\n");
        break;
      }
      case ACE_HIGH_SCORE:
      {
        fprintf(fp, "\nAce:\n");
        break;
      }
    }

    for (j = 0; j < HIGH_SCORES_SAVED; j++)
    {
      fprintf(fp, "%d.\t%s\t%d\n",
              j + 1,
              high_scores[i][j].name,
              high_scores[i][j].score);
    }
  }
}


int read_high_scores_fp(FILE* fp)
{
  char buf[PATH_MAX];
  char* token;
  const char delimiters[] = "\t";

  char* name_read;
  int score_read;
  int diff_level;


#ifdef TUXMATH_DEBUG
  printf("\nEntering read_high_scores_fp()\n");
#endif

  /* get out if file pointer invalid: */
  if(!fp)
  {
    fprintf(stderr, "In read_high_scores_fp(), file pointer invalid!\n");
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
    /* Split up line with strtok()to get needed values,  */ 
    /* then call insert_score() for each line.           */
    token = strtok(buf, delimiters);
    if (!token)
      continue;
    diff_level = atoi(token);

    token = strtok(NULL, delimiters);
    if (!token)
      continue; 
    score_read = atoi(token);
    /* Note that name can contain spaces - \t is only delimiter: */
    name_read = strtok(NULL, delimiters);
    /* Now insert entry: */
    insert_score(name_read, diff_level, score_read); 
  }
  return 1;
}

/* Write high score table to provided FILE* in format     */
/* compatible with read_high_scores() above.  For human-  */
/* readable output for debugging purposes, print_high_    */
/* scores() in highscore.c is better. write_high_scores() */
/* in fileops.c takes care of checking paths, opening     */
/* and closing the file, etc.                             */
void write_high_scores_fp(FILE* fp)
{
  int i, j;

  /* get out if file pointer invalid: */
  if(!fp)
  {
    fprintf(stderr, "In write_high_scores_fp(), file pointer invalid!\n");
    return;
  }

  for (i = 0; i < NUM_HIGH_SCORE_LEVELS; i++)
  {
    for (j = 0; j < HIGH_SCORES_SAVED; j++)
    {
      fprintf(fp, "%d\t%d\t%s\t\n", i,
                  high_scores[i][j].score,
                  high_scores[i][j].name);
    }
  }
  return;
}
