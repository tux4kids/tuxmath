/*
   options.c

   The options screen loop.

   Copyright 2001, 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011.
Authors: Bill Kendrick, David Bruce, Tim Holy, Brendan Luchen.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

options.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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




#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "SDL.h"

#include "mathcards.h"

#include "options.h"
#include "fileops.h"
#include "setup.h"
#include "game.h"
#include "globals.h"
//#include "tuxmath.h"

/* FIXME figure out what oper_override is supposed to do and make sure */
/* this file behaves accordingly! */

//int opers[NUM_OPERS], range_enabled[NUM_Q_RANGES];

/* global debug masks */
//int debug_status;

/* bitmasks for debugging options */
#define dstart  (1 << (START_CUSTOM_DEBUG))
const int debug_setup          = dstart << 0;
const int debug_fileops        = dstart << 1;
const int debug_multiplayer    = dstart << 2;
const int debug_titlescreen    = dstart << 3;
const int debug_options        = dstart << 4;
const int debug_text_and_intl  = dstart << 5;
const int debug_game           = dstart << 6;
const int debug_factoroids     = dstart << 7;
const int debug_lan            = dstart << 8;
const int debug_mathcards      = dstart << 9;
const int debug_highscore      = dstart << 10;
const int debug_lessons        = dstart << 11;

/* extern'd constants */

const char* const OPTION_TEXT[NUM_GLOBAL_OPTS+1] = {
    "PER_USER_CONFIG",
    "USE_SOUND",
    "MENU_SOUND",
    "MENU_MUSIC",
    "FULLSCREEN",
    "USE_KEYPAD",
    "USE_IGLOOS",
    "USE_TTS",
    "END_OF_OPTS"
};

const int DEFAULT_GLOBAL_OPTS[NUM_GLOBAL_OPTS] = {
    1,
    1,
    1,
    1,
    1,
    0,
    1,
    1
};


/* file scope only now that accessor functions used: */
static game_option_type* game_options = NULL;
static global_option_type* global_options = NULL;

/* windowed mode screen  size */
extern int win_res_x;
extern int win_res_y;

/*local function prototypes: */
static int int_to_bool(int i);
//static int find_and_set_option(const char* name, int val);


/********************************************************************/
/*  "Public Methods" for options struct:                            */
/********************************************************************/

