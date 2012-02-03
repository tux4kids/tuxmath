/*
   frame_counter.c:

   Count frames and limit FPS.

   Copyright 2011.
Authors: Jakub M. Spiewak.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org


frame_counter.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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

#include "SDL_timer.h"

#include "options.h"

#define SPRITE_DELAY 200


//global
float FC_time_elapsed;
int FC_frame_rate;
int FC_sprite_counter;

//'local'
static Uint32 last_time;
static Uint32 counter_time;
static Uint32 frame_begin_time;
static Uint32 sprite_counter_time;
static int frame_count;


void FC_init(void)
{
    last_time = SDL_GetTicks();
    counter_time = 0;
    sprite_counter_time = 0;
    frame_count = 0;

    FC_time_elapsed = 0.0f;
    FC_frame_rate = 0;
    FC_sprite_counter = 0;
}


void FC_frame_begin(void)
{
    frame_begin_time = SDL_GetTicks();

    Uint32 delta_time = frame_begin_time - last_time;
    last_time = frame_begin_time;

    counter_time += delta_time;
    ++frame_count;
    if(counter_time >= 1000)
    {
        FC_frame_rate = frame_count;
        frame_count = 0;
        counter_time = 0;
    }

    sprite_counter_time += delta_time;
    if(sprite_counter_time >= SPRITE_DELAY)
    {
        ++FC_sprite_counter;
        sprite_counter_time = 0;
    }

    FC_time_elapsed = delta_time/1000.0f;
}


void FC_frame_end(void)
{
    if(Opts_FPSLimit() <= 0)
        return;

    Uint32 this_frame_time = SDL_GetTicks()-frame_begin_time;
    Uint32 time_per_frame_limit = 1000/Opts_FPSLimit();

    if(this_frame_time < time_per_frame_limit)
    {
        SDL_Delay(time_per_frame_limit-this_frame_time);
    }
}
