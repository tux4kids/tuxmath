/*
   menu.h:

   Functions responsible for loading, parsing and displaying game menu.
   (interface)
 
   Copyright 2009, 2010.
   Authors: Boleslaw Kulbabinski <bkulbabinski@gmail.com>
            Brendan Luchen
	    David Bruce
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org


menu.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
  bool show_title;
  int entries_per_screen;
  int first_entry;
};

typedef struct mNode MenuNode;

/* used also by highscore.c */
extern SDL_Rect menu_rect, stop_rect, prev_rect, next_rect;
extern SDL_Surface *stop_button, *prev_arrow, *next_arrow, *prev_gray, *next_gray;


/* global functions */
void LoadMenus(void);
int RunLoginMenu(void);
void RunMainMenu(void);
void UnloadMenus(void);

#endif // MENU_H

