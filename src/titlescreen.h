/***************************************************************************
 -  file: titlescreen.h
 -  description: header for the tuxtype-derived files in tuxmath
                            ------------------

    David Bruce - 2006.
    email                : <dbruce@tampabay.rr.com>
                           <tuxmath-devel@lists.sourceforge.net>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/




#ifndef TITLESCREEN_H
#define TITLESCREEN_H

#define to_upper(c) (((c) >= 'a' && (c) <= 'z') ? (c) -32 : (c))
#define COL2RGB( col ) SDL_MapRGB( screen->format, col->r, col->g, col->b )

//#define FNLEN        200


#include <string.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

#ifndef MACOSX
//#include "config.h"
#endif

#include "tuxmath.h"
#include "loaders.h"


// Options that affect how menus are presented
typedef struct {
  int starting_entry;
  int xleft, ytop, ybottom;   // left, top, and bottom borders
  int buttonheight; // size of menu item button  (-1 if calculated)
  int ygap;  // vertical gap between entries
  int button_same_width; // should all buttons have the same width?
  char *title;
  char *trailer;
} menu_options;


#define MAX_LESSONS 100
#define MAX_NUM_WORDS   500
#define MAX_WORD_SIZE   8

//MAX_UPDATES needed for TransWipe() and friends:
#define MAX_UPDATES 180


#define WAIT_MS                               2500
#define FRAMES_PER_SEC                        50
#define FULL_CIRCLE                           140


/* Title screen animation constants */
#define ANIM_FRAMES                30 /* frames to be displayed */
#define ANIM_FPS                   25 /* max fps */



extern SDL_Event  event;


#define MUSIC_FADE_OUT_MS        80

enum {
    WIPE_BLINDS_VERT,
    WIPE_BLINDS_HORIZ,
    WIPE_BLINDS_BOX,
    RANDOM_WIPE,

    NUM_WIPES
};
// End of code from tuxtype's globals.h


/* --- SETUP MENU OPTIONS --- */

#define TITLE_MENU_ITEMS                5
#define TITLE_MENU_DEPTH                4

#define OPTIONS_SUBMENU                 4
#define GAME_OPTIONS_SUBMENU                       3
#define ARCADE_SUBMENU                        2
#define ROOTMENU                        1


/* --- timings for tux blinking --- */
#define TUX1                            115
#define TUX2                            118
#define TUX3                            121
#define TUX4                            124
#define TUX5                            127
#define TUX6                            130

#define EASTER_EGG_MS  5000 //length of time to replace cursor
#define GOBBLE_ANIM_MS 1000 //duration of the gobbling animation

/********************************/
/* "Global" Function Prototypes */
/********************************/

/*In titlescreen.c */
void TitleScreen(void);
int RenderTitleScreen(void);
void DrawTitleScreen(void);
int ChooseMission(void);  //FIXME really should be in fileops.c
int choose_menu_item(const char **menu_text, 
                     sprite **menu_sprites, 
                     int n_menu_entries, 
                     menu_options* custom_mo, 
                     void (*set_custom_menu_opts)(menu_options*) );
void set_default_menu_options(menu_options *);
SDL_Surface* current_bkg(); //appropriate background for current video mode



/* in audio.c  (from tuxtype): */
void playsound(int snd);
void audioMusicLoad(char* musicFilename, int repeatQty);
void audioMusicUnload(void);
void audioMusicPlay(Mix_Music* musicData, int repeatQty);

#endif //TITLESCREEN_H
