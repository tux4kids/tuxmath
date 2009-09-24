/*
*  C Implementation: network.c
*
*         Description: A simple function that uses SDL_Delay() to keep 
*                      loops from eating all available CPU.

*
* Author: David Bruce, and the TuxMath team, (C) 2009
* Developers list: <tuxmath-devel@lists.sourceforge.net>
*
* Copyright: See COPYING file that comes with this distribution.  (Briefly, GNU GPL).
*/


#include "SDL.h"

/* NOTE this is not thread-safe.  For multithread use we need to store */
/* last_t somewhere outside the function - DSB                         */

void Throttle(int loop_msec)
{
  static Uint32 last_t; //Will be zero first time through
  Uint32 now_t, wait_t;

  //Target loop time must be between 0 and 1000 msec:
  if(loop_msec < 0)
    loop_msec = 0;
  if(loop_msec > 1000)
    loop_msec = 1000;

  //See if we need to wait:
  now_t = SDL_GetTicks();
  if (now_t < (last_t + loop_msec))
  {
    wait_t = (last_t + loop_msec) - now_t;
    //Avoid problem if we somehow wrap past uint32 size (at 49.7 days!)
    if(wait_t < 0)
      wait_t = 0;
    if(wait_t > loop_msec)
      wait_t = loop_msec;
    SDL_Delay(wait_t);
  }
  last_t = SDL_GetTicks();
}
