/* SDL_extras.h - A few handy functions for using SDL graphics.
   
   Copyright (C) 2007, 2008, 2009, 2010.i
   Authors: David Bruce, Tim Holy, Brendan Luchen.
   email: <tuxmath-devel@lists.sourceforge.net>

SDL_extras.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.  */




#ifndef SDL_EXTRAS_H
#define SDL_EXTRAS_H

#include "SDL.h"
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define rmask 0xff000000
#define gmask 0x00ff0000
#define bmask 0x0000ff00
#define amask 0x000000ff
#else
#define rmask 0x000000ff
#define gmask 0x0000ff00
#define bmask 0x00ff0000
#define amask 0xff000000
#endif

#ifdef HAVE_LIBT4K_COMMON
# include <t4k_common.h>
#endif

/* Non-text graphics functions: */
void            DrawButton(SDL_Rect* target_rect, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void            SetRect(SDL_Rect* rect, const float* pos);
SDL_Surface*    CreateButton(int w, int h, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void            RoundCorners(SDL_Surface* s, Uint16 radius);

SDL_Surface*    Flip(SDL_Surface *in, int x, int y);
SDL_Surface*    Blend(SDL_Surface *S1, SDL_Surface *S2, float gamma);

void            FreeSurfaceArray(SDL_Surface** surfs, int length);
int             inRect(SDL_Rect r, int x, int y);
void            UpdateRect(SDL_Surface* surf, SDL_Rect* rect);

void            DarkenScreen(Uint8 bits);
void            ChangeWindowSize(int new_res_x, int new_res_y);
void            SwitchScreenMode(void);

SDL_EventType   WaitForEvent(SDL_EventMask events);
SDL_Surface*    zoom(SDL_Surface* src, int new_w, int new_h);

/*Text rendering functions: */
int             Setup_SDL_Text(void);
void            Cleanup_SDL_Text(void);
SDL_Surface*    BlackOutline(const char* t, int size, SDL_Color* c);
SDL_Surface*    SimpleText(const char *t, int size, SDL_Color* col);
SDL_Surface*    SimpleTextWithOffset(const char *t, int size, SDL_Color* col, int *glyph_offset);


#endif
