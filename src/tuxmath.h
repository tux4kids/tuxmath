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
//#define TUXMATH_DEBUG   /* for conditional compilation of debugging output */
//#define FEEDBACK_DEBUG  /* for Tim's feedback speed control code           */

#define TUXMATH_VERSION 0.97

#define PATH_MAX 4096

/* default values for game_options */
#define DEFAULT_PER_USER_CONFIG 1
#define DEFAULT_USE_SOUND 1
#define DEFAULT_FULLSCREEN 1
#define DEFAULT_USE_BKGD 1
#define DEFAULT_DEMO_MODE 0
#define DEFAULT_OPER_OVERRIDE 0
#define DEFAULT_USE_KEYPAD 0
#define DEFAULT_SPEED 1
#define DEFAULT_ALLOW_SPEEDUP 1
#define DEFAULT_SPEEDUP_FACTOR 1.2
#define DEFAULT_MAX_SPEED 10
#define DEFAULT_SLOW_AFTER_WRONG 0
#define DEFAULT_STARTING_COMETS 2
#define DEFAULT_EXTRA_COMETS_PER_WAVE 2
#define DEFAULT_MAX_COMETS 10
#define DEFAULT_SAVE_SUMMARY 1	
#define DEFAULT_SOUND_AVAILABLE 1
#define DEFAULT_NUM_CITIES 4   /* MUST BE AN EVEN NUMBER! */
#define DEFAULT_NUM_BKGDS 5
#define DEFAULT_MAX_CITY_COLORS 4
#define DEFAULT_USE_FEEDBACK 0
#define DEFAULT_DANGER_LEVEL 0.35
#define DEFAULT_DANGER_LEVEL_SPEEDUP 1.1
#define DEFAULT_DANGER_LEVEL_MAX 0.9
#define DEFAULT_CITY_EXPL_HANDICAP 0

#define MINIMUM_SPEED 0.1

/* this struct contains all options regarding general       */
/* gameplay but not having to do with math questions per se */
typedef struct game_option_type {
  /* general game options */
  int per_user_config;
  int use_sound;
  int fullscreen;
  int use_bkgd;
  int demo_mode;
  int oper_override;
  int use_keypad;
  float speed;
  int allow_speedup;
  float speedup_factor;
  float max_speed;
  int slow_after_wrong;
  int starting_comets;
  int extra_comets_per_wave;
  int max_comets;  
  char next_mission[PATH_MAX];
  int save_summary;
  int use_feedback;
  float danger_level;
  float danger_level_speedup;
  float danger_level_max;
  float city_expl_handicap;

  /* whether sound system is successfully initialized and sound files loaded: */
  /* this flag is set by the program, not the user, and is not in the config file. */
  int sound_available;
  /* not sure the rest of these belong in here */
  int num_cities;  /* MUST BE AN EVEN NUMBER! */
  int num_bkgds;
  int max_city_colors;
} game_option_type;

typedef struct range_type {
  int min;
  int max;
} range_type;

enum {
  OPER_ADD,
  OPER_SUB,
  OPER_MULT,
  OPER_DIV,
  NUM_OPERS
};

enum {
  Q_RANGE_1_5,
  Q_RANGE_6_12,
  Q_RANGE_13_20,
  NUM_Q_RANGES
};


/* Global data gets 'externed' here: */
extern game_option_type* game_options; /* used by setup.c, options.c, game.c */

extern SDL_Surface* screen; /* declared in setup.c; also used in game.c, options.c, fileops.c, credits.c, title.c */
extern SDL_Surface* images[];    /* declared in setup.c, used in same files as screen */
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
