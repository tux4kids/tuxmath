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

#include "../config.h"

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
#include "options.h"

/* Used by both write_pregame_summary() and */
/* write_postgame_summary() so defined with */
/* file scope:                              */
#ifdef BUILD_MINGW32
#define SUMMARY_EXTENSION ".txt"
#else
#define SUMMARY_EXTENSION ""
#endif 

static char* summary_filenames[NUM_SUMMARIES] = {
  "summary1" SUMMARY_EXTENSION,
  "summary2" SUMMARY_EXTENSION,
  "summary3" SUMMARY_EXTENSION,
  "summary4" SUMMARY_EXTENSION,
  "summary5" SUMMARY_EXTENSION,
  "summary6" SUMMARY_EXTENSION,
  "summary7" SUMMARY_EXTENSION,
  "summary8" SUMMARY_EXTENSION,
  "summary9" SUMMARY_EXTENSION,
  "summary10" SUMMARY_EXTENSION
};


/* local function prototypes: */
static int find_tuxmath_dir(void);
static int str_to_bool(char* val);

/* fix HOME on windows */
#ifdef BUILD_MINGW32
#include <windows.h>

/* STOLEN in tuxpaint */

/*
  Removes a single '\' or '/' from end of path 
*/
static char *remove_slash(char *path)
{
  int len = strlen(path);

  if (!len)
    return path;

  if (path[len-1] == '/' || path[len-1] == '\\')
    path[len-1] = 0;

  return path;
}

/*
  Read access to Windows Registry
*/
static HRESULT ReadRegistry(const char *key, const char *option, char *value, int size)
{
  LONG	res;
  HKEY	hKey = NULL;

  res = RegOpenKeyEx(HKEY_CURRENT_USER, key, 0, KEY_READ, &hKey);
  if (res != ERROR_SUCCESS)
    goto err_exit;
  res = RegQueryValueEx(hKey, option, NULL, NULL, (LPBYTE)value, (LPDWORD)&size);
  if (res != ERROR_SUCCESS)
    goto err_exit;
  res = ERROR_SUCCESS;

err_exit:
  if (hKey) RegCloseKey(hKey);
  return HRESULT_FROM_WIN32(res);
}


/*
  Returns heap string containing default application data path.
  Creates suffix subdirectory (only one level).
  E.g. C:\Documents and Settings\jfp\Application Data\suffix
*/
char *GetDefaultSaveDir(const char *suffix)
{
  char          prefix[MAX_PATH];
  char          path[2*MAX_PATH];
  const char   *key    = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders";
  const char   *option = "AppData";
  HRESULT hr = S_OK;

  if (SUCCEEDED(hr = ReadRegistry(key, option, prefix, sizeof(prefix))))
  {
    remove_slash(prefix);
    snprintf(path, sizeof(path), "%s/%s", prefix, suffix);
    _mkdir(path);
    return strdup(path);
  }
  return strdup("userdata");
}


/* Windows XP: User/App Data/TuxMath/ */
/* WIndows 98/ME: TuxMath install dir/userdata/Options */
#define OPTIONS_SUBDIR ""
#define OPTIONS_FILENAME "options.cfg"

#else

# define get_home getenv("HOME")
#define OPTIONS_SUBDIR "/.tuxmath"
#define OPTIONS_FILENAME "options"

#endif


/* This functions keep and returns the user data directory application path */
static char* user_data_dir = NULL;

