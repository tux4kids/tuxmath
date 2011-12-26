/*

   fileops.h: Contains headers for code involving disk operations.

   Copyright 2001, 2006, 2007, 2008, 2010, 2011
Authors: Bill Kendrick, David Bruce, Tim Holy.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

fileops.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.  */




#ifndef FILEOPS_H
#define FILEOPS_H

#include "globals.h"
#include "mathcards.h"

/* Flag basically telling whether or not to allow admin-level */
/* settings to be changed: */
enum {
    USER_CONFIG_FILE,
    GLOBAL_CONFIG_FILE
};

/* Names for images (formerly in images.h) */
enum {
    IMG_TITLE,
    IMG_LEFT,
    IMG_LEFT_GRAY,
    IMG_RIGHT,
    IMG_RIGHT_GRAY,
    IMG_TUX4KIDS,
    IMG_NBS,
    IMG_CITY_BLUE,
    IMG_CITY_BLUE_EXPL1,
    IMG_CITY_BLUE_EXPL2,
    IMG_CITY_BLUE_EXPL3,
    IMG_CITY_BLUE_EXPL4,
    IMG_CITY_BLUE_EXPL5,
    IMG_CITY_BLUE_DEAD,
    IMG_CITY_GREEN,
    IMG_CITY_GREEN_EXPL1,
    IMG_CITY_GREEN_EXPL2,
    IMG_CITY_GREEN_EXPL3,
    IMG_CITY_GREEN_EXPL4,
    IMG_CITY_GREEN_EXPL5,
    IMG_CITY_GREEN_DEAD,
    IMG_CITY_ORANGE,
    IMG_CITY_ORANGE_EXPL1,
    IMG_CITY_ORANGE_EXPL2,
    IMG_CITY_ORANGE_EXPL3,
    IMG_CITY_ORANGE_EXPL4,
    IMG_CITY_ORANGE_EXPL5,
    IMG_CITY_ORANGE_DEAD,
    IMG_CITY_RED,
    IMG_CITY_RED_EXPL1,
    IMG_CITY_RED_EXPL2,
    IMG_CITY_RED_EXPL3,
    IMG_CITY_RED_EXPL4,
    IMG_CITY_RED_EXPL5,
    IMG_CITY_RED_DEAD,
    IMG_SHIELDS,
    IMG_MINI_COMET1,
    IMG_MINI_COMET2,
    IMG_MINI_COMET3,
    IMG_NUMS,
    IMG_LEDNUMS,
    IMG_LED_NEG_SIGN,
    IMG_PAUSED,
    IMG_DEMO,
    IMG_DEMO_SMALL,
    IMG_KEYPAD,
    IMG_KEYPAD_NO_NEG,
    IMG_CONSOLE_LED,
    IMG_CONSOLE_BASH,
    IMG_TUX_CONSOLE1,
    IMG_TUX_CONSOLE2,
    IMG_TUX_CONSOLE3,
    IMG_TUX_CONSOLE4,
    IMG_TUX_RELAX1,
    IMG_TUX_RELAX2,
    IMG_TUX_EGYPT1,
    IMG_TUX_EGYPT2,
    IMG_TUX_EGYPT3,
    IMG_TUX_EGYPT4,
    IMG_TUX_DRAT,
    IMG_TUX_YIPE,
    IMG_TUX_YAY1,
    IMG_TUX_YAY2,
    IMG_TUX_YES1,
    IMG_TUX_YES2,
    IMG_TUX_SIT,
    IMG_TUX_FIST1,
    IMG_TUX_FIST2,
    IMG_PENGUIN_FLAPDOWN,
    IMG_PENGUIN_FLAPUP,
    IMG_PENGUIN_INCOMING,
    IMG_PENGUIN_GRUMPY,
    IMG_PENGUIN_WORRIED,
    IMG_PENGUIN_STANDING_UP,
    IMG_PENGUIN_SITTING_DOWN,
    IMG_PENGUIN_WALK_ON1,
    IMG_PENGUIN_WALK_ON2,
    IMG_PENGUIN_WALK_ON3,
    IMG_PENGUIN_WALK_OFF1,
    IMG_PENGUIN_WALK_OFF2,
    IMG_PENGUIN_WALK_OFF3,
    IMG_IGLOO_MELTED3,
    IMG_IGLOO_MELTED2,
    IMG_IGLOO_MELTED1,
    IMG_IGLOO_HALF,
    IMG_IGLOO_INTACT,
    IMG_IGLOO_REBUILDING1,
    IMG_IGLOO_REBUILDING2,
    IMG_STEAM1,
    IMG_STEAM2,
    IMG_STEAM3,
    IMG_STEAM4,
    IMG_STEAM5,
    IMG_CLOUD,
    IMG_SNOW1,
    IMG_SNOW2,
    IMG_SNOW3,
    IMG_EXTRA_LIFE,
    IMG_WAVE,
    IMG_SCORE,
    IMG_STOP,
    IMG_NUMBERS,
    IMG_GAMEOVER,
    IMG_GAMEOVER_WON,
    BG_STARS,
    IMG_ASTEROID1,
    IMG_ASTEROID2,
    IMG_ASTEROID3,
    IMG_SHIP01,
    IMG_SHIP_CLOAKED,
    IMG_BONUS_POWERBOMB,
    IMG_BONUS_FORCEFIELD,
    IMG_BONUS_CLOAKING,
    IMG_FACTOROIDS,
    IMG_FACTORS,
    IMG_TUX_LITTLE,
    IMG_GOOD,
    IMG_TUX1,
    IMG_TUX2,
    IMG_TUX3,
    IMG_TUX4,
    IMG_TUX5,
    IMG_TUX6,
    IMG_BUTTON2,
    IMG_BUTTON3,
    IMG_BUTTON5,
    IMG_BUTTON7,
    IMG_BUTTON11,
    IMG_BUTTON13,
    IMG_COCKPIT,
    IMG_FORCEFIELD,
    IMG_SHIP_THRUST,
    IMG_SHIP_THRUST_CLOAKED,
    IMG_ARROWS,
    NUM_IMAGES
};

