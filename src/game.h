/*
  game.h

  For TuxMath
  The main game loop!

  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/


  Part of "Tux4Kids" Project
  http://www.tux4kids.org/
      
  August 26, 2001 - February 18, 2004
*/


#ifndef GAME_H
#define GAME_H

#define MAX_COMETS 10
#define NUM_CITIES 4   /* MUST BE AN EVEN NUMBER! */

#define NUM_BKGDS 12

#define MAX_CITY_COLORS 4

typedef struct city_type {
  int alive, expl, shields;
  int x;
} city_type;

typedef struct laser_type {
  int alive;
  int x1, y1;
  int x2, y2;
} laser_type;



enum {
  GAME_IN_PROGRESS,
  GAME_OVER_WON,
  GAME_OVER_LOST,
  GAME_OVER_OTHER,
  GAME_OVER_ESCAPE,
  GAME_OVER_WINDOW_CLOSE,
  GAME_OVER_ERROR
};

int game(void);
/* draw_nums() is used in options.c so need extern linkage */
void draw_nums(char* str, int x, int y);

#endif
