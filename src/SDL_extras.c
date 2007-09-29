/*
*  C Implementation: SDL_extras
*
* Description: a few handy functions for using SDL graphics.
*
*
* Author: David Bruce,,, <dbruce@tampabay.rr.com>, (C) 2007
*
* Copyright: GPL v3 or later
*
*/
/* DrawButton() creates and draws a translucent button with */
/* rounded ends.  The location and size are taken from the */
/* SDL_Rect* and width arguments.  The sprite is used to   */
/* fill in the rect with the desired translucent color and */
/* give it nice, rounded ends.                             */
/* FIXME make it match target_rect more precisely          */

#include "SDL_extras.h"
#include "tuxmath.h"

void DrawButton(SDL_Rect* target_rect,
                int radius,
                Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
  SDL_Surface* tmp_surf = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
                                          target_rect->w,
                                          target_rect->h,
                                          32, 
                                          rmask, gmask, bmask, amask);
  Uint32 color = SDL_MapRGBA(tmp_surf->format, r, g, b, a);
  SDL_FillRect(tmp_surf, NULL, color);
  RoundCorners(tmp_surf, radius);

  SDL_BlitSurface(tmp_surf, NULL, screen, target_rect);
  SDL_FreeSurface(tmp_surf);
//  SDL_UpdateRect(screen, 0, 0, 0, 0); 

  //SDL_UpdateRect(screen, target_rect->x, target_rect->y, target_rect->w, target_rect->h); 

}



void RoundCorners(SDL_Surface* s, Uint16 radius)
{
  int y = 0;
  int x_dist, y_dist;
  Uint32* p = NULL;
  Uint32 alpha_mask;
  int bytes_per_pix;
  
  if (!s)
    return;
  if (SDL_LockSurface(s) == -1)
    return;

  bytes_per_pix = s->format->BytesPerPixel;
  if (bytes_per_pix != 4)
    return;

  /* radius cannot be more than half of width or height: */
  if (radius > (s->w)/2)
    radius = (s->w)/2;
  if (radius > (s->h)/2)
    radius = (s->h)/2;


  alpha_mask = s->format->Amask;

  /* Now round off corners: */
  /* upper left:            */
  for (y = 0; y < radius; y++) 
  {  
    p = (Uint32*)(s->pixels + (y * s->pitch));
    x_dist = radius;
    y_dist = radius - y;

    while (((x_dist * x_dist) + (y_dist * y_dist)) > (radius * radius))
    {
      /* (make pixel (x,y) transparent) */
      *p = *p & ~alpha_mask;
      p++;
      x_dist--;
    }
  }

  /* upper right:            */
  for (y = 0; y < radius; y++) 
  {  
    /* start at end of top row: */
    p = (Uint32*)(s->pixels + ((y + 1) * s->pitch) - bytes_per_pix);

    x_dist = radius;
    y_dist = radius - y;

    while (((x_dist * x_dist) + (y_dist * y_dist)) > (radius * radius))
    {
      /* (make pixel (x,y) transparent) */
      *p = *p & ~alpha_mask;
      p--;
      x_dist--;
    }
  }

  /* bottom left:            */
  for (y = (s->h - 1); y > (s->h - radius); y--) 
  {  
    /* start at beginning of bottom row */
    p = (Uint32*)(s->pixels + (y * s->pitch));
    x_dist = radius;
    y_dist = y - (s->h - radius);

    while (((x_dist * x_dist) + (y_dist * y_dist)) > (radius * radius))
    {
      /* (make pixel (x,y) transparent) */
      *p = *p & ~alpha_mask;
      p++;
      x_dist--;
    }
  }

  /* bottom right:            */
  for (y = (s->h - 1); y > (s->h - radius); y--) 
  {  
    /* start at end of bottom row */
    p = (Uint32*)(s->pixels + ((y + 1) * s->pitch) - bytes_per_pix);
    x_dist = radius;
    y_dist = y - (s->h - radius);

    while (((x_dist * x_dist) + (y_dist * y_dist)) > (radius * radius))
    {
      /* (make pixel (x,y) transparent) */
      *p = *p & ~alpha_mask;
      p--;
      x_dist--;
    }
  }
  SDL_UnlockSurface(s);
} 


