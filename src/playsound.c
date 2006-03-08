/*
  playsound.c

  For TuxMath
  Plays a sound (if sound support is enabled)

  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/

  Part of "Tux4Kids" Project
  http://www.tux4kids.org/
  
  August 28, 2001 - September 6, 2001
*/


#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#ifndef NOSOUND
#include <SDL_mixer.h>
#endif
#include "setup.h"
#include "tuxmath.h"

void playsound(int snd)
{
#ifndef NOSOUND
  if (game_options->use_sound)
    Mix_PlayChannel(-1, sounds[snd], 0);
#endif
}
