/*
   menu_lan.h: Contains headers for Tux Math's LAN selection code.

   Copyright 2007, 2008, 2009, 2010, 2011.
Authors: David Bruce, Tim Holy.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

menu_lan.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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



#ifndef MENU_LAN_H
#define MENU_LAN_H

#include "globals.h"

enum {
    PREGAME_WAITING,
    PREGAME_GAME_IN_PROGRESS,
    PREGAME_OVER_START_GAME,
    PREGAME_OVER_ESCAPE,
    PREGAME_OVER_WINDOW_CLOSE,
    PREGAME_OVER_LAN_DISCONNECT,
    PREGAME_OVER_ERROR,
};

int ConnectToServer(void);
int Pregame(void);

#endif
