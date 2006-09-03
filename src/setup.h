/*
  setup.h

  For TuxMath
  Contains functions to initialize the settings structs, 
  read in command-line arguments, and to clean up on exit.
  All code involving file I/O has been moved to fileops.h/fileops.c
  and is called from the main setup function.

  Some globals are declared in setup.c - all globals throught tuxmath
  are now extern'd in the same place in tuxmath.h

  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/


  Part of "Tux4Kids" Project
  http://www.tux4kids.org/
      
  August 26, 2001 - February 18, 2004

  Modified by David Bruce
  dbruce@tampabay.rr.com
  September 1, 2006
*/


#ifndef SETUP_H
#define SETUP_H


void setup(int argc, char * argv[]);
void cleanup(void);
void cleanup_on_error(void);

int opts_using_sound(void);
#endif
