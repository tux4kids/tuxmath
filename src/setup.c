/*
   setup.c

   Contains some globals (screen surface, images, some option flags, etc.)
   as well as the function to load data files (images, sounds, music)

   Copyright 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011.
Authors: Bill Kendrick, David Bruce, Tim Holy, Wenyuan Guo.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org


setup.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Project includes: -----------------*/
#include "tuxmath.h"
#include "options.h"
#include "mathcards.h"
#include "setup.h"
#include "fileops.h"
#include "game.h"
#include "menu.h"
#include "titlescreen.h"
#include "highscore.h"
#include "mysetenv.h"


/* SDL includes: -----------------*/
#include "SDL.h"

#ifndef NOSOUND
#include "SDL_mixer.h"
#endif

#include "SDL_image.h"

#ifdef HAVE_LIBSDL_NET
#include "SDL_net.h"
#endif

/* C library includes: -----------------*/
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>





/* Global data used in setup.c:              */
/* (These are now 'extern'd in "tuxmath.h") */


SDL_Surface* screen;
SDL_Surface* images[NUM_IMAGES];
sprite* sprites[NUM_SPRITES];
MC_MathGame* local_game;
MC_MathGame* lan_game_settings;

/* Need special handling to generate flipped versions of images. This
   is a slightly ugly hack arising from the use of the enum trick for
   NUM_IMAGES. */
#define NUM_FLIPPED_IMAGES 6
SDL_Surface* flipped_images[NUM_FLIPPED_IMAGES];
int flipped_img_lookup[NUM_IMAGES];
SDL_Surface* blended_igloos[NUM_BLENDED_IGLOOS];

const int flipped_img[] = {
    IMG_PENGUIN_WALK_ON1,
    IMG_PENGUIN_WALK_ON2,
    IMG_PENGUIN_WALK_ON3,
    IMG_PENGUIN_WALK_OFF1,
    IMG_PENGUIN_WALK_OFF2,
    IMG_PENGUIN_WALK_OFF3
};


#ifndef NOSOUND
Mix_Chunk* sounds[NUM_SOUNDS];
Mix_Music* musics[NUM_MUSICS];
#endif

/* Keep return values from locale setup: */
typedef struct locale_info {
    char setlocale_ret[64];
    char bindtextdomain_ret[64];
    char bind_textdomain_codeset_ret[64];
    char textdomain_ret[64];
} locale_info;

locale_info tuxmath_locale;

/* Local function prototypes: */
void initialize_locale(const char* desired_loc);
void initialize_options(void);
void handle_debug_args(int argc, char* argv[]);
void handle_command_args(int argc, char* argv[]);
void initialize_SDL(void);
void load_data_files(void);
void generate_flipped_images(void);
void generate_blended_images(void);

//int initialize_game_options(void);
void seticon(void);
void usage(int err, char * cmd);

void cleanup_memory(void);



/* --- Set-up function - now in four easier-to-digest courses! --- */
/* --- Er - make that six courses! --- */
/* --- Six is right out. Seven is much better. --- */
/* --- OK, now we have eight. --- */
void setup(int argc, char * argv[])
{
    /* Read debugging args from command line */
    handle_debug_args(argc, argv);
    /* initialize locale from system settings: */
    initialize_locale("");      
    /* initialize settings and read in config files: */
    /* Note this now only does the global settings   */
    initialize_options();
    /* Command-line code now in own function: */
    handle_command_args(argc, argv);
    /* initialize default user's options (for resolution)*/
    initialize_options_user();
    /* SDL setup in own function:*/
    initialize_SDL();
    /* Read image and sound files: */
    load_data_files();
    /* Generate flipped versions of walking images */
    generate_flipped_images();
    /* Generate blended images (e.g., igloos) */
    generate_blended_images();
    /* Note that the per-user options will be set after the call to
       titlescreen, to allow for user-login to occur. 

       FIXME this means that command-line args will be overridden!
       Is this desirable? */
}



