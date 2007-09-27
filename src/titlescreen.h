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

#define RES_X	640
#define RES_Y	480
#define BPP	32	


#define MAX_SPRITE_FRAMES 30

#include <string.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#ifndef MACOSX
#include "../config.h"
#endif

#include "tuxmath.h"

/* FIXME get rid of these evil macros! */
#define next_frame(SPRITE) if ((SPRITE)->num_frames) (SPRITE)->cur = (((SPRITE)->cur)+1) % (SPRITE)->num_frames;
#define rewind(SPRITE) (SPRITE)->cur = 0;

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

typedef struct {
    char lang[PATH_MAX];
    char path[PATH_MAX];
    char window[PATH_MAX];
    int sfx_volume;
    int mus_volume;
    int menu_music;
} settings;

typedef struct {
	SDL_Surface *frame[MAX_SPRITE_FRAMES];
	SDL_Surface *default_img;
	int num_frames;
	int cur;
} sprite;

// Options that affect how menus are presented
typedef struct {
  int n_entries_per_screen;
  int starting_entry;
} menu_options;

/* LOGGING works as such:
 *
 * - Use LOG if you want to output a string LOG( "Hello World");
 *   
 * - Use DOUT if you want to output a value of a variable and the
 *   name of the variable gives enough context:
 *   DOUT( specialCode );  would add to stderr: "specialCode = 1\n" or
 *   whatever value specialCode had
 *   
 * - Use DEBUGCODE if you need to do something more complicated like
 *   DEBUGCODE { fprintf(stderr, "examining letter %d\n", x); }
 *   since DOUT(x) "x = 1\n" gives little information since x is used
 *   all over the place!
 */

#define LOG( str ) if (debugOn) fprintf( stderr, str );
#define DEBUGCODE if (debugOn) 
#define DOUT(x) if (debugOn) fprintf(stderr, "%s = %d\n", #x, x);



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

/* Menu Prototypes */
enum Game_Type { 
	LESSONS, ARCADE, OPTIONS, GAME_OPTIONS, QUIT_GAME,
        ARCADE_CADET, ARCADE_SCOUT, ARCADE_RANGER, ARCADE_ACE, HIGH_SCORES,
        MAIN, INTERFACE_OPTIONS, HELP, CREDITS, PROJECT_INFO,
        INSTRUCT_CASCADE, CASCADE1, CASCADE2, CASCADE3, CASCADE4,
	INSTRUCT_LASER,    LASER1,    LASER2,    LASER3,    LASER4,
	FREETYPE, ASDF, ALL, SET_LANGUAGE, EDIT_WORDLIST,
	LEVEL1, LEVEL2, LEVEL3, LEVEL4, LASER, INSTRUCT, NOT_CODED, NONE};

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

//Game difficulty levels
enum { EASY, MEDIUM, HARD, INSANE, INF_PRACT };
#define NUM_LEVELS		        4

extern SDL_Surface *screen;
//extern TTF_Font  *font;
extern SDL_Event  event;



extern SDL_Surface *bkg;
extern SDL_Surface *letters[255];

extern unsigned char ALPHABET[256];
extern unsigned char KEYMAP[256];
extern unsigned char FINGER[256][10];
extern int ALPHABET_SIZE;

//global vars
extern int speed_up;
extern int show_tux4kids;
extern int debugOn;
extern int o_lives;
extern int sound_vol;
extern int hidden; // Read the README file in the image directory for info on this ;)

/* Alternative language/word/image/sound theming */
extern unsigned char realPath[2][PATH_MAX];
extern char themeName[PATH_MAX];
extern int useEnglish;

enum {
	WIN_WAV,
	BITE_WAV,
	LOSE_WAV,
	RUN_WAV,
	SPLAT_WAV,
	WINFINAL_WAV,
	EXCUSEME_WAV,
	PAUSE_WAV,
	NUM_WAVES
};

extern Mix_Chunk      *sound[NUM_WAVES];
extern Mix_Music      *music;
extern int sys_sound;

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

void TitleScreen( void );
void switch_screen_mode( void );
int choose_config_file(void);  //FIXME really should be in fileops.c
int choose_menu_item(const unsigned char**,int,menu_options);

/* in theme.c (from tuxtype): */
void chooseTheme(void);
void setupTheme( char *dirname );

/* in loaders.c (from tuxtype): */
int         checkFile( const char *file );
TTF_Font* LoadFont(const unsigned char* font_name, int font_size);
void         LoadLang( void );
Mix_Chunk   *LoadSound( char *datafile );
SDL_Surface *LoadImage( char *datafile, int mode );
sprite      *LoadSprite( char *name, int MODE );
sprite      *FlipSprite( sprite *in, int X, int Y );
void         FreeSprite( sprite *gfx );
Mix_Music   *LoadMusic( char *datafile );

/* in alphabet.c (from tuxtype) */
void LoadKeyboard( void );
void set_letters( unsigned char *t );
unsigned char get_letter( void );
void custom_letter_setup( void );
void show_letters( void );
void WORDS_init( void );
void WORDS_use_alphabet( void );
void WORDS_use( char *wordFn );
unsigned char* WORDS_get( void );

/* in pause.c * (from tuxtype): */
int  Pause( void );
void pause_load_media( void );
void pause_unload_media( void );
int  inRect( SDL_Rect r, int x, int y);

/* in audio.c  (from tuxtype): */
void tuxtype_playsound( Mix_Chunk *snd );
void audioMusicLoad( char *musicFilename, int repeatQty );
void audioMusicUnload( void );
void audioMusicPlay( Mix_Music *musicData, int repeatQty );

#endif //TITLESCREEN_H
