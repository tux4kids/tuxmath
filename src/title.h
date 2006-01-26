/*
  title.h

  For TuxMath
  The title screen function.

  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/


  Part of "Tux4Kids" Project
  http://www.tux4kids.org/
      
  August 26, 2001 - August 28, 2001
*/


#ifndef TITLE_H
#define TITLE_H

enum {
  CMD_GAME,
  CMD_OPTIONS,
  CMD_CREDITS,
  CMD_QUIT,
  NUM_CMDS
};

int title(void);

#endif
