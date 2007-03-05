/*
  tuxmath.c

  Main function for TuxMath
  Calls functions in other modules (eg, "setup", "title", "game", etc.)
  as needed.

  Source code by Bill Kendrick, New Breed Software
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/

  Part of "Tux4Kids" Project
  http://www.tux4kids.org/
  
  August 26, 2001 - August 28, 2001
*/


#include <stdio.h>
#include <stdlib.h>

#include "tuxmath.h"
#include "setup.h"
#include "game.h"
#include "options.h"
#include "credits.h"

#include "titlescreen.h"

/* global data: */

int main(int argc, char * argv[])
{
/* Link control of tuxtype-derived code's debug to TUXMATH_DEBUG: */
  debugOn = 0; //for tuxtype-derived code  
#ifdef TUXMATH_DEBUG
  debugOn = 1;
#endif

  setup(argc, argv);
  TitleScreen();
  cleanup();
  return 0;
}

