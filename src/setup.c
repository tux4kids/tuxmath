/*
  setup.c

  For TuxMath
  Contains some globals (screen surface, images, some option flags, etc.)
  as well as the function to load data files (images, sounds, music)
  and display a "Loading..." screen.

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
#include <stdlib.h>

#include <SDL.h>
#ifndef NOSOUND
#include <SDL_mixer.h>
#endif
#include <SDL_image.h>

#include "mathcards.h"

#include "tuxmath.h"
#include "setup.h"
#include "images.h"
#include "sounds.h"
#include "game.h"


static char * image_filenames[NUM_IMAGES] = {
  DATA_PREFIX "/images/status/standby.png",
  DATA_PREFIX "/images/status/loading.png",
  DATA_PREFIX "/images/status/title.png",
  DATA_PREFIX "/images/status/options.png",
  DATA_PREFIX "/images/status/tux4kids.png",
  DATA_PREFIX "/images/status/nbs.png",
  DATA_PREFIX "/images/status/tux_helmet1.png",
  DATA_PREFIX "/images/status/tux_helmet2.png",
  DATA_PREFIX "/images/status/tux_helmet3.png", 
  DATA_PREFIX "/images/status/cmd_play.png",
  DATA_PREFIX "/images/status/cmd_options.png",
  DATA_PREFIX "/images/status/cmd_credits.png",
  DATA_PREFIX "/images/status/cmd_quit.png",
  DATA_PREFIX "/images/status/opt_addition.png",
  DATA_PREFIX "/images/status/opt_subtraction.png",
  DATA_PREFIX "/images/status/opt_multiplication.png",
  DATA_PREFIX "/images/status/opt_division.png",
  DATA_PREFIX "/images/status/opt_max_answer.png",
  DATA_PREFIX "/images/status/opt_speed.png",
  DATA_PREFIX "/images/status/opt_q_range.png",
  DATA_PREFIX "/images/status/opt_rng_1_5.png",
  DATA_PREFIX "/images/status/opt_rng_1_5_on.png",
  DATA_PREFIX "/images/status/opt_rng_6_12.png",
  DATA_PREFIX "/images/status/opt_rng_6_12_on.png",
  DATA_PREFIX "/images/status/opt_rng_13_20.png",
  DATA_PREFIX "/images/status/opt_rng_13_20_on.png",
  DATA_PREFIX "/images/status/opt_check.png",
  DATA_PREFIX "/images/status/opt_check_on.png",
  DATA_PREFIX "/images/cities/city-blue.png",
  DATA_PREFIX "/images/cities/csplode-blue-1.png",
  DATA_PREFIX "/images/cities/csplode-blue-2.png",
  DATA_PREFIX "/images/cities/csplode-blue-3.png",
  DATA_PREFIX "/images/cities/csplode-blue-4.png",
  DATA_PREFIX "/images/cities/csplode-blue-5.png",
  DATA_PREFIX "/images/cities/cdead-blue.png",
  DATA_PREFIX "/images/cities/city-green.png",
  DATA_PREFIX "/images/cities/csplode-green-1.png",
  DATA_PREFIX "/images/cities/csplode-green-2.png",
  DATA_PREFIX "/images/cities/csplode-green-3.png",
  DATA_PREFIX "/images/cities/csplode-green-4.png",
  DATA_PREFIX "/images/cities/csplode-green-5.png",
  DATA_PREFIX "/images/cities/cdead-green.png",
  DATA_PREFIX "/images/cities/city-orange.png",
  DATA_PREFIX "/images/cities/csplode-orange-1.png",
  DATA_PREFIX "/images/cities/csplode-orange-2.png",
  DATA_PREFIX "/images/cities/csplode-orange-3.png",
  DATA_PREFIX "/images/cities/csplode-orange-4.png",
  DATA_PREFIX "/images/cities/csplode-orange-5.png",
  DATA_PREFIX "/images/cities/cdead-orange.png",
  DATA_PREFIX "/images/cities/city-red.png",
  DATA_PREFIX "/images/cities/csplode-red-1.png",
  DATA_PREFIX "/images/cities/csplode-red-2.png",
  DATA_PREFIX "/images/cities/csplode-red-3.png",
  DATA_PREFIX "/images/cities/csplode-red-4.png",
  DATA_PREFIX "/images/cities/csplode-red-5.png",
  DATA_PREFIX "/images/cities/cdead-red.png",
  DATA_PREFIX "/images/cities/shields.png",
  DATA_PREFIX "/images/comets/comet1.png",
  DATA_PREFIX "/images/comets/comet2.png",
  DATA_PREFIX "/images/comets/comet3.png",
  DATA_PREFIX "/images/comets/cometex3.png",
  DATA_PREFIX "/images/comets/cometex3.png",
  DATA_PREFIX "/images/comets/cometex2.png",
  DATA_PREFIX "/images/comets/cometex2.png",
  DATA_PREFIX "/images/comets/cometex1a.png",
  DATA_PREFIX "/images/comets/cometex1a.png",
  DATA_PREFIX "/images/comets/cometex1.png",
  DATA_PREFIX "/images/comets/cometex1.png",
  DATA_PREFIX "/images/comets/mini_comet1.png",
  DATA_PREFIX "/images/comets/mini_comet2.png",
  DATA_PREFIX "/images/comets/mini_comet3.png",
  DATA_PREFIX "/images/status/nums.png",
  DATA_PREFIX "/images/status/lednums.png",
  DATA_PREFIX "/images/status/led_neg_sign.png",
  DATA_PREFIX "/images/status/paused.png",
  DATA_PREFIX "/images/status/demo.png",
  DATA_PREFIX "/images/status/demo-small.png",
  DATA_PREFIX "/images/status/keypad.png",
  DATA_PREFIX "/images/status/keypad_no_neg.png",
  DATA_PREFIX "/images/tux/console.png",
  DATA_PREFIX "/images/tux/console_led.png",
  DATA_PREFIX "/images/tux/tux-console1.png",
  DATA_PREFIX "/images/tux/tux-console2.png",
  DATA_PREFIX "/images/tux/tux-console3.png",
  DATA_PREFIX "/images/tux/tux-console4.png",
  DATA_PREFIX "/images/tux/tux-relax1.png",
  DATA_PREFIX "/images/tux/tux-relax2.png",
  DATA_PREFIX "/images/tux/tux-egypt1.png",
  DATA_PREFIX "/images/tux/tux-egypt2.png",
  DATA_PREFIX "/images/tux/tux-egypt3.png",
  DATA_PREFIX "/images/tux/tux-egypt4.png",
  DATA_PREFIX "/images/tux/tux-drat.png",
  DATA_PREFIX "/images/tux/tux-yipe.png",
  DATA_PREFIX "/images/tux/tux-yay1.png",
  DATA_PREFIX "/images/tux/tux-yay2.png",
  DATA_PREFIX "/images/tux/tux-yes1.png",
  DATA_PREFIX "/images/tux/tux-yes2.png",
  DATA_PREFIX "/images/tux/tux-sit.png",
  DATA_PREFIX "/images/tux/tux-fist1.png",
  DATA_PREFIX "/images/tux/tux-fist2.png",
  DATA_PREFIX "/images/status/wave.png",
  DATA_PREFIX "/images/status/score.png",
  DATA_PREFIX "/images/status/numbers.png",
  DATA_PREFIX "/images/status/gameover.png",
  DATA_PREFIX "/images/status/gameover_won.png"
};


static char * sound_filenames[NUM_SOUNDS] = {
  DATA_PREFIX "/sounds/pop.wav",
  DATA_PREFIX "/sounds/laser.wav",
  DATA_PREFIX "/sounds/buzz.wav",
  DATA_PREFIX "/sounds/alarm.wav",
  DATA_PREFIX "/sounds/shieldsdown.wav",
  DATA_PREFIX "/sounds/explosion.wav",
  DATA_PREFIX "/sounds/click.wav",
  DATA_PREFIX "/sounds/SIZZLING.WAV"
};

static char * music_filenames[NUM_MUSICS] = {
  DATA_PREFIX "/sounds/game.mod",
  DATA_PREFIX "/sounds/game2.mod",
  DATA_PREFIX "/sounds/game3.mod"
};

/* Global data used in setup.c:              */
/* (These need to be 'extern'd in "setup.h") */
SDL_Surface * screen;
SDL_Surface * images[NUM_IMAGES];
#ifndef NOSOUND
Mix_Chunk * sounds[NUM_SOUNDS];
Mix_Music * musics[NUM_MUSICS];
#endif