int Opts_Initialize(void)
{
    int i;
    /* Only allocate game_options if not already done: */
    if(!game_options)
        game_options = (game_option_type*)malloc(sizeof(game_option_type));
    else
        return 1;

    /* bail out if somehow malloc failed: */
    if (!game_options)
        return 0;

    /* Same routine for global options: */
    if(!global_options)
        global_options = (global_option_type*)malloc(sizeof(global_option_type));
    if(!global_options)
        return 0;

    /* set global program options */
    for (i = 0; i < NUM_GLOBAL_OPTS; ++i)
        global_options->iopts[i] = DEFAULT_GLOBAL_OPTS[i];

    /* set general game options */
    global_options->iopts[PER_USER_CONFIG] = DEFAULT_PER_USER_CONFIG;
    global_options->iopts[USE_SOUND] = DEFAULT_USE_SOUND;
    global_options->iopts[MENU_SOUND] = DEFAULT_MENU_SOUND;
    global_options->iopts[MENU_MUSIC] = DEFAULT_MENU_MUSIC;
    global_options->iopts[FULLSCREEN] = DEFAULT_FULLSCREEN;
    global_options->iopts[USE_KEYPAD] = DEFAULT_USE_KEYPAD;
    global_options->iopts[USE_IGLOOS] = DEFAULT_USE_IGLOOS;
    strncpy(game_options->lesson_title, DEFAULT_LESSON_TITLE,
            sizeof(game_options->lesson_title));
    strncpy(game_options->current_font_name, DEFAULT_FONT_NAME,
            sizeof(game_options->current_font_name));
    game_options->lan_mode = DEFAULT_LAN_MODE;
    game_options->use_bkgd = DEFAULT_USE_BKGD;
    game_options->help_mode = DEFAULT_HELP_MODE;
    game_options->demo_mode = DEFAULT_DEMO_MODE;
    game_options->oper_override = DEFAULT_OPER_OVERRIDE;
    game_options->allow_pause = DEFAULT_ALLOW_PAUSE;
    game_options->bonus_comet_interval = DEFAULT_BONUS_COMET_INTERVAL;
    game_options->bonus_speed_ratio = DEFAULT_BONUS_SPEED_RATIO;
    game_options->speed = DEFAULT_SPEED;
    game_options->allow_speedup = DEFAULT_ALLOW_SPEEDUP;
    game_options->speedup_factor = DEFAULT_SPEEDUP_FACTOR;
    game_options->max_speed = DEFAULT_MAX_SPEED;
    game_options->slow_after_wrong = DEFAULT_SLOW_AFTER_WRONG;
    game_options->starting_comets = DEFAULT_STARTING_COMETS;
    game_options->extra_comets_per_wave = DEFAULT_EXTRA_COMETS_PER_WAVE;
    game_options->max_comets = DEFAULT_MAX_COMETS;
    game_options->use_powerup_comets = DEFAULT_USE_POWERUP_COMETS;
    game_options->powerup_freq = DEFAULT_POWERUP_FREQ;
    game_options->save_summary = DEFAULT_SAVE_SUMMARY;
    game_options->sound_hw_available = DEFAULT_SOUND_HW_AVAILABLE;
    game_options->use_feedback = DEFAULT_USE_FEEDBACK;
    game_options->danger_level = DEFAULT_DANGER_LEVEL;
    game_options->danger_level_speedup = DEFAULT_DANGER_LEVEL_SPEEDUP;
    game_options->danger_level_max = DEFAULT_DANGER_LEVEL_MAX;
    game_options->city_expl_handicap = DEFAULT_CITY_EXPL_HANDICAP;
    game_options->last_score = DEFAULT_LAST_SCORE;

    game_options->num_cities = DEFAULT_NUM_CITIES;   /* MUST BE AN EVEN NUMBER! */
    game_options->max_city_colors = DEFAULT_MAX_CITY_COLORS;

    game_options->fps_limit = DEFAULT_FPS_LIMIT;
    game_options->w_width = DEFAULT_WINDOW_WIDTH;
    game_options->w_height = DEFAULT_WINDOW_HEIGHT;
    game_options->custom_res = DEFAULT_CUSTOM_RES;

    DEBUGCODE(debug_options)
        print_game_options(stdout, 0);



    return 1;
}


void Opts_Cleanup(void)
{
    if (game_options)
    {
        free(game_options);
        game_options = 0;
    }
}


//* "Set" functions for tuxmath options struct: */
unsigned int Opts_MapTextToIndex(const char* text)
{
    int i;
    for (i = 0; i < NUM_GLOBAL_OPTS; ++i)
    {
        if (0 == strcasecmp(text, OPTION_TEXT[i]) )
        {
            return i;
        }
    }
    DEBUGMSG(debug_options, "'%s' isn't a global option\n", text);
    return -1;
}

int Opts_GetGlobalOp(const char* text)
{
    int index = Opts_MapTextToIndex(text);
    if (index < NUM_GLOBAL_OPTS)
        return Opts_GetGlobalOpt(index);
    return 0;
}

int Opts_GetGlobalOpt(unsigned int index)
{
    if (index < NUM_GLOBAL_OPTS)
        return global_options->iopts[index];
    DEBUGMSG(debug_options, "Invalid global option index: %d\n", index);
    return 0;
}

void Opts_SetGlobalOp(const char* text, int val)
{
    int index = Opts_MapTextToIndex(text);
    if (index < NUM_GLOBAL_OPTS)
        Opts_SetGlobalOpt(index, val);
}

void Opts_SetGlobalOpt(unsigned int index, int val)
{
    if (index < NUM_GLOBAL_OPTS)
    {
        //Need to make sure we don't turn on sound options if sound
        //system couldn't be set up:
        if(index == USE_SOUND || index == MENU_SOUND || index == MENU_MUSIC)
        {
            if(!Opts_SoundHWAvailable())
                val = 0;
        }
        if (index == USE_TTS)
			text_to_speech_status = val;

        global_options->iopts[index] = val;
    }
    else
        DEBUGMSG(debug_options, "Invalid global option index: %d\n", index);
}