void initialize_locale(const char* desired_loc)
{
    const char *s1, *s2, *s3, *s4;
    if(!desired_loc)
    {
        fprintf(stderr, "initialize_locale() - null desired_loc arg. \n");  
        return;
    }  

    s1 = setlocale(LC_ALL, desired_loc);
    s2 = bindtextdomain(PACKAGE, TUXLOCALE);
    s3 = bind_textdomain_codeset(PACKAGE, "UTF-8");
    s4 = textdomain(PACKAGE);

    strncpy(tuxmath_locale.setlocale_ret, s1, 64);
    strncpy(tuxmath_locale.bindtextdomain_ret, s2, 64);
    strncpy(tuxmath_locale.bind_textdomain_codeset_ret, s3, 64);
    strncpy(tuxmath_locale.textdomain_ret, s4, 64);

    DEBUGCODE(debug_setup) print_locale_info(stderr);
}


void print_locale_info(FILE* fp)
{
    if(!fp)
    {
        fprintf(stderr, "print_locale_info() - null FILE* arg. \n");  
        return;
    }  

    fprintf(fp, "PACKAGE = %s\n", PACKAGE);
    fprintf(fp, "TUXLOCALE = %s\n", TUXLOCALE);
    fprintf(fp, "setlocale(LC_ALL, \"\") returned: %s\n",
            tuxmath_locale.setlocale_ret);
    fprintf(fp, "bindtextdomain(PACKAGE, TUXLOCALE) returned: %s\n",
            tuxmath_locale.bindtextdomain_ret);
    fprintf(fp, "bind_textdomain_codeset(PACKAGE, \"UTF-8\") returned: %s\n",
            tuxmath_locale.bind_textdomain_codeset_ret);
    fprintf(fp, "textdomain(PACKAGE) returned: %s\n",
            tuxmath_locale.textdomain_ret);
    fprintf(fp, "gettext(\"Help\"): %s\n\n", gettext("Help"));
    fprintf(fp, "_(\"Help\"): %s\n\n", _("Help"));
    fprintf(fp, "dgettext(\"tuxmath\", \"Help\"): %s\n", dgettext("tuxmath", "Help"));
}


/* Set up mathcards with default values for math question options, */
/* set up game_options with defaults for general game options,     */
/* then read in global config file                                 */
void initialize_options(void)
{
    /* Initialize MathCards backend for math questions: */
    local_game = (MC_MathGame*) malloc(sizeof(MC_MathGame));
    if (local_game == NULL)
    {
        fprintf(stderr, "\nUnable to allocate MC_MathGame\n");
        exit(1);
    }
    local_game->math_opts = NULL;
    if (!MC_Initialize(local_game))
    {
        fprintf(stderr, "\nUnable to initialize MathCards\n");
        exit(1);
    }


    lan_game_settings = (MC_MathGame*) malloc(sizeof(MC_MathGame));
    if (lan_game_settings == NULL)
    {
        fprintf(stderr, "\nUnable to allocate MC_MathGame\n");
        exit(1);
    }
    lan_game_settings->math_opts = NULL;
    if (!MC_Initialize(lan_game_settings))
    {
        fprintf(stderr, "\nUnable to initialize MathCards\n");
        exit(1);
    }


    /* initialize game_options struct with defaults DSB */
    if (!Opts_Initialize())
    {
        fprintf(stderr, "\nUnable to initialize game_options\n");
        cleanup_on_error();
        exit(1);
    }

    /* Now that MathCards and game_options initialized using  */
    /* hard-coded defaults, read options from disk and mofify */
    /* as needed. First read in installation-wide settings:   */
    if (!read_global_config_file(local_game))
    {
        fprintf(stderr, "\nCould not find global config file.\n");
        /* can still proceed using hard-coded defaults.         */
    }
}

/* Read in the user-specific options (if desired)              */
/* This has been split from the above to allow it to be called */
/* from titlescreen, to allow for user-login to occur.         */
void initialize_options_user(void)
{
    /* Read in user-specific settings, if desired.  By    */
    /* default, this restores settings from the player's last */
    /* game:                                                  */
    if (Opts_GetGlobalOpt(PER_USER_CONFIG))
    {
        if (!read_user_config_file(local_game))
        {
            fprintf(stderr, "\nCould not find user's config file.\n");
            /* can still proceed using hard-coded defaults.         */
        }

        /* If game being run for first time, try to write file: */
        if (!write_user_config_file(local_game))
        {
            fprintf(stderr, "\nUnable to write user's config file.\n");
        }
    }

    /* Read the lessons directory to determine which lesson   */
    /* files are available.                                   */
    if (!parse_lesson_file_directory())
        fprintf(stderr,"\nCould not parse the lesson file directory.\n");

    /* Now set up high score tables: */
    initialize_scores();
    if (!read_high_scores())
    {
        fprintf(stderr, "\nCould not find high score table.\n");
        /* (can still proceed).         */
    }

    DEBUGCODE(debug_setup)
        print_high_scores(stdout);
}



