/*
  titlescreen.h

  Splash, background and title screen items.
  (interface)

  begin                : Thur May 4 2000
  copyright            : (C) 2000 by Sam Hart
                       : (C) 2003 by Jesse Andrews
  email                : tuxtype-dev@tux4kids.net

  Modified for use in tuxmath by David Bruce - 2006-2007.
  email                : <davidstuartbruce@gmail.com>
                         <tuxmath-devel@lists.sourceforge.net>

  Also significantly enhanced by Tim Holy - 2007

  Part of "Tux4Kids" Project
  http://www.tux4kids.com/

  Copyright: See COPYING file that comes with this distribution.
*/

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

#define MAX_LESSONS                     100
#define MAX_NUM_WORDS                   500
#define MAX_WORD_SIZE                   8

//MAX_UPDATES needed for TransWipe() and friends:
#define MAX_UPDATES                     180

#define WAIT_MS                         2500
#define FRAMES_PER_SEC                  50
#define FULL_CIRCLE                     140


/* trans_wipe() animation constants */
#define ANIM_FRAMES                     30 /* frames to be displayed */
#define ANIM_FPS                        25 /* max fps */


extern SDL_Event  event;


#define MUSIC_FADE_OUT_MS               80

#include <t4k_common.h>

// End of code from tuxtype's globals.h

/* --- timings for tux blinking --- */
#define TUX1                            115
#define TUX2                            118
#define TUX3                            121
#define TUX4                            124
#define TUX5                            127
#define TUX6                            130

#define EASTER_EGG_MS                   5000 //length of time to replace cursor
#define GOBBLE_ANIM_MS                  1000 //duration of the gobbling animation

/********************************/
/* "Global" Function Prototypes */
/********************************/

/*In titlescreen.c */
void          TitleScreen(void);
int           RenderTitleScreen(void);
void          DrawTitleScreen(void);
int           HandleTitleScreenEvents(const SDL_Event* evt);
void          HandleTitleScreenAnimations();
void          ShowMessage(int font_size, const char* str1, const char* str2, const char* str3, const char* str4);
SDL_Surface*  current_bkg(); //appropriate background for current video mode


/* in audio.c  (from tuxtype): */
void          playsound(int snd);
void          audioMusicLoad(char* musicFilename, int repeatQty);
void          audioMusicUnload(void);
void          audioMusicPlay(Mix_Music* musicData, int repeatQty);

#endif //TITLESCREEN_H
