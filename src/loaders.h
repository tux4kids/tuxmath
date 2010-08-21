/*
   loaders.h: Code for loading media files.
 
   Copyright 2009, 2010.
   Authors: David Bruce <davidstuartbruce@gmail.com>,
            Boleslaw Kulbabinski <bkulbabinski@gmail.com>
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org


loaders.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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


#ifndef LOADERS_H
#define LOADERS_H

#include "tuxmath.h"

#include <string.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>


#define IMG_REGULAR         0x01
#define IMG_COLORKEY        0x02
#define IMG_ALPHA           0x04
#define IMG_MODES           0x07

#define IMG_NOT_REQUIRED    0x10
#define IMG_NO_PNG_FALLBACK 0x20


SDL_Surface* LoadImage(const char* file_name, int mode);
SDL_Surface* LoadScaledImage(const char* file_name, int mode, int width, int height);
SDL_Surface* LoadImageOfBoundingBox(const char* file_name, int mode, int max_width, int max_height);

SDL_Surface* LoadBkgd(const char* file_name, int width, int height);
void         LoadBothBkgds(const char* file_name, SDL_Surface** fs_bkgd, SDL_Surface** win_bkgd);

sprite*      LoadSprite(const char* name, int mode);
sprite*      LoadScaledSprite(const char* name, int mode, int width, int height);
sprite*      LoadSpriteOfBoundingBox(const char* name, int mode, int max_width, int max_height);
sprite*      FlipSprite(sprite* in, int X, int Y);
void         FreeSprite(sprite* gfx);
void         NextFrame(sprite* s);

Mix_Chunk*   LoadSound(char* datafile);
Mix_Music*   LoadMusic(char *datafile);

#endif /* LOADERS_H */
