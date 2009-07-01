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

/* these are all menu choices that are available in tuxmath.
   By using a define we can create both an enum and
   a string array without writing these names twice */
#define ACTIVITIES \
  X( RUN_ACADEMY ),\
  X( RUN_FLEET_MISSIONS ),\
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
  X( RUN_QUIT ),\
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
  char* title;
  char* sprite;

  /* submenu_size = 0 if no submenu */
  int submenu_size;
  struct mNode** submenu;

  /* available only if submenu_size = 0 */
  int choice;
};

typedef struct mNode MenuNode;

/* global functions */
void LoadMenus(void);
int RunLoginMenu(void);
void RunMainMenu(void);
void RenderMenus(void);
void UnloadMenus(void);

#endif // MENU_H

