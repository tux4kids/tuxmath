/*
  tuxmath.c

  Main function for TuxMath
  Calls functions in other modules (eg, "setup", "title", "game", etc.)
  as needed.

  Source code by Bill Kendrick, New Breed Software
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/

  Part of "Tux4Kids" Project
  http://www.tux4kids.org/
  
  August 26, 2001 - August 28, 2001
*/


#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "gettext.h"
#include "tuxmath.h"
#include "setup.h"
#include "game.h"
#include "options.h"
#include "credits.h"

#include "titlescreen.h"

/* global data: */

int main(int argc, char * argv[])
{
  /* NOTE - these casts to (void) prevent compiler warnings of */
  /* "statement with no effect" - I am not sure if they do anything */
  /* else that we need - DSB */

#ifndef MACOSX
#ifndef WIN32
  (void)setlocale(LC_ALL, "");
  (void)bindtextdomain(PACKAGE, LOCALEDIR);
  (void)bind_textdomain_codeset(PACKAGE, "UTF-8");
  (void)textdomain(PACKAGE);
#endif
#endif



  setup(argc, argv);
  atexit(cleanup);  // register it so we clean up even if there is a crash
  TitleScreen();
  return 0;
}

