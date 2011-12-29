/*
   credits.h

   For TuxMath
   Contains the text of the credits display, as well
   as the function which displays the credits in the game window.

   Copyright 2001, 2008, 2009, 2010, 2011
Authors: Bill Kendrick, David Bruce.
email: <tuxmath-devel@lists.sourceforge.net>
website: http://tux4kids.alioth.debian.org

credits.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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



#ifndef CREDITS_H
#define CREDITS_H


int credits(void);
int scroll_text(char text[MAX_LINES][MAX_LINEWIDTH], SDL_Rect subscreen, int speed);
void draw_text(char* str, SDL_Rect dest);
#endif
