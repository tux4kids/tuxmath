/*
   globals.h:

   Contains global data for configuration of math questions and for
   general game options, as well as constants and defaults.  Nothing
   depending on SDL should be in here; put any SDL-related items into
   tuxmath.h.

   Copyright 2006, 2007, 2008, 2009, 2010, 2011.
Authors: David Bruce, Tim Holy, and others.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

game.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.  */



#ifndef GLOBALS_H
#define GLOBALS_H

// Translation stuff (now works for Mac and Win too!): 
#include "config.h"
#include "gettext.h"
#include <locale.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#ifdef WIN32
#define TUXLOCALE "./locale"
#else
#define TUXLOCALE LOCALEDIR
#endif

#include <wchar.h>

#include <t4k_common.h>
/* Conditional includes of t4k_common replacement functions
 * if not supplied by platform:
 */
/* Somehow, configure defines HAVE_ALPHASORT and
 * HAVE_SCANDIR for mingw32 even though they are 
 * not available for that build, so use our own:
 */
#if !defined HAVE_ALPHASORT || defined BUILD_MINGW32
#include <t4k_alphasort.h>
#endif
#if !defined HAVE_SCANDIR || defined BUILD_MINGW32
#include <t4k_scandir.h>
#endif

/* debug data (now declared in libt4k_common */
//extern int debug_status;

/* bitmasks for debugging options (declared in options.c) */
extern const int debug_setup;
extern const int debug_fileops;
extern const int debug_titlescreen;
extern const int debug_game;
extern const int debug_factoroids;
extern const int debug_lan;
extern const int debug_mathcards;
extern const int debug_lessons;
extern const int debug_highscore;
extern const int debug_options;
extern const int debug_text_and_intl;
extern const int debug_multiplayer;

/* debug macros (now in libt4k_comon) */
//#define DEBUGCODE(mask) if((mask) & debug_status)
//#define DEBUGMSG(mask, ...) if((mask) & debug_status){ fprintf(stderr, __VA_ARGS__); fflush(stderr); }

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
#define DEFAULT_USE_POWERUP_COMETS 1
#define DEFAULT_POWERUP_FREQ 10
#define DEFAULT_SAVE_SUMMARY 1        
#define DEFAULT_SOUND_HW_AVAILABLE 1
#define DEFAULT_USE_IGLOOS 1
#define DEFAULT_USE_FEEDBACK 0
#define DEFAULT_DANGER_LEVEL 0.35
#define DEFAULT_DANGER_LEVEL_SPEEDUP 1.1
#define DEFAULT_DANGER_LEVEL_MAX 0.9
#define DEFAULT_CITY_EXPL_HANDICAP 0
#define DEFAULT_LAST_SCORE 0
#define DEFAULT_FPS_LIMIT 60
#define DEFAULT_WINDOW_WIDTH 640
#define DEFAULT_WINDOW_HEIGHT 480

/* These values are hard-coded and used 'as is' by the program */
/* (i.e. these behaviors require recompilation to change)   */ 
#define DEFAULT_NUM_CITIES 4   /* MUST BE AN EVEN NUMBER! */
#define DEFAULT_MAX_CITY_COLORS 4

#define MINIMUM_SPEED 0.8
#define MAX_MAX_SPEED 50.0
#define MIN_SPEEDUP_FACTOR 1.0
#define MAX_SPEEDUP_FACTOR 2.0
#define MAX_BONUS_SPEED_RATIO 3.0
#define MIN_COMETS 1
#define MAX_MAX_COMETS 100
#define SCORE_COEFFICIENT 100

#define LESSON_TITLE_LENGTH 256
#define DEFAULT_LESSON_TITLE "DEFAULT LESSON TITLE"
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



//Declared in t4k_global.h
extern int text_to_speech_status;

#define NAME_BUF_SIZE 200

/* data for 'Training Academy' lessons: */
extern char **lesson_list_titles;
extern char **lesson_list_filenames;
extern int* lesson_list_goldstars;
extern int num_lessons;

SDL_Thread *tts_announcer_thread;

#endif

//int text_to_speech_global_switch = 1;