//void Opts_SetPerUserConfig(int val)
//{
//  global_options->iopts[PER_USER_CONFIG] = int_to_bool(val);
//}
//
//
//void Opts_SetUseSound(int val)
//{
//  if (val == -1)
//    global_options->iopts[USE_SOUND] = val;
//  else if (global_options->iopts[USE_SOUND] != -1)
//    global_options->iopts[USE_SOUND] = int_to_bool(val);
//}
//
//
//void Opts_SetMenuSound(int val)
//{
//  global_options->iopts[MENU_SOUND] = int_to_bool(val);
//}
//
//
//void Opts_SetMenuMusic(int val)
//{
//  global_options->iopts[MENU_MUSIC] = int_to_bool(val);
//}


///* FIXME need to actually change screen resolution when this is called */
//void Opts_SetFullscreen(int val)
//{
//  global_options->iopts[FULLSCREEN] = int_to_bool(val);
//}

void Opts_SetLanMode(int val)
{
    game_options->lan_mode = int_to_bool(val);
}

void Opts_SetLessonTitle(char* title)
{
    if (title && title[0] != '\0')
        strncpy(game_options->lesson_title, title, sizeof(game_options->lesson_title));
}

void Opts_SetFontName(char* font_name)
{
    if (font_name && font_name[0] != '\0')
        strncpy(game_options->current_font_name, font_name, sizeof(game_options->current_font_name));
}

void Opts_SetUseBkgd(int val)
{
    game_options->use_bkgd = int_to_bool(val);
}


void Opts_SetHelpMode(int val)
{
    game_options->help_mode = int_to_bool(val);
}


void Opts_SetDemoMode(int val)
{
    game_options->demo_mode = int_to_bool(val);
}


void Opts_SetOperOverride(int val)
{
    game_options->oper_override = int_to_bool(val);
}


//void Opts_SetUseKeypad(int val)
//{
//  global_options->iopts[USE_KEYPAD] = int_to_bool(val);
//}


void Opts_SetAllowPause(int val)
{
    game_options->allow_pause = int_to_bool(val);
}


//void Opts_SetUseIgloos(int val)
//{
//  global_options->iopts[USE_IGLOOS] = int_to_bool(val);
//}


void Opts_SetBonusCometInterval(int val)
{
    if (val < 0)
        val = 0;
    game_options->bonus_comet_interval = val;
}


void Opts_SetBonusSpeedRatio(float val)
{
    if (val < 1)
    {
        val = 1;
        fprintf(stderr,"bonus_speed_ratio must be at least 1, resetting accordingly.\n");
    }
    if (val > MAX_BONUS_SPEED_RATIO)
    {
        val = MAX_BONUS_SPEED_RATIO;
        fprintf(stderr,"Warning: requested bonus_speed_ratio above maximum, setting to %g.\n",MAX_BONUS_SPEED_RATIO);
    }
    game_options->bonus_speed_ratio = val;
}


void Opts_SetSpeed(float val)
{
    if (val < MINIMUM_SPEED)
    {
        val = MINIMUM_SPEED;
        fprintf(stderr,"Warning: requested speed below minimum, setting to %g.\n",MINIMUM_SPEED);
    }
    if (val > MAX_MAX_SPEED)
    {
        val = MAX_MAX_SPEED;
        fprintf(stderr,"Warning: requested speed above Tuxmath's maximum, setting to %g.\n",MAX_MAX_SPEED);
    }
    if (val > Opts_MaxSpeed())
    {
        val = Opts_MaxSpeed();
        fprintf(stderr,"Warning: requested speed above currently selected maximum, setting to %g.\n",
                Opts_MaxSpeed());
    }
    game_options->speed = val;
}


void Opts_SetAllowSpeedup(int val)
{
    game_options->allow_speedup = int_to_bool(val);
}


void Opts_SetSpeedupFactor(float val)
{
    if (val < MIN_SPEEDUP_FACTOR)
    {
        val = MIN_SPEEDUP_FACTOR;
        fprintf(stderr,"Warning: requested speedup factor below Tuxmath's minimum, setting to %g.\n",MIN_SPEEDUP_FACTOR);
    }
    if (val > MAX_SPEEDUP_FACTOR)
    {
        val = MAX_SPEEDUP_FACTOR;
        fprintf(stderr,"Warning: requested speedup factor above Tuxmath's maximum, setting to %g.\n",MAX_SPEEDUP_FACTOR);
    }
    game_options->speedup_factor = val;
}


