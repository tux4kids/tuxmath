/*
  setup.h

  For TuxMath
  Contains some globals (screen surface, images, some option flags, etc.)
  as well as the function to load data files (images, sounds, music)
  and display a "Loading..." screen.

  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/


  Part of "Tux4Kids" Project
  http://www.tux4kids.org/
      
  August 26, 2001 - February 18, 2004
*/


#ifndef SETUP_H
#define SETUP_H

#include <SDL.h>

#ifndef NOSOUND
#include <SDL_mixer.h>
#endif

#include "game.h"


extern SDL_Surface * screen;
extern SDL_Surface * images[];
#ifndef NOSOUND
extern Mix_Chunk * sounds[];
extern Mix_Music * musics[];
#endif

extern int opers[NUM_OPERS], range_enabled[NUM_Q_RANGES];

void setup(int argc, char * argv[]);
void cleanup(void);

void print_game_options(FILE* fp, int verbose);
#endif