int opers[NUM_OPERS], range_enabled[NUM_Q_RANGES];

game_option_type* game_options;

/* Local function prototypes: */

void seticon(void);
void usage(int err, char * cmd);

int initialize_game_options(game_option_type* opts);
void print_game_options(game_option_type* opts);

/* --- Set-up function! --- */

void setup(int argc, char * argv[])
{
  int i, j, found, total_files;
  SDL_Rect dest;

  screen = NULL;

  if (!MC_Initialize())
  {
    printf("\nUnable to initialize MathCards\n");
    fprintf(stderr, "\nUnable to initialize MathCards\n");
    exit(1);
  }


  /* initialize game_options struct with defaults DSB */
  game_options = malloc(sizeof(game_option_type));
  if (!initialize_game_options(game_options))
  {
    printf("\nUnable to initialize game_options\n");
    fprintf(stderr, "\nUnable to initialize game_options\n");
    exit(1);
  }


  for (i = 0; i < NUM_Q_RANGES; i++)
  { 
    range_enabled[i] = 1;
  }


  /* See if operator settings are being overridden by command-line: */

  for (i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--operator") == 0 ||
        strcmp(argv[i], "-o") == 0)
    {
      game_options->oper_override = 1;
    }
  }


  /* If operator settings are being overridden, clear them first: */

  if (game_options->oper_override)
  {
    for (i = 0; i < NUM_OPERS; i++)
    {
      opers[i] = 0;
    }
  }


  /* Get options from the command line: */

  for (i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
    {
      /* Display help message: */

      printf("\nTux, of Math Command\n\n"
        "Use the number keys on the keyboard to answer math equations.\n"
        "If you don't answer a comet's math equation before it hits\n"
        "one of your cities, the city's shields will be destroyed.\n"
        "If that city is hit by another comet, it is destroyed completely.\n"
	"When you lose all of your cities, the game ends.\n\n");

      printf("Run the game with:\n"
        "--norepeats      - to ask each question only once, allowing player to\n"
        "                   win game if all questions successfully answered\n"
        "--answersfirst   - to ask questions in format: ? + num2 = num3\n"
        "                   instead of default format: num1 + num2 = ?\n"
        "--answersmiddle  - to ask questions in format: num1 + ? = num3\n"
        "                   instead of default format: num1 + num2 = ?\n"
        "--nosound        - to disable sound/music\n"
	"--nobackground   - to disable background photos (for slower systems)\n"
	"--fullscreen     - to run in fullscreen, if possible (vs. windowed)\n"
        "--keypad         - to enable the on-sceen numeric keypad\n"
	"--demo           - to run the program as a cycling demonstration\n"
	"--speed S        - set initial speed of the game\n"
	"                   (S may be fractional, default is 1.0)\n"
        "--allownegatives - to allow answers to be less than zero\n"
	"--operator OP    - to automatically play with particular operators\n"
	"                   OP may be one of:\n");

      for (j = 0; j < NUM_OPERS; j++)
        printf("                   \"%s\"\n", oper_opts[j]);

      printf("            or:\n");
      
      for (j = 0; j < NUM_OPERS; j++)
        printf("                   \"%s\"\n", oper_alt_opts[j]);

      printf("\n");
      

      exit(0);
    }
    else if (strcmp(argv[i], "--copyright") == 0 ||
	     strcmp(argv[i], "-c") == 0)
    {
      printf(
	"\n\"Tux, of Math Command\" version " VERSION ", Copyright (C) 2001 Bill Kendrick\n"
        "This program is free software; you can redistribute it and/or\n"
        "modify it under the terms of the GNU General Public License\n"
        "as published by the Free Software Foundation.  See COPYING.txt\n"
	"\n"
	"This program is distributed in the hope that it will be useful,\n"
	"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
	"\n");

      exit(0);
    }
    else if (strcmp(argv[i], "--usage") == 0 ||
	     strcmp(argv[i], "-u") == 0)
    {
      /* Display (happy) usage: */
	    
      usage(0, argv[0]);
    }
    else if (strcmp(argv[i], "--fullscreen") == 0 ||
	     strcmp(argv[i], "-f") == 0)
    {
      game_options->fullscreen = 1;
    }
    else if (strcmp(argv[i], "--nosound") == 0 ||
	     strcmp(argv[i], "-s") == 0 ||
	     strcmp(argv[i], "--quiet") == 0 ||
	     strcmp(argv[i], "-q") == 0)
    {
      game_options->use_sound = 0;
    }
    else if (strcmp(argv[i], "--version") == 0 ||
	     strcmp(argv[i], "-v") == 0)
    {
      printf("Tux, of Math Command (\"tuxmath\")\n"
	     "Version " VERSION "\n");
      exit(0);
    }
    else if (strcmp(argv[i], "--nobackground") == 0 ||
             strcmp(argv[i], "-b") == 0)
    {
      game_options->use_bkgd = 0;
    }
    else if (strcmp(argv[i], "--demo") == 0 ||
	     strcmp(argv[i], "-d") == 0)
    {
      game_options->demo_mode = 1;
    }
    else if (strcmp(argv[i], "--keypad") == 0 ||
             strcmp(argv[i], "-k") == 0)
    {
      game_options->use_keypad = 1;
    }
    else if (strcmp(argv[i], "--allownegatives") == 0 ||
             strcmp(argv[i], "-n") == 0)
    {
      MC_SetAllowNegAnswer(1);
    }
    else if (strcmp(argv[i], "--norepeats") == 0 ||
             strcmp(argv[i], "-r") == 0)
    {
      MC_SetRecycleCorrects(0);
    }
    else if (strcmp(argv[i], "--answersfirst") == 0)
    {
      MC_SetFormatAnswerLast(0);
      MC_SetFormatAnswerFirst(1);
      MC_SetFormatAnswerMiddle(0);
    }
    else if (strcmp(argv[i], "--answersmiddle") == 0)
    {
      MC_SetFormatAnswerLast(0);
      MC_SetFormatAnswerFirst(0);
      MC_SetFormatAnswerMiddle(1);
    }
    else if (strcmp(argv[i], "--speed") == 0 ||
	     strcmp(argv[i], "-s") == 0)
    {
      if (i >= argc - 1)
      {
	fprintf(stderr, "%s option requires an argument\n", argv[i]);
	usage(1, argv[0]);
      }

      game_options->speed = strtod(argv[i + 1], (char **) NULL);

      if (game_options->speed <= 0)
      {
	fprintf(stderr, "Invalided argument to %s: %s\n",
		argv[i], argv[i + 1]);
	usage(1, argv[0]);
      }

      i++;
    }
    else if (strcmp(argv[i], "--operator") == 0 ||
	     strcmp(argv[i], "-o") == 0)
    {
      if (i >= argc - 1)
      {
	fprintf(stderr, "%s option requires an argument\n", argv[i]);
	usage(1, argv[0]);
      }
     
      found = 0; 
      for (j = 0; j < NUM_OPERS; j++)
      {
	if (strcmp(argv[i + 1], oper_opts[j]) == 0 ||
	    strcmp(argv[i + 1], oper_alt_opts[j]) == 0)
	{
          found = 1;
          opers[j] = 1;
	}
      }

      if (found == 0)
      {
	fprintf(stderr, "Unrecognized operator %s\n", argv[i + 1]);
	usage(1, argv[0]);
      }

      i++;
    }
    else
    {
      /* Display 'made' usage: */

      fprintf(stderr, "Unknown option: %s\n", argv[i]);
      usage(1, argv[0]);
    }
  }


  if (game_options->demo_mode && game_options->use_keypad)
  {
    fprintf(stderr, "No use for keypad in demo mode!\n");
    game_options->use_keypad = 0;
  }


  
  /* Init SDL Video: */

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
      fprintf(stderr,
	      "\nError: I could not initialize video!\n"
	      "The Simple DirectMedia error that occured was:\n"
	      "%s\n\n", SDL_GetError());
      exit(1);
    }

  printf("before SDL Audio\n");


  /* Init SDL Audio: */

