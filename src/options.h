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

  Extensively revised by David Bruce
  2004-2007
*/


#ifndef OPTIONS_H
#define OPTIONS_H

#include "globals.h"  /* needed for PATH_MAX definition */

enum {
  PER_USER_CONFIG,
  USE_SOUND,
  MENU_SOUND,
  MENU_MUSIC,
  FULLSCREEN,
  USE_KEYPAD,
  USE_IGLOOS,
  NUM_GLOBAL_OPTS
};                                 

extern const char* const OPTION_TEXT[];
extern const int OPTION_DEFAULTS[];

/* contains options that tend to apply to the progam as a whole, rather *
 * than on a per-game basis                                             */
typedef struct global_option_type {
  int iopts[NUM_GLOBAL_OPTS];
} global_option_type;

/* this struct contains all options regarding general       */
/* gameplay but not having to do with math questions per se */
typedef struct game_option_type {
  /* general game options */
  char current_font_name[FONT_NAME_LENGTH];
  int lan_mode;
  int use_bkgd;
  int help_mode;
  int demo_mode;
  int oper_override;
  int allow_pause;
  int bonus_comet_interval;
  float bonus_speed_ratio;
  float speed;
  int allow_speedup;
  float speedup_factor;
  float max_speed;
  int slow_after_wrong;
  int starting_comets;
  int extra_comets_per_wave;
  int max_comets;  
  char next_mission[PATH_MAX];
  int save_summary;
  int use_feedback;
  float danger_level;
  float danger_level_speedup;
  float danger_level_max;
  float city_expl_handicap;
  
  int mp_multiplayer;
  int mp_round;
  int mp_playernum;
  
  /* whether sound system is successfully initialized and sound files loaded: */
  /* this flag is set by the program, not the user, and is not in the config file. */
  int sound_hw_available;
  /* place to save score of last game - not read in from file: */
  int last_score;
  /* not sure the rest of these belong in here */
  int num_cities;  /* MUST BE AN EVEN NUMBER! */
  int num_bkgds;
  int max_city_colors;
  int keep_score;
} game_option_type;


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

/* global struct (until accessor functions completed) */
//extern game_option_type* game_options; /* used by setup.c, options.c, game.c */

/* main options function called from title(): */
//int options(void);

/* "Public methods" of game_option_type struct; program interacts with struct */
/* through these simple functions (rather than directly) to allow for error   */
/* checking, etc.                                                             */
int Opts_Initialize(void);
void Opts_Cleanup(void);


/* "Set" functions for tuxmath options struct: */

unsigned int Opts_MapTextToIndex(const char* text);

int  Opts_GetGlobalOpt(unsigned int index);
void Opts_SetGlobalOpt(unsigned int index, int val);

void Opts_SetFontName(char* font_name);
void Opts_SetLanMode(int val);
void Opts_SetUseBkgd(int val);
void Opts_SetHelpMode(int val);
void Opts_SetDemoMode(int val);
void Opts_SetOperOverride(int val);
void Opts_SetAllowPause(int val);
void Opts_SetBonusCometInterval(int val);
void Opts_SetBonusSpeedRatio(float val);
void Opts_SetSpeed(float val);
void Opts_SetAllowSpeedup(int val);
void Opts_SetSpeedupFactor(float val);
void Opts_SetMaxSpeed(float val);
void Opts_SetSlowAfterWrong(int val);
void Opts_SetStartingComets(int val);
void Opts_SetExtraCometsPerWave(int val);
void Opts_SetMaxComets(int val);
void Opts_SetNextMission(char* str);
void Opts_SetSaveSummary(int val);
void Opts_SetUseFeedback(int val);
void Opts_SetDangerLevel(float val);
void Opts_SetDangerLevelSpeedup(float val);
void Opts_SetDangerLevelMax(float val);
void Opts_SetCityExplHandicap(float val);

/* whether sound system is successfully initialized and sound files loaded: */
/* this flag is set by the program, not the user, and is not in the config file. */
void Opts_SetSoundHWAvailable(int val);
/* Used by high score table code, not config file: */
void Opts_SetLastScore(int val);

void Opts_SetKeepScore(int val);

/* "Get" functions for tuxmath options struct: */
const char* Opts_FontName(void);
int Opts_LanMode(void);
int Opts_UseBkgd(void);
int Opts_HelpMode(void);
int Opts_DemoMode(void);
int Opts_OperOverride(void);
int Opts_AllowPause(void);
int Opts_BonusCometInterval(void);
float Opts_BonusSpeedRatio(void);
float Opts_Speed(void);
int Opts_AllowSpeedup(void);
float Opts_SpeedupFactor(void);
float Opts_MaxSpeed(void);
int Opts_SlowAfterWrong(void);
int Opts_StartingComets(void);
int Opts_ExtraCometsPerWave(void);
int Opts_MaxComets(void);
char* Opts_NextMission(void);
int Opts_SaveSummary(void);
int Opts_UseFeedback(void);
float Opts_DangerLevel(void);
float Opts_DangerLevelSpeedup(void);
float Opts_DangerLevelMax(void);
float Opts_CityExplHandicap(void);
int Opts_KeepScore(void);

/* whether sound system is successfully initialized and sound files loaded: */
/* this flag is set by the program, not the user, and is not in the config file. */
int Opts_SoundHWAvailable(void);
/* this is the function that says if sound is both desired and actually available: */
int Opts_UsingSound(void);

/* Returns score of last Arcade-type game this session: */
int Opts_LastScore(void);

/* print options values to stream - for debugging purposes - has been */
/* superceded by write_config_file() to actually write human-readable file. */
void print_game_options(FILE* fp, int verbose);
#endif
