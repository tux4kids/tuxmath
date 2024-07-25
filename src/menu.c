/*
   menu.c

   Functions responsible for loading, parsing and displaying game menu.

   Copyright 2009, 2010, 2011.
Authors:  Boleslaw Kulbabinski, David Bruce, Brendan Luchen.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

menu.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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



#include "globals.h"
#include "menu.h"
#include "titlescreen.h"
#include "highscore.h"
#include "factoroids.h"
#include "credits.h"
#include "multiplayer.h"
#include "mathcards.h"
#include "campaign.h"
#include "game.h"
#include "options.h"
#include "fileops.h"
#include "setup.h"

#ifdef HAVE_LIBSDL_NET
#include "menu_lan.h"
#include "network.h"
#include "server.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/* create string array of activities' names */
#define X(name) #name
static char* activities[] = { ACTIVITIES };
#undef X

/* actions available while viewing the menu */
//enum { NONE, CLICK, PAGEUP, PAGEDOWN, STOP_ESC, RESIZED };

/* stop button, left and right arrow positions do not
   depend on currently displayed menu */
//SDL_Rect menu_rect, stop_rect, prev_rect, next_rect;
//SDL_Surface *stop_button, *prev_arrow, *next_arrow, *prev_gray, *next_gray;

/*TODO: move these constants into a config file (maybe together with
  titlescreen paths and rects ? ) */
//const float menu_pos[4] = {0.38, 0.23, 0.55, 0.72};
//const float stop_pos[4] = {0.94, 0.0, 0.06, 0.06};
//const float prev_pos[4] = {0.87, 0.93, 0.06, 0.06};
//const float next_pos[4] = {0.94, 0.93, 0.06, 0.06};
//const char* stop_path = "status/stop.svg";
//const char* prev_path = "status/left.svg";
//const char* next_path = "status/right.svg";
//const char* prev_gray_path = "status/left_gray.svg";
//const char* next_gray_path = "status/right_gray.svg";
//const float button_gap = 0.2, text_h_gap = 0.4, text_w_gap = 0.5, button_radius = 0.27;
//const int min_font_size = 8, default_font_size = 20, max_font_size = 33;

/* menu title rect */
SDL_Rect menu_title_rect;

/* buffer size used when reading attributes or names */
const int buf_size = 128;



/* local functions */

int             run_menu(MenuType which, bool return_choice);
int             handle_activity(int act, int param);
int             run_academy(void);
int             run_arcade(int choice);
int             run_custom_game(void);
void            run_multiplayer(int mode, int difficulty);
int             run_factoroids(int choice);
int             run_lan_join(void);
int             run_lan_host(void);
int             stop_lan_host(void);

/* convenience wrapper for T4K_RunMenu */
int run_menu(MenuType which, bool return_choice)
{
    DEBUGCODE(debug_setup)
    {
        fprintf(stderr, "From run_menu():\n");
        print_locale_info(stderr);
    }

    return T4K_RunMenu(
            which,
            return_choice,
            &DrawTitleScreen,
            &HandleTitleScreenEvents,
            &HandleTitleScreenAnimations,
            &handle_activity);
}

/*
   handlers for specific game activities
   */

/* return QUIT if user decided to quit the application while running an activity
   return 0 otherwise */
