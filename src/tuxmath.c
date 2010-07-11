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

/* (tuxmath.h brings in "gettext.h" and <locale.h> */
#include "tuxmath.h"
#include "setup.h"
#include "titlescreen.h"
#include "linewrap.h"


#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#define TUXLOCALE "./locale"
#else
#define TUXLOCALE LOCALEDIR
#endif

int main(int argc, char* argv[])
{
  const char *s1, *s2, *s3, *s4;

  s1 = setlocale(LC_ALL, "");
  s2 = bindtextdomain(PACKAGE, TUXLOCALE);
  s3 = bind_textdomain_codeset(PACKAGE, "UTF-8");
  s4 = textdomain(PACKAGE);

  setup(argc, argv);

  DEBUGMSG(debug_setup, "PACKAGE = %s\n", PACKAGE);
  DEBUGMSG(debug_setup, "TUXLOCALE = %s\n", TUXLOCALE);
  DEBUGMSG(debug_setup, "setlocale(LC_ALL, \"\") returned: %s\n", s1);
  DEBUGMSG(debug_setup, "bindtextdomain(PACKAGE, TUXLOCALE) returned: %s\n", s2);
  DEBUGMSG(debug_setup, "bind_textdomain_codeset(PACKAGE, \"UTF-8\") returned: %s\n", s3);
  DEBUGMSG(debug_setup, "textdomain(PACKAGE) returned: %s\n", s4);
  DEBUGMSG(debug_setup, "gettext(\"Help\"): %s\n\n", gettext("Help"));
  DEBUGMSG(debug_setup, "After gettext() call\n");

#ifdef SCHOOLMODE
if (argc>1)//if not used then argv[1] will segfault in normal game
 {
   if (0 == strcmp(argv[1], "--schoolmode") )
    {
     if(argc >2)  
      {  
       schoolmode(argv[2]);
      }   
     else
        fprintf(stderr, "Required an argument (XML lesson filepath)\n");
    }
 }
//else
#else
 TitleScreen();  /* Run the game! */
#endif

  cleanup();
  return 0;
}

