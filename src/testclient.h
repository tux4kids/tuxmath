/*
   testclient.h:

   Headers for command-line test client for LAN game

   Copyright 2009, 2010, 2011.
Authors: David Bruce, Akash Gangil
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org


testclient.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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




#ifndef TESTCLIENT_H
#define TESTCLIENT_H

//FIXME do we even need this header? Not sure anything in here actually used

enum {
    GAME_NOT_STARTED,
    GAME_IN_PROGRESS,
    GAME_OVER_WON,
    GAME_OVER_LOST,
    GAME_OVER_OTHER,
    GAME_OVER_ESCAPE,
    GAME_OVER_WINDOW_CLOSE,
    GAME_OVER_CHEATER,
    GAME_OVER_ERROR
};


enum {
    SEND_QUESTION
};

#endif