int handle_activity(int act, int param)
{
    DEBUGMSG(debug_menu, "entering handle_activity()\n");
    DEBUGMSG(debug_menu, "act: %d\n", act);

    T4K_OnResolutionSwitch(NULL); //in case an activity forgets to register its own resolution switch handler, prevent insanity

    switch(act)
    {
        case RUN_CAMPAIGN:
            DEBUGMSG(debug_menu, "activity: RUN_CAMPAIGN\n");
            start_campaign();
            break;

        case RUN_ACADEMY:
            DEBUGMSG(debug_menu, "activity: RUN_ACADEMY\n");
            if(run_academy() == QUIT)
                return QUIT;
            break;

        case RUN_ARCADE:
            DEBUGMSG(debug_menu, "activity: RUN_ARCADE\n");
            run_arcade(param);
            break;

        case RUN_LAN_HOST:
            DEBUGMSG(debug_menu, "activity: RUN_LAN_HOST\n");
            run_lan_host();
            break;

        case STOP_LAN_HOST:
            DEBUGMSG(debug_menu, "activity: STOP_LAN_HOST\n");
            stop_lan_host();
            break;

        case RUN_LAN_JOIN:
            DEBUGMSG(debug_menu, "activity: RUN_LAN_JOIN\n");
            run_lan_join();
            break;

        case RUN_CUSTOM:
            DEBUGMSG(debug_menu, "activity: RUN_CUSTOM\n");
            run_custom_game();
            break;

        case RUN_HALL_OF_FAME:
            DisplayHighScores(CADET_HIGH_SCORE);
            break;

        case RUN_SCORE_SWEEP:
            run_multiplayer(0, param);
            break;

        case RUN_ELIMINATION:
            run_multiplayer(1, param);
            break;

        case RUN_HELP:
            Opts_SetHelpMode(1);
            Opts_SetDemoMode(0);
            if (Opts_GetGlobalOpt(MENU_MUSIC))  //Turn menu music off for game
            {T4K_AudioMusicUnload();}
            comets_game(local_game);
            if (Opts_GetGlobalOpt(MENU_MUSIC)) //Turn menu music back on
                T4K_AudioMusicLoad( "tuxi.ogg", -1 );
            Opts_SetHelpMode(0);
            break;

        case RUN_FACTORS:
            run_factoroids(0);
            break;

        case RUN_FRACTIONS:
            run_factoroids(1);
            break;

        case RUN_DEMO:
            if(read_named_config_file(local_game, "demo"))
            {
                T4K_AudioMusicUnload();
                comets_game(local_game);
                if (Opts_GetGlobalOpt(MENU_MUSIC))
                    T4K_AudioMusicLoad( "tuxi.ogg", -1 );
            }
            else
                fprintf(stderr, "\nCould not find demo config file\n");
            break;

        case RUN_INFO:
            {
                char msg[512];
                snprintf(msg, sizeof(msg), _("TuxMath version %s is free and open-source!\nYou can help make it better.\nSuggestions, artwork, and code are all welcome!\nDiscuss TuxMath at tuxmath-devel@list.sourceforge.net"), VERSION);
                ShowMessageWrap(DEFAULT_MENU_FONT_SIZE, msg);
            }
            break;

        case RUN_CREDITS:
            credits();
            break;

        case RUN_QUIT:
            return QUIT;

        case TOGGLE_TTS:
            Opts_SetTTSMode(!Opts_GetGlobalOpt(USE_TTS));
            break;
    }

    //re-register resolution switcher
    T4K_OnResolutionSwitch(&HandleTitleScreenResSwitch);
    //redraw if necessary
    RenderTitleScreen();

    if (Opts_GetGlobalOpt(MENU_MUSIC)) //Turn menu music back on
        T4K_AudioMusicLoad( "tuxi.ogg", T4K_AUDIO_LOOP_FOREVER );

    DEBUGMSG(debug_menu, "Leaving handle_activity\n");

    return 0;
}

int run_academy(void)
{
    int chosen_lesson = -1;
    T4K_OnResolutionSwitch(&HandleTitleScreenResSwitch);

    chosen_lesson = run_menu(MENU_LESSONS, true);
    while (chosen_lesson >= 0)
    {
        DEBUGMSG(debug_menu, "chosen_lesson: %d\n", chosen_lesson);
        if (Opts_GetGlobalOpt(MENU_SOUND))
            playsound(SND_POP);

        /* Re-read global settings first in case any settings were */
        /* clobbered by other lesson or arcade games this session: */
        read_global_config_file(local_game);
        /* Now read the selected file and play the "mission": */
        if (read_named_config_file(local_game, lesson_list_filenames[chosen_lesson]))
        {
            if (Opts_GetGlobalOpt(MENU_MUSIC))  //Turn menu music off for game
            {T4K_AudioMusicUnload();}

            T4K_OnResolutionSwitch(NULL);
            comets_game(local_game);
            T4K_OnResolutionSwitch(&HandleTitleScreenResSwitch);

            /* If successful, display Gold Star for this lesson! */
            if (MC_MissionAccomplished(local_game))
            {
                lesson_list_goldstars[chosen_lesson] = 1;
                /* and save to disk: */
                write_goldstars();
            }

            if (Opts_GetGlobalOpt(MENU_MUSIC)) //Turn menu music back on
            {T4K_AudioMusicLoad("tuxi.ogg", -1);}
        }
        else  // Something went wrong - could not read lesson config file:
        {
            fprintf(stderr, "\nCould not find file: %s\n", lesson_list_filenames[chosen_lesson]);
            chosen_lesson = -1;
        }
        // Let the user choose another lesson; start with the screen and
        // selection that we ended with
        chosen_lesson = run_menu(MENU_LESSONS, true);
    }
    return chosen_lesson;
}

