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
