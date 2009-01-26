//
// C++ Interface: loaders
//
// Description: 
//
//
// Author: David Bruce <davidstuartbruce@gmail.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef LOADERS_C
#define LOADERS_C

#include "tuxmath.h"

#include <string.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAX_SPRITE_FRAMES 30

#define IMG_REGULAR  0x01
#define IMG_COLORKEY 0x02
#define IMG_ALPHA    0x04
#define IMG_MODES    0x07
#define IMG_NOT_REQUIRED 0x10
#define IMG_NO_THEME     0x20

typedef struct {
  SDL_Surface *frame[MAX_SPRITE_FRAMES];
  SDL_Surface *default_img;
  int num_frames;
  int cur;
} sprite;


/* in loaders.c (from tuxtype): */
int         checkFile( const char *file );
TTF_Font*    LoadFont(const char* font_name, int font_size);
Mix_Chunk*   LoadSound( char* datafile );
SDL_Surface* LoadImage( char* datafile, int mode );
SDL_Surface* LoadBkgd(char* datafile);
int          LoadBothBkgds(char* datafile, 
                           SDL_Surface** fs_bkgd, 
                           SDL_Surface** win_bkgd);
sprite*      LoadSprite( char* name, int MODE );
sprite*      FlipSprite( sprite* in, int X, int Y );
void         FreeSprite( sprite* gfx );
Mix_Music*   LoadMusic( char *datafile );
void next_frame(sprite* s);

#endif