int run_arcade(int choice)
{
    const char* arcade_config_files[5] =
    {"arcade/space_cadet",
        "arcade/scout",
        "arcade/ranger",
        "arcade/ace",
        "arcade/commando"
    };

    const int arcade_high_score_tables[5] =
    {CADET_HIGH_SCORE,
        SCOUT_HIGH_SCORE,
        RANGER_HIGH_SCORE,
        ACE_HIGH_SCORE,
        COMMANDO_HIGH_SCORE
    };

    int hs_table;

    if (choice < NUM_MATH_COMMAND_LEVELS) {
        // Play arcade game
        if (read_named_config_file(local_game, arcade_config_files[choice]))
        {
            T4K_AudioMusicUnload();
            comets_game(local_game);
            RenderTitleScreen();
            if (Opts_GetGlobalOpt(MENU_MUSIC))
                T4K_AudioMusicLoad( "tuxi.ogg", -1 );
            /* See if player made high score list!                        */
            read_high_scores();  /* Update, in case other users have added to it */
            hs_table = arcade_high_score_tables[choice];
            if (check_score_place(hs_table, Opts_LastScore()) < HIGH_SCORES_SAVED)
            {
                char player_name[HIGH_SCORE_NAME_LENGTH * 3];

                /* Get name from player: */
                HighScoreNameEntry(&player_name[0]);
                insert_score(player_name, hs_table, Opts_LastScore());
                /* Show the high scores. Note the user will see his/her */
                /* achievement even if (in the meantime) another player */
                /* has in fact already bumped this score off the table. */
                DisplayHighScores(hs_table);
                /* save to disk: */
                /* See "On File Locking" in fileops.c */
                append_high_score(choice,Opts_LastScore(),&player_name[0]);

                DEBUGCODE(debug_titlescreen)
                    print_high_scores(stderr);
            }
        }
        else {
            fprintf(stderr, "\nCould not find %s config file\n",arcade_config_files[choice]);
        }
    }
    return 0;
}


int run_custom_game(void)
{
    ShowMessageWrap(DEFAULT_MENU_FONT_SIZE, _("Edit 'options' file in you home directory to create customized game!\n"
                "Press a key or click your mouse to start game.\n"
                "See README.txt for more information.\n"));

    if (read_user_config_file(local_game)) {
        if (Opts_GetGlobalOpt(MENU_MUSIC))
            T4K_AudioMusicUnload();

        comets_game(local_game);
        write_user_config_file(local_game);

        if (Opts_GetGlobalOpt(MENU_MUSIC))
            T4K_AudioMusicLoad( "tuxi.ogg", -1 );
    }

    return 0;
}



void run_multiplayer(int mode, int difficulty)
{
    int nplayers = 0;
    char npstr[HIGH_SCORE_NAME_LENGTH * 3];

    while (nplayers <= 0 || nplayers > MAX_PLAYERS)
    {
        NameEntry(npstr, _("How many kids are playing?"),
                _("(Between 2 and 4 players)"), NULL);
        nplayers = atoi(npstr);
    }

    mp_set_parameter(PLAYERS, nplayers);
    mp_set_parameter(MODE, mode);
    mp_set_parameter(DIFFICULTY, difficulty);
    mp_run_multiplayer();
}