void Opts_SetMaxSpeed(float val)
{
    if (val < MINIMUM_SPEED)
    {
        val = MINIMUM_SPEED;
        fprintf(stderr,"Warning: requested max speed below minimum, setting to %g.\n",
                MINIMUM_SPEED);
    }
    if (val > MAX_MAX_SPEED)
    {
        val = MAX_MAX_SPEED;
        fprintf(stderr,"Warning: requested max speed above Tuxmath's maximum, setting to %g.\n",
                MAX_MAX_SPEED);
    }
    if (val < Opts_Speed())
    {
        val = Opts_Speed();
        fprintf(stderr,"Warning: requested max speed less than current speed, setting to %g.\n",
                Opts_MaxSpeed());
    }
    game_options->max_speed = val;
}


void Opts_SetSlowAfterWrong(int val)
{
    game_options->slow_after_wrong = int_to_bool(val);
}


void Opts_SetStartingComets(int val)
{
    if (val < MIN_COMETS)
    {
        val = MIN_COMETS;
        fprintf(stderr,"Warning: requested starting comets below Tuxmath's minimum, setting to %d.\n",
                MIN_COMETS);
    }
    if (val > MAX_MAX_COMETS)
    {
        val = MAX_MAX_COMETS;
        fprintf(stderr,"Warning: requested starting comets above Tuxmath's maximum, setting to %d.\n",
                MAX_MAX_COMETS);
    }
    if (val > Opts_MaxComets())
    {
        val = Opts_MaxComets();
        fprintf(stderr,"Warning: requested starting comets above currently selected maximum, setting to %d.\n",
                Opts_MaxComets());
    }
    game_options->starting_comets = val;
}


void Opts_SetExtraCometsPerWave(int val)
{
    if (val < 0)
    {
        val = 0;
        fprintf(stderr,"Warning: requested extra comets below Tuxmath's minimum, setting to %d.\n",
                0);
    }
    if (val > MAX_MAX_COMETS)
    {
        val = MAX_MAX_COMETS;
        fprintf(stderr,"Warning: requested extra comets above Tuxmath's maximum, setting to %d.\n",
                MAX_MAX_COMETS);
    }
    if (val > Opts_MaxComets())
    {
        val = Opts_MaxComets();
        fprintf(stderr,"Warning: requested extra comets above currently selected maximum, setting to %d.\n",
                Opts_MaxComets());
    }
    game_options->extra_comets_per_wave = val;
}


void Opts_SetMaxComets(int val)
{
    if (val < MIN_COMETS)
    {
        val = MIN_COMETS;
        fprintf(stderr,"Warning: requested max comets below Tuxmath's minimum, setting to %d.\n",
                MIN_COMETS);
    }
    if (val > MAX_MAX_COMETS)
    {
        val = MAX_MAX_COMETS;
        fprintf(stderr,"Warning: requested max comets above Tuxmath's maximum, setting to %d.\n",
                MAX_MAX_COMETS);
    }
    game_options->max_comets = val;
}

void Opts_SetUsePowerupComets(int val)
{
    game_options->use_powerup_comets = int_to_bool(val);
}

void Opts_SetPowerupFreq(int val)
{
    if(val > 0)
        game_options->powerup_freq = val;
    else
        fprintf(stderr,"Warning: requested powerup frequency illegal, setting to %d.\n",
                DEFAULT_POWERUP_FREQ);

}


void Opts_SetNextMission(char* str)
{
    int len = strlen(str);
    if (len < PATH_MAX)
    {
        strcpy(game_options->next_mission, str);
    }
    else
    {
        fprintf(stderr,"Warning: Opts_SetNextMission() - string invalid or overflow\n");
    }
}


void Opts_SetSaveSummary(int val)
{
    game_options->save_summary = int_to_bool(val);
}


void Opts_SetUseFeedback(int val)
{
    game_options->use_feedback = int_to_bool(val);
}


void Opts_SetDangerLevel(float val)
{
    if (val < 0)
    {
        val = 0;
        fprintf(stderr,"Warning: danger level must be between 0 and 1, setting to 0.\n");
    }
    if (val > 1)
    {
        val = 1;
        fprintf(stderr,"Warning: danger level must be between 0 and 1, setting to 1.\n");
    }
    game_options->danger_level = val;
}