/* Handle debugging arguments passed from command line */
/* NOTE - moved into separate, earlier pass so we can  */
/* get output for earlier setup events - DSB           */
void handle_debug_args(int argc, char* argv[])
{
    int i;

    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--debug-all") == 0)
        {
            debug_status |= debug_all;
        }
        else if (strcmp(argv[i], "--debug-setup") == 0)
        {
            debug_status |= debug_setup;
        }
        else if (strcmp(argv[i], "--debug-fileops") == 0)
        {
            debug_status |= debug_fileops;
        }
        else if (strcmp(argv[i], "--debug-loaders") == 0)
        {
            debug_status |= debug_loaders;
        }
        else if (strcmp(argv[i], "--debug-titlescreen") == 0)
        {
            debug_status |= debug_titlescreen;
        }
        else if (strcmp(argv[i], "--debug-menu") == 0)
        {
            debug_status |= debug_menu;
        }
        else if (strcmp(argv[i], "--debug-menu-parser") == 0)
        {
            debug_status |= debug_menu_parser;
        }
        else if (strcmp(argv[i], "--debug-game") == 0)
        {
            debug_status |= debug_game;
        }
        else if (strcmp(argv[i], "--debug-factoroids") == 0)
        {
            debug_status |= debug_factoroids;
        }
        else if (strcmp(argv[i], "--debug-lan") == 0)
        {
            debug_status |= debug_lan;
        }
        else if (strcmp(argv[i], "--debug-mathcards") == 0)
        {
            debug_status |= debug_mathcards;
        }
        else if (strcmp(argv[i], "--debug-sdl") == 0)
        {
            debug_status |= debug_sdl;
        }
        else if (strcmp(argv[i], "--debug-lessons") == 0)
        {
            debug_status |= debug_lessons;
        }
        else if (strcmp(argv[i], "--debug-highscore") == 0)
        {
            debug_status |= debug_highscore;
        }
        else if (strcmp(argv[i], "--debug-options") == 0)
        {
            debug_status |= debug_options;
        }
        else if (strcmp(argv[i], "--debug-text-and-intl") == 0)
        {
            debug_status |= debug_text_and_intl;
        }
    }/* end of command-line args */

    DEBUGMSG(debug_setup,"debug_status: %x", debug_status);
}