int run_factoroids(int choice)
{
    const int factoroids_high_score_tables[2] =
    {FACTORS_HIGH_SCORE, FRACTIONS_HIGH_SCORE};
    int hs_table;

    T4K_AudioMusicUnload();
    if(choice == 0)
        factors();
    else
        fractions();

    if (Opts_GetGlobalOpt(MENU_MUSIC))
        T4K_AudioMusicLoad( "tuxi.ogg", -1 );

    hs_table = factoroids_high_score_tables[choice];
    if (check_score_place(hs_table, Opts_LastScore()) < HIGH_SCORES_SAVED){
        char player_name[HIGH_SCORE_NAME_LENGTH * 3];
        /* Get name from player: */
        HighScoreNameEntry(&player_name[0]);
        insert_score(player_name, hs_table, Opts_LastScore());
        /* Show the high scores. Note the user will see his/her */
        /* achievement even if (in the meantime) another player */
        /* has in fact already bumped this score off the table. */
        DisplayHighScores(hs_table);
        /* save to disk: */
        /* See "On File Locking" in fileops.c */
        append_high_score(hs_table,Opts_LastScore(),&player_name[0]);
        DEBUGCODE(debug_titlescreen)
            print_high_scores(stderr);
    }
    else {
        fprintf(stderr, "\nCould not find config file\n");
    }

    return 0;
}


/* If pthreads available, we launch server in own thread.  Otherwise, we use  */
/* the C system() call to launch the server as a standalone program.          */
int run_lan_host(void)
{
#ifdef HAVE_LIBSDL_NET
    char msg[256];
    char buf[256];
    char server_name[150];
    char* serv_argv[3];
    int chosen_lesson = -1;

    /* For now, only allow one server instance: */
    if(OurServerRunning())
    {
        ShowMessageWrap(DEFAULT_MENU_FONT_SIZE, _("The server is already running"));
        return 0;
    }

    /* We still have possibility that a server is running on this machine as a standalone
     * program or within another tuxmath instance, in which case we still can't start our
     * server because the port is already taken:
     */

    if(!PortAvailable(DEFAULT_PORT))
    {
        ShowMessageWrap(DEFAULT_MENU_FONT_SIZE, _("The port is in use by another program on this computer, most likely another Tux Math server"));
        return 0;
    }

    NameEntry(server_name, _("Enter Server Name:"), _("(limit 50 characters)"), NULL);
    serv_argv[0] = "tuxmathserver";
    serv_argv[1] = "--name";
    //snprintf(buf, 256, "\"%s\"", server_name);
    snprintf(buf, 256, "%s", server_name);
    serv_argv[2] = buf;


    /* If we have POSIX threads available, we launch server in a thread within          */
    /* our same process. The server will use the currently selected Mathcards settings, */
    /* so we can let the user select the lesson for the server to use.                  */

#ifdef HAVE_PTHREAD_H

    ShowMessageWrap(DEFAULT_MENU_FONT_SIZE,_("Click or press key to select server lesson file"));

    {
        chosen_lesson = run_menu(MENU_LESSONS, true);

        while (chosen_lesson >= 0)
        {
            if (Opts_GetGlobalOpt(MENU_SOUND))
                playsound(SND_POP);

            /* Re-read global settings first in case any settings were */
            /* clobbered by other lesson or arcade games this session: */
            read_global_config_file(lan_game_settings);
            /* Now read the selected file and play the "mission": */
            if (read_named_config_file(lan_game_settings, lesson_list_filenames[chosen_lesson]))
                break;
            else
            {  // Something went wrong - could not read lesson config file:
                fprintf(stderr, "\nCould not find file: %s\n", lesson_list_filenames[chosen_lesson]);
                chosen_lesson = -1;
            }
            // Let the user choose another lesson; start with the screen and
            // selection that we ended with
            chosen_lesson = run_menu(MENU_LESSONS, true);
        }
        if(chosen_lesson==STOP) return 0;
    }
    sprintf(msg, _("Server Name:\n%s\nSelected Lesson:\n%s"), server_name, lesson_list_titles[chosen_lesson]);
    ShowMessageWrap(DEFAULT_MENU_FONT_SIZE, msg);

    DEBUGMSG(debug_lan, "About to launch RunServer_pthread() with:\n"
            "serv_argv[0] = %s\n"
            "serv_argv[1] = %s\n"
            "serv_argv[2] = %s\n", serv_argv[0], serv_argv[1], serv_argv[2]);
    RunServer_pthread(3, serv_argv);


    /* Without pthreads, we just launch standalone server, which for now only     */
    /* supports the hardcoded default settings.                                   */
#else

    DEBUGMSG(debug_lan, "About to launch RunServer_prog() with:\n"
            "serv_argv[0] = %s\n"
            "serv_argv[1] = %s\n"
            "serv_argv[2] = %s\n", serv_argv[0], serv_argv[1], serv_argv[2]);
    RunServer_prog(3, serv_argv);
#endif

    /* No SDL_net, so show explanatory message: */
#else
    ShowMessageWrap(DEFAULT_MENU_FONT_SIZE,_("\nSorry, this version built without network support."));
    printf( _("Sorry, this version built without network support.\n"));
#endif
    return 0;
}

