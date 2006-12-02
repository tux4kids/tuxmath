/*
  options.c

  For TuxMath
  The options screen loop.

  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/


  Part of "Tux4Kids" Project
  http://www.tux4kids.org/
      
  August 26, 2001 - January 3, 2005
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "mathcards.h"

#include "options.h"
#include "fileops.h"
#include "setup.h"
#include "playsound.h"
#include "game.h"
#include "tuxmath.h"

/* FIXME figure out what oper_override is supposed to do and make sure */
/* this file behaves accordingly! */

int opers[NUM_OPERS], range_enabled[NUM_Q_RANGES];



/* Local (to options.c) 'globals': */
/* moved to file scope to allow update_selected_option() to use them DSB */
static SDL_Rect dest;
static char tmp_str[10];
static unsigned char range_bits;
static int i, j, x, y;

/* file scope only now that accessor functions used: */
static game_option_type* game_options;

/*local function prototypes: */
static void update_selected_option(int option);
static int int_to_bool(int i);


/* Main options() function - called from title(): */
int options(void)
{
  int opt, old_opt, done, quit, img, blinking;
  SDL_Event event;
  Uint32 last_time, now_time;
  SDLKey key;
  
  
  /* Clear window: */
  
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

  dest.x = (screen->w - images[IMG_OPTIONS]->w) / 2;
  dest.y = 0;
  SDL_BlitSurface(images[IMG_OPTIONS], NULL, screen, &dest);

  /* Syncrhonize old opers[] array with selections from MathCards */
  opers[OPT_OP_ADD] = MC_AddAllowed();
  opers[OPT_OP_SUB] = MC_SubAllowed();
  opers[OPT_OP_MUL] = MC_MultAllowed();
  opers[OPT_OP_DIV] = MC_DivAllowed();

  /* FIXME range_enabled[] and oper_override will be going away */
  for (i = 0; i < NUM_Q_RANGES; i++)
  { 
    range_enabled[i] = 1;
  }

  /* Draw options: */

  for (i = 0; i < NUM_OPTS; i++)
  {
    y = (images[IMG_OPTIONS]->h + 8 + (i * images[IMG_TUX_HELMET1]->h));
    
    dest.x = 32 + images[IMG_TUX_HELMET1]->w + 4;
    dest.y = y;
    
    SDL_BlitSurface(images[IMG_OPT_ADDITION + i], NULL, screen, &dest);


    /* Checkmarks for operators: */

    if (i >= OPT_OP_ADD && i < OPT_OP_ADD + NUM_OPERS)
    {
      dest.x = screen->w - images[IMG_OPT_CHECK]->w - 16;
      dest.y = y;
      
      SDL_BlitSurface(images[IMG_OPT_CHECK + opers[i]], NULL, screen, &dest);
    }
    else if (i == OPT_A_MAX)
    {
      /* Maximum answer: */

      snprintf(tmp_str, sizeof(tmp_str), "%04d", MC_MaxAnswer());
      draw_nums(tmp_str,
		screen->w - ((images[IMG_NUMS]->w / 14) * 2) - 16,
		y + images[IMG_OPT_MAX_ANSWER]->h);

      /* Using "* 2" instead of "* 4" (even though string is 4 digits long)
         because "draw_nums()" actually centers around x; not left-justifies */
    }
    else if (i == OPT_A_SPEED)
    {
      /* Speed control: */

      snprintf(tmp_str, sizeof(tmp_str), "%.1f", game_options->speed);
      draw_nums(tmp_str,
		screen->w - ((images[IMG_NUMS]->w / 14) * 2) - 16,
		y + images[IMG_OPT_SPEED]->h);

      /* Using "* 2" instead of "* 4" (even though string is 4 digits long)
         because "draw_nums()" actually centers around x; not left-justifies */
    }
    else if (i == OPT_Q_RANGE)
    {
      x = 32 + images[IMG_TUX_HELMET1]->w + 4 + 64;
      y = y + images[IMG_TUX_HELMET1]->h;

      for (j = 0; j < NUM_Q_RANGES; j++)
      {
	dest.x = x;
	dest.y = y;

	SDL_BlitSurface(images[IMG_OPT_RNG_1_5 + j * 2 + range_enabled[j]],
			NULL,
			screen, &dest);

	x = x + images[IMG_OPT_RNG_1_5 + j * 2 + range_enabled[j]]->w + 16;
      }
    }
  }


  /* Get bits of what ranges are available: */

  range_bits = 0;
  for (j = 0; j < NUM_Q_RANGES; j++)
  {
    range_bits = range_bits << 1;
    if (range_enabled[j])
      range_bits = range_bits | 1;
  }

  
  /* --- MAIN OPTIONS SCREEN LOOP: --- */

  blinking = 0;
  opt = 0;
  done = 0;
  quit = 0;
  
  do
    {
      last_time = SDL_GetTicks();
      old_opt = opt;
      
      
      /* Handle any incoming events: */
      
      while (SDL_PollEvent(&event) > 0)
	{
	  if (event.type == SDL_QUIT)
	    {
	      /* Window close event - quit! */
	      
	      quit = 1;
	      done = 1;
	    }

	  else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
              /* FIXME Options screen should have "OK" button to accept choices with mouse DSB */
              /* figure out opt based on mouse coordinates */
              if (/* event.button.x >= left && */ /* don't see any reason to filter on x values here*/
	          /* event.button.x <= left + width && */  
		  event.button.y >= images[IMG_OPTIONS]->h + 8 &&
		  event.button.y <= (images[IMG_OPTIONS]->h + 8 +
		                   NUM_OPTS * images[IMG_TUX_HELMET1]->h))
	      {
	        opt = ((event.button.y - (images[IMG_OPTIONS]->h + 8)) /
	               images[IMG_TUX_HELMET1]->h);
                update_selected_option(opt);
	      }
              /* handle ranges below list as last option */
              if (event.button.y > (images[IMG_OPTIONS]->h + 8 +
	      	                   NUM_OPTS * images[IMG_TUX_HELMET1]->h)
                  &&

                  event.button.y <= (images[IMG_OPTIONS]->h + 8 +
	      	                    NUM_OPTS * images[IMG_TUX_HELMET1]->h)
                                    + (images[IMG_OPT_RNG_1_5]->h))                          
              { 
                opt = OPT_Q_RANGE;
                update_selected_option(opt);
              }
            }

	  else if (event.type == SDL_KEYDOWN)
	    {
	      key = event.key.keysym.sym;
	      
	      if (key == SDLK_ESCAPE)
		{
                  /* Don't leave options screen if all operations deselected! - DSB */
                  /* fixes Debian bug #336272 */
                  int i;
                  int at_least_one_oper = 0;
                  for (i =0; i < NUM_OPERS; i++)
                  {
                    if (opers[i])
                      at_least_one_oper = 1;
                  }
                  if (at_least_one_oper)
                    done = 1;
		}
	      else if (key == SDLK_DOWN)
		{
		  if (opt < NUM_OPTS - 1)
		  {
		    opt++;

		    playsound(SND_POP);
		  }
		}
	      else if (key == SDLK_UP)
		{
		  if (opt > 0)
		  {
		    opt--;
		    playsound(SND_POP);
		  }
		}
	      else if (key == SDLK_RETURN ||
		       key == SDLK_KP_ENTER ||
		       key == SDLK_SPACE)
              {
              /* code moved into function taking opt as argument so the same code can be   */
              /* used to handle mouse events DSB */
                update_selected_option(opt);
              }
	    }
	}
      
      
      /* Erase Tux (cursor) */
      
      if (opt != old_opt)
	{
	  blinking = 0;
	  
	  dest.x = 32;
	  dest.y = (images[IMG_OPTIONS]->h + 8 + 
		    (old_opt * images[IMG_TUX_HELMET1]->h));
	  dest.w = images[IMG_TUX_HELMET1]->w;
	  dest.h = images[IMG_TUX_HELMET1]->h;
	  
	  SDL_FillRect(screen, &dest,
		       SDL_MapRGB(screen->format, 0, 0, 0));
	}
      
      
      /* Handling Tux (cursor) blinking: */
      
      if ((rand() % 50) == 0 && blinking == 0)
	{
	  blinking = 6;
	}
      
      if (blinking > 0)
	blinking--;
      
      
      /* Draw Tux (cursor) */
      
      dest.x = 32;
      dest.y = images[IMG_OPTIONS]->h + 8 + (opt * images[IMG_TUX_HELMET1]->h);
      dest.w = images[IMG_TUX_HELMET1]->w;
      dest.h = images[IMG_TUX_HELMET1]->h;
      
      img = IMG_TUX_HELMET1;
      
      if (blinking >= 4 || (blinking >= 1 && blinking < 2))
	img = IMG_TUX_HELMET2;
      else if (blinking >= 2 && blinking < 4)
	img = IMG_TUX_HELMET3;
     
      SDL_BlitSurface(images[img], NULL, screen, &dest);
      
      SDL_Flip(screen);

      
      /* Pause (keep frame-rate event) */
      
      now_time = SDL_GetTicks();
      if (now_time < last_time + (1000 / 20))
	{
	  SDL_Delay(last_time + (1000 / 20) - now_time);
	}
    }
  while (!done);


  /* Return the chosen command: */
  
  return quit;
}

