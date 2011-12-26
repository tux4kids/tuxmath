/*
   lessons.h: Code for reading and parsing the lessons directory
   and for keeping track of player's progress.

   Copyright 2007, 2008, 2009, 2010, 2011.
Authors: David Bruce, Tim Holy.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org


lessons.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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




#ifndef LESSONS_H
#define LESSONS_H

#include "globals.h"

int read_goldstars_fp(FILE* fp);
void write_goldstars_fp(FILE* fp);

#endif