/* Handle any arguments passed from command line, except */
/* for debug flags which we already have dealt with.     */
void handle_command_args(int argc, char* argv[])
{
    DIR *dirp;
    int i;

    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            /* Display help message: */

            fprintf(stderr, "\nTux, of Math Command\n\n"
                    "Use the number keys on the keyboard to answer math equations,\n"
                    "and then hit the space bar or enter.\n"
                    "If you don't answer a comet's math equation before it hits\n"
                    "one of your igloos, the igloo will be damaged.\n"
                    "If an igloo is hit twice, the penguin inside walks away.\n"
                    "When you lose all of your igloos, the game ends.\n\n");

            fprintf(stderr, "There is also a \"factoroids\" game in which a ship\n"
                    "destroys asteroids if you type a valid factor of the number\n"
                    "for a particular asteroid.  Use the number keys to steer.\n\n");

            fprintf(stderr, "Note: most settings are now stored in a config file named 'options' in\n"
                    "a hidden directory named './tuxmath' within the user's home directory.\n"
                    "The file consists of simple name/value pairs. It is much easier\n"
                    "to edit this file to set game parameters than to use the command-line\n"
                    "arguments listed below. Also, many options are not selectable from the\n"
                    "command line. The config file contains extensive comments detailing how\n"
                    "to configure the behavior of Tuxmath.\n\n");

            fprintf(stderr, "Run the game with:\n"
                    "--homedir dirname      - seek for user home director(ies) in the specified\n"
                    "                         location, rather than the user's actual home\n"
                    "                         directory.  You can set up a user directory tree in\n"
                    "                         this location (see README).  This option is\n"
                    "                         especially useful for schools where all students log\n"
                    "                         in with a single user name.\n"
                    "--optionfile filename  - read config settings from named file. The locations\n"
                    "                         searched for a file with a matching name are the\n"
                    "                         current working directory, the absolute path of the\n"
                    "                         filename, tuxmath's missions directory, the user's\n"
                    "                         tuxmath directory, and the user's home.\n"
                    "--playthroughlist      - to ask each question only once, allowing player to\n"
                    "                         win game if all questions successfully answered\n"
                    "--language {language}  - set the language named {language}, if it exists\n"
                    
                    "--answersfirst   - to ask questions in format: ? + num2 = num3\n"
                    "                   instead of default format: num1 + num2 = ?\n"
                    "--answersmiddle  - to ask questions in format: num1 + ? = num3\n"
                    "                   instead of default format: num1 + num2 = ?\n"
                    "--nosound        - to disable sound/music\n"
                    "--nobackground   - to disable background photos (for slower systems)\n"
                    "--fullscreen     - to run in fullscreen, if possible (vs. windowed)\n"
                    "--windowed       - to run in a window rather than fullscreen\n"
                    "--resolution WxH - window resolution (windowed mode only)\n"
                    "--keypad         - to enable the on-sceen numeric keypad\n"
                    "--demo           - to run the program as a cycling demonstration\n"
                    "--tts 			  - Enable in game accessibility\n"
                    "--notts 		  - Disable in game accessibility\n"
                    "--speed S        - set initial speed of the game\n"
                    "                   (S may be fractional, default is 1.0)\n"
                    "--allownegatives - to allow answers to be less than zero\n"
                    "--debug-X        - prints debug information on command line\n"
                    "                   X may be one of the following:\n"
                    "                     setup: debug messages only during initialization \n"
                    "                     fileops: file operations (loading and saving data)\n"
                    "                     loaders: loading of mulitmedia (images and sounds)\n"
                    "                     titlescreen\n"
                    "                     menu: most operations dealing with menus\n"
                    "                     menu-parser: subset of operations dealing with menus\n"
                    "                     game: the comets game\n"
                    "                     factoroids: the factoroids game\n"
                    "                     lan: anything dealing with networking\n"
                    "                     mathcards: generation of math problems\n"
                    "                     sdl: the general graphical system\n"
                    "                     lessons: parsing pre-prepared lessons\n"
                    "                     highscore: loading and saving high scores\n"
                    "                     options: loading and saving options files\n"
                    "                     text-and-intl: text and internationalization\n"
                    "                     all: everything!\n"
                    );

            fprintf(stderr, "\n");

            cleanup_on_error();
            exit(0);
        }
        else if (strcmp(argv[i], "--copyright") == 0 ||
                strcmp(argv[i], "-c") == 0)
        {
            printf(
                    "\n\"Tux, of Math Command\" version " VERSION ", Copyright (C) 2001-2011,\n"
                    "Bill Kendrick, David Bruce, Tim Holy, and the Tux4Kids Project.\n"
                    "This program is free software; you can redistribute it and/or\n"
                    "modify it under the terms of the GNU General Public License\n"
                    "as published by the Free Software Foundation.  See COPYING.txt\n"
                    "\n"
                    "This program is distributed in the hope that it will be useful,\n"
                    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
                    "\n");

            cleanup_on_error();
            exit(0);
        }
        else if (strcmp(argv[i], "--usage") == 0 ||
                strcmp(argv[i], "-u") == 0)
        {
            /* Display (happy) usage: */

            usage(0, argv[0]);
        }
        else if (0 == strcmp(argv[i], "--homedir"))
        {
            // Parse the user choice of a non-default home directory
            if (i >= argc -1)
            {
                fprintf(stderr, "%s option requires an argument (dirname)\n", argv[i]);
                usage(1, argv[0]);
            }
            else // see whether the specified name is a directory
            {
                if ((dirp = opendir(argv[i+1])) == NULL)
                    fprintf(stderr,"homedir: %s is not a directory, or it could not be read\n", argv[i+1]);
                else {
                    set_user_data_dir(argv[i+1]);  // copy the homedir setting
                    closedir(dirp);
                }
                i++;   // to pass over the next argument, so remaining options parsed
            }
        }
        else if (0 == strcmp(argv[i], "--optionfile"))
        {
            if (i >= argc - 1)
            {
                fprintf(stderr, "%s option requires an argument (filename)\n", argv[i]);
                usage(1, argv[0]);
            }
            else /* try to read file named in following arg: */
            {
                if (!read_named_config_file(local_game, argv[i + 1]))
                {
                    fprintf(stderr, "Could not read config file: %s\n", argv[i + 1]);
                }
            }
            i++; /* so program doesn't barf on next arg (the filename) */
        }
        else if (strcmp(argv[i], "--fullscreen") == 0 ||
                strcmp(argv[i], "-f") == 0)
        {
            Opts_SetGlobalOpt(FULLSCREEN, 1);
        }
        else if (strcmp(argv[i], "--resolution") == 0 ||
                strcmp(argv[i], "-r") == 0)
        {
            if (i >= argc - 1)
            {
                fprintf(stderr, "%s option requires an argument\n", argv[i]);
                usage(1, argv[0]);
            }
            else
            {
                int w=0, h=0;
                sscanf(argv[i+1], "%dx%d", &w, &h);

                if(w>0 && h>0)
                {
                    Opts_SetWindowWidth(w);
                    Opts_SetWindowHeight(h);
                }
            }
            ++i;
        }
        else if (strcmp(argv[i], "--windowed") == 0 ||
                strcmp(argv[i], "-w") == 0)
        {
            Opts_SetGlobalOpt(FULLSCREEN, 0);
        }

        else if (strcmp(argv[i], "--tts") == 0 ||
                strcmp(argv[i], "-t") == 0)
        {
            Opts_SetGlobalOpt(USE_TTS, 1);
        }

        else if (strcmp(argv[i], "--notts") == 0 ||
                strcmp(argv[i], "-nt") == 0)
        {
            Opts_SetGlobalOpt(USE_TTS, 0);
        }
        

        else if (strcmp(argv[i], "--nosound") == 0 ||
                strcmp(argv[i], "-s") == 0 ||
                strcmp(argv[i], "--quiet") == 0 ||
                strcmp(argv[i], "-q") == 0)
        {
            Opts_SetGlobalOpt(USE_SOUND, -1);  // prevent options files from overwriting
        }
        else if (strcmp(argv[i], "--version") == 0 ||
                strcmp(argv[i], "-v") == 0)
        {
            fprintf(stderr, "Tux, of Math Command (\"tuxmath\")\n"
                    "Version " VERSION "\n");
            cleanup_on_error();
            exit(0);
        }
        else if (strcmp(argv[i], "--nobackground") == 0 ||
                strcmp(argv[i], "-b") == 0)
        {
            Opts_SetUseBkgd(0);
        }
        else if (strcmp(argv[i], "--demo") == 0 ||
                strcmp(argv[i], "-d") == 0)
        {
            Opts_SetDemoMode(1);
        }
        else if (strcmp(argv[i], "--keypad") == 0 ||
                strcmp(argv[i], "-k") == 0)
        {
            Opts_SetGlobalOpt(USE_KEYPAD, 1);
        }
        else if (strcmp(argv[i], "--allownegatives") == 0 ||
                strcmp(argv[i], "-n") == 0)
        {
            MC_SetOpt(local_game, ALLOW_NEGATIVES, 1);
        }
        else if (strcmp(argv[i], "--playthroughlist") == 0 ||
                strcmp(argv[i], "-l") == 0)
        {
            MC_SetOpt(local_game, PLAY_THROUGH_LIST, 1);
        }
        else if (strcmp(argv[i], "--answersfirst") == 0)
        {
            MC_SetOpt(local_game, FORMAT_ANSWER_LAST, 0);
            MC_SetOpt(local_game, FORMAT_ANSWER_FIRST, 1);
            MC_SetOpt(local_game, FORMAT_ANSWER_MIDDLE, 0);
        }
        else if (strcmp(argv[i], "--answersmiddle") == 0)
        {
            MC_SetOpt(local_game, FORMAT_ANSWER_LAST, 0);
            MC_SetOpt(local_game, FORMAT_ANSWER_FIRST, 0);
            MC_SetOpt(local_game, FORMAT_ANSWER_MIDDLE, 1);
        }

        else if (strcmp(argv[i], "--language") == 0 ||
                strcmp(argv[i], "-l") == 0)
        {
			++i;
			my_setenv("LANGUAGE",argv[i]);
			/* initialize Tts */
			T4K_Tts_init();
			T4K_Tts_set_voice(argv[i]);
        }

        else if (strcmp(argv[i], "--speed") == 0 ||
                strcmp(argv[i], "-s") == 0)
        {
            if (i >= argc - 1)
            {
                fprintf(stderr, "%s option requires an argument\n", argv[i]);
                usage(1, argv[0]);
            }

            Opts_SetSpeed(strtod(argv[i + 1], (char **) NULL));
            i++;
        }
        else /* Warn for unknown option, except debug flags */
            /* that we deal with separately:               */
        {
            if(strncmp(argv[i], "--debug", strlen("--debug")) != 0)         
                fprintf(stderr, "Unknown option: %s\n", argv[i]);
        }
    }/* end of command-line args */


    if (Opts_DemoMode() && Opts_GetGlobalOpt(USE_KEYPAD))
    {
        fprintf(stderr, "No use for keypad in demo mode!\n");
        Opts_SetGlobalOpt(USE_KEYPAD, 0);
    }
}


