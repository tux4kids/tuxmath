/*
   game.h: Contains headers for Tux Math's main game loop.

   Copyright 2001, 2002, 2003, 2004, 2006, 2007, 2008, 2009, 2010, 2011.
Authors: Bill Kendrick, David Bruce, Tim Holy.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

game.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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


#ifndef GAME_H
#define GAME_H

enum {
    GAME_IN_PROGRESS,
    GAME_OVER_WON,
    GAME_OVER_LOST,
    GAME_OVER_OTHER,
    GAME_OVER_ESCAPE,
    GAME_OVER_WINDOW_CLOSE,
    GAME_OVER_LAN_HALTED,
    GAME_OVER_LAN_DISCONNECT,
    GAME_OVER_LAN_WON,
    GAME_OVER_CHEATER,
    GAME_OVER_ERROR
};

extern int user_quit_received;

int pause_game(void);


#endif