#ifndef NOSOUND

  if (game_options->use_sound)
    { 
      if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
          fprintf(stderr,
  	          "\nWarning: I could not initialize audio!\n"
	          "The Simple DirectMedia error that occured was:\n"
	          "%s\n\n", SDL_GetError());
	  game_options->use_sound = 0;
        }
    }

   printf("middle of Audio\n");
 
  if (game_options->use_sound)
  {
    printf("using sound \n"); 
       if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048) < 0)
        {
          printf( "\nWarning: I could not set up audio for 44100 Hz "
	          "16-bit stereo.\n"
	          "The Simple DirectMedia error that occured was:\n"
	          "%s\n\n", SDL_GetError());

          fprintf(stderr,
	          "\nWarning: I could not set up audio for 44100 Hz "
	          "16-bit stereo.\n"
	          "The Simple DirectMedia error that occured was:\n"
	          "%s\n\n", SDL_GetError());
          game_options->use_sound = 0;
        }
   }

#endif

  printf("after SDL Audio\n");
 

  if (game_options->fullscreen)
  {
    screen = SDL_SetVideoMode(640, 480, 16, SDL_FULLSCREEN | SDL_HWSURFACE);

    if (screen == NULL)
    {
      fprintf(stderr,
              "\nWarning: I could not open the display in fullscreen mode.\n"
	      "The Simple DirectMedia error that occured was:\n"
	      "%s\n\n", SDL_GetError());
      game_options->fullscreen = 0;
    }
  }

  if (!game_options->fullscreen)
  {
    screen = SDL_SetVideoMode(640, 480, 16, SDL_HWSURFACE);
  }

  if (screen == NULL)
  {
    fprintf(stderr,
            "\nError: I could not open the display.\n"
	    "The Simple DirectMedia error that occured was:\n"
	    "%s\n\n", SDL_GetError());
    exit(1);
  }

  seticon();

  SDL_WM_SetCaption("Tux, of Math Command", "TuxMath");

  if (game_options->use_sound)
    total_files = NUM_IMAGES + NUM_SOUNDS + NUM_MUSICS;
  else
    total_files = NUM_IMAGES;


  
  /* Load images: */
  for (i = 0; i < NUM_IMAGES; i++)
  {
    images[i] = IMG_Load(image_filenames[i]);
    if (images[i] == NULL)
      {
	fprintf(stderr,
		"\nError: I couldn't load a graphics file:\n"
		"%s\n"
		"The Simple DirectMedia error that occured was:\n"
		"%s\n\n", image_filenames[i], SDL_GetError());
	exit(1);
      }

    
    if (i == IMG_STANDBY)
      {
	dest.x = (screen->w - images[IMG_STANDBY]->w) / 2;
	dest.y = screen->h - images[IMG_STANDBY]->h - 10;
	dest.w = images[IMG_STANDBY]->w;
	dest.h = images[IMG_STANDBY]->h;
	
	SDL_BlitSurface(images[IMG_STANDBY], NULL, screen, &dest);
	SDL_Flip(screen);
      }
    else if (i == IMG_LOADING)
      {
	dest.x = (screen->w - images[IMG_LOADING]->w) / 2;
	dest.y = 0;
	dest.w = images[IMG_LOADING]->w;
	dest.h = images[IMG_LOADING]->h;
	
	SDL_BlitSurface(images[IMG_LOADING], NULL, screen, &dest);
	SDL_Flip(screen);
      }
    else if (i == IMG_TITLE)
      {
	dest.x = (screen->w - images[IMG_TITLE]->w) / 2;
	dest.y = images[IMG_LOADING]->h;
	dest.w = images[IMG_TITLE]->w;
	dest.h = images[IMG_TITLE]->h;
	
	SDL_BlitSurface(images[IMG_TITLE], NULL, screen, &dest);
	SDL_Flip(screen);
      }
    
    
    dest.x = 0;
    dest.y = (screen->h) - 10;
    dest.w = ((screen->w) * (i + 1)) / total_files;
    dest.h = 10;
    
    SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 255, 0));
    SDL_Flip(screen);
  }

  printf("all images loaded\n");