void initialize_SDL(void)
{
    //NOTE - SDL_Init() and friends now in InitT4KCommon()

    // Audio parameters
    int frequency, channels, n_timesopened;
    Uint16 format;

    /* Init common library */
    if(!InitT4KCommon(debug_status))
    {
        fprintf(stderr, "InitT4KCommon() failed - exiting.\n");
        cleanup_on_error();
        exit(1);
    }

    /* Init SDL Video: */
    screen = NULL;


    /* Init SDL Audio: */
    Opts_SetSoundHWAvailable(0);  // By default no sound HW
#ifndef NOSOUND
    if (Opts_GetGlobalOpt(USE_SOUND))
    {
        if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, AUDIO_S16SYS, 2, 2048) < 0)
        {
            fprintf(stderr,
                    "\nWarning: I could not set up audio for 44100 Hz "
                    "16-bit stereo.\n"
                    "The Simple DirectMedia error that occured was:\n"
                    "%s\n\n", SDL_GetError());

        }
        n_timesopened = Mix_QuerySpec(&frequency,&format,&channels);
        if (n_timesopened > 0)
            Opts_SetSoundHWAvailable(1);
        else
            frequency = format = channels = 0; //more helpful than garbage
        DEBUGMSG(debug_setup, "Sound mixer: frequency = %d, "
                "format = %x, "
                "channels = %d, "
                "n_timesopened = %d\n",
                frequency,format,channels,n_timesopened);
    }