/* Names for animated images (sprites) */
enum {
    IMG_COMET,
    IMG_BONUS_COMET,
    IMG_COMET_EXPL,
    IMG_BONUS_COMET_EXPL,
    IMG_LEFT_POWERUP_COMET,
    IMG_RIGHT_POWERUP_COMET,
    IMG_POWERUP_COMET_EXPL,
    IMG_BIG_TUX,
    NUM_SPRITES
};

/* Names for game sounds (formerly in sounds.h): */
enum {
    SND_HARP,
    SND_POP,
    SND_TOCK,
    SND_LASER,
    SND_BUZZ,
    SND_ALARM,
    SND_SHIELDSDOWN,
    SND_EXPLOSION,
    SND_SIZZLE,
    SND_BONUS_COMET,
    SND_EXTRA_LIFE,
    SND_ENGINE,
    NUM_SOUNDS
};

/* Names for background music (also formerly in sounds.h): */
enum {
    MUS_GAME1,
    MUS_GAME2,
    MUS_GAME3,
    MUS_GAME4,
    MUS_GAME5,
    MUS_GAME6,
    NUM_MUSICS
};

/* Names for game summary files: */
enum {
    SUMMARY1,
    SUMMARY2,
    SUMMARY3,
    SUMMARY4,
    SUMMARY5,
    SUMMARY6,
    SUMMARY7,
    SUMMARY8,
    SUMMARY9,
    SUMMARY10,
    NUM_SUMMARIES
};

/* These functions used by setup() and titlescreen() to read in settings: */
int read_global_config_file(MC_MathGame* game);
int read_user_config_file(MC_MathGame* game);
int parse_lesson_file_directory(void);
int read_named_config_file(MC_MathGame* game, const char* fn);
int write_user_config_file(MC_MathGame* game);
int read_high_scores(void);
int append_high_score(int tableid, int score, char *player_name);
void set_high_score_path(void);
void set_user_data_dir(const char* dirname);
int write_goldstars(void);

/* These functions are used by titlescreen() to assist with the login */
int read_user_menu_entries(char ***user_names);
int read_user_login_questions(char ***user_login_questions);
int high_scores_found_in_user_dir(void);
void set_high_score_path(void);
void user_data_dirname_up(void);
void user_data_dirname_down(char *subdir);

/* These functions used by game() to record game summary: */
int write_pregame_summary(MC_MathGame* game);
int write_postgame_summary(MC_MathGame* game);

int load_image_data();


#ifndef NOSOUND
int load_sound_data();
#endif

#endif
