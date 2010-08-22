/*
   servermain.c

   main() function to allow standalone use of server program for 
   LAN-based play in Tux,of Math Command.

   Copyright 2009, 2010.
   Author: David Bruce.
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org

servermain.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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



#include "server.h"

/* This function has to be in its own file that is not linked into tuxmath */
/* itself because there can only be one main() in a program.  All of the   */
/* server functionality is contained in server.h and server.c              */
int main(int argc, char** argv)
{
#ifdef HAVE_LIBSDL_NET
  return RunServer(argc, argv);
#else
  return 0;
#endif
}
