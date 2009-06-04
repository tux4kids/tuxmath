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
  X( RUN_SUBMENU ),\
  X( RUN_HELP ),\
  X( N_OF_ACTIVITIES )  /* this one has to be the last one */

/* create enum */
#define X(name) name
typedef enum {
  ACTIVITIES
} ActivityType;
#undef X

struct mNode {
  char* title;
  char* sprite;

  ActivityType activity;

  /* submenu available only if activity = RUN_SUBMENU */
  int submenu_size;
  struct mNode** submenu;
};

typedef struct mNode MenuNode;

/* global functions */
void LoadMenus(void);
void RenderMenus(void);
void UnloadMenus(void);

#endif // MENU_H
