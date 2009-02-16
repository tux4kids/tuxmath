/*
  tuxmath.c

  Main function for TuxMath
  Calls functions in other modules (eg, "setup", "title", "game", etc.)
  as needed.

  Original source code by Bill Kendrick, New Breed Software
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/

  Part of "Tux4Kids" Project
  http://www.tux4kids.com/
  
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

#ifdef WIN32
#define TUXLOCALE "./locale"
#else
#define TUXLOCALE LOCALEDIR
#endif

//#ifdef LINEBREAK
#include "linewrap.h"
//#endif

int main(int argc, char * argv[])
{
  const char *s1, *s2, *s3, *s4;

  s1 = setlocale(LC_ALL, "");
  s2 = bindtextdomain(PACKAGE, TUXLOCALE);
  s3 = bind_textdomain_codeset(PACKAGE, "UTF-8");
  s4 = textdomain(PACKAGE);

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "PACKAGE = %s\n", PACKAGE);
  fprintf(stderr, "TUXLOCALE = %s\n", TUXLOCALE);
  fprintf(stderr, "setlocale(LC_ALL, \"\") returned: %s\n", s1);
  fprintf(stderr, "bindtextdomain(PACKAGE, TUXLOCALE) returned: %s\n", s2);
  fprintf(stderr, "bind_textdomain_codeset(PACKAGE, \"UTF-8\") returned: %s\n", s3);
  fprintf(stderr, "textdomain(PACKAGE) returned: %s\n", s4);
  fprintf(stderr, "gettext(\"Help\"): %s\n\n", gettext("Help"));
  fprintf(stderr, "After gettext() call\n");
#endif

  setup(argc, argv);
  TitleScreen();  /* Run the game! */
  cleanup();
  return 0;
}