void Opts_SetDangerLevelSpeedup(float val)
{
    if (val < 1)
    {
        val = 1;
        fprintf(stderr,"Warning: danger_level_speedup must be at least 1, setting to 1.\n");
    }
    game_options->danger_level_speedup = val;
}


void Opts_SetDangerLevelMax(float val)
{
    if (val < 0)
    {
        val = 0;
        fprintf(stderr,"Warning: danger level max must be between 0 and 1, setting to 0.\n");
    }
    if (val > 1)
    {
        val = 1;
        fprintf(stderr,"Warning: danger level max must be between 0 and 1, setting to 1.\n");
    }
    game_options->danger_level_max = val;
}


void Opts_SetCityExplHandicap(float val)
{
    if (val < 0)
    {
        val = 0;
        fprintf(stderr,"Warning: city_explode_handicap level set below minimum, setting to 0.\n");
    }
    game_options->city_expl_handicap = val;
}



/* whether sound system is successfully initialized and sound files loaded: */
/* this flag is set by the program, not the user, and is not in the config file. */
void Opts_SetSoundHWAvailable(int val)
{
    game_options->sound_hw_available = int_to_bool(val);
}

/* Allows game() to store score for high score table code: */
void Opts_SetLastScore(int val)
{
    game_options->last_score = val;
}

void Opts_SetKeepScore(int val)
{
    game_options->keep_score = val;
}

void Opts_SetFPSLimit(int val)
{
    if (val < 0)
    {
        val = 0;
        fprintf(stderr,"Warning: fps_limit level set below minimum, setting to 0 (no limit).\n");
    }
    game_options->fps_limit = val;
}


void Opts_SetWindowWidth(int val)
{
    game_options->w_width = val;
    game_options->custom_res = 1;

    if(val > 0)
    {
        win_res_x = val;
    }
}


void Opts_SetWindowHeight(int val)
{
    game_options->w_height = val;
    game_options->custom_res = 1;

    if(val > 0)
    {
        win_res_y = val;
    }
}


void Opts_SetTTSMode(int enabled)
{
    Opts_SetGlobalOpt(USE_TTS, enabled);
}


/* "Get" functions for tuxmath options struct: */
//int Opts_PerUserConfig(void)
//{
//  if (!game_options)
//  {
//    fprintf(stderr, "\nOpts_PerUserConfig(): game_options not valid!\n");
//    return GAME_OPTS_INVALID;
//  }
//  return global_options->iopts[PER_USER_CONFIG];
//}
//
//
//int Opts_UseSound(void)
//{
//  if (!game_options)
//  {
//    fprintf(stderr, "\nOpts_UseSound(): game_options not valid!\n");
//    return GAME_OPTS_INVALID;
//  }
//  return global_options->iopts[USE_SOUND] > 0;
//}
//
//
//int Opts_MenuSound(void)
//{
//  if (!game_options)
//  {
//    fprintf(stderr, "\nOpts_MenuSound(): game_options not valid!\n");
//    return GAME_OPTS_INVALID;
//  }
//  return global_options->iopts[MENU_SOUND];
//}
//
//
//int Opts_MenuMusic(void)
//{
//  if (!game_options)
//  {
//    fprintf(stderr, "\nOpts_MenuMusic(): game_options not valid!\n");
//    return GAME_OPTS_INVALID;
//  }
//  return global_options->iopts[MENU_MUSIC];
//}
//
//
//int Opts_Fullscreen(void)
//{
//  if (!game_options)
//  {
//    fprintf(stderr, "\nOpts_Fullscreen(): game_options not valid!\n");
//    return GAME_OPTS_INVALID;
//  }
//  return global_options->iopts[FULLSCREEN];
//}


int Opts_LanMode(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_LanMode(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->lan_mode;
}


const char* Opts_LessonTitle(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_LessonTitle(): game_options not valid!\n");
        return NULL;
    }
    return (const char*) game_options->lesson_title;
}


const char* Opts_FontName(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_FontName(): game_options not valid!\n");
        return NULL;
    }
    return (const char*) game_options->current_font_name;
}

