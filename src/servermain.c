/*
*  C Implementation: servermain.c
*
*       Description: main() function to allow standalone use of server program for 
*       LAN-based play in Tux,of Math Command.
*
*
* Author: David Bruce and the TuxMath team, (C) 2009
* Developers list: <tuxmath-devel@lists.sourceforge.net>
*
* Copyright: See COPYING file that comes with this distribution.  (Briefly, GNU GPL).
*
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
