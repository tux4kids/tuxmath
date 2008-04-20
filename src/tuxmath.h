/*
  tuxmath.h

  For TuxMath
  Contains global data for configuration of math questions and
  for general game options, as well as constants and defaults.

  Author: David Bruce <dbruce@tampabay.rr.com>, (C) 2006


  Part of "Tux4Kids" Project
  http://www.tux4kids.org/
      
  Added March 2, 2006

  Copyright: See COPYING file that comes with this distribution
  (briefly - GNU GPL v2 or later)
*/



#ifndef TUXMATH_H
#define TUXMATH_H

#include "config.h"

// Translation stuff (now works for Mac and Win too!): 
#include "gettext.h"
#include <locale.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#include <wchar.h>

#include "SDL.h"
#include "SDL_ttf.h"

#ifndef NOSOUND
#include "SDL_mixer.h"
#endif

//#define NOSOUND
/* for conditional compilation of debugging output */
//#define TUXMATH_DEBUG
/* for Tim's feedback speed control code           */
//#define FEEDBACK_DEBUG

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
#define DEFAULT_NUM_BKGDS 6
#define DEFAULT_MAX_CITY_COLORS 4

#define MINIMUM_SPEED 0.8
#define MAX_MAX_SPEED 20.0
#define MIN_SPEEDUP_FACTOR 1
#define MAX_SPEEDUP_FACTOR 2.0
#define MAX_BONUS_SPEED_RATIO 3.0
#define MIN_COMETS 1
#define MAX_MAX_COMETS 100

#define DEFAULT_FONT_NAME "AndikaDesRevG.ttf"
#define DEFAULT_MENU_FONT_SIZE 18
#define DEFAULT_HELP_FONT_SIZE 32


#define HIGH_SCORES_SAVED 10
#define HIGH_SCORE_NAME_LENGTH 32

#define REG_RGBA 16,16,96,96
#define SEL_RGBA 16,16,128,128

#define RES_X	640
#define RES_Y	480
#define PIXEL_BITS 32	

enum { 
  CADET_HIGH_SCORE,
  SCOUT_HIGH_SCORE,
  RANGER_HIGH_SCORE,
  ACE_HIGH_SCORE,
  NUM_HIGH_SCORE_LEVELS
};



/* Global data gets 'externed' here: */
extern SDL_Color black;
extern SDL_Color gray;
extern SDL_Color dark_blue;
extern SDL_Color red;
extern SDL_Color white;
extern SDL_Color yellow;

extern SDL_Surface* screen; /* declared in setup.c; also used in game.c, options.c, fileops.c, credits.c, titlescreen.c */
extern SDL_Surface* images[];    /* declared in setup.c, used in same files as screen */
extern SDL_Surface* flipped_images[];
#define NUM_BLENDED_IGLOOS 15
extern SDL_Surface* blended_igloos[];
extern int flipped_img_lookup[];

extern TTF_Font  *default_font;
extern TTF_Font  *help_font;


#ifndef NOSOUND
extern Mix_Chunk* sounds[];    /* declared in setup.c; also used in fileops.c, playsound.c */
extern Mix_Music* musics[];    /* declared in setup.c; also used in fileops.c, game.c  */
#endif



#define NAME_BUF_SIZE 200

/* data for 'Training Academy' lessons: */
extern unsigned char **lesson_list_titles;
extern unsigned char **lesson_list_filenames;
extern int* lesson_list_goldstars;
extern int num_lessons;

/* NOTE: default values for math options are now in mathcards.h */

#endif