#endif
    /* If couldn't set up sound, deselect sound options: */
    if(!Opts_SoundHWAvailable())
    {
        DEBUGMSG(debug_setup, "Sound setup failed - deselecting sound options\n");        
        Opts_SetGlobalOpt(USE_SOUND, 0);
        Opts_SetGlobalOpt(MENU_SOUND, 0);
        Opts_SetGlobalOpt(MENU_MUSIC, 0);
    }



    {
        const SDL_VideoInfo *videoInfo;
        Uint32 surfaceMode;
        videoInfo = SDL_GetVideoInfo();
        if (videoInfo->hw_available)
        {
            surfaceMode = SDL_HWSURFACE;
            DEBUGMSG(debug_setup, "HW mode\n");
        }
        else
        {
            surfaceMode = SDL_SWSURFACE;
            DEBUGMSG(debug_setup, "SW mode\n");
        }

        // Determine the current resolution: this will be used as the
        // fullscreen resolution, if the user wants fullscreen.
        DEBUGMSG(debug_setup, "Current resolution: w %d, h %d.\n",videoInfo->current_w,videoInfo->current_h);
        fs_res_x = videoInfo->current_w;
        fs_res_y = videoInfo->current_h;

        if (Opts_GetGlobalOpt(FULLSCREEN))
        {
            screen = SDL_SetVideoMode(fs_res_x, fs_res_y, PIXEL_BITS, SDL_FULLSCREEN | surfaceMode);
            if (screen == NULL)
            {
                fprintf(stderr,
                        "\nWarning: I could not open the display in fullscreen mode.\n"
                        "The Simple DirectMedia error that occured was:\n"
                        "%s\n\n", SDL_GetError());
                Opts_SetGlobalOpt(FULLSCREEN, 0);
            }
        }

        if (!Opts_GetGlobalOpt(FULLSCREEN))
        {
            screen = SDL_SetVideoMode(Opts_WindowWidth(), Opts_WindowHeight(), PIXEL_BITS, surfaceMode);
        }

        if (screen == NULL)
        {
            fprintf(stderr,
                    "\nError: I could not open the display.\n"
                    "The Simple DirectMedia error that occured was:\n"
                    "%s\n\n", SDL_GetError());
            cleanup_on_error();
            exit(1);
        }

        seticon();

        SDL_WM_SetCaption("Tux, of Math Command", "TuxMath");


    }
}


