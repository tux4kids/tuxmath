/***************************************************************************
 -  file: loaders.c
 -  description: Functions to load multimedia for Tux Typing
                             -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Sam Hart
                         : (C) 2003 by Jesse Andrews
    email                : tuxtype-dev@tux4kids.net

    Modified for use in tuxmath by David Bruce - 2006.
    email                : <dbruce@tampabay.rr.com>
                           <tuxmath-devel@lists.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//#include "globals.h"
//#include "funcs.h"

#include "tuxmath.h"     // for TUXMATH_DEBUG
#include "loaders.h"
#include "setup.h"  // for cleanup_on_error()
#include "SDL_extras.h"

/* FIXME Doesn't seem to work consistently on all versions of Windows */
/* check to see if file exists, if so return true */
// int checkFile( const char *file ) {
//         static struct stat fileStats;
// 
//         fileStats.st_mode = 0;
// 
//         stat( file, &fileStats );
//                 
//         return (S_IFREG & fileStats.st_mode);
// }


/* Returns 1 if valid file, 2 if valid dir, 0 if neither: */
int checkFile(const char* file)
{
  FILE* fp = NULL;
  DIR* dp = NULL;

  if (!file)
  {
    fprintf(stderr, "CheckFile(): invalid char* argument!");
    return 0;
  }

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "CheckFile() - checking: %s\n", file);
#endif

  dp = opendir(file);
  if (dp)
  {

#ifdef TUXMATH_DEBUG
    fprintf(stderr, "Opened successfully as DIR\n");
#endif

    closedir(dp);
    return 2;
  }

  fp = fopen(file, "r");
  if (fp)
  {

#ifdef TUXMATH_DEBUG
    fprintf(stderr, "Opened successfully as FILE\n");
#endif

    fclose(fp);
    return 1;
  }

  fprintf(stderr, "Unable to open '%s' as either FILE or DIR\n", file);
  return 0;
}


int max( int n1, int n2 ) {
  return (n1 > n2 ? n1 : n2);
}



/* FIXME: I think we need to provide a single default font with the program data, */
/* then more flexible code to try to locate or load system fonts. DSB             */
/* Returns ptr to loaded font if successful, NULL otherwise. */
TTF_Font* LoadFont(const char* font_name, int font_size)
{
  TTF_Font* f;
  char fontfile[PATH_MAX];
  sprintf(fontfile, "%s/fonts/%s", DATA_PREFIX, font_name);

  f = TTF_OpenFont(fontfile, font_size);

  /* HACK - better font searching needed! */
  /* This should mean that font wasn't bundled into data path, which for  */
  /* now means we are using Debian, so grab from Debian installation loc: */
  if (!f)
  { 
    sprintf(fontfile, "/usr/share/fonts/truetype/ttf-sil-andika/AndikaDesRevG.ttf");
    f = TTF_OpenFont(fontfile, font_size);
  }


  if (f)
  {
#ifdef TUXMATH_DEBUG
    fprintf(stderr, "LoadFont(): %s loaded successfully\n\n", fontfile);
#endif
    return f;
  }
  else
  {
   fprintf(stderr, "LoadFont(): %s NOT loaded successfully.\n", fontfile);
   return NULL;
  }
}



