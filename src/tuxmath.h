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

#include <SDL.h>

#ifndef NOSOUND
#include <SDL_mixer.h>
#endif

//#define NOSOUND
#define TUXMATH_DEBUG   /* for conditional compilation of debugging output */
//#define FEEDBACK_DEBUG  /* for Tim's feedback speed control code           */

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

/* These values are hard-coded and used 'as is' by the program */
/* (i.e. these behaviors require recompilation to change)   */ 
#define DEFAULT_NUM_CITIES 4   /* MUST BE AN EVEN NUMBER! */
#define DEFAULT_NUM_BKGDS 6
#define DEFAULT_MAX_CITY_COLORS 4

#define MINIMUM_SPEED 0.3
#define MAX_MAX_SPEED 20.0
#define MIN_SPEEDUP_FACTOR 0.5
#define MAX_SPEEDUP_FACTOR 2.0
#define MAX_BONUS_SPEED_RATIO 3.0
#define MIN_COMETS 1
#define MAX_MAX_COMETS 100


/* Going away soon: */
typedef struct range_type {
  int min;
  int max;
} range_type;

/* Going away soon: */
enum {
  OPER_ADD,
  OPER_SUB,
  OPER_MULT,
  OPER_DIV,
  NUM_OPERS
};

/* Going away soon: */
enum {
  Q_RANGE_1_5,
  Q_RANGE_6_12,
  Q_RANGE_13_20,
  NUM_Q_RANGES
};


/* Global data gets 'externed' here: */

extern SDL_Surface* screen; /* declared in setup.c; also used in game.c, options.c, fileops.c, credits.c, title.c */
extern SDL_Surface* images[];    /* declared in setup.c, used in same files as screen */
extern SDL_Surface* flipped_images[];
extern int flipped_img_lookup[];
#ifndef NOSOUND
extern Mix_Chunk* sounds[];    /* declared in setup.c; also used in fileops.c, playsound.c */
extern Mix_Music* musics[];    /* declared in setup.c; also used in fileops.c, game.c  */
#endif

extern char operchars[NUM_OPERS];
extern char* oper_opts[NUM_OPERS];
extern char* oper_alt_opts[NUM_OPERS];
extern range_type ranges[NUM_Q_RANGES];
extern int opers[NUM_OPERS], range_enabled[NUM_Q_RANGES];

/* NOTE: default values for math options are now in mathcards.h */

#endif
