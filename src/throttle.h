/*
   throttle.h:

   A simple function that uses SDL_Delay() to keep loops from 
   eating all available CPU.

   Copyright 2009, 2010.
   Authors: David Bruce
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org


   throttle.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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




#ifndef THROTTLE_H
#define THROTTLE_H

#include "SDL.h"

// This simple function uses SDL_Delay() to wait to return until 'loop_msec'
// milliseconds after it returned the last time. Per SDL docs, the granularity
// is likely no better than 10 msec
// NOTE Uint32* last_t arg added to make function thread-safe
void Throttle(int loop_msec, Uint32* last_t);

#endif
