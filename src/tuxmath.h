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

//#define NOSOUND
//#define TUXMATH_DEBUG   /* for conditional compilation of debugging output */
#define TUXMATH_VERSION 0.92

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
  /* not sure the rest of these belong in here */
  int num_cities;  /* MUST BE AN EVEN NUMBER! */
  int num_bkgds;
  int max_city_colors;
} game_option_type;


/* make option data accessible to rest of program */
extern game_option_type* game_options; /* used by setup.c, options.c, game.c */

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
#define DEFAULT_NUM_CITIES 4   /* MUST BE AN EVEN NUMBER! */
#define DEFAULT_NUM_BKGDS 5
#define DEFAULT_MAX_CITY_COLORS 4

/* NOTE: default values for math options are now in mathcards.h */

#endif