int Opts_UseBkgd(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_UserBkgd(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->use_bkgd;
}


int Opts_HelpMode(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_HelpMode(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->help_mode;
}


int Opts_DemoMode(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_DemoMode(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->demo_mode;
}


int Opts_OperOverride(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_OperOverride(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->oper_override;
}


//int Opts_UseKeypad(void)
//{
//  if (!game_options)
//  {
//    fprintf(stderr, "\nOpts_UseKeypad(): game_options not valid!\n");
//    return GAME_OPTS_INVALID;
//  }
//  return global_options->iopts[USE_KEYPAD];
//}
//
//
int Opts_AllowPause(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_AllowPause(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->allow_pause;
}


//int Opts_UseIgloos(void)
//{
//  if (!game_options)
//  {
//    fprintf(stderr, "\nOpts_UseIgloos(): game_options not valid!\n");
//    return GAME_OPTS_INVALID;
//  }
//  return global_options->iopts[USE_IGLOOS];
//}

int Opts_BonusCometInterval(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_BonusCometInterval(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->bonus_comet_interval;
}


float Opts_BonusSpeedRatio(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_BonusSpeedRatio(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->bonus_speed_ratio;
}


float Opts_Speed(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_Speed(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->speed;
}


int Opts_AllowSpeedup(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_AllowSpeedup(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->allow_speedup;
}


float Opts_SpeedupFactor(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_SpeedupFactor(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->speedup_factor;
}


float Opts_MaxSpeed(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_MaxSpeed(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->max_speed;
}


int Opts_SlowAfterWrong(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_SlowAfterWrong(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->slow_after_wrong;
}


int Opts_StartingComets(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_StartingComets(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->starting_comets;
}


int Opts_ExtraCometsPerWave(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_ExtraCometsPerWave(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->extra_comets_per_wave;
}


int Opts_MaxComets(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_MaxComets(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->max_comets;
}

int Opts_UsePowerupComets(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_UsePowerupComets(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->use_powerup_comets;
}

int Opts_PowerupFreq(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_PowerupFreq(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->powerup_freq;
}

const char* Opts_NextMission(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_NextMission(): game_options not valid!\n");
        return NULL;
    }
    return (const char*) game_options->next_mission;
}


int Opts_SaveSummary(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_SaveSummary(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->save_summary;
}


int Opts_LastScore(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_LastScore(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->last_score;
}


int Opts_UseFeedback(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_UseFeedback(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->use_feedback;
}


float Opts_DangerLevel(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_DangerLevel(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->danger_level;
}


float Opts_DangerLevelSpeedup(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_DangerLevelSpeedup(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->danger_level_speedup;
}


float Opts_DangerLevelMax(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_DangerLevelMax(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->danger_level_max;
}


float Opts_CityExplHandicap(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_CityExplHandicap(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->city_expl_handicap;
}



/* whether sound system is successfully initialized and sound files loaded: */
/* this flag is set by the program, not the user, and is not in the config file. */
int Opts_SoundHWAvailable(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_SoundHWAvailable(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->sound_hw_available;
}


/* Returns true if only if the player wants to use sound */
/* and the sound system is actually available:           */
int Opts_UsingSound(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_UsingSound(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return (global_options->iopts[USE_SOUND]>0 && game_options->sound_hw_available);
}

int Opts_KeepScore(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_KeepScore(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->keep_score;
}


int Opts_FPSLimit(void)
{
    if (!game_options)
    {
        fprintf(stderr, "\nOpts_KeepScore(): game_options not valid!\n");
        return GAME_OPTS_INVALID;
    }
    return game_options->fps_limit;
}


int Opts_WindowWidth(void)
{
    return game_options->w_width;
}


int Opts_WindowHeight(void)
{
    return game_options->w_height;
}

int Opts_CustomRes(void)
{
    return game_options->custom_res;
}

/********************************************************************/
/*  "private methods" (static functions only visible in options.c)  */
/********************************************************************/





/* to prevent option settings in math_opts from getting set to */
/* values other than 0 or 1                                    */
int int_to_bool(int i)
{
    if (i)
        return 1;
    else
        return 0;
}

///* determine which option class a name belongs to, and set it */
///* accordingly. Returns 1 on success, 0 on failure            */
//static int find_and_set_option(const char* name, int val)
//{
//  int index = -1;
//
//  if ((index = MC_MapTextToIndex(name)) != -1) //is it a math opt?
//    MC_SetOpt(index, val);
//  else if ((index = Opts_MapTextToIndex(name)) != -1) //is it a global opt?
//    Opts_SetGlobalOpt(index, val);
//  else //no? oh well.
//    return 0;
//
//  return 1;
//}

/* prints struct to stream: */
void print_game_options(FILE* fp, int verbose)
{
    /* bail out if no struct */
    if (!game_options)
    {
        fprintf(stderr, "print_game_options(): invalid game_option_type struct");
        return;
    }

    if(verbose)
    {
        fprintf (fp, "\n############################################################\n"
                "#                                                          #\n"
                "#                 General Game Options                     #\n"
                "#                                                          #\n"
                "# The following options are boolean (true/false) variables #\n"
                "# that control various aspects of Tuxmath's behavior.      #\n"
                "# The program writes the values to the file as either '0'  #\n"
                "# or '1'. However, the program accepts 'n', 'no', 'f', and #\n"
                "# 'false' as synonyms for '0', and similarly accepts 'y',  #\n"
                "# 'yes', 't', and 'true' as synonyms for '1' (all case-    #\n"
                "# insensitive).                                            #\n"
                "############################################################\n\n");
    }

    if(verbose)
    {
        fprintf (fp, "############################################################\n"
                "# 'PER_USER_CONFIG' determines whether Tuxmath will look   #\n"
                "# in the user's home directory for settings. Default is 1  #\n"
                "# (yes). If deselected, the program will ignore the user's #\n"
                "# .tuxmath file and use the the global settings in the     #\n"
                "# installation-wide config file.                           #\n"
                "# This setting cannot be changed by an ordinary user.      #\n"
                "############################################################\n");
    }
    fprintf(fp, "PER_USER_CONFIG = %d\n", global_options->iopts[PER_USER_CONFIG]);

    if(verbose)
    {
        fprintf (fp, "\n# Self-explanatory, default is 1:\n");
    }
    fprintf(fp, "USE_SOUND = %d\n", global_options->iopts[USE_SOUND]>0);

    if(verbose)
    {
        fprintf (fp, "\n# Use FULLSCREEN at 640x480 resolution instead of\n"
                "640x480 window. Default is 1 (FULLSCREEN). Change to 0\n"
                "if SDL has trouble with FULLSCREEN on your system.\n");
    }
    fprintf(fp, "FULLSCREEN = %d\n", global_options->iopts[FULLSCREEN]);

    if(verbose)
    {
        fprintf (fp, "\n# Use 640x480 jpg image for background; default is 1.\n");
    }
    fprintf(fp, "use_bkgd = %d\n", game_options->use_bkgd);

    if(verbose)
    {
        fprintf (fp, "\n# Program runs as demo; default is 0.\n");
    }
    fprintf(fp, "demo_mode = %d\n", game_options->demo_mode);

    if(verbose)
    {
        fprintf (fp, "\n# Use operator selection from command line; default is 0.\n");
    }
    fprintf(fp, "oper_override = %d\n", game_options->oper_override);

    if(verbose)
    {
        fprintf (fp, "\n# Display onscreen numeric keypad; default is 0.\n");
    }
    fprintf(fp, "USE_KEYPAD = %d\n", global_options->iopts[USE_KEYPAD]);

    if(verbose)
    {
        fprintf (fp, "\n############################################################\n"
                "# The next settings determine the speed and number         #\n"
                "# of comets.  The speed settings are float numbers (mean-  #\n"
                "# ing decimals allowed). The comet settings are integers.  #\n"
                "#                                                          #\n"
                "# Starting comet speed and max comet speed are generally   #\n"
                "# applicable. The main choice is whether you want to use   #\n"
                "# feedback, i.e., to adjust the speed automatically based  #\n"
                "# on the player's performance.                             #\n"
                "#                                                          #\n"
                "# Without feedback, the speed increases by a user-         #\n"
                "# settable factor ('speedup_factor'), with an option       #\n"
                "# ('slow_after_wrong') to go back to the starting speed    #\n"
                "# when a city gets hit.                                    #\n"
                "#                                                          #\n"
                "# With feedback, you set a desired 'danger level,' which   #\n"
                "# determines how close the comets should typically         #\n"
                "# approach the cities before the player succeeds in        #\n"
                "# destroying them.  The game will adjust its speed         #\n"
                "# accordingly, getting faster when the player is easily    #\n"
                "# stopping the comets, and slowing down when there are     #\n"
                "# too many close calls or hits. You can also have the      #\n"
                "# danger level increase with each wave.                    #\n"
                "############################################################\n");
    }

    if(verbose)
    {
        fprintf (fp, "\n# Whether to increase speed and number of comets with \n"
                "# each wave.  May want to turn this off for smaller kids.\n"
                "# Default is 1 (allow game to speed up)\n");
    }
    fprintf(fp, "allow_speedup = %d\n", game_options->allow_speedup);


    fprintf(fp, "slow_after_wrong = %d\n", game_options->slow_after_wrong);

    if(verbose)
    {
        fprintf (fp, "\n# Starting comet speed. Default is 1.\n");
    }
    fprintf(fp, "speed = %f\n", game_options->speed);

    if(verbose)
    {
        fprintf (fp, "\n# If feedback is not used but 'allow_speedup' is\n"
                "# enabled, the comet speed will be\n"
                "# multiplied by this factor with each new wave.\n"
                "# Default is 1.2 (i.e. 20 percent increase per wave)\n");
    }
    fprintf(fp, "speedup_factor = %f\n", game_options->speedup_factor);

    if(verbose)
    {
        fprintf (fp, "\n# Maximum speed. Default is 10.\n");
    }
    fprintf(fp, "max_speed = %f\n", game_options->max_speed);

    if(verbose)
    {
        fprintf (fp, "\n# Number of comets for first wave. Default is 2.\n");
    }
    fprintf(fp, "starting_comets = %d\n", game_options->starting_comets);

    if(verbose)
    {
        fprintf (fp, "\n# Comets to add for each successive wave. Default is 2.\n");
    }
    fprintf(fp, "extra_comets_per_wave = %d\n", game_options->extra_comets_per_wave);

    if(verbose)
    {
        fprintf (fp, "\n# Maximum number of comets. Default is 10.\n");
    }
    fprintf(fp, "max_comets = %d\n", game_options->max_comets);

    if(verbose)
    {
        fprintf (fp, "\n# Use feedback? Default (for now) is false, 0.\n");
    }
    fprintf(fp, "use_feedback = %d\n", game_options->use_feedback);


    if(verbose)
    {
        fprintf (fp, "\n# (Feedback) Set the desired danger level.\n"
                "# 0 = too safe, comets typically exploded right at the very top\n"
                "# 1 = too dangerous, comets typically exploded at the moment they hit cities\n"
                "# Set it somewhere between these extremes. As a guideline, early\n"
                "# elementary kids might feel comfortable around 0.2-0.3, older kids\n"
                "# at around 0.4-0.6. Default 0.35.\n");
    }
    fprintf(fp, "danger_level = %f\n", game_options->danger_level);

    if(verbose)
    {
        fprintf (fp, "\n# (Feedback) Set danger level speedup.\n"
                "# The margin of safety will decrease by this factor each wave.\n"
                "# Default 1.1. Note 1 = no increase in danger level.\n");
    }
    fprintf(fp, "danger_level_speedup = %f\n", game_options->danger_level_speedup);

    if(verbose)
    {
        fprintf (fp, "\n# (Feedback) Set the maximum danger level.\n"
                "# Default 0.9.\n");
    }
    fprintf(fp, "danger_level_max = %f\n", game_options->danger_level_max);

    if (verbose)
    {
        fprintf (fp, "\n# (Feedback) Set the handicap for hitting cities.\n"
                "# When bigger than 0, this causes the game to slow down\n"
                "# by an extra amount after a wave in which one or more\n"
                "# cities get hit. Note that this is similar to slow_after_wrong,\n"
                "# but allows for more gradual changes.\n"
                "# Default 0 (no extra handicap).\n");
    }
    fprintf(fp, "city_explode_handicap = %f\n", game_options->city_expl_handicap);

    /*
       fprintf(fp, "num_cities = %d\n", game_options->num_cities);
       fprintf(fp, "num_bkgds = %d\n", game_options->num_bkgds);
       fprintf(fp, "max_city_colors = %d\n", game_options->max_city_colors);
       */
}