#ifndef NOSOUND
  if (game_options->use_sound)
  {
    for (i = 0; i < NUM_SOUNDS; i++)
    {
      sounds[i] = Mix_LoadWAV(sound_filenames[i]);

      if (sounds[i] == NULL)
      {
        fprintf(stderr,
	        "\nError: I couldn't load a sound file:\n"
                "%s\n"
                "The Simple DirectMedia error that occured was:\n"
                "%s\n\n", sound_filenames[i], SDL_GetError());
        exit(1);
      }
      
      dest.x = 0;
      dest.y = (screen->h) - 10;
      dest.w = ((screen->w) * (i + 1 + NUM_IMAGES)) / total_files;
      dest.h = 10;

      SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 255, 0));
      SDL_Flip(screen);
    }


    for (i = 0; i < NUM_MUSICS; i++)
    {
      musics[i] = Mix_LoadMUS(music_filenames[i]);

      if (musics[i] == NULL)
      {
        fprintf(stderr,
	        "\nError: I couldn't load a music file:\n"
                "%s\n"
                "The Simple DirectMedia error that occured was:\n"
                "%s\n\n", music_filenames[i], SDL_GetError());
        exit(1);
      }
      
      dest.x = 0;
      dest.y = (screen->h) - 10;
      dest.w = ((screen->w) * (i + 1 + NUM_IMAGES + NUM_SOUNDS)) / total_files;
      dest.h = 10;

      SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 255, 0));
      SDL_Flip(screen);
    }
  }
