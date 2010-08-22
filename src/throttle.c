/*
   throttle.c:

   A simple function that uses SDL_Delay() to keep loops from eating
   all available CPU.

   Copyright 2009, 2010.
   Author: David Bruce
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org

throttle.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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



#include "SDL.h"

/* NOTE now store the time elsewhere to make function thread-safe                          */

void Throttle(int loop_msec, Uint32* last_t)
{
  Uint32 now_t, wait_t;

  if(!last_t)
    return;

  //Target loop time must be between 0 and 1000 msec:
  if(loop_msec < 0)
    loop_msec = 0;
  if(loop_msec > 1000)
    loop_msec = 1000;

  //See if we need to wait:
  now_t = SDL_GetTicks();
  if (now_t < (*last_t + loop_msec))
  {
    wait_t = (*last_t + loop_msec) - now_t;
    //Avoid problem if we somehow wrap past uint32 size (at 49.7 days!)
    if(wait_t < 0)
      wait_t = 0;
    if(wait_t > loop_msec)
      wait_t = loop_msec;
    SDL_Delay(wait_t);
  }
  *last_t = SDL_GetTicks();
}
