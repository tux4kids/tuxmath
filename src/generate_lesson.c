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
}

void initialize_scores(void)
{
  /* This is a stub to let things compile */
}  

int main(int argc,char *argv[])
{
  int i;

  /* Initialize MathCards backend for math questions: */
  if (!MC_Initialize())
  {
    printf("\nUnable to initialize MathCards\n");
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
    printf("Reading %s\n",argv[i]);
    read_named_config_file(argv[i]);
  }
  printf("All done reading!\n");

  MC_StartGame();
  MC_PrintQuestionList(stdout);
}