#endif
  

  for (i = images[IMG_LOADING]->h; i >= 0; i = i - 10)
    {
      SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

      dest.x = (screen->w - images[IMG_TITLE]->w) / 2;
      dest.y = i;
      dest.w = images[IMG_TITLE]->w;
      dest.h = images[IMG_TITLE]->h;
      
      SDL_BlitSurface(images[IMG_TITLE], NULL, screen, &dest);
      SDL_Flip(screen);
      SDL_Delay(10);
    }
}

/* free any heap memory used during game DSB */
void cleanup()
{
  if (game_options)
    free(game_options);
  /* frees any heap used by MathCards: */
  MC_EndGame();
}



int initialize_game_options(game_option_type* opts)
{
  /* bail out if no struct */
  if (!opts)
    return 0;

  /* set general game options */
  opts->use_sound = DEFAULT_USE_SOUND;
  opts->fullscreen = DEFAULT_FULLSCREEN;
  opts->use_bkgd = DEFAULT_USE_BKGD;
  opts->demo_mode = DEFAULT_DEMO_MODE;
  opts->oper_override = DEFAULT_OPER_OVERRIDE;
  opts->use_keypad = DEFAULT_USE_KEYPAD;
  opts->speed = DEFAULT_SPEED;
  opts->allow_speedup = DEFAULT_ALLOW_SPEEDUP;
  opts->speedup_factor = DEFAULT_SPEEDUP_FACTOR;
  opts->max_speed = DEFAULT_MAX_SPEED;
  opts->slow_after_wrong = DEFAULT_SLOW_AFTER_WRONG;
  opts->reuse_questions = DEFAULT_REUSE_QUESTIONS;
  opts->extra_comets = DEFAULT_EXTRA_COMETS;
  opts->max_comets = DEFAULT_MAX_COMETS;
  opts->num_cities = DEFAULT_NUM_CITIES;   /* MUST BE AN EVEN NUMBER! */
  opts->num_bkgds = DEFAULT_NUM_BKGDS;
  opts->max_city_colors = DEFAULT_MAX_CITY_COLORS;

  /* for testing purposes */
  /* print_game_options(opts); */
  return 1;
}



