/*
   setup.h:

   Contains functions to initialize the settings structs, 
   read in command-line arguments, and to clean up on exit.
   All code involving file I/O has been moved to fileops.h/fileops.c
   and is called from the main setup function.

   Some globals are declared in setup.c - all globals throughout tuxmath
   are now extern'd in the same place in tuxmath.h

   Copyright 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011.
Authors: Bill Kendrick, David Bruce, Tim Holy.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org


setup.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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


#ifndef SETUP_H
#define SETUP_H


void setup(int argc, char * argv[]);
void cleanup(void);
void cleanup_on_error(void);
extern void initialize_options_user(void);
/* for debugging gettext behavior */
void print_locale_info(FILE* fp);
#endif