int stop_lan_host(void)
{
    DEBUGMSG(debug_lan|debug_menu, "Entering stop_lan_join()\n");
    if(!OurServerRunning())
    {
        ShowMessageWrap(DEFAULT_MENU_FONT_SIZE, _("The server is not running."));
        return 0;
    }

    if(SrvrGameInProgress())
    {
        ShowMessageWrap(DEFAULT_MENU_FONT_SIZE, _("Cannot stop server until current game finishes."));
        return 0;
    }
    StopServer();
    ShowMessageWrap(DEFAULT_MENU_FONT_SIZE, _("The server has been stopped."));

    return 1;
}


int run_lan_join(void)
{
    DEBUGMSG(debug_menu|debug_lan, "Enter run_lan_join()\n");

#ifdef HAVE_LIBSDL_NET
    int pregame_status;

    /* autodetect servers, allowing player to choose if > 1 found: */
    if(!ConnectToServer())
        /* Could not connect: */
    {
        ShowMessageWrap(DEFAULT_MENU_FONT_SIZE, _("Sorry, could not connect to server."));
        DEBUGMSG(debug_menu|debug_lan, _("Sorry, could not connect to server.\n"));
        return 0;
    }

    /* Connected to server but not yet in game */
    pregame_status = Pregame();
    switch(pregame_status)
    {
        case PREGAME_OVER_START_GAME:
            playsound(SND_TOCK);
            T4K_AudioMusicUnload();
            Opts_SetLanMode(1);  // Tells game() we are playing over network
            comets_game(lan_game_settings);
            Opts_SetLanMode(0);  // Go back to local play
            if (Opts_GetGlobalOpt(MENU_MUSIC))
                T4K_AudioMusicLoad( "tuxi.ogg", -1 );
            break;

        case PREGAME_GAME_IN_PROGRESS:
            playsound(SND_TOCK);
            ShowMessageWrap(DEFAULT_MENU_FONT_SIZE, _("Sorry, game already in progress"));
            LAN_Cleanup();
            break;

        case PREGAME_OVER_LAN_DISCONNECT:
            playsound(SND_TOCK);
            ShowMessageWrap(DEFAULT_MENU_FONT_SIZE, _("Connection with server was lost"));
            LAN_Cleanup();
            break;

        case PREGAME_OVER_ESCAPE:
            LAN_Cleanup();
            return 0;

        default:
            { /* do nothing */ }

    }
#else
    ShowMessageWrap(DEFAULT_MENU_FONT_SIZE, _("Sorry, this version built without network support"));
    DEBUGMSG(debug_menu|debug_lan,  _("Sorry, this version built without network support.\n"));
    return 0;
#endif

    DEBUGMSG(debug_menu|debug_lan, "Leaving run_lan_join()\n");
    return 1;
}


/* load menu trees from disk and prerender them */
void LoadMenus(void)
{
    T4K_SetMenuSpritePrefix("sprites");
    T4K_SetActivitiesList(N_OF_ACTIVITIES, activities);
    /* main menu */
    T4K_LoadMenu(MENU_MAIN, "main_menu.xml");

    //NOTE level_menu.xml doesn't exist, and as it's not being used I'm skipping the load for now -Cheez
    /* difficulty menu */
    //  T4K_LoadMenu(MENU_DIFFICULTY, "level_menu.xml");
    T4K_SetMenuFontSize(MF_BESTFIT, 0);
    T4K_PrerenderAll();
}



