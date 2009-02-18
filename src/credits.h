/*
  credits.h

  For TuxMath
  Contains the text of the credits display, as well
  as the function which displays the credits in the game window.

  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/


  Part of "Tux4Kids" Project
  http://www.tux4kids.org/
      
  August 26, 2001 - August 28, 2001
*/


#ifndef CREDITS_H
#define CREDITS_H

//needed for MAX_LINES and MAX_LINEWIDTH
#include "linewrap.h"

int credits(void);
int scroll_text(char text[MAX_LINES][MAX_LINEWIDTH], SDL_Rect subscreen, int speed);
void draw_text(char* str, SDL_Rect dest);
#endif
