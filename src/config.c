/*
*  C Implementation: config.c
*
* (Note: this code was made possible by studying the file prefs.c in gtkpod:
*  URL: http://www.gtkpod.org/
*  URL: http://gtkpod.sourceforge.net/
*  Copyright (C) 2002-2005 Jorg Schuler <jcsjcs at users sourceforge net>.
*  Licensed under GNU GPL v2.
*  This code is a nearly complete rewrite but I would like to express my thanks.)
*
*
* Description: This file contains functions to read and write config files.
* The config file contains name-value pairs, one pair per line, to control
* settings for the behavior of Tuxmath.
*

* 
* Author: David Bruce <dbruce@tampabay.rr.com>, (C) 2006
*
* Copyright: See COPYING file that comes with this distribution (briefly, GNU GPL)
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "tuxmath.h"
#include "setup.h"
#include "mathcards.h"

static int str_to_bool(char* val);

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
    printf("Beginning fgets() loop\n");
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
      printf("Skipping comment line\n");
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
      fprintf(stderr, "Error while reading prefs - line with no '='!\n");
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

/* All leftover options will be stored into the prefs
		 setting hash (generic options -- should have had this
		 idea much sooner... */
// 	      int skip = 0;
// 	      if (1)
// 	      {line
// 		  if(arg_comp (line, "itdb_", NULL) == 0)
// 		  {   /* set incorrectly in 0.90 -- delete */
// 		      skip = 1;
// 		  }
// 	      }
// 	      if (!skip)
// 		  prefs_set_string_value (line, arg);
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




/* this function writes the settings for all game options to a */
/* human-readable file.                                        */
/* TODO write help on each setting into file.                  */

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
