/*
*  C Implementation: fileops.c
*
* (Note: read_config_file() was made possible by studying the file prefs.c in gtkpod:
*  URL: http://www.gtkpod.org/
*  URL: http://gtkpod.sourceforge.net/
*  Copyright (C) 2002-2005 Jorg Schuler <jcsjcs at users sourceforge net>.
*  Licensed under GNU GPL v2.
*  This code is a nearly complete rewrite but I would like to express my thanks.)
*
*
* Description: File operations - together, fileops.h and fileops.c contain 
* all code involving disk operations.  The intention is to make it easier to
* port tuxmath to other operating systems, as code to read and write as 
* well as paths and file locations may be more OS-dependent.
*
* This file contains functions to read and write config files.
* The config file contains name-value pairs, one pair per line, to control
* settings for the behavior of Tuxmath.
*
* Code for loading program data from disk is now also found here.
* 
* Author: David Bruce <dbruce@tampabay.rr.com>, (C) 2006
*
* Copyright: See COPYING file that comes with this distribution (briefly, GNU GPL)
*
*/

/* Standard C includes: */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* OS includes - NOTE: these may not be very portable */
#include <dirent.h>  /* for opendir() */
#include <sys/stat.h>/* for mkdir() */
#include <unistd.h>  /* for getcwd() */

/* SDL includes: */
#include <SDL.h>

#ifndef NOSOUND
#include <SDL_mixer.h>
#endif

#include <SDL_image.h>


/* Tuxmath includes: */
#include "tuxmath.h"
#include "fileops.h"
#include "setup.h"
#include "mathcards.h"



/* local function prototypes: */
static int str_to_bool(char* val);


/* FIXME should have better file path (/etc or /usr/local/etc) and name */
int read_global_config_file(void)
{
  FILE* fp;
  fp = fopen(DATA_PREFIX "/missions/options", "r");
  if (fp)
  {
    read_config_file(fp, GLOBAL_CONFIG_FILE);
    fclose(fp);
    fp = NULL;
    return 1;
  }
  else
    return 0;
}

/* Attempts to read in user's config file - on a *nix system, */
/* something like: /home/laura/.tuxmath/options               */
int read_user_config_file(void)
{
  FILE* fp;
  char opt_path[PATH_MAX];

  /* find $HOME and tack on file name: */
  strcpy(opt_path, getenv("HOME"));
  strcat(opt_path, "/.tuxmath/options");

  #ifdef TUXMATH_DEBUG
  printf("\nIn setup() full path to config file is: = %s\n", opt_path);
  #endif

  fp = fopen(opt_path, "r");
  if (fp) /* file exists */
  {
    read_config_file(fp, USER_CONFIG_FILE);
    fclose(fp);
    fp = NULL;
    return 1;
  }
  else  /* could not open config file: */
  {
    return 0;
  }
}

