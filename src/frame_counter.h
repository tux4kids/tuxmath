/*
   frame_counter.h:

   Contains function and global definitions for manipulating frame counter in tuxmath.

   Copyright 2011.
Authors: Jakub M. Spiewak.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org


frame_counter.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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

#ifndef FRAME_COUNTER_H
#define FRAME_COUNTER_H


//FC_time_elapsed stores time elapsed since last frame in seconds
//It's intended for use in calculating frame rate independent game logic
extern float FC_time_elapsed;


//FC_frame_rate stores number of frames rendered during last second
extern int FC_frame_rate;


//FC_sprite_counter is used for animating sprites
extern int FC_sprite_counter;


//FC_init() should be called before entering the game loop
void FC_init(void);


//FC_frame_begin() should be called on the beginning of every frame
//This function updates FC_time_elapsed and FC_frame_rate
void FC_frame_begin(void);


//FC_frame_end() should be called at the end of every frame
//This function is responsible for limiting frame rate
void FC_frame_end(void);


#endif
