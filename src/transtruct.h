/*
   transtruct.h:

Description: contains headers for the data structures
that would be transferred between the server and the client
during the multiplayer LAN game.

Copyright 2009, 2010, 2011.
Authors: David Bruce, Akash Gangil
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org


transtruct.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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



#ifndef TRANSTRUCT_H
#define TRANSTRUCT_H

#define NET_BUF_LEN 512
#define NAME_SIZE 50
#define MAX_SERVERS 50
#define MAX_CLIENTS 16

#define MC_USE_NEWARC
#define MC_FORMULA_LEN 40
#define MC_ANSWER_LEN 5

#define QUEST_QUEUE_SIZE 10


typedef struct _MC_FlashCard {
    char formula_string[MC_FORMULA_LEN];
    char answer_string[MC_ANSWER_LEN];
    int question_id;
    int answer;
    int difficulty;
} MC_FlashCard;


#endif