/* FIXME checkFile() not working right in Win32 - skipping. */
/***********************
        LoadImage : Load an image and set transparent if requested
************************/
SDL_Surface* LoadImage( char *datafile, int mode )
{
  SDL_Surface* tmp_pic = NULL;
  SDL_Surface* final_pic = NULL;

  char fn[PATH_MAX];

  sprintf( fn, "%s/images/%s", DATA_PREFIX, datafile );

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "LoadImage(): looking in %s\n", fn);
#endif


  /* Try to load it with SDL_image: */
  tmp_pic = IMG_Load(fn);

  if (NULL == tmp_pic) /* Could not load image: */
  {
    if (mode & IMG_NOT_REQUIRED)
    { 
#ifdef TUXMATH_DEBUG
      fprintf(stderr, "Warning: could not load optional graphics file %s\n", datafile);
#endif
      return NULL;  /* Allow program to continue */
    }
    /* If image was required, exit from program: */
    fprintf(stderr, "ERROR could not load required graphics file %s\n", datafile);
    fprintf(stderr, "%s", SDL_GetError() );
    cleanup_on_error();
  }

  /* "else" - now setup the image to the proper format */
  switch (mode & IMG_MODES)
  {
    case IMG_REGULAR:
    { 

      final_pic = SDL_DisplayFormat(tmp_pic);
      SDL_FreeSurface(tmp_pic);
      break;
    }

    case IMG_ALPHA:
    {

      final_pic = SDL_DisplayFormatAlpha(tmp_pic);
      SDL_FreeSurface(tmp_pic);
      break;
    }

    case IMG_COLORKEY:
    {

      SDL_LockSurface(tmp_pic);
      SDL_SetColorKey(tmp_pic, (SDL_SRCCOLORKEY | SDL_RLEACCEL),
                      SDL_MapRGB(tmp_pic->format, 255, 255, 0));
      final_pic = SDL_DisplayFormat(tmp_pic);
      SDL_FreeSurface(tmp_pic);
      break;
    }

    default:
    {
#ifdef TUXMATH_DEBUG
      fprintf(stderr, "Image mode not recognized\n");
#endif
      SDL_FreeSurface(tmp_pic);
    }
  }
#ifdef TUXMATH_DEBUG
  fprintf(stderr, "Leaving LoadImage()\n\n");
#endif
  return final_pic;
}

/***********************
        LoadBkgd() : a wrapper for LoadImage() that scales the
        image to the size of the screen using zoom(), taken
        from TuxPaint
************************/
SDL_Surface* LoadBkgd(char* datafile)
{
  SDL_Surface* orig;
  orig = IMG_Load(datafile);

  if (!orig)
  {
    tmdprintf("In LoadBkgd(), LoadImage() returned NULL on %s\n",
              datafile);
    return NULL;
  }

  if ((orig->w == screen->w)
   && (orig->h == screen->h))
  {
    tmdprintf("No zoom required - return bkgd as is\n");
    return orig;
  }
  else
  { 
    tmdprintf("Image is %dx%d\n", orig->w, orig->h);
    tmdprintf("Screen is %dx%d\n", screen->w, screen->h);
    tmdprintf("Calling zoom() to rescale\n");
    return zoom(orig, screen->w, screen->h);
  }
}

/**********************
LoadBothBkgds() : loads two scaled images: one for the user's native 
resolution and one for 640x480 fullscreen. 
Returns: the number of images that were scaled
Now we also optimize the format for best performance
**********************/
int LoadBothBkgds(char* datafile, SDL_Surface** fs_bkgd, SDL_Surface** win_bkgd)
{
  int ret = 0;
  SDL_Surface* orig = NULL;
  SDL_Surface* tmp = NULL;

  tmdprintf("Entering LoadBothBkgds()\n");
  orig = LoadImage(datafile, IMG_REGULAR);
  tmdprintf("Scaling %dx%d to: %dx%d, %dx%d\n", 
           orig->w, orig->h, RES_X, RES_Y, fs_res_x, fs_res_y);
  if (orig->w == RES_X && orig->h == RES_Y)
  {
    *win_bkgd = orig;
  }
  else
  {
    *win_bkgd = zoom(orig, RES_X, RES_Y);
    ++ret;
  }
  
  if (orig->w == fs_res_x && orig->h == fs_res_y)
  {
    *fs_bkgd = orig;
  }
  else
  {
    *fs_bkgd = zoom(orig, fs_res_x, fs_res_y);
    ++ret;
  }
  
  if (ret == 2) //orig won't be used at all
    SDL_FreeSurface(orig);

  // Optimize images before we leave:
  // turn off transparency, since it's the background:
  if (*fs_bkgd)  //avoid segfault...
  {
    SDL_SetAlpha(*fs_bkgd, SDL_RLEACCEL,SDL_ALPHA_OPAQUE);
    tmp = SDL_DisplayFormat(*fs_bkgd);  // optimize the format
    SDL_FreeSurface(*fs_bkgd);
    *fs_bkgd = tmp;
  }
  if (*win_bkgd)
  {
    SDL_SetAlpha(*win_bkgd, SDL_RLEACCEL,SDL_ALPHA_OPAQUE);
    tmp = SDL_DisplayFormat(*win_bkgd);  // optimize the format
    SDL_FreeSurface(*win_bkgd);
    *win_bkgd = tmp;
  }

  tmdprintf("%d images scaled\nLeaving LoadBothBkgds()\n", ret);
  return ret;
}


