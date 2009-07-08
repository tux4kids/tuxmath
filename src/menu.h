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

#include "SDL.h"
#include "globals.h"
#include "loaders.h"

/* titlescreen & menu frame rate */
#define MAX_FPS                    30
/* number of "real" frames per one sprite frame */
#define SPRITE_FRAME_DELAY         6

/* these are all menu choices that are available in tuxmath.
   By using a define we can create both an enum and
   a string array without writing these names twice */
#define QUIT -2
#define STOP -1

#define ACTIVITIES \
  X( RUN_QUIT ),\
  X( RUN_ACADEMY ),\
  X( RUN_CAMPAIGN ),\
  X( RUN_ARCADE ),\
  X( RUN_CUSTOM ),\
  X( RUN_MAIN_MENU ),\
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

struct mNode {
  struct mNode* parent;

  char* title;
  int font_size;

  char* icon_name;
  sprite* icon;

  SDL_Rect button_rect;
  SDL_Rect icon_rect;
  SDL_Rect text_rect;

  /* submenu_size = 0 if no submenu */
  int submenu_size;
  struct mNode** submenu;

  /* these fields are used only if submenu_size = 0 */
  int activity;
  int param;

  /* these fields are used only if submenu_size > 0 */
  bool display_title;
  int entries_per_screen;
  int first_entry;
};

typedef struct mNode MenuNode;

/* global functions */
void LoadMenus(void);
int RunLoginMenu(void);
void RunMainMenu(void);
void UnloadMenus(void);

#endif // MENU_H

