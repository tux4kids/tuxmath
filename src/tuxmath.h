/*
   tuxmath.h:

   Contains global data for configuration of math questions and
   for general game options, as well as constants and defaults.

   Copyright 2005, 2006, 2007, 2008, 2009, 2010.
Authors: David Bruce, Tim Holy
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org


tuxmath.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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




#ifndef TUXMATH_H
#define TUXMATH_H

#include "globals.h"
#include "compiler.h"

#include "SDL.h"
#include "SDL_image.h"

#ifndef NOSOUND
#include "SDL_mixer.h"
#endif


#include <t4k_common.h>
#include "mathcards.h"

/* Global data gets 'externed' here: */

/* declared in setup.c */
extern MC_MathGame* local_game;
extern MC_MathGame* lan_game_settings;

/* full screen size */
extern int fs_res_x;
extern int fs_res_y;


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
