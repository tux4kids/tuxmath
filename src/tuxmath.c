/*
  tuxmath.c

  Main function for TuxMath
  Calls functions in other modules (eg, "setup", "title", "game", etc.)
  as needed.

  Original source code by Bill Kendrick, New Breed Software
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/

  Part of "Tux4Kids" Project
  http://www.tux4kids.org/
  
  August 26, 2001 - August 28, 2001

  Largely rewritten by David Bruce, Karl Ove Hufthammer,
  and Tim Holy.
  2006-2007
*/


#include <stdio.h>
#include <stdlib.h>
/* (tuxmath.h brings in "gettext.h" and <locale.h> */
#include "tuxmath.h"
#include "setup.h"
#include "titlescreen.h"

int main(int argc, char * argv[])
{
#ifndef MACOSX
#ifndef WIN32
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  bind_textdomain_codeset(PACKAGE, "UTF-8");
  textdomain(PACKAGE);

  printf("gettext(\"help\"): %s\n\n", gettext("help"));
#endif
#endif

  setup(argc, argv);
  atexit(cleanup);  // register it so we clean up even if there is a crash
  TitleScreen();
  return 0;
}