/* prints struct to stdout for testing purposes */
void print_game_options(game_option_type* opts)
{
 /* bail out if no struct */
  if (!opts)
    return;

  printf("\nPrinting members of game_options struct:\n");
  printf("\nGeneral game options:\n");
  printf("use_sound:\t%d\n", opts->use_sound);
  printf("fullscreen:\t%d\n", opts->fullscreen);
  printf("use_bkgd:\t%d\n", opts->use_bkgd);
  printf("demo_mode:\t%d\n", opts->demo_mode);
  printf("oper_override:\t%d\n", opts->oper_override);
  printf("use_keypad:\t%d\n", opts->use_keypad);
  printf("reuse_questions:\t%d\n", opts->reuse_questions);
  printf("speed:\t%f\n", opts->speed);
  printf("allow_speedup:\t%d\n", opts->allow_speedup);
  printf("speedup_factor:\t%f\n", opts->speedup_factor);
  printf("max_speed:\t%f\n", opts->max_speed);
  printf("slow_after_wrong:\t%d\n", opts->slow_after_wrong);
  printf("extra_comets:\t%d\n", opts->extra_comets);
  printf("max_comets:\t%d\n", opts->max_comets);
  printf("num_cities:\t%d\n", opts->num_cities);
  printf("num_bkgds:\t%d\n", opts->num_bkgds);
  printf("max_city_colors:\t%d\n", opts->max_city_colors);
}

