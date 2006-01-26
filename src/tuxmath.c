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
#include "setup.h"
#include "title.h"
#include "game.h"
#include "options.h"
#include "credits.h"


int main(int argc, char * argv[])
{
  int cmd, done;


  setup(argc, argv);
  
  done = 0;
  
  do
  {
    cmd = title();

    if (cmd == CMD_GAME)
      done = game();
    else if (cmd == CMD_OPTIONS)
      done = options();
    else if (cmd == CMD_CREDITS)
      done = credits();
    else if (cmd == CMD_QUIT)
      done = 1;
  }
  while (!done);
  
  SDL_Quit();

  return 0;
}