/********************************************************************/
/*  "Public Methods" for options struct:                            */
/********************************************************************/

int Opts_Initialize(void)
{
  game_options = malloc(sizeof(game_option_type));
  /* bail out if no struct */
  if (!game_options)
    return 0;

  /* set general game options */
  game_options->per_user_config = DEFAULT_PER_USER_CONFIG;
  game_options->use_sound = DEFAULT_USE_SOUND;
  game_options->fullscreen = DEFAULT_FULLSCREEN;
  game_options->use_bkgd = DEFAULT_USE_BKGD;
  game_options->demo_mode = DEFAULT_DEMO_MODE;
  game_options->oper_override = DEFAULT_OPER_OVERRIDE;
  game_options->use_keypad = DEFAULT_USE_KEYPAD;
  game_options->speed = DEFAULT_SPEED;
  game_options->allow_speedup = DEFAULT_ALLOW_SPEEDUP;
  game_options->speedup_factor = DEFAULT_SPEEDUP_FACTOR;
  game_options->max_speed = DEFAULT_MAX_SPEED;
  game_options->slow_after_wrong = DEFAULT_SLOW_AFTER_WRONG;
  game_options->starting_comets = DEFAULT_STARTING_COMETS;
  game_options->extra_comets_per_wave = DEFAULT_EXTRA_COMETS_PER_WAVE;
  game_options->max_comets = DEFAULT_MAX_COMETS;
  game_options->save_summary = DEFAULT_SAVE_SUMMARY;
  game_options->sound_hw_available = DEFAULT_SOUND_HW_AVAILABLE;
  game_options->use_feedback = DEFAULT_USE_FEEDBACK;
  game_options->danger_level = DEFAULT_DANGER_LEVEL;
  game_options->danger_level_speedup = DEFAULT_DANGER_LEVEL_SPEEDUP;
  game_options->danger_level_max = DEFAULT_DANGER_LEVEL_MAX;
  game_options->city_expl_handicap = DEFAULT_CITY_EXPL_HANDICAP;

  game_options->num_cities = DEFAULT_NUM_CITIES;   /* MUST BE AN EVEN NUMBER! */
  game_options->num_bkgds = DEFAULT_NUM_BKGDS;
  game_options->max_city_colors = DEFAULT_MAX_CITY_COLORS;

  #ifdef TUXMATH_DEBUG
  print_game_options(stdout, 0);
  #endif

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


/* "Set" functions for tuxmath options struct: */
void Opts_SetPerUserConfig(int val)
{
  game_options->per_user_config = int_to_bool(val);
}


void Opts_SetUseSound(int val)
{
  game_options->use_sound = int_to_bool(val);
}


void Opts_SetFullscreen(int val)
{
  game_options->fullscreen = int_to_bool(val);
}


void Opts_SetUseBkgd(int val)
{
  game_options->use_bkgd = int_to_bool(val);
}


void Opts_SetDemoMode(int val)
{
  game_options->demo_mode = int_to_bool(val);
}


void Opts_SetOperOverride(int val)
{
  game_options->oper_override = int_to_bool(val);
}


void Opts_SetUseKeypad(int val)
{
  game_options->use_keypad = int_to_bool(val);
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



/* "Get" functions for tuxmath options struct: */
int Opts_PerUserConfig(void)
{
  if (!game_options)
  {
    fprintf(stderr, "\nOpts_PerUserConfig(): game_options not valid!\n");
    return GAME_OPTS_INVALID;
  }
  return game_options->per_user_config;
}


int Opts_UseSound(void)
{
  if (!game_options)
  {
    fprintf(stderr, "\nOpts_UseSound(): game_options not valid!\n");
    return GAME_OPTS_INVALID;
  }
  return game_options->use_sound;

}


int Opts_Fullscreen(void)
{
  if (!game_options)
  {
    fprintf(stderr, "\nOpts_Fullscreen(): game_options not valid!\n");
    return GAME_OPTS_INVALID;
  }
  return game_options->fullscreen;
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


int Opts_UseKeypad(void)
{
  if (!game_options)
  {
    fprintf(stderr, "\nOpts_UseKeypad(): game_options not valid!\n");
    return GAME_OPTS_INVALID;
  }
  return game_options->use_keypad;
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


char* Opts_NextMission(void)
{
  char* str;
  int length;
  length = strlen(game_options->next_mission);
  str = malloc((length * sizeof(char)) + 1);
  strcpy(str, game_options->next_mission);
  return str;
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
  return (game_options->use_sound && game_options->sound_hw_available);
}

/********************************************************************/
/*  "private methods" (static functions only visible in options.c)  */
/********************************************************************/


/* can be called by either keystroke or mouse click - moved into */
/* separate function to reduce code duplication - DSB */
static void update_selected_option(int option)
{
  {
    if (option >= OPT_OP_ADD && option < OPT_OP_ADD + NUM_OPERS)
    {
      /* toggle selection of math operation - old opers array */
      /* FIXME opers[] to go away                             */
      opers[option - OPT_OP_ADD] = !opers[option - OPT_OP_ADD];
      /* toggle selection in new MathCards backend: */
      switch (option)
      {
        case OPT_OP_ADD:
        {
          MC_SetAddAllowed(!MC_AddAllowed());
          break;
	}
        case OPT_OP_SUB:
        {
          MC_SetSubAllowed(!MC_SubAllowed());
          break;
        }
        case OPT_OP_MUL:
        {
          MC_SetMultAllowed(!MC_MultAllowed());
          break;
        }
        case OPT_OP_DIV:
        {
          MC_SetDivAllowed(!MC_DivAllowed());
          break;
        }
      }
 
      dest.x = screen->w - images[IMG_OPT_CHECK]->w - 16;
      dest.y = (images[IMG_OPTIONS]->h + 8 +
               ((option - OPT_OP_ADD) *
               images[IMG_TUX_HELMET1]->h));

      SDL_BlitSurface(images[IMG_OPT_CHECK + opers[option - OPT_OP_ADD]],
                      NULL,
                      screen,
                      &dest);
    }

    else if (option == OPT_A_MAX)
    {
      MC_SetMaxAnswer((MC_MaxAnswer() * 2) / 3);
      if (MC_MaxAnswer() < 12)
        MC_SetMaxAnswer(144);

      dest.x = screen->w - ((images[IMG_NUMS]->w / 14) * 4) - 16;
      dest.y = (images[IMG_OPTIONS]->h + 8 +
               ((option - OPT_OP_ADD) *
               images[IMG_TUX_HELMET1]->h));
      dest.w = ((images[IMG_NUMS]->w / 14) * 4);
      dest.h = images[IMG_OPT_MAX_ANSWER]->h;

      SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 0, 0));
      snprintf(tmp_str, sizeof(tmp_str), "%04d", MC_MaxAnswer());
      draw_nums(tmp_str,
                screen->w - ((images[IMG_NUMS]->w / 14) * 2) - 16,
                (images[IMG_OPTIONS]->h + 8 +
                ((option - OPT_OP_ADD) *
                images[IMG_TUX_HELMET1]->h)) +
                images[IMG_OPT_MAX_ANSWER]->h);
    }

    else if (option == OPT_A_SPEED)
    {
      game_options->speed = game_options->speed - 0.1;
      if (game_options->speed < 0.1)
        game_options->speed = 5;

      dest.x = screen->w - ((images[IMG_NUMS]->w / 14) * 4) - 16;
      dest.y = (images[IMG_OPTIONS]->h + 8 +
               ((option - OPT_OP_ADD) *
               images[IMG_TUX_HELMET1]->h));
      dest.w = ((images[IMG_NUMS]->w / 14) * 4);
      dest.h = images[IMG_OPT_SPEED]->h;

      SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 0, 0));

      snprintf(tmp_str, sizeof(tmp_str), "%.1f", game_options->speed);
      draw_nums(tmp_str,
                screen->w - ((images[IMG_NUMS]->w / 14) * 2) - 16,
                (images[IMG_OPTIONS]->h + 8 +
                ((option - OPT_OP_ADD) *
                images[IMG_TUX_HELMET1]->h)) +
                images[IMG_OPT_SPEED]->h);
    }

    else if (option == OPT_Q_RANGE)
    {
    /* FIXME question ranges now handled in MathCards */
    /* Change which ranges are available: */
      range_bits = range_bits + 1;
      if (range_bits >= (1 << NUM_Q_RANGES))
        range_bits = 1;

      for (j = 0; j < NUM_Q_RANGES; j++)
      {
        if ((range_bits & (1 << j)) != 0)
          range_enabled[j] = 1;
        else
          range_enabled[j] = 0;
      }

      /* Redraw ranges: */
      x = 32 + images[IMG_TUX_HELMET1]->w + 4 + 64;
      y = (images[IMG_OPTIONS]->h + 8 +
          (option * images[IMG_TUX_HELMET1]->h)) +
           images[IMG_TUX_HELMET1]->h;

      for (j = 0; j < NUM_Q_RANGES; j++)
      {
        dest.x = x;
        dest.y = y;

        SDL_BlitSurface(images[IMG_OPT_RNG_1_5 + j * 2 +
                        range_enabled[j]],
                        NULL,
                        screen, 
                        &dest);

        x = x
            + images[IMG_OPT_RNG_1_5 + j * 2 + range_enabled[j]]-> w 
            + 16;
      }
      
      /* update settings in MathCards: */
      {
        int lowest_range, highest_range, minimum, maximum;
        lowest_range = NUM_Q_RANGES - 1;
        highest_range = 0;

        /* find lowest and highest enabled ranges */
        for (j = 0; j < NUM_Q_RANGES; j++)
        {
          if (range_enabled[j] && j < lowest_range)
            lowest_range = j;
          if (range_enabled[j] && j > highest_range)
            highest_range = j;
        } 
        minimum = ranges[lowest_range].min;
        maximum = ranges[highest_range].max;

        /* update MathCards: */
        MC_SetAddMin(minimum);
        MC_SetAddMax(maximum);
        MC_SetSubMin(minimum);
        MC_SetSubMax(maximum);
        MC_SetMultMin(minimum);
        MC_SetMultMax(maximum);
        MC_SetDivMin(minimum);
        MC_SetDivMax(maximum);
      }
    }
 
    /* same sound for all option updates */
    playsound(SND_LASER);
  }
}