/* Looks for matching file in various locations:        */
/*   1. Current working directory                       */
/*   2. As an absolute path filename                    */
/*   3. In tuxmath's missions (i.e. lessons) directory. */
/*   4. In user's own .tuxmath directory                */
int read_named_config_file(char* filename)
{
  FILE* fp;
  char opt_path[PATH_MAX];

  #ifdef TUXMATH_DEBUG
  printf("\nIn read_named_config_file() filename is: = %s\n", filename);
  #endif


  /* First look in current working directory:  */
  getcwd(opt_path, PATH_MAX); /* get current working directory */
  /* add separating '/' unless cwd is '/' : */
  if (0 != strcmp("/", opt_path)) 
  {
    strcat(opt_path, "/");
  }
  strcat(opt_path, filename); /* tack on filename              */

  #ifdef TUXMATH_DEBUG


  printf("\nIn read_named_config_file() checking for %s (cwd)\n", opt_path);
  #endif

  fp = fopen(opt_path, "r");  /* try to open file */
  if (fp) /* file exists */
  {
    #ifdef TUXMATH_DEBUG
    printf("\nFound %s\n", opt_path);
    #endif

    if (read_config_file(fp, USER_CONFIG_FILE))
    {
      fclose(fp);
      fp = NULL;
      return 1;
    }
    else /* try matching filename elsewhere */
    {
      fclose(fp);
      fp = NULL;
    }
  }


  /* Next try matching filename as absolute:      */
  /* Supply leading '/' if not already there:   */
  if (0 == strncmp ("/", filename, 1))
  { 
    strcpy(opt_path, filename);
  }
  else
  {
    strcpy(opt_path, "/");
    strcat(opt_path, filename);
  }

  #ifdef TUXMATH_DEBUG
  printf("\nIn read_named_config_file() checking for %s (abs)\n", opt_path);
  #endif

  fp = fopen(opt_path, "r");
  if (fp) /* file exists */
  {
    #ifdef TUXMATH_DEBUG
    printf("\nFound %s\n", opt_path);
    #endif

    if (read_config_file(fp, USER_CONFIG_FILE))
    {
      fclose(fp);
      fp = NULL;
      return 1;
    }
    else /* keep trying to match filename elsewhere */
    {
      fclose(fp);
      fp = NULL;
    }
  }


  /* Next look in missions folder (for prepared "lessons curriculum"):      */
  strcpy(opt_path, DATA_PREFIX);
  strcat(opt_path, "missions/");
  strcat(opt_path, filename);

  #ifdef TUXMATH_DEBUG
  printf("\nIn read_named_config_file() checking for %s (missions)\n", opt_path);
  #endif

  fp = fopen(opt_path, "r");
  if (fp) /* file exists */
  {
    #ifdef TUXMATH_DEBUG
    printf("\nFound %s\n", opt_path);
    #endif

    if (read_config_file(fp, USER_CONFIG_FILE))
    {
      fclose(fp);
      fp = NULL;
      return 1;
    }
    else /* keep trying to match filename elsewhere */
    {
      fclose(fp);
      fp = NULL;
    }
  }  


  /* Look in user's hidden .tuxmath directory  */
  /* find $HOME and tack on file name: */
  strcpy(opt_path, getenv("HOME"));
  strcat(opt_path, "/.tuxmath/");
  strcat(opt_path, filename);

  #ifdef TUXMATH_DEBUG
  printf("\nIn read_named_config_file() checking for %s (.tuxmath)\n", opt_path);
  #endif

  fp = fopen(opt_path, "r");
  if (fp) /* file exists */
  {
    #ifdef TUXMATH_DEBUG
    printf("\nFound %s\n", opt_path);
    #endif

    if (read_config_file(fp, USER_CONFIG_FILE))
    {
      fclose(fp);
      fp = NULL;
      return 1;
    }
    else /* keep trying to match filename elsewhere */
    {
      fclose(fp);
      fp = NULL;
    }
  }


  /* Look in user's home directory  */
  /* find $HOME and tack on file name: */
  strcpy(opt_path, getenv("HOME"));
  strcat(opt_path, "/");
  strcat(opt_path, filename);

  #ifdef TUXMATH_DEBUG
  printf("\nIn read_named_config_file() checking for %s (home)\n", opt_path);
  #endif

  fp = fopen(opt_path, "r");
  if (fp) /* file exists */
  {
    #ifdef TUXMATH_DEBUG
    printf("\nFound %s\n", opt_path);
    #endif

    if (read_config_file(fp, USER_CONFIG_FILE))
    {
      fclose(fp);
      fp = NULL;
      return 1;
    }
    else /* keep trying to match filename elsewhere */
    {
      fclose(fp);
      fp = NULL;
    }
  }

  /* Could not find file (or read it if found) in any location: */
  #ifdef TUXMATH_DEBUG
  printf("\nread_named_config_file() could not find/read: %s\n", opt_path);
  #endif
  return 0;
}