char *get_user_data_dir ()
{ 
  if (! user_data_dir)
#ifdef BUILD_MINGW32
     user_data_dir = GetDefaultSaveDir(PROGRAM_NAME);
#else
     user_data_dir = strdup(getenv("HOME"));
#endif

  return user_data_dir;  
}

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
  strcpy(opt_path, get_user_data_dir());
  strcat(opt_path, OPTIONS_SUBDIR "/" OPTIONS_FILENAME);

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
  strcpy(opt_path, get_user_data_dir());
  strcat(opt_path, OPTIONS_SUBDIR "/");
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
  strcpy(opt_path, get_user_data_dir());
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
    //parameter = strndup(param_begin, (param_end - param_begin));

    /* Next three lines do same as strndup(), which may not be available: */
    parameter = malloc((sizeof(char) * (param_end - param_begin)) + 1);
    strncpy(parameter, param_begin, (param_end - param_begin));
    parameter[param_end - param_begin] = '\0';
 
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
        Opts_SetPerUserConfig(v);
    }

    if(0 == strcasecmp(parameter, "use_sound"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        Opts_SetUseSound(v);
    }

    else if(0 == strcasecmp(parameter, "fullscreen"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        Opts_SetFullscreen(v);
    }

    else if(0 == strcasecmp(parameter, "use_bkgd"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        Opts_SetUseBkgd(v);
    }

    else if(0 == strcasecmp(parameter, "demo_mode"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        Opts_SetDemoMode(v);
    }

    else if(0 == strcasecmp(parameter, "oper_override"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        Opts_SetOperOverride(v);
    }

    else if(0 == strcasecmp(parameter, "use_keypad"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        Opts_SetUseKeypad(v);
    }

    else if(0 == strcasecmp(parameter, "save_summary"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        Opts_SetSaveSummary(v);
    }

    else if(0 == strcasecmp(parameter, "speed"))
    {
      Opts_SetSpeed(atof(value));
    }

    else if(0 == strcasecmp(parameter, "use_feedback"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        Opts_SetUseFeedback(v);
    }

    else if(0 == strcasecmp(parameter, "danger_level"))
    {
      Opts_SetDangerLevel(atof(value));
    }

    else if(0 == strcasecmp(parameter, "danger_level_speedup"))
    {
      Opts_SetDangerLevelSpeedup(atof(value));
    }

    else if(0 == strcasecmp(parameter, "danger_level_max"))
    {
      Opts_SetDangerLevelMax(atof(value));
    }

    else if(0 == strcasecmp(parameter, "city_explode_handicap"))
    {
      Opts_SetCityExplHandicap(atof(value));
    }

    else if(0 == strcasecmp(parameter, "allow_speedup"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        Opts_SetAllowSpeedup(v);
    }

    else if(0 == strcasecmp(parameter, "speedup_factor"))
    {
      Opts_SetSpeedupFactor(atof(value));
    }

    else if(0 == strcasecmp(parameter, "max_speed"))
    {
      Opts_SetMaxSpeed(atof(value));
    }

    else if(0 == strcasecmp(parameter, "slow_after_wrong"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        Opts_SetSlowAfterWrong(v);
    }

    else if(0 == strcasecmp(parameter, "starting_comets"))
    {
      Opts_SetStartingComets(atoi(value));
    }

    else if(0 == strcasecmp(parameter, "extra_comets_per_wave"))
    {
      Opts_SetExtraCometsPerWave(atoi(value));
    }

    else if(0 == strcasecmp(parameter, "max_comets"))
    {
      Opts_SetMaxComets(atoi(value));
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
      MC_SetCopiesRepeatedWrongs(atoi(value));
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

    else if(0 == strcasecmp(parameter, "format_add_answer_last"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetFormatAddAnswerLast(v);
    }

    else if(0 == strcasecmp(parameter, "format_add_answer_first"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetFormatAddAnswerFirst(v);
    }

    else if(0 == strcasecmp(parameter, "format_add_answer_middle"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetFormatAddAnswerMiddle(v);
    }

    else if(0 == strcasecmp(parameter, "format_sub_answer_last"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetFormatSubAnswerLast(v);
    }

    else if(0 == strcasecmp(parameter, "format_sub_answer_first"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetFormatSubAnswerFirst(v);
    }

    else if(0 == strcasecmp(parameter, "format_sub_answer_middle"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetFormatSubAnswerMiddle(v);
    }

    else if(0 == strcasecmp(parameter, "format_mult_answer_last"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetFormatMultAnswerLast(v);
    }

    else if(0 == strcasecmp(parameter, "format_mult_answer_first"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetFormatMultAnswerFirst(v);
    }

    else if(0 == strcasecmp(parameter, "format_mult_answer_middle"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetFormatMultAnswerMiddle(v);
    }

    else if(0 == strcasecmp(parameter, "format_div_answer_last"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetFormatDivAnswerLast(v);
    }

    else if(0 == strcasecmp(parameter, "format_div_answer_first"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetFormatDivAnswerFirst(v);
    }

    else if(0 == strcasecmp(parameter, "format_div_answer_middle"))
    {
      int v = str_to_bool(value);
      if (v != -1)
        MC_SetFormatDivAnswerMiddle(v);
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

  if (!find_tuxmath_dir())
  {
    fprintf(stderr, "\nCould not find or create tuxmath dir\n");
    return 0;
  }

  /* find $HOME and add rest of path to config file: */
  strcpy(opt_path, get_user_data_dir());
  strcat(opt_path, OPTIONS_SUBDIR "/" OPTIONS_FILENAME);

  #ifdef TUXMATH_DEBUG
  printf("\nIn write_user_config_file() full path to config file is: = %s\n", opt_path);
  #endif

  /* save settings: */
  fp = fopen(opt_path, "w");
  if (fp)
  {
    write_config_file(fp, 1);
    fclose(fp);
    fp = NULL;
    return 1;
  }
  else
    return 0;
}



/* this function writes the settings for all game options to a */
/* human-readable file.                                        */
/* FIXME add 'verbose flag' */
int write_config_file(FILE *fp, int verbose)
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

  if (verbose)
  {
    fprintf(fp, 
          "############################################################\n"
          "#                                                          #\n"
          "#              Tuxmath Configuration File                  #\n"
          "#                                                          #\n"
          "# The behavior of Tuxmath can be controlled to a great     #\n"
          "# extent by editing this file with any and saving it in    #\n"
          "# the default options location ($HOME/.tuxmath/options).   #\n"
          "# The file consists of 'NAME = VALUE' pairs, one pair per  #\n"
          "# line. Each option is one of the following types:         #\n"
          "#                                                          #\n"
          "#     boolean: 1 (synonyms 'true', 'T', 'yes', 'Y', 'on')  #\n"
          "#              or                                          #\n"
          "#              0 (synonyms 'false, 'F', 'no', 'N', 'off')  #\n"
          "#     integer  (i.e. non-fractional numbers)               #\n"
          "#     float    (i.e decimal fractions)                     #\n"
          "#                                                          #\n"
          "# Lines beginning with '#' or ';' are ignored as comments. #\n"
          "# The synonyms for boolean '0' and '1' are accepted as     #\n"
          "# input, but always written as '0' or '1' when Tuxmath     #\n"
          "# writes a config file to disk.                            #\n"
          "# The file is organized with the more important options    #\n"
          "# first.                                                   #\n"
          "############################################################\n"
          "\n"
    );
  }

  if (verbose)
  {
    fprintf(fp, 
          "############################################################\n"
          "#                                                          #\n"
          "#                       Game Mode                          #\n"
          "#                                                          #\n"
          "# Parameter: play_through_list (Boolean)                   #\n"
          "# Default: 1                                               #\n"
          "#                                                          #\n"
          "# Tuxmath generates a list of math questions based on      #\n"
          "# parameters set below.  By default, (play_through_list =  #\n"
          "# 1) the questions are asked in a random order.            #\n"
          "# Correctly answered questions are removed from the list.  #\n"
          "# If the player fails to correctly answer a question       #\n"
          "# before it hits a city, the question will be reinserted   #\n"
          "# into the list in a random location.                      #\n"
          "# The player wins if all questions are answered correctly  #\n"
          "# before the cities are destroyed.                         #\n"
          "#                                                          #\n"
          "# Alternatively, Tuxmath can be played in 'Arcade Mode'    #\n"
          "# by setting play_through_list = 0 (i.e. 'false'). If this #\n"
          "# is done, all questions will be randomly reinserted into  #\n"
          "# the list whether or not they are answered correctly, and #\n"
          "# the game continues as long as there is a surviving city. #\n"
          "############################################################\n"
          "\n"
    );
  }
  fprintf (fp, "play_through_list = %d\n", MC_PlayThroughList());


  if (verbose)
  {
    fprintf (fp, "\n############################################################\n" 
                 "#                                                          #\n"
                 "#                 Speed and Number of Comets               #\n"
                 "#                                                          #\n"
                 "# Parameter: allow_speedup (boolean)                       #\n"
                 "# Default: 1                                               #\n"
                 "# Parameter: use_feedback  (boolean)                       #\n"
                 "# Default: 0                                               #\n"
                 "#                                                          #\n"
                 "# By default, the comets become faster and more numerous   #\n"
                 "# with each succeeding. The increase can be prevented      #\n"
                 "# by setting 'allow_speedup' to 0.                         #\n"
                 "#                                                          #\n"
                 "# If 'allow_speedup' is enabled, it is also possible to    #\n"
                 "# dynamically adjust the speed to the player's performance #\n"
                 "# by setting 'use_feedback' to 1.  This feature attempts   #\n"
                 "# to speed the game up if it is too easy for the player,   #\n"
                 "# and to slow it down if the player is having trouble.     #\n"
                 "#                                                          #\n"
                 "# Many additional parameters under 'Advanced Options' can  #\n"
                 "# be used to fine-tune these behaviors.                    #\n"
                 "############################################################\n\n");
  }

  fprintf(fp, "allow_speedup = %d\n", Opts_AllowSpeedup());
  fprintf(fp, "use_feedback = %d\n", Opts_UseFeedback());


  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "#                                                          #\n"
                 "#               Selecting Math Operations                  #\n"
                 "#                                                          #\n"
                 "# Parameter: addition_allowed (boolean)                    #\n"
                 "# Default: 1                                               #\n"
                 "# Parameter: subtraction_allowed (boolean)                 #\n"
                 "# Default: 1                                               #\n"
                 "# Parameter: multiplication_allowed (boolean)              #\n"
                 "# Default: 1                                               #\n"
                 "# Parameter: division_allowed (boolean)                    #\n"
                 "# Default: 1                                               #\n"
                 "#                                                          #\n"
                 "# These options enable questions for each of the four math #\n"
                 "# operations.  All are 1 (yes) by default.                 #\n"
                 "############################################################\n\n");
  }

  fprintf(fp, "addition_allowed = %d\n", MC_AddAllowed());
  fprintf(fp, "subtraction_allowed = %d\n", MC_SubAllowed());
  fprintf(fp, "multiplication_allowed = %d\n", MC_MultAllowed());
  fprintf(fp, "division_allowed = %d\n", MC_DivAllowed());

  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "#                                                          #\n"
                 "#                 Negative Number Support                  #\n"
                 "#                                                          #\n"
                 "# Parameter: allow_negatives (boolean)                     #\n"
                 "# Default: 0                                               #\n"
                 "#                                                          #\n"
                 "# 'allow_negatives' allows or disallows use of negative    #\n"
                 "# numbers as both operands and answers.  Default is 0      #\n"
                 "# (no), which disallows questions like:                    #\n"
                 "#          2 - 4 = ?                                       #\n"
                 "# Note: this option must be enabled in order to set the    #\n"
                 "# operand ranges to include negatives. If it is changed    #\n"
                 "# from 1 (yes) to 0 (no), any negative operand limits will #\n"
                 "# be reset to 0.                                           #\n"
                 "############################################################\n\n");
  }

  fprintf (fp, "allow_negatives = %d\n", MC_AllowNegatives());

  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "#                                                          #\n"
                 "#      Minimum and Maximum Values for Operand Ranges       #\n"
                 "#                                                          #\n"
                 "# Parameters: (multiple - all integer type)                #\n"
                 "#                                                          #\n"
                 "# Operand limits can be set to any integer up to the       #\n"
                 "# value of 'max_answer'. Tuxmath will generate questions   #\n"
                 "# for every value in the specified range. The maximum must #\n"
                 "# be greater than or equal to the corresponding minimum    #\n"
                 "# for any questions to be generated for that operation.    #\n"
                 "# Defaults are 0 for minima and 12 for maxima.             #\n"
                 "#                                                          #\n"
                 "# Note: 'allow_negatives' must be set to 1 for negative    #\n"
                 "# values to be accepted (see 'Advanced Options').          #\n"
                 "############################################################\n");
  }

  if (verbose)
  {
    fprintf(fp, "\n# Addition operands:\n"
              "# augend + addend = sum\n\n");
  }
  fprintf(fp, "min_augend = %d\n", MC_AddMinAugend());
  fprintf(fp, "max_augend = %d\n", MC_AddMaxAugend());
  fprintf(fp, "min_addend = %d\n", MC_AddMinAddend());
  fprintf(fp, "max_addend = %d\n", MC_AddMaxAddend());

  if (verbose)
  {
    fprintf(fp, "\n# Subtraction operands:"
              "\n# minuend - subtrahend = difference\n\n");
  }
  fprintf(fp, "min_minuend = %d\n", MC_SubMinMinuend());
  fprintf(fp, "max_minuend = %d\n", MC_SubMaxMinuend());
  fprintf(fp, "min_subtrahend = %d\n", MC_SubMinSubtrahend());
  fprintf(fp, "max_subtrahend = %d\n", MC_SubMaxSubtrahend());

  if (verbose)
  {
    fprintf(fp, "\n# Multiplication operands:"
              "\n# multiplier * multiplicand = product\n\n");
  }
  fprintf(fp, "min_multiplier = %d\n", MC_MultMinMultiplier());
  fprintf(fp, "max_multiplier = %d\n", MC_MultMaxMultiplier());
  fprintf(fp, "min_multiplicand = %d\n", MC_MultMinMultiplicand());
  fprintf(fp, "max_multiplicand = %d\n", MC_MultMaxMultiplicand());

  if (verbose)
  {
    fprintf(fp, "\n# Division operands:"
              "\n# dividend/divisor = quotient\n\n");
  }
  fprintf(fp, "min_divisor = %d\n",MC_DivMinDivisor());
  fprintf(fp, "max_divisor = %d\n", MC_DivMaxDivisor());
  fprintf(fp, "min_quotient = %d\n", MC_DivMinQuotient());
  fprintf(fp, "max_quotient = %d\n", MC_DivMaxQuotient());


  if (verbose)
  {
    fprintf (fp, "\n\n############################################################\n" 
                 "#                                                          #\n"
                 "#                 General Game Options                     #\n"
                 "#                                                          #\n"
                 "# Parameter: use_sound (boolean)                           #\n"
                 "# Default: 1                                               #\n"
                 "# Parameter: fullscreen (boolean)                          #\n"
                 "# Default: 1                                               #\n"
                 "# Parameter: demo_mode (boolean)                           #\n"
                 "# Default: 0                                               #\n"
                 "# Parameter: use_keypad (boolean)                          #\n"
                 "# Default: 0                                               #\n"
                 "# Parameter: save_game_summary (boolean)                   #\n"
                 "# Default: 1                                               #\n"
                 "#                                                          #\n"
                 "# These parameters control various aspects of Tuxmath's    #\n"
                 "# not directly related to the math question to be asked.   #\n"
                 "############################################################\n");

  }
  if (verbose)
  {
    fprintf (fp, "\n# Use game sounds and background music if possible:\n");
  }
  fprintf(fp, "use_sound = %d\n", Opts_UseSound());

  if (verbose)
  {
    fprintf (fp, "\n# Use fullscreen at 640x480 resolution instead of\n"
                 "# 640x480 window. Change to 0 if SDL has trouble with\n"
                 "# fullscreen on your system:\n");
  }
  fprintf(fp, "fullscreen = %d\n", Opts_Fullscreen());

  if (verbose)
  {
    fprintf (fp, "\n# Display jpg images for background:\n");
  }
  fprintf(fp, "use_bkgd = %d\n", Opts_UseBkgd());

  if (verbose)
  {
    fprintf (fp, "\n# Run Tuxmath as demo (i.e. without user input):\n");
  }
  fprintf(fp, "demo_mode = %d\n", Opts_DemoMode());

  if (verbose)
  {
    fprintf (fp, "\n# Display onscreen numeric keypad - allows mouse-only\n"
               "# gameplay or use with touchscreens:\n");
  }
  fprintf(fp, "use_keypad = %d\n", Opts_UseKeypad());

  if (verbose)
  {
    fprintf (fp, "\n# By default, Tuxmath saves summaries of the last\n"
               "# ten games in the user's .tuxmath directory. Set\n"
               "# this parameter to '0' to turn off.\n");
  }
  fprintf(fp, "save_summary = %d\n", Opts_SaveSummary());

  if (verbose)
  {
    fprintf (fp, "\n\n\n############################################################\n" 
                 "#                                                          #\n"
                 "#                   Advanced Options                       #\n"
                 "#                                                          #\n"
                 "# The remaining settings further customize Tuxmath's       #\n"
                 "# behavior.  Most users will probably not change them.     #\n"
                 "############################################################\n\n");
  }

  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "#                                                          #\n"
                 "#           Advanced Math Question List Options            #\n"
                 "#                                                          #\n"
                 "# Parameter: question_copies (integer)                     #\n"
                 "# Default: 1                                               #\n"
                 "# Parameter: repeat_wrongs (boolean)                       #\n"
                 "# Default: 1                                               #\n"
                 "# Parameter: copies_repeated_wrongs (integer)              #\n"
                 "# Default: 1                                               #\n"
                 "#                                                          #\n"
                 "# These settings offer further control over the question   #\n"
                 "# list and are generally only useful if 'play_through_list'#\n"
                 "# is enabled (as it is by default).                        #\n"
                 "#                                                          #\n"
                 "# 'question_copies' is the number of times each question   #\n"
                 "# is put into the initial list. It can be 1 to 10.         #\n"
                 "#                                                          #\n"
                 "# 'repeat_wrongs' determines whether questions the player  #\n"
                 "# failed to answer correctly will be asked again.          #\n"
                 "#                                                          #\n"
                 "# 'copies_repeated_wrongs' gives the number of times a     #\n"
                 "# missed question will reappear. This can be set anywhere  #\n"
                 "# from 1 to 10.                                            #\n"
                 "#                                                          #\n"
                 "# The defaults for these values result in a 'mission'      #\n" 
                 "# for Tux that is accomplished by answering all            #\n"
                 "# questions correctly with at least one surviving city.    #\n"
                 "############################################################\n\n");
  }

  fprintf (fp, "question_copies = %d\n", MC_QuestionCopies());
  fprintf (fp, "repeat_wrongs = %d\n", MC_RepeatWrongs());
  fprintf (fp, "copies_repeated_wrongs = %d\n", MC_CopiesRepeatedWrongs());



  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "#                                                          #\n"
                 "#                 Math Question Formats                    #\n"
                 "#                                                          #\n"
                 "# The 'format_<op>_answer_<place>  options control         #\n"
                 "# generation of questions with the answer in different     #\n"
                 "# places in the equation.  i.e.:                           #\n"
                 "#                                                          #\n"
                 "#    format_add_answer_last:    2 + 2 = ?                  #\n"
                 "#    format_add_answer_first:   ? + 2 = 4                  #\n"
                 "#    format_add_answer_middle:  2 + ? = 4                  #\n"
                 "#                                                          #\n"
                 "# By default, 'format_answer_first' is enabled and the     #\n"
                 "# other two formats are disabled.  Note that the options   #\n"
                 "# are not mutually exclusive - the question list may       #\n"
                 "# contain questions with different formats.                #\n"
                 "#                                                          #\n"
                 "# The formats are set independently for each of the four   #\n"
                 "# math operations. All parameters are type 'boolean'.      #\n"
                 "############################################################\n\n");
  }

  fprintf (fp, "format_add_answer_last = %d\n", MC_FormatAddAnswerLast());
  fprintf (fp, "format_add_answer_first = %d\n", MC_FormatAddAnswerFirst());
  fprintf (fp, "format_add_answer_middle = %d\n", MC_FormatAddAnswerMiddle());
  fprintf (fp, "format_sub_answer_last = %d\n", MC_FormatSubAnswerLast());
  fprintf (fp, "format_sub_answer_first = %d\n", MC_FormatSubAnswerFirst());
  fprintf (fp, "format_sub_answer_middle = %d\n", MC_FormatSubAnswerMiddle());
  fprintf (fp, "format_mult_answer_last = %d\n", MC_FormatMultAnswerLast());
  fprintf (fp, "format_mult_answer_first = %d\n", MC_FormatMultAnswerFirst());
  fprintf (fp, "format_mult_answer_middle = %d\n", MC_FormatMultAnswerMiddle());
  fprintf (fp, "format_div_answer_last = %d\n", MC_FormatDivAnswerLast());
  fprintf (fp, "format_div_answer_first = %d\n", MC_FormatDivAnswerFirst());
  fprintf (fp, "format_div_answer_middle = %d\n", MC_FormatDivAnswerMiddle());


  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "#                                                          #\n"
                 "# Parameter: max_answer (integer)                          #\n"
                 "# Default: 999                                             #\n"
                 "#                                                          #\n"
                 "# 'max_answer' is the largest absolute value allowed in    #\n"
                 "# any value in a question (not only the answer). Default   #\n"
                 "# is 999, which is as high as it can be set. It can be set #\n"
                 "# lower to fine-tune the list for certain 'lessons'.       #\n"
                 "############################################################\n\n");
  }
  fprintf (fp, "max_answer = %d\n", MC_MaxAnswer());

  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "#                                                          #\n"
                 "# Parameter: max_questions (integer)                       #\n"
                 "# Default: 5000                                            #\n"
                 "#                                                          #\n"
                 "# 'max_questions' is limit of the length of the question   #\n"
                 "# list. Default is 5000 - only severe taskmasters will     #\n"
                 "# need to raise it!                                        #\n"
                 "############################################################\n\n");
  }
  fprintf (fp, "max_questions = %d\n", MC_MaxQuestions());  

  if (verbose)
  {
    fprintf (fp, "\n############################################################\n"
                 "#                                                          #\n"
                 "# Parameter: randomize (boolean)                           #\n"
                 "# Default: 1                                               #\n"
                 "#                                                          #\n"
                 "# If 'randomize' selected, the list will be shuffled       #\n"
                 "# at the start of the game. Otherwise, the questions       #\n"
                 "# appear in the order the program generates them.          #\n"
                 "############################################################\n\n");
  }
  fprintf (fp, "randomize = %d\n", MC_Randomize());


  if (verbose)
  {
    fprintf (fp, "\n############################################################\n" 
                 "#                                                          #\n"
                 "#                Advanced Comet Speed Options              #\n"
                 "#                                                          #\n"
                 "# Parameter: starting_comets (integer)                     #\n"
                 "# Default: 2                                               #\n"
                 "# Parameter: extra_comets_per_wave (integer)               #\n"
                 "# Default: 2                                               #\n"
                 "# Parameter: max_comets (integer)                          #\n"
                 "# Default: 10                                              #\n"
                 "# Parameter: speed (float)                                 #\n"
                 "# Default: 1.00                                            #\n"
                 "# Parameter: max_speed (float)                             #\n"
                 "# Default: 10.00                                           #\n"
                 "# Parameter: speedup_factor (float)                        #\n"
                 "# Default: 1.20                                            #\n"
                 "# Parameter: slow_after_wrong (bool)                       #\n"
                 "# Default: 0                                               #\n"
                 "#                                                          #\n"
                 "# (for 'feedback' speed control system):                   #\n"
                 "# Parameter: danger_level (float)                          #\n"
                 "# Default: 0.35                                            #\n"
                 "# Parameter: danger_level_speedup (float)                  #\n"
                 "# Default: 1.1                                             #\n"
                 "# Parameter: danger_level_max (float)                      #\n"
                 "# Default: 0.9                                             #\n"
                 "# Parameter: city_explode_handicap (float)                 #\n"
                 "# Default: 0                                               #\n"
                 "#                                                          #\n"
                 "# The comet number parameters and initial/max speed apply  #\n"
                 "# whether or not the feedback system is activated.         #\n"
                 "#                                                          #\n"
                 "# 'speedup_factor' and 'slow_after_wrong' only apply if    #\n"
                 "# feedback is not activated.                               #\n"
                 "#                                                          #\n"
                 "# The 'danger_level_*' and 'city_explode_handicap'         #\n"
                 "# parameters are only used if feedback is activated.       #\n"
                 "############################################################\n\n");
  }

  if(verbose)
  {
    fprintf (fp, "\n# Number of comets for first wave. Default is 2.\n");
  }
  fprintf(fp, "starting_comets = %d\n", Opts_StartingComets());

  if(verbose)
  {
    fprintf (fp, "\n# Comets to add for each successive wave. Default is 2.\n");
  }
  fprintf(fp, "extra_comets_per_wave = %d\n", Opts_ExtraCometsPerWave());

  if(verbose)
  {
    fprintf (fp, "\n# Maximum number of comets. Default is 10.\n");
  }
  fprintf(fp, "max_comets = %d\n", Opts_MaxComets());

  if(verbose)
  {
    fprintf (fp, "\n# Starting comet speed. Default is 1.\n");
  }
  fprintf(fp, "speed = %.2f\n", Opts_Speed());

  if(verbose)
  {
    fprintf (fp, "\n# Maximum speed. Default is 10.\n");
  }
  fprintf(fp, "max_speed = %.2f\n", Opts_MaxSpeed());

  if(verbose)
  {
    fprintf (fp, "\n# 'speedup_factor': If feedback is not used but \n"
                 "# 'allow_speedup' is enabled, the comet speed will be\n"
                 "# multiplied by this factor with each new wave.\n"
                 "# Values from 0.5 to 2 are accepted (note that a \n"
                 "# value less than 1 causes the comets to be \n"
                 "# slower with each wave!).\n"
                 "# Default is 1.2 (i.e. 20 percent increase per wave)\n\n");
  }
  fprintf(fp, "speedup_factor = %.2f\n", Opts_SpeedupFactor());

  if(verbose)
  {
    fprintf (fp, "\n# 'slow_after_wrong' tells Tuxmath to go back to  \n"
                 "# starting speed and number of comets if the player misses \n"
                 "# a question. Useful for smaller kids. Default is 0.\n\n");
  }

  fprintf(fp, "slow_after_wrong = %d\n", Opts_SlowAfterWrong());


  if(verbose)
  {
     fprintf (fp, "\n# (Feedback) Set the desired danger level.\n"
             "# 0 = too safe, comets typically exploded at the very top\n"
             "# 1 = too dangerous, comets typically exploded as they\n"
             "# hit cities. Set it somewhere between these extremes. As\n"
             "# a guideline, early elementary kids might prefer\n"
             "# 0.2-0.3, older kids at around 0.4-0.6. Default 0.35.\n\n");
  }
  fprintf(fp, "danger_level = %.2f\n", Opts_DangerLevel());

  if(verbose)
  {
     fprintf (fp, "\n# (Feedback) Set danger level speedup.\n"
                  "# The margin of safety will decrease by this factor each\n"
                  "# wave. Default 1.1. Note 1 = no increase in danger level.\n\n");
  }
  fprintf(fp, "danger_level_speedup = %.2f\n", Opts_DangerLevelSpeedup());

  if(verbose)
  {
     fprintf (fp, "\n# (Feedback) Set the maximum danger level.\n"
                  "# Default 0.9.\n");
  }
  fprintf(fp, "danger_level_max = %.2f\n", Opts_DangerLevelMax());

  if (verbose)
  { 
     fprintf (fp, "\n# (Feedback) Set the handicap for hitting cities.\n"
                  "# When bigger than 0, this causes the game to slow down\n"
                  "# by an extra amount after a wave in which one or more\n"
                  "# cities get hit. Note that this is similar to\n"
                  "# 'slow_after_wrong', but allows for more gradual\n"
                  "# changes. Default 0 (no extra handicap).\n\n");
  }
  fprintf(fp, "city_explode_handicap = %.2f\n", Opts_CityExplHandicap());

  if(verbose)
  {
    fprintf (fp, "\n\n############################################################\n" 
                 "#                                                          #\n"
                 "#                 Restricting User Settings                #\n"
                 "#                                                          #\n"
                 "# Parameter: per_user_config (boolean)                     #\n"
                 "# Default: 1                                               #\n"
                 "#                                                          #\n"
                 "# 'per_user_config' determines whether Tuxmath will look   #\n"
                 "# in the user's home directory for settings. Default is 1  #\n"
                 "# (yes). If set to 0, the program will ignore the user's   #\n"
                 "# .tuxmath file and use the the global settings in the     #\n"
                 "# installation-wide config file.                           #\n"
                 "#                                                          #\n"
                 "# This setting cannot be changed by an ordinary user, i.e. #\n"
                 "# it is ignored unless the config file is Tuxmath's global #\n"
                 "# config file. Thus, users cannot 'lock themselves out'    #\n"
                 "# by accidentally setting this to 0.                       #\n"
                 "############################################################\n\n");
  }
  fprintf(fp, "per_user_config = %d\n", Opts_PerUserConfig());


  /* print general game options (passing '1' as second arg causes */
  /* "help" info for each option to be written to file as comments) */
//  print_game_options(fp, 1);
  /* print options pertaining to math questions from MathCards: */
//  MC_PrintMathOptions(fp, 1);

  #ifdef TUXMATH_DEBUG
  printf("Leaving write_config_file()\n");
  #endif

  return 1;
}


/* write_pregame_summary() and write_postgame_summary() are used to */
/* record data about the player's game to file for review (perhaps by */
/* teacher). write_pregame_summary() is called at the start of each  */
/* game and records the question list along with identifying data. It */
/* also rotates old game summaries to successive filenames, keeping   */
/* the last ten summaries for review. write_postgame_summary()       */
/* the list of questions that were not answered correctly and         */
/* calculates the percent correct.                                    */
int write_pregame_summary(void)
{
  int i;
  FILE* fp;
  char filepath1[PATH_MAX];
  char filepath2[PATH_MAX];

  /* Make sure tuxmath dir exists or can be created: */
  if (!find_tuxmath_dir())
  {
    fprintf(stderr, "\nCould not find or create tuxmath dir\n");
    return 0;
  }



  /* Rotate filenames of old summaries, oldest summary if present */
  /* and leaving summary1 available for current game:             */

  /* find $HOME and tack on file name: */
  strcpy(filepath1, get_user_data_dir());
  strcat(filepath1, OPTIONS_SUBDIR "/");
  strcat(filepath1, summary_filenames[NUM_SUMMARIES - 1]);

  fp = fopen(filepath1, "r");
  if (fp)
  {
    #ifdef TUXMATH_DEBUG
    printf("\nIn write_pregame_summary() - removing oldest summary file\n");
    #endif

    fclose(fp);
    remove(filepath1);
  }

  /* Now shift each old file back by one:       */
  /* 'filepath1' is the old name for each file, */
  /* 'filepath2' is the new name (i.e. we go from i - 1 to i). */
  for (i = NUM_SUMMARIES - 1; i > 0; i--)
  {
    /* old filename: */
    strcpy(filepath1, get_user_data_dir());
    strcat(filepath1, OPTIONS_SUBDIR "/");
    strcat(filepath1, summary_filenames[i - 1]);
    /* new filename: */
    strcpy(filepath2, get_user_data_dir());
    strcat(filepath2, OPTIONS_SUBDIR "/");
    strcat(filepath2, summary_filenames[i]);
    /* now change the name: */
    rename(filepath1, filepath2);
  } 

  /* summary_filenames[0] (i.e. 'summary1') should now be vacant:     */
  strcpy(filepath1, get_user_data_dir());
  strcat(filepath1, OPTIONS_SUBDIR "/");
  strcat(filepath1, summary_filenames[0]);

  fp = fopen(filepath1, "w"); /* "w" means start writing with empty file */
  if (fp)
  {
    /* Write header and identifying data for summary file:       */
    fprintf(fp, "************************\n"
                "* Tuxmath Game Summary *\n"
                "************************\n");
    fprintf(fp, "\nPlayer: %s\n", getenv("USER"));

    /* Write question list:  */
    fprintf(fp, "\nStarting Question List:");
    MC_PrintQuestionList(fp);
    fprintf(fp, "\n\nNumber of Questions: %d", MC_StartingListLength());

    fclose(fp);
    return 1;
  }
  else /* Couldn't write file for some reason: */
  {
    return 0;
  }
}

int write_postgame_summary(void)
{
  FILE* fp;
  char filepath1[PATH_MAX];
  int total_answered;

  strcpy(filepath1, get_user_data_dir());
  strcat(filepath1, OPTIONS_SUBDIR "/");
  strcat(filepath1, summary_filenames[0]);

  fp = fopen(filepath1, "a"); /* "a" means append to end of file */
  if (fp)
  {
    /* Write list of questions missed: */
    fprintf(fp, "\n\n\nList Of Questions Not Answered Correctly:");
                MC_PrintWrongList(fp);
    fprintf(fp, "\n\nNumber Of Distinct Questions Not Answered Correctly: %d",
                MC_WrongListLength());

    /* Write post-game statistics:     */
    total_answered = MC_NumAnsweredCorrectly() + MC_NumNotAnsweredCorrectly();

    fprintf(fp, "\n\nSummary:\n");
    fprintf(fp, "Questions Answered:\t%d\n", total_answered);
    fprintf(fp, "Questions Correct:\t%d\n",
                MC_NumAnsweredCorrectly());
    fprintf(fp, "Questions Missed:\t%d\n",
                MC_NumNotAnsweredCorrectly());
    /* Avoid divide-by-zero errror: */
    if (total_answered)
    {
      fprintf(fp, "Percent Correct:\t%d %%\n", 
              ((MC_NumAnsweredCorrectly() * 100)/ total_answered) );
    }
    else
      fprintf(fp, "Percent Correct: (not applicable)\n");

    fprintf(fp, "Mission Accomplished:\t");
    if (MC_MissionAccomplished())
    {
      fprintf(fp, "Yes!\n\n8^)\n");
    }
    else
    {
      fprintf(fp, "No.\n\n:^(\n");
    }
    return 1;
  }
  else /* Couldn't write file for some reason: */
  {
    return 0;
  }
}



/* Checks to see if user's .tuxmath directory exists and, if not, tries  */
/* to create it. Returns 1 if .tuxmath dir found or successfully created */
static int find_tuxmath_dir(void)
{
  char opt_path[PATH_MAX];
  DIR* dir_ptr;

  /* find $HOME */
  strcpy(opt_path, get_user_data_dir());

  #ifdef TUXMATH_DEBUG
  printf("\nIn find_tuxmath_dir() home directory is: = %s\n", opt_path);
  #endif

  /* add rest of path to user's tuxmath dir: */
  strcat(opt_path, OPTIONS_SUBDIR);

  #ifdef TUXMATH_DEBUG
  printf("\nIn find_tuxmath_dir() tuxmath dir is: = %s\n", opt_path);
  #endif

  /* find out if directory exists - if not, create it: */
  dir_ptr = opendir(opt_path);
  if (dir_ptr)  /* don't leave DIR* open if it was already there */
  {
    #ifdef TUXMATH_DEBUG
    printf("\nIn find_tuxmath_dir() tuxmath dir opened OK\n");
    #endif

    closedir(dir_ptr);
    return 1;
  }
  else /* need to create tuxmath config directory: */
  {
    FILE* fp;
    int status;

    /* if user's home has a _file_ named .tuxmath (as from previous version */
    /* of program), need to get rid of it or directory creation will fail:  */
    fp = fopen(opt_path, "r");
    if (fp)
    {
      #ifdef TUXMATH_DEBUG
      printf("\nIn find_tuxmath_dir() - removing old .tuxmath file\n");
      #endif

      fclose(fp);
      remove(opt_path);
    }

    #ifdef TUXMATH_DEBUG
    printf("\nIn find_tuxmath_dir() - trying to create .tuxmath dir\n");
    #endif

    //status = mkdir(opt_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

#ifndef BUILD_MINGW32
    status = mkdir(opt_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#else
    status = mkdir(opt_path);
#endif

    #ifdef TUXMATH_DEBUG
    printf("\nIn find_tuxmath_dir() - mkdir returned: %d\n", status);
    #endif

    /* mkdir () returns 0 if successful */
    if (0 == status)
    {
      fprintf(stderr, "\nfind_tuxmath_dir() - $HOME" OPTIONS_SUBDIR " created\n");
      return 1;
    }
    else
    {
      fprintf(stderr, "\nfind_tuxmath_dir() - mkdir failed\n");
      return 0;
    }
  }
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
    ||(0 == strcasecmp(val, "y"))
    ||(0 == strcasecmp(val, "on")))
  {
    return 1;
  }

  if ((0 == strcasecmp(val, "false"))
    ||(0 == strcasecmp(val, "f"))
    ||(0 == strcasecmp(val, "no"))
    ||(0 == strcasecmp(val, "n"))
    ||(0 == strcasecmp(val, "off")))
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

  if (Opts_UsingSound())
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
  if (Opts_UsingSound())
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
