/*
  options.h

  For TuxMath
  The options screen loop.

  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/


  Part of "Tux4Kids" Project
  http://www.tux4kids.org/
      
  August 26, 2001 - February 21, 2003
*/


#ifndef OPTIONS_H
#define OPTIONS_H

enum {
  OPT_OP_ADD,
  OPT_OP_SUB,
  OPT_OP_MUL,
  OPT_OP_DIV,
  OPT_A_MAX,
  OPT_A_SPEED,
  OPT_Q_RANGE,
  NUM_OPTS
};

int options(void);

#endif
