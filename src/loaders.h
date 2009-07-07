/*
  loaders.h

  Functions responsible for loading multimedia.
  (interface)

  Author: David Bruce <davidstuartbruce@gmail.com>, (C) 2009
          Boleslaw Kulbabinski <bkulbabinski@gmail.com>, (C) 2009

  Part of "Tux4Kids" Project
  http://www.tux4kids.com/

  Copyright: See COPYING file that comes with this distribution.
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

#define MAX_SPRITE_FRAMES   30

#define IMG_REGULAR         0x01
#define IMG_COLORKEY        0x02
#define IMG_ALPHA           0x04
#define IMG_MODES           0x07

#define IMG_NOT_REQUIRED    0x10
#define IMG_NO_PNG_FALLBACK 0x20

typedef struct {
  SDL_Surface *frame[MAX_SPRITE_FRAMES];
  SDL_Surface *default_img;
  int num_frames;
  int cur;
} sprite;


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