/**********************
 Flip:
   input: a SDL_Surface, x, y
   output: a copy of the SDL_Surface flipped via rules:

     if x is a positive value, then flip horizontally
     if y is a positive value, then flip vertically

     note: you can have it flip both
**********************/
SDL_Surface* Flip( SDL_Surface *in, int x, int y ) {
	SDL_Surface *out, *tmp;
	SDL_Rect from_rect, to_rect;
	Uint32	flags;
	Uint32  colorkey=0;

	/* --- grab the settings for the incoming pixmap --- */

	SDL_LockSurface(in);
	flags = in->flags;

	/* --- change in's flags so ignore colorkey & alpha --- */

	if (flags & SDL_SRCCOLORKEY) {
		in->flags &= ~SDL_SRCCOLORKEY;
		colorkey = in->format->colorkey;
	}
	if (flags & SDL_SRCALPHA) {
		in->flags &= ~SDL_SRCALPHA;
	}

	SDL_UnlockSurface(in);

	/* --- create our new surface --- */

	out = SDL_CreateRGBSurface(
		SDL_SWSURFACE,
		in->w, in->h, 32, rmask, gmask, bmask, amask);

	/* --- flip horizontally if requested --- */

	if (x) {
		from_rect.h = to_rect.h = in->h;
		from_rect.w = to_rect.w = 1;
		from_rect.y = to_rect.y = 0;
		from_rect.x = 0;
		to_rect.x = in->w - 1;

		do {
			SDL_BlitSurface(in, &from_rect, out, &to_rect);
			from_rect.x++;
			to_rect.x--;
		} while (to_rect.x >= 0);
	}

	/* --- flip vertically if requested --- */

	if (y) {
		from_rect.h = to_rect.h = 1;
		from_rect.w = to_rect.w = in->w;
		from_rect.x = to_rect.x = 0;
		from_rect.y = 0;
		to_rect.y = in->h - 1;

		do {
			SDL_BlitSurface(in, &from_rect, out, &to_rect);
			from_rect.y++;
			to_rect.y--;
		} while (to_rect.y >= 0);
	}

	/* --- restore colorkey & alpha on in and setup out the same --- */

	SDL_LockSurface(in);

	if (flags & SDL_SRCCOLORKEY) {
		in->flags |= SDL_SRCCOLORKEY;
		in->format->colorkey = colorkey;
		tmp = SDL_DisplayFormat(out);
		SDL_FreeSurface(out);
		out = tmp;
		out->flags |= SDL_SRCCOLORKEY;
		out->format->colorkey = colorkey;
	} else if (flags & SDL_SRCALPHA) {
		in->flags |= SDL_SRCALPHA;
		tmp = SDL_DisplayFormatAlpha(out);
		SDL_FreeSurface(out);
		out = tmp;
	} else {
		tmp = SDL_DisplayFormat(out);
		SDL_FreeSurface(out);
		out = tmp;
	}

	SDL_UnlockSurface(in);

	return out;
}


/* BlackOutline() creates a surface containing text of the designated */
/* foreground color, surrounded by a black shadow, on a transparent    */
/* background.  The appearance can be tuned by adjusting the number of */
/* background copies and the offset where the foreground text is       */
/* finally written (see below).                                        */
SDL_Surface* BlackOutline(unsigned char *t, TTF_Font *font, SDL_Color *c)
{
  SDL_Surface* out = NULL;
  SDL_Surface* black_letters = NULL;
  SDL_Surface* white_letters = NULL;
  SDL_Surface* bg = NULL;
  SDL_Rect dstrect;
  Uint32 color_key;

  if (!t || !font || !c)
  {
    fprintf(stderr, "BlackOutline(): invalid ptr parameter, returning.");
    return NULL;
  }

#ifdef TUXMATH_DEBUG
  fprintf( stderr, "\nEntering BlackOutline(): \n");
  fprintf( stderr, "BlackOutline of \"%s\"\n", t );
#endif

  black_letters = TTF_RenderUTF8_Blended(font, t, black);

  if (!black_letters)
  {
    fprintf (stderr, "Warning - BlackOutline() could not create image for %s\n", t);
    return NULL;
  }

  bg = SDL_CreateRGBSurface(SDL_SWSURFACE,
                            (black_letters->w) + 5,
                            (black_letters->h) + 5,
                             32,
                             rmask, gmask, bmask, amask);
  /* Use color key for eventual transparency: */
  color_key = SDL_MapRGB(bg->format, 01, 01, 01);
  SDL_FillRect(bg, NULL, color_key);

  /* Now draw black outline/shadow 2 pixels on each side: */
  dstrect.w = black_letters->w;
  dstrect.h = black_letters->h;

  /* NOTE: can make the "shadow" more or less pronounced by */
  /* changing the parameters of these loops.                */
  for (dstrect.x = 1; dstrect.x < 4; dstrect.x++)
    for (dstrect.y = 1; dstrect.y < 3; dstrect.y++)
      SDL_BlitSurface(black_letters , NULL, bg, &dstrect );

  SDL_FreeSurface(black_letters);

  /* --- Put the color version of the text on top! --- */
  white_letters = TTF_RenderUTF8_Blended(font, t, *c);
  dstrect.x = 1;
  dstrect.y = 1;
  SDL_BlitSurface(white_letters, NULL, bg, &dstrect);
  SDL_FreeSurface(white_letters);

  /* --- Convert to the screen format for quicker blits --- */
  SDL_SetColorKey(bg, SDL_SRCCOLORKEY|SDL_RLEACCEL, color_key);
  out = SDL_DisplayFormatAlpha(bg);
  SDL_FreeSurface(bg);

#ifdef TUXMATH_DEBUG
  fprintf( stderr, "\nLeaving BlackOutline(): \n");
#endif

  return out;
}
