/*
   multiplayer.h:

   Provides routines for organizing and running a turn-based
   multiplayer that can accommodate up to four players (more with
   a recompilation).  Note that this is separate from the LAN-based
   "real" multiplayer mode.

   Copyright 2008, 2010, 2011.
Authors:  Brendan Luchen, David Bruce.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org


multiplayer.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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



#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H


#define MAX_PLAYERS 4

enum {
    PLAYERS,
    ROUNDS,
    DIFFICULTY,
    MODE,
    NUM_PARAMS
};

typedef enum {
    SCORE_SWEEP,
    ELIMINATION
} MP_Mode;

void mp_run_multiplayer();
void mp_set_parameter(unsigned int param, int value);
int mp_set_player_score(int playernum, int score);
int mp_get_parameter(unsigned int param);
int mp_get_player_score(int playernum);
int mp_get_currentplayer(void);
const char* mp_get_player_name(int playernum);
int mp_num_players();

#endif // MULTIPLAYER_H