/* to prevent option settings in math_opts from getting set to */
/* values other than 0 or 1                                    */
int int_to_bool(int i)
{
  if (i)
    return 1;
  else
    return 0;
}


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
                 "# 'per_user_config' determines whether Tuxmath will look   #\n"
                 "# in the user's home directory for settings. Default is 1  #\n"
                 "# (yes). If deselected, the program will ignore the user's #\n"
                 "# .tuxmath file and use the the global settings in the     #\n"
                 "# installation-wide config file.                           #\n"
                 "# This setting cannot be changed by an ordinary user.      #\n"
                 "############################################################\n");
  }
  fprintf(fp, "per_user_config = %d\n", game_options->per_user_config);

  if(verbose)
  {
    fprintf (fp, "\n# Self-explanatory, default is 1:\n");
  }
  fprintf(fp, "use_sound = %d\n", game_options->use_sound);

  if(verbose)
  {
    fprintf (fp, "\n# Use fullscreen at 640x480 resolution instead of\n"
                 "640x480 window. Default is 1 (fullscreen). Change to 0\n"
                 "if SDL has trouble with fullscreen on your system.\n");
  } 
  fprintf(fp, "fullscreen = %d\n", game_options->fullscreen);

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
  fprintf(fp, "use_keypad = %d\n", game_options->use_keypad);

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
