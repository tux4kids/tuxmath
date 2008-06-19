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

#include "tuxmath.h"  /* needed for PATH_MAX definition */

/* this struct contains all options regarding general       */
/* gameplay but not having to do with math questions per se */
typedef struct game_option_type {
  /* general game options */
  int per_user_config;
  int use_sound;
  int menu_sound;
  int menu_music;
  int fullscreen;
  int use_bkgd;
  int help_mode;
  int demo_mode;
  int oper_override;
  int use_keypad;
  int use_igloos;
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

  /* whether sound system is successfully initialized and sound files loaded: */
  /* this flag is set by the program, not the user, and is not in the config file. */
  int sound_hw_available;
  /* place to save score of last game - not read in from file: */
  int last_score;
  /* not sure the rest of these belong in here */
  int num_cities;  /* MUST BE AN EVEN NUMBER! */
  int num_bkgds;
  int max_city_colors;
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
void Opts_SetPerUserConfig(int val);
void Opts_SetUseSound(int val);
void Opts_SetMenuSound(int val);
void Opts_SetMenuMusic(int val);
void Opts_SetFullscreen(int val);
void Opts_SetUseBkgd(int val);
void Opts_SetHelpMode(int val);
void Opts_SetDemoMode(int val);
void Opts_SetOperOverride(int val);
void Opts_SetUseKeypad(int val);
void Opts_SetUseIgloos(int val);
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


/* "Get" functions for tuxmath options struct: */
int Opts_PerUserConfig(void);
int Opts_UseSound(void);
int Opts_MenuSound(void);
int Opts_MenuMusic(void);
int Opts_Fullscreen(void);
int Opts_UseBkgd(void);
int Opts_HelpMode(void);
int Opts_DemoMode(void);
int Opts_OperOverride(void);
int Opts_AllowPause(void);
int Opts_UseKeypad(void);
int Opts_UseIgloos(void);
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