void load_data_files(void)
{
    /* Tell libt4k_common where TuxMath-specific data can be found */
    T4K_AddDataPrefix(DATA_PREFIX);
    if (!load_sound_data())
    {
        fprintf(stderr, "\nCould not load sound file - attempting to proceed without sound.\n");
        Opts_SetSoundHWAvailable(0);
    }

    /* This now has to come after loading the font, because it replaces
       a couple of images with translatable versions. */
    /* NOTE now the text code will load the font if it isn't already loaded */
    if (!load_image_data())
    {
        fprintf(stderr, "\nCould not load image file - exiting!\n");
        cleanup_on_error();
        exit(1);
    }
}



/* Create flipped versions of certain images; also set up the flip
   lookup table */
void generate_flipped_images(void)
{
    int i;

    /* Zero out the flip lookup table */
    for (i = 0; i < NUM_IMAGES; i++)
        flipped_img_lookup[i] = 0;

    for (i = 0; i < NUM_FLIPPED_IMAGES; i++) {
        flipped_images[i] = T4K_Flip(images[flipped_img[i]],1,0);
        flipped_img_lookup[flipped_img[i]] = i;
    }
}

/* Created images that are blends of two other images to smooth out
   the transitions. */
void generate_blended_images(void)
{
    blended_igloos[0] = T4K_Blend(images[IMG_IGLOO_REBUILDING1],NULL,0.06);
    blended_igloos[1] = T4K_Blend(images[IMG_IGLOO_REBUILDING1],NULL,0.125);
    blended_igloos[2] = T4K_Blend(images[IMG_IGLOO_REBUILDING1],NULL,0.185);
    blended_igloos[3] = T4K_Blend(images[IMG_IGLOO_REBUILDING1],NULL,0.25);
    blended_igloos[4] = T4K_Blend(images[IMG_IGLOO_REBUILDING1],NULL,0.5);
    blended_igloos[5] = T4K_Blend(images[IMG_IGLOO_REBUILDING1],NULL,0.75);
    blended_igloos[6] = images[IMG_IGLOO_REBUILDING1];
    blended_igloos[7] = T4K_Blend(images[IMG_IGLOO_REBUILDING2],images[IMG_IGLOO_REBUILDING1],0.25);
    blended_igloos[8] = T4K_Blend(images[IMG_IGLOO_REBUILDING2],images[IMG_IGLOO_REBUILDING1],0.5);
    blended_igloos[9] = T4K_Blend(images[IMG_IGLOO_REBUILDING2],images[IMG_IGLOO_REBUILDING1],0.75);
    blended_igloos[10] = images[IMG_IGLOO_REBUILDING2];
    blended_igloos[11] = T4K_Blend(images[IMG_IGLOO_INTACT],images[IMG_IGLOO_REBUILDING2],0.25);
    blended_igloos[12] = T4K_Blend(images[IMG_IGLOO_INTACT],images[IMG_IGLOO_REBUILDING2],0.5);
    blended_igloos[13] = T4K_Blend(images[IMG_IGLOO_INTACT],images[IMG_IGLOO_REBUILDING2],0.75);
    blended_igloos[14] = images[IMG_IGLOO_INTACT];
}