/* This function does the heavy lifting, so to speak:     */
/* Note that file_type simply indicates whether or not    */
/* to change admin-only settings such as per_user_config. */
/* FIXME return value only tells whether file pointer valid */
int read_config_file(FILE *fp, int file_type)
{
  char buf[PATH_MAX];
  char *parameter, *param_begin, *param_end, *value, *value_end;

  #ifdef TUXMATH_DEBUG
  printf("\nEntering read_config_file()\n");
  #endif

  /* get out if file pointer invalid: */
  if(!fp)
  {
    #ifdef TUXMATH_DEBUG
    printf("config file pointer invalid!\n");
    printf("Leaving read_config_file()\n");
    #endif

    fprintf(stderr, "config file pointer invalid!\n");
    return 0;
  }

  /* make sure we start at beginning: */
  rewind(fp);

  /* read in a line at a time: */
  while (fgets (buf, PATH_MAX, fp))
  { 
    #ifdef TUXMATH_DEBUG
    //printf("Beginning fgets() loop\n");
    #endif
    /* "parameter" and "value" will contain the non-whitespace chars */
    /* before and after the '=' sign, respectively.  e.g.:           */
    /*                                                               */
    /* fullscreen = 0;                                               */
    /* parameter is "fullscreen"                                     */
    /* value is '0'                                                  */
    /*                                                               */

    /* ignore comment lines */
    if ((buf[0] == ';') || (buf[0] == '#'))
    {
      #ifdef TUXMATH_DEBUG
      //printf("Skipping comment line\n");
      #endif
      continue;
    }
 
    /* First find parameter string and make a copy: */
    /* start at beginning: */
    param_begin = buf;
    /* skip leading whitespace */
    while (isspace(*param_begin))
    {
      ++param_begin;
    }
    /* now go from here to end of string, stopping at either */
    /* whitespace or '=':   */
    param_end = param_begin;
    while (!isspace(*param_end)
         && ('=' != (*param_end)))
    {
      ++param_end;
    }

    /* copy chars from start of non-whitespace up to '=': */
    parameter = strndup(param_begin, (param_end - param_begin));

    /* Now get value string: */
    /* set value to first '=' in line: */
    value = strchr(buf, '=');

    if (!value || (value == buf))
    {
      #ifdef TUXMATH_DEBUG
      //fprintf(stderr, "Error while reading prefs - line with no '='!\n");
      #endif

      continue;
    }

    /* move past '=' sign: */
    ++value;

    /* skip leading whitespace */
    while (isspace(*value))
    { 
      ++value;
    }

    value_end = value;

    /* remove trailing whitespace or newline */
    while (!isspace(*value_end)
         && (0x0a != (*value_end))
         && (*value_end))
    {
      ++value_end;
    }
    /* terminate string here: */
    *value_end = 0;

    #ifdef TUXMATH_DEBUG
    printf("parameter = '%s'\t, length = %d\n", parameter, strlen(parameter));
    printf("value = '%s'\t, length = %d\t, atoi() = %d\n", value, strlen(value), atoi(value));
    #endif

    /* Now ready to handle each name/value pair! */

    /* Set general game_options struct (see tuxmath.h): */ 
    /* FIXME should have error checking to make sure game_options->* */
    /* settings are sane values (MC_Set*() functions already do this). */
    if(0 == strcasecmp(parameter, "per_user_config")
       && file_type == GLOBAL_CONFIG_FILE) 
    /* Only let administrator change this setting */
    {
      int v = str_to_bool(value);
      if (v != -1)
        game_options->per_user_config = v;
    }

    if(0 == strcasecmp(parameter, "use_sound"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        game_options->use_sound = v;
    }

    else if(0 == strcasecmp(parameter, "fullscreen"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        game_options->fullscreen = v;
    }

    else if(0 == strcasecmp(parameter, "use_bkgd"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        game_options->use_bkgd = v;
    }

    else if(0 == strcasecmp(parameter, "demo_mode"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        game_options->demo_mode = v;
    }

    else if(0 == strcasecmp(parameter, "oper_override"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        game_options->oper_override = v;
    }

    else if(0 == strcasecmp(parameter, "use_keypad"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        game_options->use_keypad = v;
    }

    else if(0 == strcasecmp(parameter, "speed"))
    {
      game_options->speed = atof(value);
    }

    else if(0 == strcasecmp(parameter, "allow_speedup"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        game_options->allow_speedup = v;
    }

    else if(0 == strcasecmp(parameter, "speedup_factor"))
    {
      game_options->speedup_factor = atof(value);
    }

    else if(0 == strcasecmp(parameter, "max_speed"))
    {
      game_options->max_speed = atof(value);
    }

    else if(0 == strcasecmp(parameter, "slow_after_wrong"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        game_options->slow_after_wrong = v;
    }

    else if(0 == strcasecmp(parameter, "starting_comets"))
    {
      game_options->starting_comets = atoi(value);
    }

    else if(0 == strcasecmp(parameter, "extra_comets_per_wave"))
    {
      game_options->extra_comets_per_wave = atoi(value);
    }

    else if(0 == strcasecmp(parameter, "max_comets"))
    {
      game_options->max_comets = atoi(value);
    }


    /* Begin setting of math question options (see mathcards.h):   */ 

    /* General math options */

    else if(0 == strcasecmp(parameter, "allow_negatives"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetAllowNegatives(v);
    }

    else if(0 == strcasecmp(parameter, "max_answer"))
    {
      MC_SetMaxAnswer(atoi(value));
    }

    else if(0 == strcasecmp(parameter, "max_questions"))
    {
      MC_SetMaxQuestions(atoi(value));
    }

    else if(0 == strcasecmp(parameter, "play_through_list"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetPlayThroughList(v);
    }

    else if(0 == strcasecmp(parameter, "repeat_wrongs"))
    {
      int v = str_to_bool(value);
      if (v != -1) 
        MC_SetRepeatWrongs(v);
    }

    else if(0 == strcasecmp(parameter, "copies_repeated_wrongs"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetCopiesRepeatedWrongs(v);
    }

    else if(0 == strcasecmp(parameter, "format_answer_last"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetFormatAnswerLast(v);
    }

    else if(0 == strcasecmp(parameter, "format_answer_first"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetFormatAnswerFirst(v);
    }

    else if(0 == strcasecmp(parameter, "format_answer_middle"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetFormatAnswerMiddle(v);
    }

    else if(0 == strcasecmp(parameter, "format_answer_last"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetFormatAnswerLast(v);
    }

    else if(0 == strcasecmp(parameter, "question_copies"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetQuestionCopies(v);
    }

    else if(0 == strcasecmp(parameter, "randomize"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetRandomize(v);
    }


    /* Set the allowed math operations: */


    else if(0 == strcasecmp(parameter, "addition_allowed"))
    {
      MC_SetAddAllowed(atoi(value));
    }

    else if(0 == strcasecmp(parameter, "subtraction_allowed"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetSubAllowed(v);
    }

    else if(0 == strcasecmp(parameter, "multiplication_allowed"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetMultAllowed(v);
    }

    else if(0 == strcasecmp(parameter, "division_allowed"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetDivAllowed(v);
    }


    /* Set min and max for addition: */


    else if(0 == strcasecmp(parameter, "min_augend"))
    {
      MC_SetAddMinAugend(atoi(value));
    }

    else if(0 == strcasecmp(parameter, "max_augend"))
    {
      MC_SetAddMaxAugend(atoi(value));
    }

    else if(0 == strcasecmp(parameter, "min_addend"))
    {
      MC_SetAddMinAddend(atoi(value));
    }

    else if(0 == strcasecmp(parameter, "max_addend"))
    {
      MC_SetAddMaxAddend(atoi(value));
    }


    /* Set min and max for subtraction: */


    else if(0 == strcasecmp(parameter, "min_minuend"))
    {
      MC_SetSubMinMinuend(atoi(value));
    }

    else if(0 == strcasecmp(parameter, "max_minuend"))
    {
      MC_SetSubMaxMinuend(atoi(value));
    }

    else if(0 == strcasecmp(parameter, "min_subtrahend"))
    {
      MC_SetSubMinSubtrahend(atoi(value));
    }

    else if(0 == strcasecmp(parameter, "max_subtrahend"))
    {
      MC_SetSubMaxSubtrahend(atoi(value));
    }


    /* Set min and max for multiplication: */


    else if(0 == strcasecmp(parameter, "min_multiplier"))
    {
      MC_SetMultMinMultiplier(atoi(value));
    }

    else if(0 == strcasecmp(parameter, "max_multiplier"))
    {
      MC_SetMultMaxMultiplier(atoi(value));
    }

    else if(0 == strcasecmp(parameter, "min_multiplicand"))
    {
      MC_SetMultMinMultiplicand(atoi(value));
    }

    else if(0 == strcasecmp(parameter, "max_multiplicand"))
    {
      MC_SetMultMaxMultiplicand(atoi(value));
    }


    /* Set min and max for division: */


    else if(0 == strcasecmp(parameter, "min_divisor"))
    {
      MC_SetDivMinDivisor(atoi(value));
    }

    else if(0 == strcasecmp(parameter, "min_quotient"))
    {
      MC_SetDivMinQuotient(atoi(value));
    }

    else if(0 == strcasecmp(parameter, "max_divisor"))
    {
      MC_SetDivMaxDivisor(atoi(value));
    }

    else if(0 == strcasecmp(parameter, "max_quotient"))
    {
      MC_SetDivMaxQuotient(atoi(value));
    }

    else
    {   
      #ifdef TUXMATH_DEBUG
      printf("parameter not recognized: %s\n", parameter);
      #endif    
    }
    free(parameter);
  }

  #ifdef TUXMATH_DEBUG
  printf("\nAfter file read in:\n");
  print_game_options(stdout, 0);
  MC_PrintMathOptions(stdout, 0);
  printf("Leaving read_config_file()\n");
  #endif

  return 1;
}



int write_user_config_file(void)
{
  char opt_path[PATH_MAX];
  FILE* fp;
  DIR* dir_ptr;

  /* find $HOME */
  strcpy(opt_path, getenv("HOME"));

  #ifdef TUXMATH_DEBUG
  printf("\nIn write_user_config_file() home directory is: = %s\n", opt_path);
  #endif

  /* add rest of path to user's tuxmath dir: */
  strcat(opt_path, "/.tuxmath");

  #ifdef TUXMATH_DEBUG
  printf("\nIn write_user_config_file() tuxmath dir is: = %s\n", opt_path);
  #endif

  /* find out if directory exists - if not, create it: */
  dir_ptr = opendir(opt_path);
  if (dir_ptr)  /* don't leave DIR* open if it was already there */
  {
    #ifdef TUXMATH_DEBUG
    printf("\nIn write_user_config_file() tuxmath dir opened OK\n");
    #endif

    closedir(dir_ptr);
  }
  else /* need to create tuxmath config directory: */
  {
    int status;

    /* if user's home has a _file_ named .tuxmath (as from previous version */
    /* of program), need to get rid of it or directory creation will fail:  */
    fp = fopen(opt_path, "r");
    if (fp)
    {
      #ifdef TUXMATH_DEBUG
      printf("\nIn write_user_config_file() - removing old .tuxmath file\n");
      #endif

      fclose(fp);
      remove(opt_path);
    }

    #ifdef TUXMATH_DEBUG
    printf("\nIn write_user_config_file() - trying to create .tuxmath dir\n");
    #endif

    status = mkdir(opt_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    #ifdef TUXMATH_DEBUG
    printf("\nIn write_user_config_file() - mkdir returned: %d\n", status);
    #endif

    /* mkdir () returns 0 if successful */
    if (0 != status)
    {
      fprintf(stderr, "\nwrite_user_config_file() - mkdir failed\n");
      return 0;
    }
       
  }

  /* Add filename: */
  strcat(opt_path, "/options");

  #ifdef TUXMATH_DEBUG
  printf("\nIn write_user_config_file() full path to config file is: = %s\n", opt_path);
  #endif

  /* save settings: */
  fp = fopen(opt_path, "w");
  if (fp)
  {
    write_config_file(fp);
    fclose(fp);
    fp = NULL;
    return 1;
  }
  else
    return 0;
}



/* this function writes the settings for all game options to a */
/* human-readable file.                                        */

int write_config_file(FILE *fp)
{
  #ifdef TUXMATH_DEBUG
  printf("\nEntering write_config_file()\n");
  #endif

  /* get out if file pointer null */
  if(!fp)
  {
    fprintf (stderr, "write_config_file() - file pointer invalid/n");

    #ifdef TUXMATH_DEBUG
    printf("Leaving write_config_file()\n");
    #endif

    return 0;
  }

  fprintf(fp, 
          "############################################################\n"
          "#                                                          #\n"
          "#                 Tuxmath Config File                      #\n"
          "#                                                          #\n"
          "############################################################\n"
          "\n"
  );

  /* print general game options (passing '1' as second arg causes */
  /* "help" info for each option to be written to file as comments) */
  print_game_options(fp, 1);
  /* print options pertaining to math questions from MathCards: */
  MC_PrintMathOptions(fp, 1);

  #ifdef TUXMATH_DEBUG
  printf("Leaving write_config_file()\n");
  #endif

  return 1;
}


/* prints struct to stream: */
void print_game_options(FILE* fp, int verbose)
{
 /* bail out if no struct */
  if (!game_options)
  {
    fprintf(stderr, "print_game_options(): invalid game_option_type struct");
    return;
  }

  if(verbose)
  {
    fprintf (fp, "\n############################################################\n" 
                 "#                                                          #\n"
                 "#                 General Game Options                     #\n"
                 "#                                                          #\n"
                 "# The following options are boolean (true/false) variables #\n"
                 "# that control various aspects of Tuxmath's behavior.      #\n"
                 "# The program writes the values to the file as either '0'  #\n"
                 "# or '1'. However, the program accepts 'n', 'no', 'f', and #\n"
                 "# 'false' as synonyms for '0', and similarly accepts 'y',  #\n"
                 "# 'yes', 't', and 'true' as synonyms for '1' (all case-    #\n"
                 "# insensitive).                                            #\n"
                 "############################################################\n\n");
  }

  if(verbose)
  {
    fprintf (fp, "############################################################\n" 
                 "# 'per_user_config' determines whether Tuxmath will look   #\n"
                 "# in the user's home directory for settings. Default is 1  #\n"
                 "# (yes). If deselected, the program will ignore the user's #\n"
                 "# .tuxmath file and use the the global settings in the     #\n"
                 "# installation-wide config file.                           #\n"
                 "# This setting cannot be changed by an ordinary user.      #\n"
                 "############################################################\n");
  }
  fprintf(fp, "per_user_config = %d\n", game_options->per_user_config);

  if(verbose)
  {
    fprintf (fp, "\n# Self-explanatory, default is 1:\n");
  }
  fprintf(fp, "use_sound = %d\n", game_options->use_sound);

  if(verbose)
  {
    fprintf (fp, "\n# Use either fullscreen at 640x480 resolution or window of that size\n"
                 "# Default is 1.  Change to 0 if SDL has trouble with fullscreen.\n");
  } 
  fprintf(fp, "fullscreen = %d\n", game_options->fullscreen);

  if(verbose)
  {
    fprintf (fp, "\n# Use 640x480 jpg image for background; default is 1.\n");
  }
  fprintf(fp, "use_bkgd = %d\n", game_options->use_bkgd);

  if(verbose)
  {
    fprintf (fp, "\n# Program runs as demo; default is 0.\n");
  }
  fprintf(fp, "demo_mode = %d\n", game_options->demo_mode);

  if(verbose)
  {
    fprintf (fp, "\n# Use operator selection from command line; default is 0.\n");
  }
  fprintf(fp, "oper_override = %d\n", game_options->oper_override);

  if(verbose)
  {
    fprintf (fp, "\n# Display onscreen numeric keypad; default is 0.\n");
  }
  fprintf(fp, "use_keypad = %d\n", game_options->use_keypad);

  if(verbose)
  {
    fprintf (fp, "\n############################################################\n" 
                 "# The remaining settings determine the speed and number    #\n"
                 "# of comets.  The speed settings are float numbers (mean-  #\n"
                 "# ing decimals allowed). The comet settings are integers.  #\n"
                 "############################################################\n");
  }

  if(verbose)
  {
    fprintf (fp, "\n# Starting comet speed. Default is 1.\n");
  }
  fprintf(fp, "speed = %f\n", game_options->speed);

  if(verbose)
  {
    fprintf (fp, "\n# Speed is multiplied by this factor with each new wave.\n"
                 "# Default is 1.2\n");
  }
  fprintf(fp, "speedup_factor = %f\n", game_options->speedup_factor);

  if(verbose)
  {
    fprintf (fp, "\n# Maximum speed. Default is 10.\n");
  }
  fprintf(fp, "max_speed = %f\n", game_options->max_speed);

  if(verbose)
  {
    fprintf (fp, "\n# Number of comets for first wave. Default is 2.\n");
  }
  fprintf(fp, "starting_comets = %d\n", game_options->starting_comets);

  if(verbose)
  {
    fprintf (fp, "\n# Comets to add for each successive wave. Default is 2.\n");
  }
  fprintf(fp, "extra_comets_per_wave = %d\n", game_options->extra_comets_per_wave);

  if(verbose)
  {
    fprintf (fp, "\n# Maximum number of comets. Default is 10.\n");
  }
  fprintf(fp, "max_comets = %d\n", game_options->max_comets);

  if(verbose)
  {
    fprintf (fp, "\n# Allow speed and number of comets to increase with each\n"
                 "# wave.  May want to turn this off for smaller kids. Default is 1.\n");
  }
  fprintf(fp, "allow_speedup = %d\n", game_options->allow_speedup);

  if(verbose)
  {
    fprintf (fp, "\n# Go back to starting speed and number of comets if player\n"
                 "# misses a question. Useful for smaller kids. Default is 0.\n");
  }
  fprintf(fp, "slow_after_wrong = %d\n", game_options->slow_after_wrong);

/*
  fprintf(fp, "num_cities = %d\n", game_options->num_cities);
  fprintf(fp, "num_bkgds = %d\n", game_options->num_bkgds);
  fprintf(fp, "max_city_colors = %d\n", game_options->max_city_colors);
*/
}



/* Allows use of "true", "YES", T, etc. in text file for boolean values. */
/* Return value of -1 means value string is not recognized.              */
static int str_to_bool(char* val)
{
  char* ptr;

  /* Check for recognized boolean strings: */
  if ((0 == strcasecmp(val, "true"))
    ||(0 == strcasecmp(val, "t"))
    ||(0 == strcasecmp(val, "yes"))
    ||(0 == strcasecmp(val, "y")))
  {
    return 1;
  }

  if ((0 == strcasecmp(val, "false"))
    ||(0 == strcasecmp(val, "f"))
    ||(0 == strcasecmp(val, "no"))
    ||(0 == strcasecmp(val, "n")))
  {
    return 0;
  }  

  /* Return -1 if any chars are non-digits: */
  ptr = val;
  while (*ptr)
  {
    if (!isdigit(*ptr))
      return -1;
    ptr++;
  }

  /* If we get to here, val should be an integer. */
  if (atoi(val))
    return 1;
  else
    return 0;
}





/*****************************************************************/
/*   Loading of data files for images and sounds.                */
/*   These functions also draw some user feedback to             */
/*   display the progress of the loading.                        */
/*****************************************************************/

/* returns 1 if all data files successfully loaded, 0 otherwise. */
int load_image_data()
{
  int total_files, i;

  SDL_Rect dest;

  static char* image_filenames[NUM_IMAGES] = {
  DATA_PREFIX "/images/status/standby.png",
  DATA_PREFIX "/images/status/loading.png",
  DATA_PREFIX "/images/status/title.png",
  DATA_PREFIX "/images/status/options.png",
  DATA_PREFIX "/images/status/tux4kids.png",
  DATA_PREFIX "/images/status/nbs.png",
  DATA_PREFIX "/images/status/tux_helmet1.png",
  DATA_PREFIX "/images/status/tux_helmet2.png",
  DATA_PREFIX "/images/status/tux_helmet3.png", 
  DATA_PREFIX "/images/status/cmd_play.png",
  DATA_PREFIX "/images/status/cmd_options.png",
  DATA_PREFIX "/images/status/cmd_credits.png",
  DATA_PREFIX "/images/status/cmd_quit.png",
  DATA_PREFIX "/images/status/opt_addition.png",
  DATA_PREFIX "/images/status/opt_subtraction.png",
  DATA_PREFIX "/images/status/opt_multiplication.png",
  DATA_PREFIX "/images/status/opt_division.png",
  DATA_PREFIX "/images/status/opt_max_answer.png",
  DATA_PREFIX "/images/status/opt_speed.png",
  DATA_PREFIX "/images/status/opt_q_range.png",
  DATA_PREFIX "/images/status/opt_rng_1_5.png",
  DATA_PREFIX "/images/status/opt_rng_1_5_on.png",
  DATA_PREFIX "/images/status/opt_rng_6_12.png",
  DATA_PREFIX "/images/status/opt_rng_6_12_on.png",
  DATA_PREFIX "/images/status/opt_rng_13_20.png",
  DATA_PREFIX "/images/status/opt_rng_13_20_on.png",
  DATA_PREFIX "/images/status/opt_check.png",
  DATA_PREFIX "/images/status/opt_check_on.png",
  DATA_PREFIX "/images/cities/city-blue.png",
  DATA_PREFIX "/images/cities/csplode-blue-1.png",
  DATA_PREFIX "/images/cities/csplode-blue-2.png",
  DATA_PREFIX "/images/cities/csplode-blue-3.png",
  DATA_PREFIX "/images/cities/csplode-blue-4.png",
  DATA_PREFIX "/images/cities/csplode-blue-5.png",
  DATA_PREFIX "/images/cities/cdead-blue.png",
  DATA_PREFIX "/images/cities/city-green.png",
  DATA_PREFIX "/images/cities/csplode-green-1.png",
  DATA_PREFIX "/images/cities/csplode-green-2.png",
  DATA_PREFIX "/images/cities/csplode-green-3.png",
  DATA_PREFIX "/images/cities/csplode-green-4.png",
  DATA_PREFIX "/images/cities/csplode-green-5.png",
  DATA_PREFIX "/images/cities/cdead-green.png",
  DATA_PREFIX "/images/cities/city-orange.png",
  DATA_PREFIX "/images/cities/csplode-orange-1.png",
  DATA_PREFIX "/images/cities/csplode-orange-2.png",
  DATA_PREFIX "/images/cities/csplode-orange-3.png",
  DATA_PREFIX "/images/cities/csplode-orange-4.png",
  DATA_PREFIX "/images/cities/csplode-orange-5.png",
  DATA_PREFIX "/images/cities/cdead-orange.png",
  DATA_PREFIX "/images/cities/city-red.png",
  DATA_PREFIX "/images/cities/csplode-red-1.png",
  DATA_PREFIX "/images/cities/csplode-red-2.png",
  DATA_PREFIX "/images/cities/csplode-red-3.png",
  DATA_PREFIX "/images/cities/csplode-red-4.png",
  DATA_PREFIX "/images/cities/csplode-red-5.png",
  DATA_PREFIX "/images/cities/cdead-red.png",
  DATA_PREFIX "/images/cities/shields.png",
  DATA_PREFIX "/images/comets/comet1.png",
  DATA_PREFIX "/images/comets/comet2.png",
  DATA_PREFIX "/images/comets/comet3.png",
  DATA_PREFIX "/images/comets/cometex3.png",
  DATA_PREFIX "/images/comets/cometex3.png",
  DATA_PREFIX "/images/comets/cometex2.png",
  DATA_PREFIX "/images/comets/cometex2.png",
  DATA_PREFIX "/images/comets/cometex1a.png",
  DATA_PREFIX "/images/comets/cometex1a.png",
  DATA_PREFIX "/images/comets/cometex1.png",
  DATA_PREFIX "/images/comets/cometex1.png",
  DATA_PREFIX "/images/comets/mini_comet1.png",
  DATA_PREFIX "/images/comets/mini_comet2.png",
  DATA_PREFIX "/images/comets/mini_comet3.png",
  DATA_PREFIX "/images/status/nums.png",
  DATA_PREFIX "/images/status/lednums.png",
  DATA_PREFIX "/images/status/led_neg_sign.png",
  DATA_PREFIX "/images/status/paused.png",
  DATA_PREFIX "/images/status/demo.png",
  DATA_PREFIX "/images/status/demo-small.png",
  DATA_PREFIX "/images/status/keypad.png",
  DATA_PREFIX "/images/status/keypad_no_neg.png",
  DATA_PREFIX "/images/tux/console.png",
  DATA_PREFIX "/images/tux/console_led.png",
  DATA_PREFIX "/images/tux/console_bash.png",
  DATA_PREFIX "/images/tux/tux-console1.png",
  DATA_PREFIX "/images/tux/tux-console2.png",
  DATA_PREFIX "/images/tux/tux-console3.png",
  DATA_PREFIX "/images/tux/tux-console4.png",
  DATA_PREFIX "/images/tux/tux-relax1.png",
  DATA_PREFIX "/images/tux/tux-relax2.png",
  DATA_PREFIX "/images/tux/tux-egypt1.png",
  DATA_PREFIX "/images/tux/tux-egypt2.png",
  DATA_PREFIX "/images/tux/tux-egypt3.png",
  DATA_PREFIX "/images/tux/tux-egypt4.png",
  DATA_PREFIX "/images/tux/tux-drat.png",
  DATA_PREFIX "/images/tux/tux-yipe.png",
  DATA_PREFIX "/images/tux/tux-yay1.png",
  DATA_PREFIX "/images/tux/tux-yay2.png",
  DATA_PREFIX "/images/tux/tux-yes1.png",
  DATA_PREFIX "/images/tux/tux-yes2.png",
  DATA_PREFIX "/images/tux/tux-sit.png",
  DATA_PREFIX "/images/tux/tux-fist1.png",
  DATA_PREFIX "/images/tux/tux-fist2.png",
  DATA_PREFIX "/images/status/wave.png",
  DATA_PREFIX "/images/status/score.png",
  DATA_PREFIX "/images/status/numbers.png",
  DATA_PREFIX "/images/status/gameover.png",
  DATA_PREFIX "/images/status/gameover_won.png"
  };

  if (opts_using_sound())
    total_files = NUM_IMAGES + NUM_SOUNDS + NUM_MUSICS;
  else
    total_files = NUM_IMAGES;

  /* Load images: */
  for (i = 0; i < NUM_IMAGES; i++)
  {
    images[i] = IMG_Load(image_filenames[i]);
    if (images[i] == NULL)
      {
	fprintf(stderr,
		"\nError: I couldn't load a graphics file:\n"
		"%s\n"
		"The Simple DirectMedia error that occured was:\n"
		"%s\n\n", image_filenames[i], SDL_GetError());
        return 0;
      }

    /* Draw suitable user feedback during loading: */
    if (i == IMG_STANDBY)
      {
	dest.x = (screen->w - images[IMG_STANDBY]->w) / 2;
	dest.y = screen->h - images[IMG_STANDBY]->h - 10;
	dest.w = images[IMG_STANDBY]->w;
	dest.h = images[IMG_STANDBY]->h;
	
	SDL_BlitSurface(images[IMG_STANDBY], NULL, screen, &dest);
	SDL_Flip(screen);
      }
    else if (i == IMG_LOADING)
      {
	dest.x = (screen->w - images[IMG_LOADING]->w) / 2;
	dest.y = 0;
	dest.w = images[IMG_LOADING]->w;
	dest.h = images[IMG_LOADING]->h;
	
	SDL_BlitSurface(images[IMG_LOADING], NULL, screen, &dest);
	SDL_Flip(screen);
      }
    else if (i == IMG_TITLE)
      {
	dest.x = (screen->w - images[IMG_TITLE]->w) / 2;
	dest.y = images[IMG_LOADING]->h;
	dest.w = images[IMG_TITLE]->w;
	dest.h = images[IMG_TITLE]->h;
	
	SDL_BlitSurface(images[IMG_TITLE], NULL, screen, &dest);
	SDL_Flip(screen);
      }
    
    /* Green 'status bar' during loading: */
    dest.x = 0;
    dest.y = (screen->h) - 10;
    dest.w = ((screen->w) * (i + 1)) / total_files;
    dest.h = 10;
    
    SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 255, 0));
    SDL_Flip(screen);
  }
  /* If we make it to here OK, return 1: */
  return 1;
}

#ifndef NOSOUND
int load_sound_data(void)
{
  int total_files, i;

  SDL_Rect dest;

  static char* sound_filenames[NUM_SOUNDS] = {
  DATA_PREFIX "/sounds/pop.wav",
  DATA_PREFIX "/sounds/laser.wav",
  DATA_PREFIX "/sounds/buzz.wav",
  DATA_PREFIX "/sounds/alarm.wav",
  DATA_PREFIX "/sounds/shieldsdown.wav",
  DATA_PREFIX "/sounds/explosion.wav",
  DATA_PREFIX "/sounds/click.wav",
  DATA_PREFIX "/sounds/sizzling.wav"
  };

  static char* music_filenames[NUM_MUSICS] = {
  DATA_PREFIX "/sounds/game.mod",
  DATA_PREFIX "/sounds/game2.mod",
  DATA_PREFIX "/sounds/game3.mod"
  };

  /* skip loading sound files if sound system not available: */
  if (opts_using_sound())
  {
    total_files = NUM_IMAGES + NUM_SOUNDS + NUM_MUSICS;

    for (i = 0; i < NUM_SOUNDS; i++)
    {
      sounds[i] = Mix_LoadWAV(sound_filenames[i]);

      if (sounds[i] == NULL)
      {
        fprintf(stderr,
	        "\nError: I couldn't load a sound file:\n"
                "%s\n"
                "The Simple DirectMedia error that occured was:\n"
                "%s\n\n", sound_filenames[i], SDL_GetError());
        return 0;
      }
      
      dest.x = 0;
      dest.y = (screen->h) - 10;
      dest.w = ((screen->w) * (i + 1 + NUM_IMAGES)) / total_files;
      dest.h = 10;

      SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 255, 0));
      SDL_Flip(screen);
    }


    for (i = 0; i < NUM_MUSICS; i++)
    {
      musics[i] = Mix_LoadMUS(music_filenames[i]);

      if (musics[i] == NULL)
      {
        fprintf(stderr,
	        "\nError: I couldn't load a music file:\n"
                "%s\n"
                "The Simple DirectMedia error that occured was:\n"
                "%s\n\n", music_filenames[i], SDL_GetError());
        return 0;
      }
      
      dest.x = 0;
      dest.y = (screen->h) - 10;
      dest.w = ((screen->w) * (i + 1 + NUM_IMAGES + NUM_SOUNDS)) / total_files;
      dest.h = 10;

      SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 255, 0));
      SDL_Flip(screen);
    }
  }
  return 1;
}
#endif
