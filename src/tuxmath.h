/*
  tuxmath.h

  For TuxMath
  Contains global data for configuration of math questions and
  for general game options, as well as constants and defaults.

  Author: David Bruce <davidstuartbruce@gmail.com>, (C) 2006


  Part of "Tux4Kids" Project
  http://www.tux4kids.org/
      
  Added March 2, 2006

  Copyright: See COPYING file that comes with this distribution
  (briefly - GNU GPL v2 or later)
*/



#ifndef TUXMATH_H
#define TUXMATH_H

#include "globals.h"

#include "SDL.h"
#include "SDL_image.h"

#ifndef NOSOUND
#include "SDL_mixer.h"
#endif


#include <t4k_common.h>

/* Global data gets 'externed' here: */

/* declared in setup.c */
/* windowed mode screen  size */
extern int win_res_x;
extern int win_res_y;

/* full screen size */
extern int fs_res_x;
extern int fs_res_y;

extern SDL_Color black;
extern SDL_Color gray;
extern SDL_Color dark_blue;
extern SDL_Color red;
extern SDL_Color white;
extern SDL_Color yellow;

extern SDL_Surface* screen; /* declared in setup.c; also used in game.c, options.c, fileops.c, credits.c, titlescreen.c */
extern SDL_Surface* images[];    /* declared in setup.c, used in same files as screen */
extern sprite* sprites[];
extern SDL_Surface* flipped_images[];
#define NUM_BLENDED_IGLOOS 15
extern SDL_Surface* blended_igloos[];
extern int flipped_img_lookup[];

extern int glyph_offset;


#ifndef NOSOUND
extern Mix_Chunk* sounds[];    /* declared in setup.c; also used in fileops.c, playsound.c */
extern Mix_Music* musics[];    /* declared in setup.c; also used in fileops.c, game.c  */
#endif


/* NOTE: default values for math options are now in mathcards.h */

#endif