/* Set the application's icon: */

void seticon(void)
{
  int masklen;
  Uint8 * mask;
  SDL_Surface * icon;
  
  
  /* Load icon into a surface: */
  
  icon = IMG_Load(DATA_PREFIX "/images/icon.png");
  if (icon == NULL)
    {
      fprintf(stderr,
              "\nWarning: I could not load the icon image: %s\n"
              "The Simple DirectMedia error that occured was:\n"
              "%s\n\n", DATA_PREFIX "images/icon.png", SDL_GetError());
      return;
    }
  
  
  /* Create mask: */
  
  masklen = (((icon -> w) + 7) / 8) * (icon -> h);
  mask = malloc(masklen * sizeof(Uint8));
  memset(mask, 0xFF, masklen);
  
  
  /* Set icon: */
  
  SDL_WM_SetIcon(icon, mask);
  
  
  /* Free icon surface & mask: */
  
  free(mask);
  SDL_FreeSurface(icon);
  
  
  /* Seed random-number generator: */
  
  srand(SDL_GetTicks());
}

void usage(int err, char * cmd)
{
  FILE * f;

  if (err == 0)
    f = stdout;
  else
    f = stderr;

  fprintf(f,
   "\nUsage: %s {--help | --usage | --copyright}\n"
   "       %s [--fullscreen] [--nosound] [--nobackground]\n"
   "          [--demo] [--keypad] [--allownegatives]\n"
   "          [--operator {add | subtract | multiply | divide} ...]\n"
   "          [--speed <val>]\n"
    "\n", cmd, cmd);

  exit (err);
}