sprite* FlipSprite( sprite *in, int X, int Y ) {
  sprite *out;

  out = malloc(sizeof(sprite));
  if (in->default_img != NULL)
          out->default_img = Flip( in->default_img, X, Y );
  else
          out->default_img = NULL;
  for ( out->num_frames=0; out->num_frames<in->num_frames; out->num_frames++ )
          out->frame[out->num_frames] = Flip( in->frame[out->num_frames], X, Y );
  out->cur = 0;
  return out;
}


sprite* LoadSprite( char* name, int MODE ) {
  sprite *new_sprite;
  char fn[PATH_MAX];
  int x;

  /* JA --- HACK check out what has changed with new code */

  new_sprite = malloc(sizeof(sprite));

  sprintf(fn, "%sd.png", name);  // The 'd' means the default image
  new_sprite->default_img = LoadImage( fn, MODE|IMG_NOT_REQUIRED );
  for (x = 0; x < MAX_SPRITE_FRAMES; x++) {
          sprintf(fn, "%s%d.png", name, x);
          new_sprite->frame[x] = LoadImage( fn, MODE|IMG_NOT_REQUIRED );
          if ( new_sprite->frame[x] == NULL ) {
                  new_sprite->cur = 0;
                  new_sprite->num_frames = x;
                  break;
          }
  }


  
  return new_sprite;
}



void FreeSprite(sprite* gfx )
{
  int x;
  if (!gfx)
    return;

  tmdprintf("Freeing image at %p", gfx);
  for (x = 0; x < gfx->num_frames; x++)
  {
    tmdprintf(".");
    if (gfx->frame[x])
    {
      SDL_FreeSurface(gfx->frame[x]);
      gfx->frame[x] = NULL;
    }
  }

  if (gfx->default_img)
  {
    SDL_FreeSurface(gfx->default_img);
    gfx->default_img = NULL;
  }

  tmdprintf("FreeSprite() - done\n");
  free(gfx);
}

void next_frame(sprite* s)
{
  if (s && s->num_frames)
    s->cur = (s->cur + 1) % s->num_frames;
}

/***************************
        LoadSound : Load a sound/music patch from a file.
****************************/
Mix_Chunk* LoadSound( char *datafile )
{ 
  Mix_Chunk* tempChunk = NULL;
  char fn[PATH_MAX];

//    sprintf(fn , "%s/sounds/%s", realPath[i], datafile);
  sprintf(fn , "%s/sounds/%s", DATA_PREFIX, datafile);
  tempChunk = Mix_LoadWAV(fn);
  if (!tempChunk)
  {
    fprintf(stderr, "LoadSound(): %s not found\n\n", fn);
  }
  return tempChunk;
}

/************************
        LoadMusic : Load
        music from a datafile
*************************/
Mix_Music *LoadMusic(char *datafile )
{ 
  char fn[PATH_MAX];
  Mix_Music* tempMusic = NULL;

  sprintf( fn , "%s/sounds/%s", DATA_PREFIX, datafile );
  if (1 != checkFile(fn))
  {
    fprintf(stderr, "LoadMusic(): %s not found\n\n", fn);
    return NULL;
  }

  tempMusic = Mix_LoadMUS(fn);

  if (!tempMusic)
  {
    fprintf(stderr, "LoadMusic(): %s not loaded successfully\n", fn);
    printf("Error was: %s\n\n", Mix_GetError());
  }
  return tempMusic;
}
