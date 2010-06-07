/*
  menu.h

  Functions responsible for loading, parsing and displaying game menu.
  (interface)

  Part of "Tux4Kids" Project
  http://www.tux4kids.com/

  Author: Boleslaw Kulbabinski <bkulbabinski@gmail.com>, (C) 2009

  Copyright: See COPYING file that comes with this distribution.
*/

#ifndef MENU_H
#define MENU_H

#include "globals.h"
#include "loaders.h"

#include "SDL.h"
/* titlescreen & menu frame rate */
#define MAX_FPS                    30
/* number of "real" frames per one sprite frame */
#define SPRITE_FRAME_DELAY         6

#ifdef HAVE_LIBT4KCOMMON
# include <t4kcommon.h>
#else
# define RUN_MAIN_MENU -3
# define QUIT -2
# define STOP -1
#endif //HAVE_LIBT4KCOMMON

/* these are all menu choices that are available in tuxmath.
   By using a define we can create both an enum and
   a string array without writing these names twice */
#define ACTIVITIES \
  X( RUN_QUIT ),\
  X( RUN_ACADEMY ),\
  X( RUN_CAMPAIGN ),\
  X( RUN_ARCADE ),\
  X( RUN_CUSTOM ),\
  X( RUN_LAN_HOST ),\
  X( RUN_LAN_JOIN ),\
  X( RUN_SCORE_SWEEP ),\
  X( RUN_ELIMINATION ),\
  X( RUN_FACTORS ),\
  X( RUN_FRACTIONS ),\
  X( RUN_HELP ),\
  X( RUN_DEMO ),\
  X( RUN_INFO ),\
  X( RUN_CREDITS ),\
  X( RUN_HALL_OF_FAME ),\
  X( RUN_SPACE_CADET ),\
  X( RUN_SCOUT ),\
  X( RUN_RANGER ),\
  X( RUN_ACE ),\
  X( RUN_COMMANDO ),\
  X( N_OF_ACTIVITIES )  /* this one has to be the last one */

/* create enum */
#define X(name) name
enum { ACTIVITIES };
#undef X


/* used also by highscore.c */
extern SDL_Rect menu_rect, stop_rect, prev_rect, next_rect;
extern SDL_Surface *stop_button, *prev_arrow, *next_arrow, *prev_gray, *next_gray;

/* global functions */
void LoadMenus(void);
int RunLoginMenu(void);
void RunMainMenu(void);
void UnloadMenus(void);

#endif // MENU_H