/* save options and free heap */
/* use for successful exit */
void cleanup(void)
{
    /* No longer write settings here, because we only */
    /* want to save settings from certain types of games. */
    //write_user_config_file();
    cleanup_memory();
    //  exit(0);
}



/* save options and free heap */
/* use for fail exit */
void cleanup_on_error(void)
{
    cleanup_memory();
    exit(1);
}



/* free any heap memory used during game DSB */
/* and also quit SDL properly:               */
/* NOTE - this function will get called twice if   */
/* exit occurs because of window close, so we      */
/* need to check all pointers before freeing them, */
/* and set them to NULL after freeing them, so we  */
/* avoid segfaults at exit from double free()      */
void cleanup_memory(void)
{
    int i;

    /* Free all images and sounds used by SDL: */
    for (i = 0; i < NUM_IMAGES; i++)
    {
        if (images[i])
            SDL_FreeSurface(images[i]);
        images[i] = NULL;
    }

    for (i = 0; i < NUM_SPRITES; i++)
    {
        if (sprites[i])
            T4K_FreeSprite(sprites[i]);
        sprites[i] = NULL;
    }

    for (i = 0; i < NUM_SOUNDS; i++)
    {
        if (sounds[i])
            Mix_FreeChunk(sounds[i]);
        sounds[i] = NULL;
    }

    for (i = 0; i < NUM_MUSICS; i++)
    {
        if (musics[i])
            Mix_FreeMusic(musics[i]);
        musics[i] = NULL;
    }

    if (lesson_list_titles)
    {
        for (i = 0; i < num_lessons; i++)
        {
            if (lesson_list_titles[i])
            {
                free(lesson_list_titles[i]);
                lesson_list_titles[i] = NULL;
            }
        }
        free(lesson_list_titles);
        lesson_list_titles = NULL;
    }

    if (lesson_list_filenames)
    {
        for (i = 0; i < num_lessons; i++)
        {
            if (lesson_list_filenames[i])
            {
                free(lesson_list_filenames[i]);
                lesson_list_filenames[i] = NULL;
            }
        }
        free(lesson_list_filenames);
        lesson_list_filenames = NULL;
    }

    if (lesson_list_goldstars)
    {
        free(lesson_list_goldstars);
        lesson_list_goldstars = NULL;
    }

    /* frees the game_options struct: */
    Opts_Cleanup();

    /* frees any heap used by MathCards: */
    if(local_game)
    {       
        MC_EndGame(local_game);
        free(local_game);
        local_game = NULL;
    }
    if(lan_game_settings)
    {       
        MC_EndGame(lan_game_settings);
        free(lan_game_settings);
        lan_game_settings = NULL;
    }

    /* Cleanup SDL+friends and anything else used by t4k_common: */
    CleanupT4KCommon();
}



/* Set the application's icon: */

void seticon(void)
{
    int masklen;
    Uint8* mask;
    SDL_Surface* icon;


    /* Load icon into a surface: */
    icon = IMG_Load(DATA_PREFIX "/images/icons/icon.png");
    if (icon == NULL)
    {
        fprintf(stderr,
                "\nWarning: I could not load the icon image: %s\n"
                "The Simple DirectMedia error that occured was:\n"
                "%s\n\n", DATA_PREFIX "/images/icons/icon.png", SDL_GetError());
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
}


void usage(int err, char* cmd)
{
    FILE* f;

    if (err == 0)
        f = stdout;
    else
        f = stderr;

    fprintf(f,
            "\nUsage: %s {--help | --usage | --copyright}\n"
            "          [--optionfile <filename>]\n"
            "          [--playthroughlist] [--answersfirst] [--answersmiddle]\n"
            "       %s [--fullscreen] [--nosound] [--nobackground]\n"
            "          [--demo] [--keypad] [--allownegatives]\n"
            //   "          [--operator {add | subtract | multiply | divide} ...]\n"
            "          [--speed <val>]\n"
            "\n", cmd, cmd);

    exit (err);
}