/* create login menu tree, run it and set the user home directory
   -1 indicates that the user wants to quit without logging in,
   0 indicates that a choice has been made. */
int RunLoginMenu(void)
{
    const char *trailer_quit = "Quit";
    const char *trailer_back = "Back";
    int n_login_questions = 0;
    char **user_login_questions = NULL;
    char *title = NULL;
    int n_users = 0;
    char **user_names = NULL;
    int chosen_login = -1;
    int level;
    int i;
    char *trailer = NULL;
    SDLMod mod;

    DEBUGMSG(debug_menu, "Entering RunLoginMenu()");
    // Check for & read user_login_questions file
    n_login_questions = read_user_login_questions(&user_login_questions);

    // Check for & read user_menu_entries file
    n_users = read_user_menu_entries(&user_names);

    if (n_users == 0)
        return 0;   // a quick exit, there's only one user

    // Check for a highscores file
    if (high_scores_found_in_user_dir())
        set_high_score_path();

    level = 0;

    if (n_login_questions > 0)
        title = user_login_questions[0];

    T4K_CreateOneLevelMenu(MENU_LOGIN, n_users, user_names, NULL, title, trailer_quit);

    while (n_users) {
        // Get the user choice
        T4K_PrerenderMenu(MENU_LOGIN);
        chosen_login = run_menu(MENU_LOGIN, true);
        // Determine whether there were any modifier (CTRL) keys pressed
        mod = SDL_GetModState();
        if (chosen_login < 0 || chosen_login == n_users) {
            // User pressed escape or selected Quit/Back, handle by quitting
            // or going up a level
            if (level == 0) {
                // We are going to quit without logging in.
                // Clean up memory (prob. not necessary, but prevents Valgrind errors!)
                for (i = 0; i < n_login_questions; i++)
                    free(user_login_questions[i]);
                free(user_login_questions);
                for (i = 0; i < n_users; i++)
                    free(user_names[i]);
                free(user_names);
                return -1;
            }
            else {
                // Go back up one level of the directory tree
                user_data_dirname_up();
                level--;
            }
        }
        else {
            // User chose an entry, set it up
            user_data_dirname_down(user_names[chosen_login]);
            level++;
        }
        // Check for a highscores file
        if (high_scores_found_in_user_dir())
            set_high_score_path();
        // Free the entries from the previous menu
        for (i = 0; i < n_users; i++)
            free(user_names[i]);
        free(user_names);
        user_names = NULL;
        // If the CTRL key was pressed, choose this as the identity, even
        // if there is a lower level to the hierarchy
        if (mod & KMOD_CTRL)
            break;
        // Set the title appropriately for the next menu
        if (level < n_login_questions)
            title = user_login_questions[level];
        else
            title = NULL;
        if (level == 0)
            trailer = trailer_quit;
        else
            trailer = trailer_back;
        // Check to see if there are more choices to be made
        n_users = read_user_menu_entries(&user_names);
        T4K_CreateOneLevelMenu(MENU_LOGIN, n_users, user_names, NULL, title, trailer);
    }

    // The user home directory is set, clean up remaining memory
    for (i = 0; i < n_login_questions; i++)
        free(user_login_questions[i]);
    free(user_login_questions);

    // Signal success
    return 0;
}

/* run main menu. If this function ends it means that tuxmath is going to quit */
void RunMainMenu(void)
{
    int i;
    //  char* lltitle = "Lesson List"; //lesson list menu title
    char* icon_names[num_lessons];

    DEBUGMSG(debug_menu, "Entering RunMainMenu()\n");

    /* lessons menu */
    DEBUGMSG(debug_menu, "RunMainMenu(): Generating lessons submenu. (%d lessons)\n", num_lessons);

    for(i = 0; i < num_lessons; i++)
    {
        icon_names[i] = (lesson_list_goldstars[i] ? "goldstar" : "no_goldstar");
    }

    T4K_CreateOneLevelMenu(MENU_LESSONS, num_lessons, lesson_list_titles, icon_names, NULL, "Back");

    T4K_PrerenderMenu(MENU_LESSONS);

    run_menu(MENU_MAIN, false);
    DEBUGMSG(debug_menu, "Leaving RunMainMenu()\n");
}

