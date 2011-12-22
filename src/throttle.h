/*

        throttle.h

        Description: A simple function that uses SDL_Delay() to keep loops from eating all available
                     CPU
        Author: David Bruce and the TuxMath team, (C) 2009

        Copyright: See COPYING file that comes with this distribution (briefly, GNU GPL version 2 or later)

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
