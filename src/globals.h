/*
  globals.h

  For TuxMath

  Contains global data for configuration of math questions and for
  general game options, as well as constants and defaults.  Nothing
  depending on SDL should be in here; put any SDL-related items into
  tuxmath.h.

  Author: David Bruce <davidstuartbruce@gmail.com>, (C) 2006


  Part of "Tux4Kids" Project
  http://www.tux4kids.org/

  Added March 2, 2006

  Copyright: See COPYING file that comes with this distribution
  (briefly - GNU GPL v2 or later)
*/



#ifndef GLOBALS_H
#define GLOBALS_H

//#include "config.h"
#include "config.h"

// Translation stuff (now works for Mac and Win too!): 
#include "gettext.h"
#include <locale.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#include <wchar.h>

#ifdef HAVE_LIBT4K_COMMON
# include <t4k_common.h>
#else
typedef enum { false, true } bool;
#endif

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

/* for Tim's feedback speed control code           */
//#define FEEDBACK_DEBUG
//#define LINEBREAK

/* debug data (declared in options.c) */
extern int debug_status;

/* bitmasks for debugging options (declared in options.c) */
extern const int debug_setup;
extern const int debug_fileops;
extern const int debug_loaders;
extern const int debug_titlescreen;
extern const int debug_menu;
extern const int debug_menu_parser;
extern const int debug_game;
extern const int debug_factoroids;
extern const int debug_lan;
extern const int debug_mathcards;
extern const int debug_sdl;
extern const int debug_lessons;
extern const int debug_highscore;
extern const int debug_options;
extern const int debug_text_and_intl;
extern const int debug_multiplayer;
extern const int debug_all;

/* debug macros */
#define DEBUGCODE(mask) if((mask) & debug_status)
#define DEBUGMSG(mask, ...) if((mask) & debug_status){ fprintf(stderr, __VA_ARGS__); fflush(stderr); }

/* Maximum length of file path: */
#define PATH_MAX 4096

/* Error code if game_options not valid: */
#define GAME_OPTS_INVALID 9999

/* Default values for game_options */
/* They can be changed in the struct to other values at run-time */
#define DEFAULT_PER_USER_CONFIG 1
#define DEFAULT_USE_SOUND 1
#define DEFAULT_MENU_SOUND 1
#define DEFAULT_MENU_MUSIC 1
#define DEFAULT_FULLSCREEN 1
#define DEFAULT_LAN_MODE 0
#define DEFAULT_USE_BKGD 1
#define DEFAULT_HELP_MODE 0
#define DEFAULT_DEMO_MODE 0
#define DEFAULT_OPER_OVERRIDE 0
#define DEFAULT_USE_KEYPAD 0
#define DEFAULT_ALLOW_PAUSE 1
#define DEFAULT_BONUS_SPEED_RATIO 1.5
#define DEFAULT_BONUS_COMET_INTERVAL 10
#define DEFAULT_SPEED 1
#define DEFAULT_ALLOW_SPEEDUP 1
#define DEFAULT_SPEEDUP_FACTOR 1.2
#define DEFAULT_MAX_SPEED 10
#define DEFAULT_SLOW_AFTER_WRONG 0
#define DEFAULT_STARTING_COMETS 2
#define DEFAULT_EXTRA_COMETS_PER_WAVE 2
#define DEFAULT_MAX_COMETS 10
#define DEFAULT_SAVE_SUMMARY 1        
#define DEFAULT_SOUND_HW_AVAILABLE 1
#define DEFAULT_USE_IGLOOS 1
#define DEFAULT_USE_FEEDBACK 0
#define DEFAULT_DANGER_LEVEL 0.35
#define DEFAULT_DANGER_LEVEL_SPEEDUP 1.1
#define DEFAULT_DANGER_LEVEL_MAX 0.9
#define DEFAULT_CITY_EXPL_HANDICAP 0
#define DEFAULT_LAST_SCORE 0

/* These values are hard-coded and used 'as is' by the program */
/* (i.e. these behaviors require recompilation to change)   */ 
#define DEFAULT_NUM_CITIES 4   /* MUST BE AN EVEN NUMBER! */
#define DEFAULT_MAX_CITY_COLORS 4

#define MINIMUM_SPEED 0.8
#define MAX_MAX_SPEED 20.0
#define MIN_SPEEDUP_FACTOR 1.0
#define MAX_SPEEDUP_FACTOR 2.0
#define MAX_BONUS_SPEED_RATIO 3.0
#define MIN_COMETS 1
#define MAX_MAX_COMETS 100
#define SCORE_COEFFICIENT 100

#define DEFAULT_FONT_NAME "AndikaDesRevG.ttf"
#define FONT_NAME_LENGTH 64
#define DEFAULT_MENU_FONT_SIZE 18
#define DEFAULT_HELP_FONT_SIZE 32


#define HIGH_SCORES_SAVED 10
#define HIGH_SCORE_NAME_LENGTH 32

#define REG_RGBA 16,16,96,96
#define SEL_RGBA 16,16,128,128

#define PIXEL_BITS 32

enum {
  CADET_HIGH_SCORE,
  SCOUT_HIGH_SCORE,
  RANGER_HIGH_SCORE,
  ACE_HIGH_SCORE,
  COMMANDO_HIGH_SCORE,
  NUM_MATH_COMMAND_LEVELS
};

enum {
  FACTORS_HIGH_SCORE = NUM_MATH_COMMAND_LEVELS,
  FRACTIONS_HIGH_SCORE,
  NUM_HIGH_SCORE_LEVELS
};



#define NAME_BUF_SIZE 200

/* data for 'Training Academy' lessons: */
extern char **lesson_list_titles;
extern char **lesson_list_filenames;
extern int* lesson_list_goldstars;
extern int num_lessons;

#endif
