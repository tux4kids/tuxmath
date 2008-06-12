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

//#define FNLEN	200

#define MAX_SPRITE_FRAMES 30

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
#include "SDL_ttf.h"

#ifndef MACOSX
#include "config.h"
#endif

#include "tuxmath.h"




typedef struct {
	SDL_Surface *frame[MAX_SPRITE_FRAMES];
	SDL_Surface *default_img;
	int num_frames;
	int cur;
} sprite;

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




#define menu_font  "AndikaDesRevA.ttf"  /*  "GenAI102.ttf" */
#define menu_font_size	18

#define ttf_font  "AndikaDesRevA.ttf"  /*   "GenAI102.ttf" */
#define ttf_font_size	18

#define MAX_LESSONS 100
#define MAX_NUM_WORDS   500
#define MAX_WORD_SIZE   8

//MAX_UPDATES needed for TransWipe() and friends:
#define MAX_UPDATES 180


#define WAIT_MS				2500
#define	FRAMES_PER_SEC	                50
#define FULL_CIRCLE		        140


/* Title sequence constants */
#define PRE_ANIM_FRAMES			10
#define PRE_FRAME_MULT			3
#define MENU_SEP			20

/* paths */

#define IMG_REGULAR  0x01
#define IMG_COLORKEY 0x02
#define IMG_ALPHA    0x04
#define IMG_MODES    0x07

#define IMG_NOT_REQUIRED 0x10
#define IMG_NO_THEME     0x20



//extern SDL_Surface *screen;
//extern TTF_Font  *font;
extern SDL_Event  event;

extern SDL_Surface *bkg;

#define MUSIC_FADE_OUT_MS	80

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
#define GAME_OPTIONS_SUBMENU	       	3
#define ARCADE_SUBMENU	        	2
#define ROOTMENU		        1


/* --- timings for tux blinking --- */
#define TUX1                            115
#define TUX2                            118
#define TUX3                            121
#define TUX4                            124
#define TUX5                            127
#define TUX6                            130



/********************************/
/* "Global" Function Prototypes */
/********************************/

/*In titlescreen.c */
void TitleScreen(void);
int ChooseMission(void);  //FIXME really should be in fileops.c
int choose_menu_item(const unsigned char**, sprite**, int, menu_options* menu_opts, void (*)(menu_options*) );
void set_default_menu_options(menu_options *);


/* in loaders.c (from tuxtype): */
int         checkFile( const char *file );
TTF_Font* LoadFont(const unsigned char* font_name, int font_size);
Mix_Chunk   *LoadSound( char* datafile );
SDL_Surface *LoadImage( char* datafile, int mode );
sprite      *LoadSprite( char* name, int MODE );
sprite      *FlipSprite( sprite* in, int X, int Y );
void         FreeSprite( sprite* gfx );
Mix_Music   *LoadMusic( char *datafile );
void next_frame(sprite* s);

/* in audio.c  (from tuxtype): */
void playsound(int snd);
void audioMusicLoad(char* musicFilename, int repeatQty);
void audioMusicUnload(void);
void audioMusicPlay(Mix_Music* musicData, int repeatQty);

#endif //TITLESCREEN_H
