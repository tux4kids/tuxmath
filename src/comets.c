/*
   comets.c: Contains Comets game loop!

   Copyright 2001, 2002, 2003, 2004, 2006, 2007, 2008, 2009, 2010, 2011.
Authors: Bill Kendrick, David Bruce, Tim Holy, Brendan Luchen,
Akash Gangil, Jakub Spiewak.

Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

comets.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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




/* put this first so we get <config.h> and <gettext.h> immediately: */
#include "tuxmath.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"
#include "SDL_image.h"

#ifndef NOSOUND
#include "SDL_mixer.h"
#endif

/* Make sure we don't try to call network code if we built without */
/* network support:                                                */
#ifdef HAVE_LIBSDL_NET
#include "network.h"
#endif

#include "comets_graphics.h"
#include "transtruct.h"
#include "game.h"
#include "fileops.h"
#include "frame_counter.h"
#include "setup.h"
#include "mathcards.h"
#include "multiplayer.h"
#include "titlescreen.h"
#include "options.h"
#include "draw_utils.h"


#define CITY_EXPL_START (3 * 5)  /* Must be mult. of 5 (number of expl frames) */
#define ANIM_FRAME_START (4 * 2) /* Must be mult. of 2 (number of tux frames) */
#define GAMEOVER_COUNTER_START 40
#define LEVEL_START_WAIT_START 20
#define LASER_START 5
#define FLAPPING_START 12
#define FLAPPING_INTERVAL 500
#define STEAM_START 15
#define IGLOO_SWITCH_START 8
#define STANDING_COUNTER_START 8
#define EVAPORATING_COUNTER_START 100

#define PENGUIN_WALK_SPEED 90
#define SNOWFLAKE_SPEED 180
#define SNOWFLAKE_SEPARATION 3

#define MAX_LASER 10

#define POWERUP_Y_POS 100
#define MS_POWERUP_SPEED 250

#define SMARTBOMB_ICON_W 40 
#define SMARTBOMB_ICON_H 47 
#define SMARTBOMB_ICON_X screen->w - SMARTBOMB_ICON_W 
#define SMARTBOMB_ICON_Y screen->h - SMARTBOMB_ICON_H 

#define BASE_COMET_FONTSIZE 24

static MC_MathGame* curr_game;

static int powerup_comet_running = 0;
static int smartbomb_alive = 0;

const int SND_IGLOO_SIZZLE = SND_SIZZLE;

int user_quit_received;

/* Local (to game.c) 'globals': */

static char* comets_music_filenames[NUM_MUSICS] = {
    "01_rush.ogg",
    "02_on_the_edge_of_the_universe.ogg",
    "03_gravity.ogg",
    "game.mod",
    "game2.mod",
    "game3.mod",
};

static int gameover_counter;
static int comets_status;
static int total_questions_left;
static int paused;
static int wave;
static int score;
static int pre_wave_score;
static int prev_wave_comets;
static int slowdown;
static int comet_fontsize;
static int num_attackers;
static float speed;
static int demo_countdown;
static int tux_anim;
static int tux_anim_frame;
static int num_cities_alive;
static int tux_img;
static int old_tux_img;
static int neg_answer_picked;
static int tux_pressing;
static int doing_answer;
static int smartbomb_firing;
static int level_start_wait;
static int last_bkgd;
static int igloo_vertical_offset;
//static int extra_life_counter;
static int bonus_comet_counter;
static int extra_life_earned;
static int key_pressed;
static int game_over_other;
static int game_over_won;
static int network_error;
static int comets_halted_by_server;
/* Feedback-related variables */
static int city_expl_height;
static int comet_feedback_number;
static float comet_feedback_height;
static float danger_level;

static int digits[MC_MAX_DIGITS];

static comet_type* comets = NULL;
static powerup_comet_type* powerup_comet = NULL;

static city_type* cities = NULL;
static penguin_type* penguins = NULL;
static steam_type* steam = NULL;

static cloud_type cloud;
static laser_type laser[MAX_LASER];

static SDL_Surface* bkgd = NULL; //640x480 background (windowed)
static SDL_Surface* scaled_bkgd = NULL; //fullscreen resolution (from OS)


static game_message s1, s2, s3, s4, s5;
static int start_message_chosen = 0;

/*****************************************************************/
#ifdef HAVE_LIBSDL_NET
SDL_Surface* player_left_surf = NULL;
int player_left_time = 0;
SDL_Rect player_left_pos = {0};
#endif
/****************************************************************/

static help_controls_type help_controls;

/* Local function prototypes: */
static int  comets_initialize(void);
static void comets_cleanup(void);
static void comets_handle_help(void);
static void comets_handle_user_events(void);
static void comets_handle_demo(void);
static void comets_handle_answer(void);
static void comets_countdown(void);
static void comets_handle_tux(void);
static void comets_handle_comets(void);
static void comets_handle_cities(void);
static void comets_handle_penguins(void);
static void comets_handle_steam(void);
static void comets_handle_extra_life(void);
static void comets_draw(void);
static void comets_handle_game_over(int comets_status);

static SDL_Surface* current_bkgd()
{ return screen->flags & SDL_FULLSCREEN ? scaled_bkgd : bkgd; } //too clever for my brain to process

static int check_extra_life(void);
static int check_exit_conditions(void);

static void game_set_message(game_message*, const char* ,int x, int y);
static void comets_clear_message(game_message*);
static void comets_clear_messages(void);
void comets_write_message(const game_message* msg);
static void comets_write_messages(void);

static void reset_level(void);
static int add_comet(void);
static void add_score(int inc);
static void reset_comets(void);
static int num_comets_alive(void);

static void comets_mouse_event(SDL_Event event);
static void comets_key_event(SDLKey key, SDLMod mod);
static void free_on_exit(void);

static void help_add_comet(const char* formula_str, const char* ans_str);
static int help_renderframe_exit(void);
static void comets_recalc_positions(int xres, int yres);

int powerup_initialize(void);
PowerUp_Type powerup_gettype(void);
int powerup_add_comet(void);
void comets_handle_powerup(void);
void smartbomb_activate(void);

void putpixel(SDL_Surface* surface, int x, int y, Uint32 pixel);

/*****************************************************/
#ifdef HAVE_LIBSDL_NET
void comets_handle_net_messages(void);
void comets_handle_net_msg(char* buf);
int lan_add_comet(MC_FlashCard* fc);
int add_quest_recvd(char* buf);
int remove_quest_recvd(char* buf);
int wave_recvd(char* buf);
int player_left_recvd(char* buf);
int comets_halted_recvd(char* buf);
int erase_comet_on_screen(comet_type* zapped_comet, int answered_by);
//MC_FlashCard* search_queue_by_id(int id);
comet_type* search_comets_by_id(int id);
int compare_scores(const void* p1, const void* p2);
#endif
/******************************************************/

void print_current_quests(void);

static void print_exit_conditions(void);
static void print_status(void);





/* --- MAIN GAME FUNCTION!!! --- */


int comets_game(MC_MathGame* mgame)
{
    DEBUGMSG(debug_game, "Entering game():\n");

    srand(time(0));

    if(!mgame && !Opts_LanMode())
    {
        fprintf(stderr, "Error - null game struct passed for non_LAN game\n");
        return 0;
    }

    //Save this in a "file global" so we don't have to pass it to every function:
    curr_game = mgame;

    //see if the option matches the actual screen
    //FIXME figure out how this is happening so we don't need this workaround
    if (Opts_GetGlobalOpt(FULLSCREEN) == !(screen->flags & SDL_FULLSCREEN) )
    {
        fprintf(stderr, "\nWarning: Opts_GetGlobalOpt(FULLSCREEN) does not match"
                " actual screen resolution! Resetting selected option.\n");
        Opts_SetGlobalOpt(FULLSCREEN, !Opts_GetGlobalOpt(FULLSCREEN));
    }


    /* most code moved into smaller functions (comets_*()): */
    if (!comets_initialize())
    {
        fprintf(stderr, "\ncomets_initialize() failed!");
        return 0;
    }

    if (Opts_HelpMode()) {
        comets_handle_help();
        comets_cleanup();
        return GAME_OVER_OTHER;
    }

    DEBUGMSG(debug_game, "About to enter main game loop.\n");

    /* --- MAIN GAME LOOP: --- */
    do
    {
        FC_frame_begin();

        /* reset or increment various things with each loop: */
        old_tux_img = tux_img;
        tux_pressing = 0;
        int i;    
        for(i=0;i<MAX_LASER;i++)
        {
            if (laser[i].alive > 0)
                laser[i].alive -= 15*FC_time_elapsed;
        }
#ifdef HAVE_LIBSDL_NET
        /* Check for server messages if we are playing a LAN game: */
        if(Opts_LanMode())
        {    
            comets_handle_net_messages();
            /* Ask server to send our index if somehow we don't yet have it: */
            if(LAN_MyIndex() < 0)
                LAN_RequestIndex();
        }
#endif

        /* Most code now in smaller functions: */

        // 1. Check for user input
        comets_handle_user_events();
        // 2. Update state of various game elements
        comets_handle_demo();
        comets_handle_answer();
        comets_countdown();
        comets_handle_tux();
        comets_handle_comets();
        comets_handle_powerup();
        comets_handle_cities();
        comets_handle_penguins();
        comets_handle_steam();
        comets_handle_extra_life();
        // 3. Redraw:
        comets_draw();
        // 4. Figure out if we should leave loop:
        comets_status = check_exit_conditions();

        /* If we're in "PAUSE" mode, pause! */
        if (paused)
        {
            pause_game();
            paused = 0;
        }

        /* Keep playing music: */

#ifndef NOSOUND
        if(Opts_GetGlobalOpt(USE_SOUND))
        {
            if (!Mix_PlayingMusic())
            {
                T4K_AudioMusicLoad(comets_music_filenames[(rand() % NUM_MUSICS)], T4K_AUDIO_PLAY_ONCE);
            }
        }
#endif

        FC_frame_end();
    }
    while(GAME_IN_PROGRESS == comets_status);
    /* END OF MAIN GAME LOOP! */


    comets_handle_game_over(comets_status);

    comets_cleanup();

    /* Write post-game info to game summary file: */
    if (Opts_SaveSummary())
    {
        write_postgame_summary(curr_game);
    }

    /* Save score in case needed for high score table: */
    Opts_SetLastScore(score);

    /* Return the chosen command: */
    if (GAME_OVER_WINDOW_CLOSE == comets_status)
    {
        /* program exits: */
        cleanup();
        DEBUGMSG(debug_game, "Leaving game() from window close\n");
        return 1;
    }
    else
    {
        /* return to title() screen: */
        DEBUGMSG(debug_game, "Leaving game() normally\n");
        return comets_status;
    }
}



int comets_initialize(void)
{
    int i, img;

    DEBUGMSG(debug_game,"Entering comets_initialize()\n");
    DEBUGCODE(debug_game) print_game_options(stderr, 0);

    /* Clear window: */
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_Flip(screen);

    comets_status = GAME_IN_PROGRESS;
    gameover_counter = -1;
    user_quit_received = 0;
    game_over_won = 0;
    game_over_other = 0;
    network_error = 0;
    comets_halted_by_server = 0;

    /* Make sure we don't try to call network code if we built without  */
    /* network support:                                                 */
    /* NOTE with this check it should be safe to assume we have SDL_net */
    /* for the rest of this file if Opts_LanMode() == 1                 */
#ifndef HAVE_LIBSDL_NET
    Opts_SetLanMode(0);
#endif

    /* Start MathCards backend, unless we are getting questions from network: */
    /* FIXME may need to move this into tuxmath.c to accomodate option */
    /* to use MC_StartUsingWrongs() */
    /* NOTE MC_StartGame() will return 0 if the list length is zero due */
    /* (for example) to all math operations being deselected */
    /* NOTE if we are playing a network game, MC_StartGame() has already */
    /* been called on the server by the time we get to here:             */

    if(!Opts_LanMode())
    {
        if (!MC_StartGame(curr_game))
        {
            fprintf(stderr, "\nMC_StartGame() failed!");
            return 0;
        } 
        DEBUGMSG(debug_mathcards | debug_game,"MC_StartGame() finished.\n")
    }
    else  
    {
        /* Reset question queue and player name/score lists: */
        //int i;

        //for(i = 0; i < QUEST_QUEUE_SIZE; i ++)
        //MC_ResetFlashCard(&(quest_queue[i]));

        //  for(i = 0; i < MAX_CLIENTS; i++)
        //   {
        //    lan_player_info[i].name[0] = '\0';
        //   lan_player_info[i].score = -1;
        //     lan_player_info[i].mine = 0;
        //   }
        /* Ask server to send a message telling which socket is ours: */
        LAN_RequestIndex();
        /* Disable pausing and feedback mode: */
        Opts_SetAllowPause(0);
        Opts_SetUseFeedback(0);
    }

    /* Allocate memory */
    comets = NULL;  // set in case allocation fails partway through
    cities = NULL;
    penguins = NULL;
    steam = NULL;

    comets = (comet_type *) malloc(MAX_MAX_COMETS * sizeof(comet_type));
    if (comets == NULL)
    {
        fprintf(stderr, "Allocation of comets failed");
        return 0;
    }

    /* create only one powerup comet */
    powerup_comet = (powerup_comet_type *) malloc(sizeof(powerup_comet_type));
    if(powerup_comet == NULL)
    {
        fprintf(stderr, "Allocation of powerup comet failed");
        return 0;
    }

    cities = (city_type *) malloc(NUM_CITIES * sizeof(city_type));
    if (cities == NULL)
    {
        fprintf(stderr, "Allocation of cities failed");
        return 0;
    }

    penguins = (penguin_type *) malloc(NUM_CITIES * sizeof(penguin_type));
    if (penguins == NULL)
    {
        fprintf(stderr, "Allocation of penguins failed");
        return 0;
    }

    steam = (steam_type *) malloc(NUM_CITIES * sizeof(steam_type));
    if (steam == NULL)
    {
        fprintf(stderr, "Allocation of steam failed");
        return 0;
    }


    /* Write pre-game info to game summary file: */
    if (Opts_SaveSummary())
    {
        write_pregame_summary(curr_game);
    }

    /* Prepare to start the game: */
    city_expl_height = screen->h - images[IMG_CITY_BLUE]->h;

    /* Initialize feedback parameters */
    comet_feedback_number = 0;
    comet_feedback_height = 0;
    danger_level = Opts_DangerLevel();

    wave = 1;
    num_attackers = prev_wave_comets = Opts_StartingComets();
    speed = Opts_Speed()*15; //The old fps limit was 15
    slowdown = 0;
    score = 0;
    demo_countdown = 2000;
    total_questions_left = 0;
    level_start_wait = LEVEL_START_WAIT_START;
    neg_answer_picked = 0;
    smartbomb_firing = 0;

    /* (Create and position cities) */

    if (Opts_GetGlobalOpt(USE_IGLOOS))
        img = IMG_IGLOO_INTACT;
    else
        img = IMG_CITY_BLUE;
    for (i = 0; i < NUM_CITIES; i++)
    {
        cities[i].hits_left = 2;
        cities[i].status = CITY_PRESENT;
        cities[i].counter = 0;
        cities[i].threatened = 0;
        cities[i].layer = 0;

        /* Left vs. Right - makes room for Tux and the console */
        if (i < NUM_CITIES / 2)
        {
            cities[i].x = (((screen->w / (NUM_CITIES + 1)) * i) +
                    ((images[img] -> w) / 2));
        }
        else
        {
            cities[i].x = (screen->w -
                    ((((screen->w / (NUM_CITIES + 1)) *
                       (i - (NUM_CITIES / 2)) +
                       ((images[img] -> w) / 2)))));
        }
    }

    num_cities_alive = NUM_CITIES;

    igloo_vertical_offset = images[IMG_CITY_BLUE]->h - images[IMG_IGLOO_INTACT]->h;

    /* Create and position the penguins and steam */
    for (i = 0; i < NUM_CITIES; i++)
    {
        penguins[i].status = PENGUIN_HAPPY;
        penguins[i].counter = 0;
        penguins[i].x = cities[i].x;
        penguins[i].layer = 0;
        steam[i].status = STEAM_OFF;
        steam[i].layer = 0;
        steam[i].counter = 0;
    }

    if (Opts_BonusCometInterval())
    {
        bonus_comet_counter = Opts_BonusCometInterval() + 1;
        DEBUGMSG(debug_game,"\nInitializing with bonus_comet_counter = %d\n",bonus_comet_counter);
    }

    extra_life_earned = 0;
    cloud.status = EXTRA_LIFE_OFF;

    /* (Clear laser) */
    for(i= 0; i < MAX_LASER; i++)
        laser[i].alive = 0; 

    /* Assign all comet surfs to NULL initially: */
    for (i = 0; i < MAX_MAX_COMETS; i++)
    {
        comets[i].formula_surf = NULL;
        comets[i].answer_surf = NULL;
    }

    /* Reset remaining stuff: */
    comet_fontsize = (int)(BASE_COMET_FONTSIZE * get_scale());
    bkgd = scaled_bkgd = NULL;
    last_bkgd = -1;
    reset_comets();
    reset_level();
    powerup_initialize();

    paused = 0;
    doing_answer = 0;
    tux_pressing = 0;
    tux_img = IMG_TUX_RELAX1;
    tux_anim = -1;
    tux_anim_frame = 0;

    // Initialize the messages
    comets_clear_message(&s5);
    if (!start_message_chosen)
    {
        comets_clear_message(&s1);
        comets_clear_message(&s2);
        comets_clear_message(&s3);
        comets_clear_message(&s4);
    }

    help_controls.x_is_blinking = 0;
    help_controls.extra_life_is_blinking = 0;
    help_controls.laser_enabled = 1;

    //This tells t4k_common what function we want called
    //when the screen size changes.
    T4K_OnResolutionSwitch(comets_recalc_positions);

    DEBUGMSG(debug_game,"Exiting comets_initialize()\n");

    FC_init();

    return 1;
}


void comets_cleanup(void)
{
    DEBUGMSG(debug_game, "Enter comets_cleanup():\n");

    /* Stop music: */
#ifndef NOSOUND
    if(Opts_GetGlobalOpt(USE_SOUND))
    {
        if (Mix_PlayingMusic())
        {
            Mix_HaltMusic();
        }
    }
#endif


#ifdef HAVE_LIBSDL_NET  
    if (Opts_LanMode() )
        LAN_Cleanup();
#endif


    /* clear start message */
    start_message_chosen = 0;

    /* Free dynamically-allocated items */
    free_on_exit();

    DEBUGMSG(debug_game, "Leaving comets_cleanup():\n");
}


/* 
   Set one to four lines of text to display at the game's start. Eventually
   this should stylishly fade out over the first few moments of the game.
   */
void game_set_start_message(const char* m1, const char* m2,
        const char* m3, const char* m4)
{
    game_set_message(&s1, m1, -1, screen->h * 2 / 10);
    game_set_message(&s2, m2, screen->w / 2 - 40, screen->h * 3 / 10);
    game_set_message(&s3, m3, screen->w / 2 - 40, screen->h * 4 / 10);
    game_set_message(&s4, m4, screen->w / 2 - 40, screen->h * 5 / 10);
    start_message_chosen = 1;
}


void comets_handle_help(void)
{
    const int left_edge = 140;
    float timer = 0;
    int quit_help = 0;

    help_controls.laser_enabled = 0;
    help_controls.x_is_blinking = 0;
    help_controls.extra_life_is_blinking = 0;

    // Here are some things that have to happen before we can safely
    // draw the screen
    tux_img = IMG_TUX_CONSOLE1;
    old_tux_img = tux_img;
    tux_pressing = 0;

    // Write the introductory text
    game_set_message(&s1,_("Welcome to TuxMath!"),-1,50);

#ifndef NOSOUND
    if(Opts_GetGlobalOpt(USE_SOUND))
    {
        if (!Mix_PlayingMusic())
        {
            T4K_AudioMusicLoad(comets_music_filenames[(rand() % NUM_MUSICS)], T4K_AUDIO_PLAY_ONCE);
        }
    }
#endif

    // Wait 2 seconds while rendering frames
    while ((timer+=FC_time_elapsed) < 2 && !(quit_help = help_renderframe_exit()));
    if (quit_help)
        return;

    game_set_message(&s2,_("Your mission is to save your"), left_edge, 100);
    game_set_message(&s3,_("penguins' igloos from the"), left_edge, 135);
    game_set_message(&s4,_("falling comets."), left_edge, 170);

    timer = 0;
    while ((timer+=FC_time_elapsed) < 5 && !(quit_help = help_renderframe_exit()));  // wait 5 more secs
    if (quit_help)
        return;

    // Bring in a comet
    speed = 30;
    help_add_comet("2 + 1 = ?", "3");
    help_controls.laser_enabled = 1;
    level_start_wait = 0;

    timer = 0;
    while (comets[0].alive && (timer+=FC_time_elapsed) < 7 && !(quit_help = help_renderframe_exit())); // advance comet
    if (quit_help)
        return;

    if (comets[0].alive == 1) {
        game_set_message(&s1,_("Stop a comet by typing"),left_edge,100);
        game_set_message(&s2,_("the answer to the math problem"),left_edge,135);
        game_set_message(&s3,_("and hitting 'space' or 'enter'."),left_edge,170);
        game_set_message(&s4,_("Try it now!"),left_edge,225);

        speed = 0;
        while (comets[0].alive && !(quit_help = help_renderframe_exit()));
        if (quit_help)
            return;
    }

    game_set_message(&s1,_("Good shot!"),left_edge,100);
    comets_clear_message(&s2);
    comets_clear_message(&s3);
    comets_clear_message(&s4);

    help_controls.laser_enabled = 0;
    timer = 0;
    while ((timer+=FC_time_elapsed) < 3 && !(quit_help = help_renderframe_exit()));  // wait 3 secs

    speed = 30;
    game_set_message(&s1,_("If an igloo gets hit by a comet,"),left_edge,100);
    game_set_message(&s2,_("it melts. But don't worry: the"),left_edge,135);
    game_set_message(&s3,_("penguin is OK!"),left_edge,170);
    game_set_message(&s4,_("Just watch what happens:"),left_edge,225);
    game_set_message(&s5,_("(Press a key to start)"),left_edge,260);

    key_pressed = 0;
    while (!key_pressed && !(quit_help = help_renderframe_exit()));
    if (quit_help)
        return;
    comets_clear_message(&s5);

    help_add_comet("3 x 3 = ?", "9");
    comets[0].y = 2*(screen->h)/3;   // start it low down
    while ((comets[0].expl == -1) && !(quit_help = help_renderframe_exit()));  // wait 3 secs
    if (quit_help)
        return;
    game_set_message(&s4,_("Notice the answer"),left_edge,comets[0].y-100);
    help_renderframe_exit();
    SDL_Delay(4000);
    comets_clear_message(&s4);

    timer = 0;
    while ((timer+=FC_time_elapsed) < 5 && !(quit_help = help_renderframe_exit()));  // wait 5 secs
    if (quit_help)
        return;

    game_set_message(&s1,_("If it gets hit again, the"),left_edge,100);
    game_set_message(&s2,_("penguin leaves."),left_edge,135);
    game_set_message(&s3,_("(Press a key when ready)"),left_edge,200);

    key_pressed = 0;
    while (!key_pressed && !(quit_help = help_renderframe_exit()));
    if (quit_help)
        return;
    comets_clear_message(&s3);

    help_add_comet("56 ÷ 8 = ?", "7");
    comets[0].y = 2*(screen->h)/3;   // start it low down

    while (comets[0].alive && !(quit_help = help_renderframe_exit()));

    if (quit_help)
        return;
    timer = 0;

    while ((timer+=FC_time_elapsed) < 3 && !(quit_help = help_renderframe_exit()));

    if (quit_help)
        return;

    help_controls.laser_enabled = 1;
    game_set_message(&s1,_("You can fix the igloos"), left_edge,100);
    game_set_message(&s2,_("by stopping bonus comets."), left_edge,135);
    help_add_comet("2 + 2 = ?", "4");
    comets[0].bonus = 1;
    timer = 0;

    while (comets[0].alive && ((timer+=FC_time_elapsed) < 3) && !(quit_help = help_renderframe_exit()));

    if (quit_help)
        return;
    if (comets[0].alive)
        speed = 0;
    game_set_message(&s3,_("Zap it now!"),left_edge,225);

    while (comets[0].alive && !(quit_help = help_renderframe_exit()));

    if (quit_help)
        return;
    game_set_message(&s1,_("Great job!"),left_edge,100);
    comets_clear_message(&s2);
    comets_clear_message(&s3);
    timer = 0;

    while (((timer+=FC_time_elapsed) < 2) && !(quit_help = help_renderframe_exit()));

    if (quit_help)
        return;
    check_extra_life();
    timer = 0;

    while (((timer+=FC_time_elapsed) < 10) && !(quit_help = help_renderframe_exit()));

    if (quit_help)
        return;

    /* Demo of "superbonus" powerup comet: */
    help_controls.laser_enabled = 1;
    game_set_message(&s1,_("Fast-moving powerup comets"), left_edge,100);
    game_set_message(&s2,_("earn you a secret weapon:"), left_edge,135);
    powerup_add_comet();
    timer = 0;

    while (powerup_comet->comet.alive && ((timer+=FC_time_elapsed) < 1) && !(quit_help = help_renderframe_exit()));

    if (quit_help)
        return;
    if (powerup_comet->comet.alive)
        powerup_comet->inc_speed = 0;
    game_set_message(&s3,_("Zap it now!"),left_edge,225);

    while (powerup_comet->comet.alive && !(quit_help = help_renderframe_exit()));

    if (quit_help)
        return;

    game_set_message(&s1,_("Quit at any time by pressing"),left_edge,100);
    game_set_message(&s2,_("'Esc' or clicking the 'X'"),left_edge,135);
    game_set_message(&s3,_("in the upper right corner."),left_edge,170);
    game_set_message(&s4,_("Do it now, and then play!"),left_edge,225);

    help_controls.x_is_blinking = 1;

    while (!help_renderframe_exit());
}

// This function handles all the interactions expected during help
// screens and renders a single frame. This function normally returns
// 0, but returns 1 if the user chooses to exit help.
int help_renderframe_exit(void)
{
    FC_frame_begin();

    tux_pressing = 0;
    int i;
    for(i=0;i<MAX_LASER;i++)
    {
        if (laser[i].alive > 0)
            laser[i].alive -= 15*FC_time_elapsed;
    }
    comets_handle_user_events();
    comets_handle_answer();
    comets_handle_tux();
    comets_handle_comets();
    comets_handle_powerup();
    comets_handle_cities();
    comets_handle_penguins();
    comets_handle_steam();
    comets_handle_extra_life();
    comets_draw();
    comets_status = check_exit_conditions();

    // Delay to keep frame rate constant. Do this in a way
    // that won't cause a freeze if the timer wraps around.
    FC_frame_end();

    return (comets_status != GAME_IN_PROGRESS);
}

/* explicitly create a comet with a hardcoded problem */
void help_add_comet(const char* formula_str, const char* ans_str)
{
    //  char probstr[MC_FORMULA_LEN];
    //  char ansstr[MC_ANSWER_LEN];

    comets[0].alive = 1;
    comets[0].expl = -1;
    comets[0].answer = atoi(ans_str);
    //  num_comets_alive = 1;
    comets[0].city = 0;
    comets[0].x = cities[0].x;
    comets[0].y = 0;
    comets[0].zapped = 0;
    comets[0].bonus = 0;

    strncpy(comets[0].flashcard.formula_string,formula_str, MC_MaxFormulaSize() );
    strncpy(comets[0].flashcard.answer_string,ans_str,MC_MaxAnswerSize() );
    if(comets[0].formula_surf) SDL_FreeSurface(comets[0].formula_surf);
    if(comets[0].answer_surf) SDL_FreeSurface(comets[0].answer_surf);
    comets[0].formula_surf = T4K_BlackOutline(comets[0].flashcard.formula_string, comet_fontsize, &white);
    comets[0].answer_surf = T4K_BlackOutline(comets[0].flashcard.answer_string, comet_fontsize, &white);
}

void game_set_message(game_message *msg,const char *txt,int x,int y)
{
    if (msg && txt)
    {
        msg->x = x;
        msg->y = y;
        msg->alpha = SDL_ALPHA_OPAQUE;
        strncpy(msg->message,txt,GAME_MESSAGE_LENGTH);
    }
}

void comets_clear_message(game_message *msg)
{
    game_set_message(msg,"",0,0);
}

void comets_clear_messages()
{
    comets_clear_message(&s1);
    comets_clear_message(&s2);
    comets_clear_message(&s3);
    comets_clear_message(&s4);
    comets_clear_message(&s5);
}

void comets_write_message(const game_message *msg)
{
    SDL_Surface* surf;
    SDL_Rect rect;

    if (strlen(msg->message) > 0)
    {
        surf = T4K_BlackOutline( _(msg->message), DEFAULT_HELP_FONT_SIZE, &white);
        if(surf)
        {
            rect.w = surf->w;
            rect.h = surf->h;
            if (msg->x < 0)
                rect.x = (screen->w/2) - (rect.w/2);   // centered
            else
                rect.x = msg->x;              // left justified
            rect.y = msg->y;
            //FIXME alpha blending doesn't seem to work properly
            SDL_SetAlpha(surf, SDL_SRCALPHA, msg->alpha);
            SDL_BlitSurface(surf, NULL, screen, &rect);
            SDL_FreeSurface(surf);
        }
    }
}

void comets_write_messages(void)
{
    comets_write_message(&s1);
    comets_write_message(&s2);
    comets_write_message(&s3);
    comets_write_message(&s4);
    comets_write_message(&s5);
}

void comets_handle_user_events(void)
{
    SDL_Event event;
    SDLKey key;
    SDLMod mod;

    while (SDL_PollEvent(&event) > 0)
    {

        T4K_HandleStdEvents(&event);

        if (event.type == SDL_QUIT)
        {
            user_quit_received = GAME_OVER_WINDOW_CLOSE;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            key = event.key.keysym.sym;
            mod = event.key.keysym.mod;
            comets_key_event(key, mod);
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            comets_mouse_event(event);
        }
    }
}

void comets_handle_demo(void)
{
    /* If not in demo mode get out: */
    if (!Opts_DemoMode())
    {
        return;
    }

    /* Demo mode! */
    {
        static int demo_answer = 0;
        static int answer_digit = 0;
        static int picked_comet=-1;

        if (picked_comet == -1 && (rand() % 10) < 3)
        {
            /* Demo mode!  Randomly pick a comet to destroy: */
            picked_comet = (rand() % Opts_MaxComets());

            if (!(comets[picked_comet].alive &&
                        comets[picked_comet].expl == -1)
                    || comets[picked_comet].y < 80)
            {
                picked_comet = -1;
            }
            else
            {
                /* found a comet to blow up! */
                demo_answer = comets[picked_comet].answer;
                if ((rand() % 3) < 1)
                    demo_answer--;  // sometimes get it wrong on purpose

                DEBUGMSG(debug_game, "Demo mode, comet %d attacked with answer %d\n", picked_comet,demo_answer);

                /* handle negative answer: */
                if (demo_answer < 0)
                {
                    demo_answer = -demo_answer;
                    neg_answer_picked = 1;
                }
                if (demo_answer >= 100)
                    answer_digit = 0;
                else if (demo_answer >= 10)
                    answer_digit = 1;
                else
                    answer_digit = 2;
            }
        }

        /* Add a digit: */
        if (picked_comet != -1 && (FC_sprite_counter % 5) == 0 && (rand() % 10) < 8)
        {
            tux_pressing = 1;

            if (answer_digit < 3)
            {
                digits[0] = digits[1];
                digits[1] = digits[2];

                if (answer_digit == 0)
                {
                    digits[2] = demo_answer / 100;
                }
                else if (answer_digit == 1)
                {
                    digits[2] = (demo_answer % 100) / 10;
                }
                else if (answer_digit == 2)
                {
                    digits[2] = (demo_answer % 10);
                }

                answer_digit++;
            }
            else
            {
                /* "Press Return" */
                DEBUGMSG(debug_game, "Demo mode firing with these digits: %d%d%d\n",
                        digits[0], digits[1], digits[2]);
                doing_answer = 1;
                picked_comet = -1;
            }
        }

        /* Count down counter: */
        demo_countdown--;
    }
}

void comets_handle_answer(void)
{
    int i, j, num_zapped;
    int comets_answer[MAX_MAX_COMETS] = {-1}; 
    char ans[MC_MAX_DIGITS + 2]; //extra space for negative, and for final '\0'
    Uint32 ctime;
    int powerup_ans = 0;

    if (!doing_answer)
    {
        return;
    }

    doing_answer = 0;

    /* negative answer support DSB */

    ans[0] = '-'; //for math questions only, this is just replaced.
    for (i = 0; i < MC_MAX_DIGITS - 1 && !digits[i]; ++i); //skip leading 0s
    for (j = neg_answer_picked ? 1 : 0; i < MC_MAX_DIGITS; ++i, ++j)
        ans[j] = digits[i] + '0';
    ans[j] = '\0';

    num_zapped = 0;

    /* Register the indices of comets with correct answers in the comets_answer[]
     * array.  If smartbomb_firing, all answers are automatically correct here
     */
    for (i = 0; i < MAX_MAX_COMETS; i++)
    {
        if (comets[i].alive &&
                comets[i].expl == -1 &&
                (smartbomb_firing || 0 == strncmp(comets[i].flashcard.answer_string, ans, MC_MAX_DIGITS + 1))) 
        {
            comets_answer[num_zapped] = i;
            num_zapped++;
        }
    }

    smartbomb_firing = 0;

    /* powerup comet */
    if( powerup_comet->comet.alive && 
            strncmp(powerup_comet->comet.flashcard.answer_string, ans, MC_MAX_DIGITS + 1) == 0)
    {
        powerup_ans = 1;
    }

    /* If there was a comet with this answer, destroy it! */
    if (num_zapped != 0 || powerup_ans) 
    {
        float t;
        ctime = SDL_GetTicks();
        /* Store the time the question was present on screen (do this */
        /* in a way that avoids storing it if the time wrapped around */
        for(i = 0; i < num_zapped; i++)
        {
            int index_comets = comets_answer[i];
            if (ctime > comets[index_comets].time_started)
                t = ((float)(ctime - comets[index_comets].time_started)/1000);
            else
                t = -1;   //Mathcards will ignore t == -1
            /* Tell Mathcards or the server that we answered correctly: */
            if(Opts_LanMode())
#ifdef HAVE_LIBSDL_NET
                LAN_AnsweredCorrectly(comets[index_comets].flashcard.question_id, t);
#else
            {}  // Needed for compiler, even though this path can't occur
#endif      
            else
            {
                MC_AnsweredCorrectly(curr_game, comets[index_comets].flashcard.question_id, t);
            }
            /* Destroy comet: */
            comets[index_comets].expl = 0;
            comets[index_comets].zapped = 1;
            /* Fire laser: */
            laser[i].alive = LASER_START;
            laser[i].x1 = screen->w / 2;
            laser[i].y1 = screen->h;
            laser[i].x2 = comets[index_comets].x;
            laser[i].y2 = comets[index_comets].y;
            if(num_zapped == 1)
            {
                playsound(SND_LASER);
                playsound(SND_SIZZLE);
            }
            else if(num_zapped > 1 && i == num_zapped-1) //only play sounds once for group
            {
                playsound(SND_LASER);
                playsound(SND_SIZZLE);
                playsound(SND_EXTRA_LIFE);

                tux_anim = IMG_TUX_YES1;
                tux_anim_frame = ANIM_FRAME_START; 
            }

            /* Record data for feedback */
            if (Opts_UseFeedback())
            {
                comet_feedback_number++;
                comet_feedback_height += comets[index_comets].y/city_expl_height;

#ifdef FEEDBACK_DEBUG
                fprintf(stderr, "Added comet feedback with height %g\n",comets[index_comets].y/city_expl_height);
#endif
            }

            /* Pick Tux animation: */
            /* 50% of the time.. */
            if(num_zapped == 1)
            {
                if ((rand() % 10) < 5)
                {
                    /* ... pick an animation to play: */
                    if ((rand() % 10) < 5)
                        tux_anim = IMG_TUX_YES1;
                    else
                        tux_anim = IMG_TUX_YAY1;
                    tux_anim_frame = ANIM_FRAME_START;
                }
            }  

            /* Increment score: */

            /* [ add = 25, sub = 50, mul = 75, div = 100 ] */
            /* [ the higher the better ] */
            /* FIXME looks like it might score a bit differently based on screen mode? */
            add_score(25 * comets[index_comets].flashcard.difficulty *
                    (screen->h - comets[index_comets].y + 1) /
                    screen->h);
        } 

        if(powerup_ans)
        {
            powerup_comet->comet.expl = 0;
            powerup_comet->comet.zapped = 1;
            powerup_comet_running = 0;
            laser[i].alive = LASER_START;
            laser[i].x1 = screen->w / 2;
            laser[i].y1 = screen->h;
            laser[i].x2 = powerup_comet->comet.x;
            laser[i].y2 = powerup_comet->comet.y;

            /* Tell Mathcards or the server that we answered correctly: */
            /* NOTE - need to do this or the counter for the number of
             * remaining questions will not be correct - DSB
             */
            if (ctime > powerup_comet->comet.time_started)
                t = ((float)(ctime - powerup_comet->comet.time_started)/1000);
            else
                t = -1;   //Mathcards will ignore t == -1

            if(Opts_LanMode())
#ifdef HAVE_LIBSDL_NET
                LAN_AnsweredCorrectly(powerup_comet->comet.flashcard.question_id, t);
#else
            {}  // Needed for compiler, even though this path can't occur
#endif      
            else
                MC_AnsweredCorrectly(curr_game, powerup_comet->comet.flashcard.question_id, t);
        }
    }
    else
    {
        /* Didn't hit anything! */
        laser[0].alive = LASER_START;
        laser[0].x1 = screen->w / 2;
        laser[0].y1 = screen->h;
        laser[0].x2 = laser[0].x1;
        laser[0].y2 = 0;
        playsound(SND_LASER);
        playsound(SND_BUZZ);

        if ((rand() % 10) < 5)
            tux_img = IMG_TUX_DRAT;
        else
            tux_img = IMG_TUX_YIPE;
    }

    /* Clear digits: */
    for (i = 0; i < MC_MAX_DIGITS; ++i)
        digits[i] = 0;
    neg_answer_picked = 0;

}

void comets_countdown(void)
{
    if (level_start_wait <= 0)
    {
        comets_clear_messages();
        return;
    }

    //dim start messages
    s1.alpha -= SDL_ALPHA_OPAQUE / LEVEL_START_WAIT_START;
    s2.alpha -= SDL_ALPHA_OPAQUE / LEVEL_START_WAIT_START;
    s3.alpha -= SDL_ALPHA_OPAQUE / LEVEL_START_WAIT_START;
    s4.alpha -= SDL_ALPHA_OPAQUE / LEVEL_START_WAIT_START;
    DEBUGMSG(debug_game, "alpha = %d\n", s1.alpha);

    level_start_wait--;
    if (level_start_wait > LEVEL_START_WAIT_START / 4)
        tux_img = IMG_TUX_RELAX1;
    else if (level_start_wait > 0)
        tux_img = IMG_TUX_RELAX2;
    else
        tux_img = IMG_TUX_SIT;

    if (level_start_wait == LEVEL_START_WAIT_START / 4)
    {
        playsound(SND_ALARM);
    }
}

void comets_handle_tux(void)
{
    static int tux_same_counter;
    /* If Tux pressed a button, pick a new (different!) stance: */
    if (tux_pressing)
    {
        do { tux_img = IMG_TUX_CONSOLE1 + (rand() % 4); }
        while (tux_img == old_tux_img);
        playsound(SND_TOCK);
    }

    /* If Tux is being animated, show the animation: */
    if (tux_anim != -1)
    {
        tux_anim_frame--;
        if (tux_anim_frame < 0)
            tux_anim = -1;
        else
            tux_img = tux_anim + 1 - (tux_anim_frame / (ANIM_FRAME_START / 2));
    }

    /* Reset Tux to sitting if he's been doing nothing for a while: */
    if (old_tux_img == tux_img)
    {
        tux_same_counter++;
        if (tux_same_counter >= 20)
        {
            tux_img = IMG_TUX_SIT;
        }
    }
    else
        tux_same_counter = 0;
}

//FIXME might be simpler to store vertical position (and speed) in terms of time
//rather than absolute position, and determine the latter in comets_draw_comets()
void comets_handle_comets(void)
{
    /* Handle comets. Since the comets also are the things that trigger
       changes in the cities, we set some flags in them, too. */
    int i, this_city;
    Uint32 ctime;

    //  num_comets_alive = 0;

    /* Clear the threatened flag on each city */
    for (i = 0; i < NUM_CITIES; i++)
        cities[i].threatened = 0;

    for (i = 0; i < MAX_MAX_COMETS; i++)
    {
        if (comets[i].alive)
        {
            //     num_comets_alive++;
            this_city = comets[i].city;

            /* Update comet position */
            comets[i].x = comets[i].x + 0; /* no lateral motion for now! */
            /* Make bonus comet move faster at chosen ratio: */
            /* NOTE y increment scaled to make game play similar at any resolution */
            if (comets[i].bonus)
            {
                comets[i].y += FC_time_elapsed * speed * Opts_BonusSpeedRatio() *
                    city_expl_height / (480 - images[IMG_CITY_BLUE]->h);
            }
            else /* Regular comet: */
            {
                comets[i].y += FC_time_elapsed * speed *
                    city_expl_height / (480 - images[IMG_CITY_BLUE]->h);
            }

            /* Does it threaten a city? */
            if (comets[i].y > 3 * screen->h / 4)
                cities[this_city].threatened = 1;

            /* Did it hit a city? */
            if (comets[i].y >= city_expl_height &&
                    comets[i].expl == -1)
                /* Oh no - an igloo or city has been hit!        */     
            {
                /* Tell MathCards about it - question not answered correctly: */
                if(Opts_LanMode())
#ifdef HAVE_LIBSDL_NET
                    LAN_NotAnsweredCorrectly(comets[i].flashcard.question_id);
#else
                {}
#endif
                else
                    MC_NotAnsweredCorrectly(curr_game, comets[i].flashcard.question_id);


                /* Destroy comet: */
                comets[i].expl = 0;

                /* Store the time the question was present on screen (do this */
                /* in a way that avoids storing it if the time wrapped around */
                ctime = SDL_GetTicks();
                if (ctime > comets[i].time_started) {
                    MC_AddTimeToList(curr_game, (float)(ctime - comets[i].time_started)/1000);
                }

                /* Record data for speed feedback */
                /* Do this only for cities that are alive; dead cities */
                /* might not get much protection from the player */
                if (Opts_UseFeedback() && cities[this_city].hits_left) {
                    comet_feedback_number++;
                    comet_feedback_height += 1.0 + Opts_CityExplHandicap();

#ifdef FEEDBACK_DEBUG
                    fprintf(stderr, "Added comet feedback with height %g\n",
                            1.0 + Opts_CityExplHandicap());
#endif
                }

                /* Disable shields/destroy city/create steam cloud: */
                if (cities[this_city].hits_left)
                {
                    cities[this_city].status = CITY_EXPLODING;
                    if (Opts_GetGlobalOpt(USE_IGLOOS)) {
                        playsound(SND_IGLOO_SIZZLE);
                        cities[this_city].counter = IGLOO_SWITCH_START;
                        steam[this_city].status = STEAM_ON;
                        steam[this_city].counter = STEAM_START;
                    }
                    else {
                        if (cities[comets[i].city].hits_left == 2) {
                            playsound(SND_SHIELDSDOWN);
                            cities[this_city].counter = 1;  /* Will act immediately */
                        }
                        else {
                            playsound(SND_EXPLOSION);
                            cities[this_city].counter = CITY_EXPL_START;
                        }
                    }
                    cities[this_city].hits_left--;
                }

                /* If this was a bonus comet, restart the counter */
                if (comets[i].bonus)
                    bonus_comet_counter = Opts_BonusCometInterval()+1;

                /* If slow_after_wrong selected, set flag to go back to starting speed and */
                /* number of attacking comets: */
                if (Opts_SlowAfterWrong())
                {
                    speed = Opts_Speed()*15; //The old fps limit was 15;
                    slowdown = 1;
                }

                tux_anim = IMG_TUX_FIST1;
                tux_anim_frame = ANIM_FRAME_START;

            }

            /* Handle animation of any comets that are "exploding": */
            if (comets[i].expl >= 0)
            {
                comets[i].expl++;
                if (comets[i].expl >= sprites[IMG_COMET_EXPL]->num_frames * 2) {
                    comets[i].alive = 0;
                    comets[i].expl = -1;
                    if(comets[i].answer_surf)
                    {SDL_FreeSurface(comets[i].answer_surf); comets[i].answer_surf = NULL; }
                    if(comets[i].formula_surf)
                    {SDL_FreeSurface(comets[i].formula_surf); comets[i].formula_surf = NULL; }
                    if (bonus_comet_counter > 1 && comets[i].zapped) {
                        bonus_comet_counter--;
                        DEBUGMSG(debug_game, "bonus_comet_counter is now %d\n",bonus_comet_counter);
                    }
                    if (comets[i].bonus && comets[i].zapped) {
                        playsound(SND_EXTRA_LIFE);
                        extra_life_earned = 1;
                        DEBUGMSG(debug_game, "Extra life earned!");
                    }
                }
            }
        }
    }

    /* add more comets if needed: */

    /* Don't add comets if in Help mode: */
    if (Opts_HelpMode())
        return;

    /* Don't add comets until done waiting at start of new wave: */
    if (level_start_wait > 0)
        return;



    /* In LAN mode, the server keeps track of when to add comets
     * and when to go on to the next level.
     */
    if(!Opts_LanMode())
    {
        /* num_attackers is how many comets are left in wave */
        if (num_attackers <= 0)  /* Go on to next wave */
        {
            if (!num_comets_alive()
                    && !check_extra_life())
            {
                wave++;
                reset_level();
            }
        }
        else /* Get next question:  */
        {
            if (add_comet())
            {
                num_attackers--;
            }
        }
    }
}


void comets_handle_cities(void)
{
    /* Update the status of the cities. These also determine the changes
       in the penguins. */
    int i;
    num_cities_alive = 0;

    for (i = 0; i < NUM_CITIES; i++)
    {
        /* Note: when status is CITY_REBUILDING, status and image
           selection is handled by the extra_life code */
        if (cities[i].status == CITY_REBUILDING)
            continue;
        if (cities[i].hits_left)
            num_cities_alive++;
        /* Handle counter for animated explosion: */
        if (cities[i].status == CITY_EXPLODING)
        {
            cities[i].counter--;
            if (cities[i].counter == 0) {
                if (cities[i].hits_left)
                    cities[i].status = CITY_PRESENT;
                else {
                    if (Opts_GetGlobalOpt(USE_IGLOOS)) {
                        cities[i].status = CITY_EVAPORATING;
                        cities[i].counter = EVAPORATING_COUNTER_START;
                        cities[i].img = IMG_IGLOO_MELTED1;
                    } else {
                        cities[i].status = CITY_GONE;
                        cities[i].img = IMG_CITY_NONE;
                    }
                }
            }
        }
        /* Choose the correct city/igloo image */
        if (Opts_GetGlobalOpt(USE_IGLOOS)) {
            if (cities[i].status == CITY_EVAPORATING) {
                /* Handle the evaporation animation */
                cities[i].layer = 0;  /* these have to be drawn below the penguin */
                cities[i].counter--;
                if (cities[i].counter == 0) {
                    cities[i].img--;
                    if (cities[i].img < IMG_IGLOO_MELTED3) {
                        cities[i].img = IMG_CITY_NONE;
                        cities[i].status = CITY_GONE;
                    }
                    else
                        cities[i].counter = EVAPORATING_COUNTER_START;
                }
            }        else {
                if (cities[i].status != CITY_GONE) {
                    cities[i].layer = 1;  /* these have to be drawn above the penguin */
                    cities[i].img = IMG_IGLOO_MELTED1 + cities[i].hits_left;
                    /* If we're in the middle of an "explosion," don't switch to the
                       new igloo. Note the steam may have a different counter than
                       the igloo on this matter; the switch is designed to occur
                       halfway through the steam cloud. */
                    if (cities[i].status == CITY_EXPLODING)
                        cities[i].img++;
                }
            }
        }
        else {
            /* We're using the original "city" graphics */
            cities[i].layer = 0;   /* No layering needed */
            if (cities[i].hits_left)
                cities[i].img = IMG_CITY_BLUE;
            else if (cities[i].status == CITY_EXPLODING)
                cities[i].img = (IMG_CITY_BLUE_EXPL5 - (cities[i].counter / (CITY_EXPL_START / 5)));
            else
                cities[i].img = IMG_CITY_BLUE_DEAD;

            /* Change image to appropriate color: */
            cities[i].img = cities[i].img + ((wave % MAX_CITY_COLORS) *
                    (IMG_CITY_GREEN - IMG_CITY_BLUE));

        }
    }
}


void comets_handle_penguins(void)
{
    int i,direction,walk_counter;

    if (!Opts_GetGlobalOpt(USE_IGLOOS))
        return;
    for (i = 0; i < NUM_CITIES; i++) {
        penguins[i].layer = 0;
        if (cities[i].status == CITY_EVAPORATING)
            penguins[i].layer = 1;  /* will go higher in certain cases */
        /* Handle interaction with comets & city status (ducking) */
        if (cities[i].threatened && penguins[i].status < PENGUIN_WALKING_OFF
                && penguins[i].status != PENGUIN_OFFSCREEN)
            penguins[i].status = PENGUIN_DUCKING;
        else if (!cities[i].threatened && penguins[i].status == PENGUIN_DUCKING) {
            if (cities[i].hits_left == 2)
                penguins[i].status = PENGUIN_HAPPY;
            else
                penguins[i].status = PENGUIN_GRUMPY;
        }
        switch (penguins[i].status) {
            case PENGUIN_HAPPY:
                penguins[i].img = IMG_PENGUIN_FLAPDOWN;
                if (rand() % FLAPPING_INTERVAL == 0) {
                    penguins[i].status = PENGUIN_FLAPPING;
                    penguins[i].counter = FLAPPING_START;
                }
                break;
            case PENGUIN_FLAPPING:
                if (penguins[i].counter % 4 >= 2)
                    penguins[i].img = IMG_PENGUIN_FLAPUP;
                else
                    penguins[i].img = IMG_PENGUIN_FLAPDOWN;
                penguins[i].counter--;
                if (penguins[i].counter == 0)
                    penguins[i].status = PENGUIN_HAPPY;
                break;
            case PENGUIN_DUCKING:
                penguins[i].img = IMG_PENGUIN_INCOMING;
                break;
            case PENGUIN_GRUMPY:
                penguins[i].img = IMG_PENGUIN_GRUMPY;
                if (rand() % FLAPPING_INTERVAL == 0) {
                    penguins[i].status = PENGUIN_WORRIED;
                    penguins[i].counter = FLAPPING_START;
                }
                break;
            case PENGUIN_WORRIED:
                penguins[i].img = IMG_PENGUIN_WORRIED;
                penguins[i].counter--;
                if (penguins[i].counter == 0)
                    penguins[i].status = PENGUIN_GRUMPY;
                break;
            case PENGUIN_STANDING_UP:
                penguins[i].img = IMG_PENGUIN_STANDING_UP;
                penguins[i].counter--;
                if (penguins[i].counter == 0)
                    penguins[i].status = PENGUIN_WALKING_OFF;
                break;
            case PENGUIN_SITTING_DOWN:
                penguins[i].img = IMG_PENGUIN_SITTING_DOWN;
                penguins[i].counter--;
                if (penguins[i].counter == 0) {
                    penguins[i].status = PENGUIN_FLAPPING;
                    penguins[i].counter = FLAPPING_START;
                }
                break;
            case PENGUIN_WALKING_ON:
                walk_counter = (penguins[i].counter % 8)/2;
                if (walk_counter == 3)
                    walk_counter = 1;
                penguins[i].img = IMG_PENGUIN_WALK_ON1 + walk_counter;
                penguins[i].counter++;
                direction = 2*(i < NUM_CITIES/2)-1;  /* +1 for walk right, -1 for left */
                penguins[i].x += FC_time_elapsed*direction*PENGUIN_WALK_SPEED;
                if (direction*penguins[i].x >= direction*cities[i].x) {
                    penguins[i].status = PENGUIN_SITTING_DOWN;
                    penguins[i].counter = STANDING_COUNTER_START;
                    penguins[i].x = cities[i].x;
                }
                penguins[i].layer = 3;  /* Stand in front of steam */
                break;
            case PENGUIN_WALKING_OFF:
                walk_counter = (penguins[i].counter % 8)/2;
                if (walk_counter == 3)
                    walk_counter = 1;
                penguins[i].img = IMG_PENGUIN_WALK_OFF1 + walk_counter;
                penguins[i].counter++;
                direction = 1-2*(i < NUM_CITIES/2);
                penguins[i].x += FC_time_elapsed*direction*PENGUIN_WALK_SPEED;
                if (direction < 0) {
                    if (penguins[i].x + images[IMG_PENGUIN_WALK_OFF1]->w/2 <= 0)
                        penguins[i].status = PENGUIN_OFFSCREEN;
                } else {
                    if (penguins[i].x - images[IMG_PENGUIN_WALK_OFF1]->w/2 >= screen->w)
                        penguins[i].status = PENGUIN_OFFSCREEN;
                }
                penguins[i].layer = 3;
                break;
            case PENGUIN_OFFSCREEN:
                penguins[i].img = -1;
                break;
        }
    }
}

void comets_handle_steam(void)
{
    int i;

    if (!Opts_GetGlobalOpt(USE_IGLOOS))
        return;
    for (i = 0; i < NUM_CITIES; i++) {
        if (steam[i].counter) {
            steam[i].counter--;
            if (!steam[i].counter) {
                steam[i].status = STEAM_OFF;
                if (cloud.status != EXTRA_LIFE_ON || cloud.city != i) {
                    /* The penguin was ducking, now we can stop */
                    if (cities[i].hits_left)
                        penguins[i].status = PENGUIN_GRUMPY;
                    else {
                        penguins[i].status = PENGUIN_STANDING_UP;
                        penguins[i].counter = STANDING_COUNTER_START;
                    }
                }
            }
        }
        if (steam[i].status == STEAM_OFF)
            steam[i].img = -1;
        else {
            steam[i].img = IMG_STEAM5 - steam[i].counter/3;
            steam[i].layer = 2;
        }
    }
}

int check_extra_life(void)
{
    /* This is called at the end of a wave. Returns 1 if we're in the
       middle of handling an extra life, otherwise 0 */
    int i,fewest_hits_left,fewest_index,snow_width;

    if (cloud.status == EXTRA_LIFE_ON)
        return 1;
    DEBUGCODE(debug_game)
        print_status();

    if (extra_life_earned) {
        /* Check to see if any ingloo has been hit */
        fewest_hits_left = 2;
        fewest_index = -1;
        for (i = 0; i < NUM_CITIES; i++) {
            if (cities[i].hits_left < fewest_hits_left) {
                fewest_hits_left = cities[i].hits_left;
                fewest_index = i;
            }
        }
        if (fewest_hits_left == 2)
            return 0;   /* Don't need an extra life, there's no damage */
        /* Begin the extra life sequence */
        extra_life_earned = 0;
        cloud.status = EXTRA_LIFE_ON;
        cloud.y = screen->h/3;
        cloud.city = fewest_index;
        bonus_comet_counter = Opts_BonusCometInterval()+1;

        DEBUGMSG(debug_game, "Bonus comet counter restored to %d\n",bonus_comet_counter);

        if (cloud.city < NUM_CITIES/2)
            cloud.x = -images[IMG_CLOUD]->w/2;  /* come in from the left */
        else
            cloud.x = screen->w + images[IMG_CLOUD]->w/2; /* come from the right */
        penguins[cloud.city].status = PENGUIN_WALKING_ON;
        /* initialize the snowflakes */
        snow_width = images[IMG_CLOUD]->w - images[IMG_SNOW1]->w;
        for (i = 0; i < NUM_SNOWFLAKES; i++) {
            cloud.snowflake_y[i] = cloud.y - i*SNOWFLAKE_SEPARATION;
            cloud.snowflake_x[i] = - snow_width/2  + (rand() % snow_width);
            cloud.snowflake_size[i] = rand() % 3;
        }
        DEBUGCODE(debug_game)
            print_status();
        return 1;
    }
    else
        return 0;
}


void comets_handle_extra_life(void)
{
    // This handles the animation sequence during the rebuilding of an igloo
    int i, igloo_top, num_below_igloo, direction;

    if (cloud.status == EXTRA_LIFE_ON) {

        DEBUGCODE(debug_game)
        {
            if (penguins[cloud.city].status == PENGUIN_WALKING_OFF) {
                print_status();
                pause_game();
            }
        }

        // Get the cloud moving in the right direction, if not yet "parked"
        direction = 2*(cloud.city < NUM_CITIES/2) - 1;
        if (direction*cloud.x < direction*cities[cloud.city].x) {
            cloud.x += FC_time_elapsed*direction*PENGUIN_WALK_SPEED;
        }
        else {
            // Cloud is "parked," handle the snowfall and igloo rebuilding
            cities[cloud.city].status = CITY_REBUILDING;
            igloo_top = screen->h - igloo_vertical_offset
                - images[IMG_IGLOO_INTACT]->h;
            for (i = 0, num_below_igloo = 0; i < NUM_SNOWFLAKES; i++) {
                cloud.snowflake_y[i] += FC_time_elapsed*SNOWFLAKE_SPEED;
                if (cloud.snowflake_y[i] > igloo_top)
                    num_below_igloo++;
            }
            if (cloud.snowflake_y[NUM_SNOWFLAKES-1] > igloo_top) {
                cities[cloud.city].hits_left = 2;
                cities[cloud.city].img = IMG_IGLOO_INTACT; // completely rebuilt
            } else if (cities[cloud.city].hits_left == 0) {
                // We're going to draw one of the blended igloos
                // FIXME: It's a hack to encode a blended igloo with a negative number!
                penguins[cloud.city].layer = 0;
                cities[cloud.city].layer = 1;
                if (num_below_igloo < 3)
                    num_below_igloo = 0;   // Don't show progress until a few have fallen
                cities[cloud.city].img = -((float) (num_below_igloo)/NUM_SNOWFLAKES) * NUM_BLENDED_IGLOOS;
            }
            if (cloud.snowflake_y[NUM_SNOWFLAKES-1] > screen->h - igloo_vertical_offset) {
                /* exit rebuilding when last snowflake at igloo bottom */
                cloud.status = EXTRA_LIFE_OFF;
                cities[cloud.city].status = CITY_PRESENT;
            }
        }
    }
}

void comets_draw(void)
{
    SDL_Rect dest;

    /* Clear screen: */
    comets_draw_background(current_bkgd(), wave);

    /* Draw miscellaneous informational items */
    comets_draw_misc(curr_game, wave, extra_life_earned, bonus_comet_counter,
            score, total_questions_left, &help_controls);

    /* Draw cities/igloos and (if applicable) penguins: */
    comets_draw_cities(igloo_vertical_offset, &cloud, cities, penguins, steam);

    /* Draw smart bomb icon */
    comets_draw_smartbomb(smartbomb_alive);

    /* Draw normal comets first, then bonus comets */
    comets_draw_comets(comets);

    /* Draw powerup comet */
    comets_draw_powerup(powerup_comet);

    /* Draw laser: */
    int i;
    for(i = 0; i < MAX_LASER; i++)
    {
        if (laser[i].alive > 0)
        {
            draw_line(screen, laser[i].x1, laser[i].y1, laser[i].x2, laser[i].y2,
                    255 / ((LASER_START + 1) - laser[i].alive),
                    192 / ((LASER_START + 1) - laser[i].alive),
                    64);
        }
    }

    /* Draw numeric keypad: */
    if (Opts_GetGlobalOpt(USE_KEYPAD))
    {
        /* pick image to draw: */
        int keypad_image;
        if (MC_GetOpt(curr_game, ALLOW_NEGATIVES) )
        {
            /* draw regular keypad */
            keypad_image = IMG_KEYPAD;
        }
        else
        {
            /* draw keypad with with grayed-out '+' and '-' */
            keypad_image = IMG_KEYPAD_NO_NEG;
        }

        /* now draw it: */
        dest.x = (screen->w - images[keypad_image]->w) / 2;
        dest.y = (screen->h - images[keypad_image]->h) / 2;
        dest.w = images[keypad_image]->w;
        dest.h = images[keypad_image]->h;
        SDL_BlitSurface(images[keypad_image], NULL, screen, &dest);
    }

    /* Draw console, LED numbers, & tux: */
    comets_draw_led_console(curr_game, neg_answer_picked, digits);
    draw_console_image(tux_img);

    /* Draw any messages on the screen (used for the help mode) */
    comets_write_messages();

#ifdef HAVE_LIBSDL_NET
    /* Display message indicating that a player left */
    if(player_left_surf != NULL && (SDL_GetTicks() - player_left_time) < 2000)
        SDL_BlitSurface(player_left_surf, NULL, T4K_GetScreen(), &player_left_pos);
#endif

    /* Swap buffers: */
    SDL_Flip(screen);
}


int check_exit_conditions(void)
{
    //  int x;

    if (user_quit_received)
    {
        if (user_quit_received != GAME_OVER_WINDOW_CLOSE &&
                user_quit_received != GAME_OVER_ESCAPE &&
                user_quit_received != GAME_OVER_CHEATER)
        {
            fprintf(stderr, "Unexpected value %d for user_quit_received\n", user_quit_received);
            return GAME_OVER_OTHER;
        }
        return user_quit_received;    
    }

    /* determine if game lost (i.e. all igloos melted): */
    if (!num_cities_alive)
    {
        if (gameover_counter < 0)
            gameover_counter = GAMEOVER_COUNTER_START;
        gameover_counter--;
        if (gameover_counter == 0)
            return GAME_OVER_LOST;
    }

    /* determine if game won (i.e. all questions in mission answered correctly): */
    if(Opts_LanMode())
    {
        //Different victory display for LAN multiplayer game:
        if(game_over_won)
            return GAME_OVER_LAN_WON;
        //Also report if we lost the server:
        if(network_error)
            return GAME_OVER_LAN_DISCONNECT;
        //Also report if the server aborted the game:
        if(comets_halted_by_server)
            return GAME_OVER_LAN_HALTED;
    }
    else
    {
        if (MC_MissionAccomplished(curr_game))
        {
            DEBUGMSG(debug_game,"Mission accomplished!\n");
            return GAME_OVER_WON;
        }
    }


    /* Could have situation where mathcards doesn't have more questions */
    /* even though not all questions answered correctly:                */
    if(Opts_LanMode())
    {
        if(game_over_other)
            return GAME_OVER_OTHER;
    }
    else
    {
        if(!MC_TotalQuestionsLeft(curr_game))
            return GAME_OVER_OTHER;
    }


    //NOTE can't use this check in LAN mode because we don't know if the server has 
    //questions left

    /* Need to get out if no comets alive and MathCards has no questions left in list, */
    /* even though MathCards thinks there are still questions "in play".  */
    /* This SHOULD NOT HAPPEN and means we have a bug somewhere. */
    if (!Opts_LanMode())
    {
        if (!MC_ListQuestionsLeft(curr_game) && !num_comets_alive())
        {
            fprintf(stderr, "Error - no questions left but game not over\n");
            DEBUGMSG(debug_game, "ListQuestionsLeft() = %d ", MC_ListQuestionsLeft(curr_game));
            DEBUGMSG(debug_game, "num_comets_alive() = %d", num_comets_alive());
            return GAME_OVER_ERROR;
        }
    } 

    /* If using demo mode, see if counter has run out: */
    if (Opts_DemoMode())
    {
        if (demo_countdown <= 0 )
            return GAME_OVER_OTHER;
    }

    /* if we made it to here, the game goes on! */
    return GAME_IN_PROGRESS;
}


void print_exit_conditions(void)
{
    fprintf(stderr, "\ncomets_status:\t");
    switch (comets_status)
    {
        case GAME_IN_PROGRESS:
            {
                fprintf(stderr, "GAME_IN_PROGRESS\n");
                break;
            }
        case GAME_OVER_WON:
            {
                fprintf(stderr, "comets_OVER_WON\n");
                break;
            }
        case GAME_OVER_LOST:
            {
                fprintf(stderr, "GAME_OVER_LOST\n");
                break;
            }
        case GAME_OVER_OTHER:
            {
                fprintf(stderr, "GAME_OVER_OTHER\n");
                break;
            }
        case GAME_OVER_ESCAPE:
            {
                fprintf(stderr, "GAME_OVER_ESCAPE\n");
                print_status();
                break;
            }
        case GAME_OVER_WINDOW_CLOSE:
            {
                fprintf(stderr, "GAME_OVER_WINDOW_CLOSE\n");
                break;
            }
        case GAME_OVER_LAN_HALTED:
            {
                fprintf(stderr, "GAME_OVER_LAN_HALTED\n");
                break;
            }
        case GAME_OVER_LAN_DISCONNECT:
            {
                fprintf(stderr, "GAME_OVER_LAN_DISCONNECT\n");
                break;
            }
        case GAME_OVER_LAN_WON:
            {
                fprintf(stderr, "GAME_OVER_LAN_WON\n");
                break;
            }
        case GAME_OVER_ERROR:
            {
                fprintf(stderr, "GAME_OVER_ERROR\n");
                break;
            }
        default:
            {
                fprintf(stderr, "Unrecognized value\n");
                break;
            }
    }
}


void comets_handle_game_over(int game_status)
{
    DEBUGCODE(debug_game)
    {    
        fprintf(stderr, "Entering comets_handle_comets_over() - game status = %d\n", game_status);
        print_exit_conditions();
    }

    /* For turn-based multiplayer, don't show victory screen after
     * each player's "game":
     */
    if(mp_get_parameter(PLAYERS))
        return;

    /* TODO: need better "victory" screen with animation, special music, etc., */
    /* as well as options to review missed questions, play again using missed  */
    /* questions as question list, etc.                                        */
    switch (game_status)
    {
        SDL_Rect dest_message;
        SDL_Rect dest_tux;
        SDL_Event event;

        case GAME_OVER_WON:
        {
            int looping = 1;
            int tux_offset = 0;
            int tux_step = -3;

            /* set up victory message: */
            dest_message.x = (screen->w - images[IMG_GAMEOVER_WON]->w) / 2;
            dest_message.y = (screen->h - images[IMG_GAMEOVER_WON]->h) / 2;
            dest_message.w = images[IMG_GAMEOVER_WON]->w;
            dest_message.h = images[IMG_GAMEOVER_WON]->h;

            do
            {
                FC_frame_begin();

                while (SDL_PollEvent(&event) > 0)
                {
                    if  (event.type == SDL_QUIT
                            || event.type == SDL_KEYDOWN
                            || event.type == SDL_MOUSEBUTTONDOWN)
                    {
                        looping = 0;
                    }
                }

                if (current_bkgd() )
                    SDL_BlitSurface(current_bkgd(), NULL, screen, NULL);

                /* draw flashing victory message: */
                if ((FC_sprite_counter / 2) % 4)
                {
                    SDL_BlitSurface(images[IMG_GAMEOVER_WON], NULL, screen, &dest_message);
                }

                /* draw dancing tux: */
                draw_console_image(IMG_CONSOLE_BASH);
                /* walk tux back and forth */
                tux_offset += tux_step;
                /* select tux_egypt images according to which way tux is headed: */
                if (tux_step < 0)
                    tux_img = IMG_TUX_EGYPT1 + ((FC_sprite_counter / 3) % 2);
                else
                    tux_img = IMG_TUX_EGYPT3 + ((FC_sprite_counter / 3) % 2);

                /* turn around if we go far enough: */
                if (tux_offset >= (screen->w)/2
                        || tux_offset <= -(screen->w)/2)
                {
                    tux_step = -tux_step;
                }

                dest_tux.x = ((screen->w - images[tux_img]->w) / 2) + tux_offset;
                dest_tux.y = (screen->h - images[tux_img]->h);
                dest_tux.w = images[tux_img]->w;
                dest_tux.h = images[tux_img]->h;

                SDL_BlitSurface(images[tux_img], NULL, screen, &dest_tux);

                /* draw_console_image(tux_img);*/

                SDL_Flip(screen);
                FC_frame_end();
            }
            while (looping);
            break;
        }

        case GAME_OVER_LAN_WON:
        {
            int looping = 1;
            int tux_offset = 0;
            int tux_step = -3;
            int i = 0;
            int rank = 1;
            int entries = 0;
            int first = 1;
            char str[64];
            SDL_Surface* surf = NULL;
            SDL_Rect loc;

            //Adjust font size for resolution:
            int fontsize = (int)(DEFAULT_MENU_FONT_SIZE * get_scale());


            //For sorted list of scores:
            lan_player_type sorted_scores[MAX_CLIENTS];
            /* Sort scores: */
            for(i = 0; i < MAX_CLIENTS; i++)
            {
                strncpy(sorted_scores[i].name, LAN_PlayerName(i), NAME_SIZE);
                sorted_scores[i].mine = LAN_PlayerMine(i);
                sorted_scores[i].score = LAN_PlayerScore(i);
                sorted_scores[i].connected = LAN_PlayerConnected(i);
            }         
            qsort((void*)sorted_scores, MAX_CLIENTS, sizeof(lan_player_type), compare_scores);


            /* Begin display loop: */
            do
            {
                FC_frame_begin();
                entries = 0;
                rank = 1;

                while (SDL_PollEvent(&event) > 0)
                {
                    if  (event.type == SDL_QUIT
                            || event.type == SDL_KEYDOWN
                            || event.type == SDL_MOUSEBUTTONDOWN)
                    {
                        looping = 0;
                    }
                }

                if (current_bkgd() )
                    SDL_BlitSurface(current_bkgd(), NULL, screen, NULL);

                /* Make top heading a little bigger: */
                surf = T4K_BlackOutline(_("The Penguins Have Been Saved!"),
                        1.2 * fontsize, &white);
                if(surf)
                {
                    loc.x = screen->w/2 - surf->w/2;
                    loc.y = surf->h * 2;
                    loc.w = surf->w;
                    loc.h = surf->h;
                    /* Make this blink: */
                    if ((FC_sprite_counter / 2) % 4)
                        SDL_BlitSurface(surf, NULL, screen, &loc);
                    SDL_FreeSurface(surf);
                    surf = NULL;
                }

                surf = T4K_BlackOutline(_("Final Scores:"), fontsize, &white);
                if(surf)
                {
                    loc.x = screen->w/2 - surf->w/2;
                    loc.y += surf->h;
                    loc.w = surf->w;
                    loc.h = surf->h;
                    SDL_BlitSurface(surf, NULL, screen, &loc);
                    SDL_FreeSurface(surf);
                    surf = NULL;
                }


                /* draw sorted list of scores: */
                for (i = 0; i < MAX_CLIENTS; i++)
                {
                    if(sorted_scores[i].connected)
                    {
                        snprintf(str, 64, "%d.\t%s: %d", rank, sorted_scores[i].name, sorted_scores[i].score);
                        rank++;
                        if(sorted_scores[i].mine)
                            surf = T4K_BlackOutline(str, fontsize, &yellow);
                        else
                            surf = T4K_BlackOutline(str, fontsize, &white);
                        if(surf)
                        {
                            if(first)
                            {
                                loc.x = screen->w/2 - surf->w/2;
                                first = 0;
                            }
                            loc.w = surf->w;
                            loc.h = surf->h;
                            loc.y += surf->h;
                            SDL_BlitSurface(surf, NULL, screen, &loc);
                            entries++;
                            SDL_FreeSurface(surf);
                            surf = NULL;
                        }
                    }
                }
                /* draw dancing tux: */
                draw_console_image(IMG_CONSOLE_BASH);
                /* walk tux back and forth */
                tux_offset += tux_step;
                /* select tux_egypt images according to which way tux is headed: */
                if (tux_step < 0)
                    tux_img = IMG_TUX_EGYPT1 + ((FC_sprite_counter / 3) % 2);
                else
                    tux_img = IMG_TUX_EGYPT3 + ((FC_sprite_counter / 3) % 2);

                /* turn around if we go far enough: */
                if (tux_offset >= (screen->w)/2
                        || tux_offset <= -(screen->w)/2)
                {
                    tux_step = -tux_step;
                }

                dest_tux.x = ((screen->w - images[tux_img]->w) / 2) + tux_offset;
                dest_tux.y = (screen->h - images[tux_img]->h);
                dest_tux.w = images[tux_img]->w;
                dest_tux.h = images[tux_img]->h;

                SDL_BlitSurface(images[tux_img], NULL, screen, &dest_tux);

                /* draw_console_image(tux_img);*/

                SDL_Flip(screen);
                FC_frame_end();
            }
            while (looping);
            break;
        }

        case GAME_OVER_LAN_HALTED:
        {
            ShowMessageWrap(DEFAULT_MENU_FONT_SIZE, 
                    _("Network game terminated by server.\n The server is still running.")); 
            break;
        }


        case GAME_OVER_LAN_DISCONNECT:
        {
            ShowMessageWrap(DEFAULT_MENU_FONT_SIZE, 
                    _("Network game terminated.\n Connection with server was lost.")); 
            break;
        }

        case GAME_OVER_ERROR:
        DEBUGMSG(debug_game, "game() exiting with error:\n");
        case GAME_OVER_LOST:
        case GAME_OVER_OTHER:
        {
            int looping = 1;

            /* set up GAMEOVER message: */
            dest_message.x = (screen->w - images[IMG_GAMEOVER]->w) / 2;
            dest_message.y = (screen->h - images[IMG_GAMEOVER]->h) / 2;
            dest_message.w = images[IMG_GAMEOVER]->w;
            dest_message.h = images[IMG_GAMEOVER]->h;

            do
            {
                FC_frame_begin();

                while (SDL_PollEvent(&event) > 0)
                {
                    if  (event.type == SDL_QUIT
                            || event.type == SDL_KEYDOWN
                            || event.type == SDL_MOUSEBUTTONDOWN)
                    {
                        looping = 0;
                    }
                }

                SDL_BlitSurface(images[IMG_GAMEOVER], NULL, screen, &dest_message);
                SDL_Flip(screen);

                FC_frame_end();
            }
            while (looping);

            break;
        }

        case GAME_OVER_ESCAPE:
        case GAME_OVER_WINDOW_CLOSE:
        break;
    }
}



/* Reset stuff for the next level! */
void reset_level(void)
{
    char fname[1024];
    int i;
    int next_wave_comets;
    int use_feedback;
    float comet_avg_height, height_differential;


    /* Clear all comets: */
    /* NOTE - we should not need to reset them to not alive, as the wave is supposed to be over.
     * In LAN game, we can clobber the first comet in the new wave if the messages come out of order,
     * so we just warn here instead of resetting "alive" to 0 - DSB
     * We initialize all the fields at the start of the game independent of this function anyway.
     */

    for (i = 0; i < MAX_MAX_COMETS; i++)
    {
        DEBUGCODE(debug_game)
        {
            if(comets[i].alive)
                fprintf(stderr, "Warning - changing wave but comet[%d] still alive (could be OK in LAN mode)\n", i);
        }
        //comets[i].alive = 0;
    }

    /* Clear LED: */

    for (i = 0; i < MC_MAX_DIGITS; ++i)
        digits[i] = 0;
    neg_answer_picked = 0;

    /* Load random background image, but ensure it's different from this one: */
    for (i = last_bkgd; i == last_bkgd; i = rand() % NUM_BKGDS);

    last_bkgd = i;

    sprintf(fname, "backgrounds/%d.jpg", i);

    if (bkgd != NULL)
    {
        SDL_FreeSurface(bkgd);
        bkgd = NULL;
    }
    if (scaled_bkgd != NULL)
    {
        SDL_FreeSurface(scaled_bkgd);
        scaled_bkgd = NULL;
    }

    if (Opts_UseBkgd())
    {
        T4K_LoadBothBkgds(fname, &scaled_bkgd, &bkgd);
        if (bkgd == NULL || scaled_bkgd == NULL)
        {
            fprintf(stderr,
                    "\nWarning: Could not load background image:\n"
                    "%s\n"
                    "The Simple DirectMedia error that ocurred was: %s\n",
                    fname, SDL_GetError());
            Opts_SetUseBkgd(0);
        }
    }



    /* Record score before this wave: */

    pre_wave_score = score;

    /* Set speed and number of comets for this wave.
     * Note that in LAN mode, the number of comets is handled by the 
     * server, and feedback/slowdown are disallowed, so we just set
     * the speed and get out.
     *  */

    if (Opts_LanMode())
    {
        speed *= DEFAULT_SPEEDUP_FACTOR;
        return;
    }

    /* Rest of function ONLY for non-LAN mode: ----------------- */

    /* On first wave or if slowdown flagged due to wrong answer: */
    if (wave == 1 || slowdown)
    {
        next_wave_comets = Opts_StartingComets();
        speed = Opts_Speed()*15; //The old fps limit was 15;
        slowdown = 0;
    }

    else /* Otherwise increase comets and speed if selected, not to */
        /* exceed maximum:                                         */
    {
        next_wave_comets = prev_wave_comets;
        if (Opts_AllowSpeedup())
        {
            next_wave_comets += Opts_ExtraCometsPerWave();
            if (next_wave_comets > Opts_MaxComets())
            {
                next_wave_comets = Opts_MaxComets();
            }

            use_feedback = Opts_UseFeedback();

            if (use_feedback)
            {
#ifdef FEEDBACK_DEBUG
                fprintf(stderr, "Evaluating feedback...\n  old danger level = %g,",danger_level);
#endif

                /* Update our danger level, i.e., the target height */
                danger_level = 1 - (1-danger_level) /
                    Opts_DangerLevelSpeedup();
                if (danger_level > Opts_DangerLevelMax())
                    danger_level = Opts_DangerLevelMax();

#ifdef FEEDBACK_DEBUG
                fprintf(stderr, " new danger level = %g.\n",danger_level);
#endif

                /* Check to see whether we have any feedback data. If not, skip it. */
                if (comet_feedback_number == 0)
                {
                    use_feedback = 0;  /* No comets above living cities, skip feedback */

#ifdef FEEDBACK_DEBUG
                    fprintf(stderr, "No feedback data available, aborting.\n\n");
#endif
                }
                else
                {
                    /* Compute the average height of comet destruction. */
                    comet_avg_height = comet_feedback_height/comet_feedback_number;

                    /* Determine how this average height compares with target. */
                    height_differential = comet_avg_height - danger_level;

                    /* Set the speed so that we move halfway towards the target */
                    /* height. That makes the changes a bit more conservative. */

#ifdef FEEDBACK_DEBUG
                    fprintf(stderr, "  comet average height = %g, height differential = %g.\n",
                            comet_avg_height, height_differential);
                    fprintf(stderr, "  old speed = %g,",speed);
#endif

                    speed *= (1 - height_differential/danger_level/2);

                    /* Enforce bounds on speed */
                    if (speed < MINIMUM_SPEED)
                        speed = MINIMUM_SPEED;
                    if (speed > Opts_MaxSpeed())
                        speed = Opts_MaxSpeed();

#ifdef FEEDBACK_DEBUG
                    fprintf(stderr, " new speed = %g.\n",speed);
                    fprintf(stderr, "Feedback evaluation complete.\n\n");
#endif
                }
            }

            if (!use_feedback)
            {
                /* This is not an "else" because we might skip feedback */
                /* when comet_feedback_number == 0 */
                speed = speed * Opts_SpeedupFactor();
                if (speed > Opts_MaxSpeed())
                {
                    speed = Opts_MaxSpeed();
                }
            }
        }
    }

    comet_feedback_number = 0;
    comet_feedback_height = 0;

    prev_wave_comets = next_wave_comets;
    num_attackers = next_wave_comets;
}



/* Add a comet to the game (if there's room): */
int add_comet(void)
{
    static int prev_city = -1;
    int i;
    float y_spacing;

    int com_found = -1;

    y_spacing = (images[IMG_NUMS]->h) * 1.5;

    /* Return if any previous comet too high up to create another one yet: */
    for (i = 0; i < MAX_MAX_COMETS; i++)
    {
        if (comets[i].alive)
            if (comets[i].y < y_spacing)
            {
                DEBUGMSG(debug_game,
                        "add_comet() - returning because comet[%d] not"
                        " far enough down: %f\n", i, comets[i].y);
                return 0;
            }
    }  

    /* Now look for a free comet slot: */
    for (i = 0; i < MAX_MAX_COMETS; i++)
    {
        if (!comets[i].alive)
        {
            com_found = i;
            break;
        }
    }

    if (-1 == com_found)
    {
        /* free comet slot not found - no comet added: */
        DEBUGMSG(debug_game, "add_comet() called but no free comet slot\n");
        DEBUGCODE(debug_game) print_current_quests();
        return 0;
    }

    if (!MC_NextQuestion(curr_game, &(comets[com_found].flashcard)))
    {
        /* no more questions available - cannot create comet.  */
        return 0;
    }

    DEBUGCODE(debug_game)
    {
        fprintf(stderr, "In add_comet(), card is\n");
        print_card(comets[com_found].flashcard);
    }

    /* Make sure question is "sane" before we add it: */
    if( (comets[com_found].flashcard.answer > 999)
            ||(comets[com_found].flashcard.answer < -999))
    {
        fprintf(stderr, "Warning, card with invalid answer encountered: %d\n",
                comets[com_found].flashcard.answer);
        MC_ResetFlashCard(&(comets[com_found].flashcard));
        return 0;
    }

    /* If we make it to here, create a new comet!*/
    comets[com_found].answer = comets[com_found].flashcard.answer;
    comets[com_found].alive = 1;
    if(comets[com_found].formula_surf) SDL_FreeSurface(comets[com_found].formula_surf);
    if(comets[com_found].answer_surf) SDL_FreeSurface(comets[com_found].answer_surf);
    comets[com_found].formula_surf = T4K_BlackOutline(comets[com_found].flashcard.formula_string, comet_fontsize, &white);
    comets[com_found].answer_surf = T4K_BlackOutline(comets[com_found].flashcard.answer_string, comet_fontsize, &white);
    //  num_comets_alive++;

    /* Pick a city to attack that was not attacked last time */
    /* (so formulas are less likely to overlap). */
    do
    {
        i = rand() % NUM_CITIES;
    }
    while (i == prev_city);

    prev_city = i;

    /* Set in to attack that city: */
    comets[com_found].city = i;
    /* Start at the top, above the city in question: */
    comets[com_found].x = cities[i].x;
    comets[com_found].y = 0;
    comets[com_found].zapped = 0;
    /* Should it be a bonus comet? */
    comets[com_found].bonus = 0;

    DEBUGMSG(debug_game, "bonus_comet_counter is %d\n",bonus_comet_counter);

    if (bonus_comet_counter == 1)
    {
        bonus_comet_counter = 0;
        comets[com_found].bonus = 1;
        playsound(SND_BONUS_COMET);
        DEBUGMSG(debug_game, "Created bonus comet");
    }

    DEBUGMSG(debug_game, "add_comet(): formula string is: %s\n", comets[com_found].flashcard.formula_string);

    /* Record the time at which this comet was created */
    comets[com_found].time_started = SDL_GetTicks();

    /* If enabled, add powerup comet occasionally:
    */
    if(Opts_UsePowerupComets()
            && !mp_get_parameter(PLAYERS)  // no powerups in mp game, for now 
            && !powerup_comet_running)
    {
        int t = rand()%Opts_PowerupFreq();
        if( t < 1 )
        {
            powerup_add_comet();
        } 
    }
    /* comet slot found and question found so return successfully: */
    return 1;
}


/* Translates mouse events into keyboard events when on-screen keypad used */
/* or when exit button clicked.                                            */
void comets_mouse_event(SDL_Event event)
{
    int keypad_w, keypad_h, x, y, row, column;
    SDLKey key = SDLK_UNKNOWN;
    SDLMod mod = event.key.keysym.mod;
    keypad_w = 0;
    keypad_h = 0;

    /* Check to see if user clicked exit button: */
    /* The exit button is in the upper right corner of the screen: */
    if(event.button.button == SDL_BUTTON_LEFT &&
            (event.button.x >= (screen->w - images[IMG_STOP]->w)) &&
            (event.button.y <= images[IMG_STOP]->h))
    {
        key = SDLK_ESCAPE;
        comets_key_event(key, mod);
        return;
    }

    if(event.button.button == SDL_BUTTON_LEFT &&
            event.button.x >= SMARTBOMB_ICON_X && event.button.x <= SMARTBOMB_ICON_X+SMARTBOMB_ICON_W &&
            event.button.y >= SMARTBOMB_ICON_Y && event.button.y <= SMARTBOMB_ICON_Y+SMARTBOMB_ICON_H)
    {
        if(smartbomb_alive)
        {
            smartbomb_activate();
            smartbomb_alive = 0;
        }
    }

    /* get out unless we really are using keypad */
    if ( level_start_wait
            || Opts_DemoMode()
            || !Opts_GetGlobalOpt(USE_KEYPAD))
    {
        return;
    }


    /* make sure keypad image is valid and has non-zero dimensions: */
    /* FIXME maybe this checking should be done once at the start */
    /* of game() rather than with every mouse click */
    if (MC_GetOpt(curr_game, ALLOW_NEGATIVES))
    {
        if (!images[IMG_KEYPAD])
            return;
        else
        {
            keypad_w = images[IMG_KEYPAD]->w;
            keypad_h = images[IMG_KEYPAD]->h;
        }
    }
    else
    {
        if (!images[IMG_KEYPAD_NO_NEG])
            return;
        else
        {
            keypad_w = images[IMG_KEYPAD]->w;
            keypad_h = images[IMG_KEYPAD]->h;
        }
    }

    if (!keypad_w || !keypad_h)
    {
        return;
    }


    /* only proceed if click falls within keypad: */
    if (!((event.button.x >=
                    (screen->w / 2) - (keypad_w / 2) &&
                    event.button.x <=
                    (screen->w / 2) + (keypad_w / 2) &&
                    event.button.y >=
                    (screen->h / 2) - (keypad_h / 2) &&
                    event.button.y <=
                    (screen->h / 2) + (keypad_h / 2))))
        /* click outside of keypad - do nothing */
    {
        return;
    }

    else /* click was within keypad */
    {
        x = (event.button.x - ((screen->w / 2) - (keypad_w / 2)));
        y = (event.button.y - ((screen->h / 2) - (keypad_h / 2)));

        /* Now determine what onscreen key was pressed */
        /*                                             */
        /* The on-screen keypad has a 4 x 4 layout:    */
        /*                                             */
        /*    *********************************        */
        /*    *       *       *       *       *        */
        /*    *   7   *   8   *   9   *   -   *        */
        /*    *       *       *       *       *        */
        /*    *********************************        */
        /*    *       *       *       *       *        */
        /*    *   4   *   5   *   6   *       *        */
        /*    *       *       *       *       *        */
        /*    *************************   +   *        */
        /*    *       *       *       *       *        */
        /*    *   1   *   2   *   3   *       *        */
        /*    *       *       *       *       *        */
        /*    *********************************        */
        /*    *       *                       *        */
        /*    *   0   *         Enter         *        */
        /*    *       *                       *        */
        /*    *********************************        */
        /*                                             */
        /*  The following code simply figures out the  */
        /*  row and column based on x and y and looks  */
        /*  up the SDlKey accordingly.                 */

        column = x/((keypad_w)/4);
        row    = y/((keypad_h)/4);

        /* make sure row and column are sane */
        if (column < 0
                || column > 3
                || row    < 0
                || row    > 3)
        {
            fprintf(stderr, "\nIllegal row or column value!\n");
            return;
        }

        /* simple but tedious - I am sure this could be done more elegantly */

        if (0 == row)
        {
            if (0 == column)
                key = SDLK_7;
            if (1 == column)
                key = SDLK_8;
            if (2 == column)
                key = SDLK_9;
            if (3 == column)
                key = SDLK_MINUS;
        }
        if (1 == row)
        {
            if (0 == column)
                key = SDLK_4;
            if (1 == column)
                key = SDLK_5;
            if (2 == column)
                key = SDLK_6;
            if (3 == column)
                key = SDLK_PLUS;
        }
        if (2 == row)
        {
            if (0 == column)
                key = SDLK_1;
            if (1 == column)
                key = SDLK_2;
            if (2 == column)
                key = SDLK_3;
            if (3 == column)
                key = SDLK_PLUS;
        }
        if (3 == row)
        {
            if (0 == column)
                key = SDLK_0;
            if (1 == column)
                key = SDLK_RETURN;
            if (2 == column)
                key = SDLK_RETURN;
            if (3 == column)
                key = SDLK_RETURN;
        }

        if (key == SDLK_UNKNOWN)
        {
            return;
        }

        /* now can proceed as if keyboard was used */
        comets_key_event(key, mod);
    }
}

/* called by either key presses or mouse clicks on */
/* on-screen keypad */
void comets_key_event(SDLKey key, SDLMod mod)
{
    int i;
    key_pressed = 1;   // Signal back in cases where waiting on any key

    if (key == SDLK_ESCAPE)
    {
        /* Escape key - quit! */
        user_quit_received = GAME_OVER_ESCAPE;
    }
    DEBUGCODE(debug_game)
    {
        if (key == SDLK_LEFTBRACKET) //a nice nonobvious/unused key
        {
            user_quit_received = GAME_OVER_CHEATER;
        }
    }
    else if (key == SDLK_TAB
            || key == SDLK_p)
    {
        /* [TAB] or [P]: Pause! (if settings allow) */
        if (Opts_AllowPause())
        {
            paused = 1;
        }
    }

    /* Adjust speed if settings allow: */
    else if (key == SDLK_UP)
    {
        if (Opts_AllowPause())
        {
            speed *= 1.2;
        }
    }

    else if (key == SDLK_DOWN)
    {
        if (Opts_AllowPause())
        {
            speed /= 1.2;
        }
    }

    if (level_start_wait > 0 || Opts_DemoMode() || !help_controls.laser_enabled)
    {
        /* Eat other keys until level start wait has passed,
           or if game is in demo mode: */
        key = SDLK_UNKNOWN;
    }


    /* The rest of the keys control the numeric answer console: */

    if (key >= SDLK_0 && key <= SDLK_9)
    {
        /* [0]-[9]: Add a new digit: */
        for (i = 0; i < MC_MAX_DIGITS-1; ++i)
            digits[i] = digits[i+1];
        digits[MC_MAX_DIGITS-1] = key - SDLK_0;

        //    digits[0] = digits[1];
        //    digits[1] = digits[2];
        //    digits[2] = key - SDLK_0;
        tux_pressing = 1;
    }
    else if (key >= SDLK_KP0 && key <= SDLK_KP9)
    {
        /* Keypad [0]-[9]: Add a new digit: */
        for (i = 0; i < MC_MAX_DIGITS-1; ++i)
            digits[i] = digits[i+1];
        digits[MC_MAX_DIGITS-1] = key - SDLK_KP0;

        //    digits[0] = digits[1];
        //    digits[1] = digits[2];
        //    digits[2] = key - SDLK_KP0;
        tux_pressing = 1;
    }
    /* support for negative answer input DSB */
    else if ((key == SDLK_MINUS || key == SDLK_KP_MINUS)
            && MC_GetOpt(curr_game, ALLOW_NEGATIVES) )  /* do nothing unless neg answers allowed */
    {
        /* allow player to make answer negative: */
        neg_answer_picked = 1;
        tux_pressing = 1;
    }
    else if (     /* Effort to make logical operators clear: */
            (
             ( /* HACK this hard-codes the plus sign to the US layout: */
               (key == SDLK_EQUALS) && (mod & KMOD_SHIFT)
             ) 
             ||
             (
              key == SDLK_KP_PLUS
             )
            )
            &&
            MC_GetOpt(curr_game, ALLOW_NEGATIVES)
            )  /* do nothing unless neg answers allowed */
    {
        /* allow player to make answer positive: */
        fprintf(stderr, "SDKL_PLUS received\n");
        neg_answer_picked = 0;
        tux_pressing = 1;
    }
    else if (key == SDLK_BACKSPACE ||
            key == SDLK_CLEAR ||
            key == SDLK_DELETE)
    {
        /* [BKSP]: Clear digits! */
        for (i = 0; i < MC_MAX_DIGITS; ++i)
            digits[i] = 0;
        tux_pressing = 1;
    }
    else if (key == SDLK_RETURN ||
            key == SDLK_KP_ENTER ||
            key == SDLK_SPACE)
    {
        /* [ENTER]: Accept digits! */
        doing_answer = 1;
    }
    else if(key == SDLK_LSHIFT || key == SDLK_RSHIFT)
    {
        if(smartbomb_alive)
        {
            smartbomb_activate();
            smartbomb_alive = 0;
        }
    }
}

/* Increment score: */

void add_score(int inc)
{
    score += inc;
    DEBUGMSG(debug_game,"Score is now: %d\n", score);
    /* For turn-based multiplayer game, update score in mp info: */
    if (mp_get_parameter(PLAYERS)) 
    {
        int new_score = mp_get_player_score(mp_get_currentplayer()) + inc;
        mp_set_player_score(mp_get_currentplayer(), new_score);
    }
}



void reset_comets(void)
{
    int i = 0;

    for (i = 0; i < MAX_MAX_COMETS; i++)
    {
        comets[i].alive = 0;
        comets[i].expl = -1;
        comets[i].city = 0;
        comets[i].x = 0;
        comets[i].y = 0;
        comets[i].answer = 0;
        MC_ResetFlashCard(&(comets[i].flashcard));
        comets[i].bonus = 0;
        if(comets[i].formula_surf) SDL_FreeSurface(comets[i].formula_surf);
        comets[i].formula_surf = NULL;
        if(comets[i].answer_surf) SDL_FreeSurface(comets[i].answer_surf);
        comets[i].answer_surf = NULL;
    }
}


void print_status(void)
{
    int i;

    fprintf(stderr, "\nCities:");
    fprintf(stderr, "\nHits left: ");
    for (i = 0; i < NUM_CITIES; i++)
        fprintf(stderr, "%02d ",cities[i].hits_left);
    fprintf(stderr, "\nStatus:    ");
    for (i = 0; i < NUM_CITIES; i++)
        fprintf(stderr, "%02d ",cities[i].status);

    fprintf(stderr, "\nPenguins:");
    fprintf(stderr, "\nStatus:    ");
    for (i = 0; i < NUM_CITIES; i++)
        fprintf(stderr, "%02d ",penguins[i].status);

    fprintf(stderr, "\nCloud:");
    fprintf(stderr, "\nStatus:    %d",cloud.status);
    fprintf(stderr, "\nCity:      %d",cloud.city);
    fprintf(stderr, "\n");
}


void free_on_exit(void)
{
    int i;

    DEBUGMSG(debug_game,"Enter free_on_exit\n");

    for (i = 0; i < MAX_MAX_COMETS; ++i)
    {
        DEBUGMSG(debug_game,"About to free surfaces for comet %d\n", i);
        if (comets[i].formula_surf)
        {
            SDL_FreeSurface(comets[i].formula_surf);
            comets[i].formula_surf = NULL;
        }
        if (comets[i].answer_surf)
        {
            SDL_FreeSurface(comets[i].answer_surf);
            comets[i].answer_surf = NULL;
        }
    }

    if(comets)
    {
        free(comets);
        comets = NULL;
    }

    if(cities)
    {
        free(cities);
        cities = NULL;
    }

    if(penguins)
    {
        free(penguins);
        penguins = NULL;
    }

    if(steam)
    {
        free(steam);
        steam = NULL;
    }

    if(powerup_comet)
    {
        free(powerup_comet);
        powerup_comet = NULL;
    }

    /* Free background: */
    if (bkgd)
    {
        SDL_FreeSurface(bkgd);
        bkgd = NULL;
    }
    if (scaled_bkgd)
    {
        SDL_FreeSurface(scaled_bkgd);
        scaled_bkgd = NULL;
    }

#ifdef HAVE_LIBSDL_NET  
    if(player_left_surf)
    {
        SDL_FreeSurface(player_left_surf);
        player_left_surf = NULL;
    }
#endif

    DEBUGMSG(debug_game,"Leave free_on_exit\n");
}

/* Recalculate on-screen city & comet locations when screen dimensions change */
void comets_recalc_positions(int xres, int yres)
{
    int i, img;
    int old_city_expl_height = city_expl_height;

    DEBUGMSG(debug_game,"Recalculating positions\n");


    if (Opts_GetGlobalOpt(USE_IGLOOS))
        img = IMG_IGLOO_INTACT;
    else
        img = IMG_CITY_BLUE;

    for (i = 0; i < NUM_CITIES; ++i)
    {
        /* Left vs. Right - makes room for Tux and the console */
        if (i < NUM_CITIES / 2)
        {
            cities[i].x = (((xres / (NUM_CITIES + 1)) * i) +
                    ((images[img] -> w) / 2));
            DEBUGMSG(debug_game,"%d,", cities[i].x);
        }
        else
        {
            cities[i].x = xres -
                (xres / (NUM_CITIES + 1) *
                 (i - NUM_CITIES / 2) +
                 images[img]->w / 2);
            DEBUGMSG(debug_game,"%d,", cities[i].x);
        }

        penguins[i].x = cities[i].x;
    }

    //Handle resize for comets: -------------

    city_expl_height = yres - images[IMG_CITY_BLUE]->h;
    comet_fontsize = (int)(BASE_COMET_FONTSIZE * get_scale());

    for (i = 0; i < MAX_MAX_COMETS; ++i)
    {
        if (!comets[i].alive)
            continue;

        //move comets to a location 'equivalent' to where they were
        //i.e. with the same amount of time left before impact
        comets[i].x = cities[comets[i].city].x;
        comets[i].y = comets[i].y * city_expl_height / old_city_expl_height;
        //  Re-render the numbers of any living comets at the new resolution:
        if(comets[i].formula_surf != NULL)  //for safety, but shouldn't occur if comet is alive
        {
            SDL_FreeSurface(comets[i].formula_surf);
            comets[i].formula_surf = T4K_BlackOutline(comets[i].flashcard.formula_string, comet_fontsize, &white);
        }
        if(comets[i].answer_surf != NULL)
        {
            SDL_FreeSurface(comets[i].answer_surf);
            comets[i].answer_surf = T4K_BlackOutline(comets[i].flashcard.answer_string, comet_fontsize, &white);
        }
    }
}

static int num_comets_alive()
{
    int i = 0;
    int living = 0;
    for(i = 0; i < MAX_MAX_COMETS; i++)
        if(comets[i].alive)
            living++;
    return living;
}



/* Functions for "smart bomb" super bonus comet powerup:  --------------------------------------*/

void smartbomb_activate(void)
{
    if(!smartbomb_alive)
        return;
    smartbomb_firing = 1;
    doing_answer = 1;
    playsound(SND_EXPLOSION);
}


int powerup_initialize(void)
{
    if(powerup_comet == NULL)
        return 0;

    powerup_comet->comet.alive = 0;
    powerup_comet->comet.expl = -1;
    powerup_comet->comet.x = 0;
    powerup_comet->comet.y = 0;
    powerup_comet->comet.zapped = 0;
    powerup_comet->comet.answer = 0;
    powerup_comet->comet.formula_surf = NULL;
    powerup_comet->comet.answer_surf = NULL;
    powerup_comet->inc_speed = 0;
    powerup_comet->direction = POWERUP_DIR_UNKNOWN;
    MC_ResetFlashCard(&(powerup_comet->comet.flashcard));

    powerup_comet_running = 0;
    smartbomb_alive = 0;

    return 1;
}

PowerUp_Type powerup_gettype(void)
{
    return rand()%NPOWERUP;
}

int powerup_add_comet(void)
{
    PowerUp_Type puType;

    DEBUGMSG( debug_game, "Enter powerup_add_comet()\n");

    if(smartbomb_alive)
        return 0;

    if(powerup_comet == NULL)
        return 0;

    if(powerup_comet_running)
        return 0;

    /* add only one powerup */
    powerup_comet_running = 1;

    /* get the type of the powerup */
    /* currently only smart bombs */
    puType = powerup_gettype();
    powerup_comet->type = puType;
    DEBUGMSG( debug_game, "Power-Up Type: %d\n", puType );

    /* create the flashcard */
    if(!MC_NextQuestion(curr_game, &(powerup_comet->comet.flashcard)))
        return 0;

    /* Make sure question is "sane" before we add it: */
    if((powerup_comet->comet.flashcard.answer > 999) || 
            (powerup_comet->comet.flashcard.answer < -999))
    {
        fprintf(stderr, "Warning, card with invalid answer encountered: %d\n",
                powerup_comet->comet.flashcard.answer);
        MC_ResetFlashCard(&(powerup_comet->comet.flashcard));
        return 0;
    }

    /* Now make the powerup comet alive */
    powerup_comet->comet.answer = powerup_comet->comet.flashcard.answer;
    powerup_comet->comet.alive = 1;
    if(powerup_comet->comet.formula_surf)
        SDL_FreeSurface(powerup_comet->comet.formula_surf);
    if(powerup_comet->comet.answer_surf)
        SDL_FreeSurface(powerup_comet->comet.answer_surf);
    powerup_comet->comet.formula_surf = T4K_BlackOutline(powerup_comet->comet.flashcard.formula_string, comet_fontsize, &white);
    powerup_comet->comet.answer_surf = T4K_BlackOutline(powerup_comet->comet.flashcard.answer_string, comet_fontsize, &white);

    /* Set the direction */
    /* Only two direction, left or right */
    powerup_comet->direction = rand()%2;

    /* Set the initial coordinates */
    powerup_comet->comet.y = POWERUP_Y_POS;
    if(powerup_comet->direction == POWERUP_DIR_LEFT)
    {
        powerup_comet->comet.x = screen->w; 
        powerup_comet->inc_speed = -MS_POWERUP_SPEED;
    }
    else
    {
        powerup_comet->comet.x = 0; 
        powerup_comet->inc_speed = MS_POWERUP_SPEED;
    }

    powerup_comet->comet.time_started = SDL_GetTicks();

    DEBUGMSG( debug_game, "Leave powerup_add_comet()\n");

    return 1;
}

void comets_handle_powerup(void)
{
    if(powerup_comet == NULL)
        return;

    if(!powerup_comet->comet.alive)
        return;

    powerup_comet->comet.x += powerup_comet->inc_speed*FC_time_elapsed*screen->w/640;

    if(powerup_comet->comet.expl >= 0)
    {
        powerup_comet->comet.expl++;
        if(powerup_comet->comet.expl >= sprites[IMG_COMET_EXPL]->num_frames * 2)
        {
            powerup_comet->comet.alive = 0;
            powerup_comet->comet.expl = -1;
            if(powerup_comet->comet.answer_surf)
            {SDL_FreeSurface(powerup_comet->comet.answer_surf); powerup_comet->comet.answer_surf = NULL; }
            if(powerup_comet->comet.formula_surf)
            {SDL_FreeSurface(powerup_comet->comet.formula_surf); powerup_comet->comet.formula_surf = NULL; }
            if(powerup_comet->comet.zapped)
            {
                switch(powerup_comet->type)
                {
                    case SMARTBOMB:
                        smartbomb_alive = 1;
                        powerup_comet_running = 0;
                        break;
                    default:  //do nothing
                        {}
                }
            }
        } 
    }
    else
    {
        switch(powerup_comet->direction)
        {
            case POWERUP_DIR_LEFT:
                if(powerup_comet->comet.x <= 0)
                {
                    powerup_comet->comet.alive = 0;
                    powerup_comet_running = 0;
                }
                break;

            case POWERUP_DIR_RIGHT:
                if(powerup_comet->comet.x >= screen->w)
                {
                    powerup_comet->comet.alive = 0; 
                    powerup_comet_running = 0;
                }
                break;

            default:  //do nothing
                {}
        }
        //Tell MathCards user missed it:
        if(powerup_comet_running == 0)
        {           
            if(Opts_LanMode())
#ifdef HAVE_LIBSDL_NET
                LAN_NotAnsweredCorrectly(powerup_comet->comet.flashcard.question_id);
#else
            {}
#endif
            else
                MC_NotAnsweredCorrectly(curr_game, powerup_comet->comet.flashcard.question_id);
        }
    }
}


#ifdef HAVE_LIBSDL_NET
/*****************   Functions for LAN support  *****************/

/*Examines the network messages from the buffer and calls
  appropriate function accordingly*/

void comets_handle_net_messages(void)
{
    char buf[NET_BUF_LEN];
    int done = 0;
    while(!done)
    {
        switch(LAN_NextMsg(buf))
        {
            case 1:   //Message received (e.g. a new question):
                comets_handle_net_msg(buf);
                break;
            case 0:   //No more messages:
                done = 1;
                break;
            case -1:  //Error in networking or server:
                done = 1;
                network_error = 1;
            default:
                {}
        }
        //"empty" buffer before next time through:
        buf[0] = '\0';
    }
}


void comets_handle_net_msg(char* buf)
{
    DEBUGMSG(debug_game|debug_lan, "Received server message: %s\n", buf);

    if(strncmp(buf, "PLAYER_MSG", strlen("PLAYER_MSG")) == 0)
    {
        DEBUGMSG(debug_game|debug_lan, "buf is %s\n", buf);                                                  
    }

    else if(strncmp(buf, "ADD_QUESTION", strlen("ADD_QUESTION")) == 0)
    {
        if(!add_quest_recvd(buf))
            fprintf(stderr, "ADD_QUESTION received but could not add question\n");
        else  
            DEBUGCODE(debug_game|debug_lan) print_current_quests();
    }

    else if(strncmp(buf, "REMOVE_QUESTION", strlen("REMOVE_QUESTION")) == 0)
    {
        if(!remove_quest_recvd(buf)) //remove the question with id in buf
        {
            DEBUGMSG(debug_game|debug_lan, "REMOVE_QUESTION received but could not remove question\n");
            DEBUGMSG(debug_game|debug_lan, "(this is OK if it was answered by this player, as it was removed already)\n");
        }
        else 
            DEBUGCODE(debug_game|debug_lan) print_current_quests();
    }

    else if(strncmp(buf, "TOTAL_QUESTIONS", strlen("TOTAL_QUESTIONS")) == 0)
    {
        sscanf(buf,"%*s %d", &total_questions_left);
        if(!total_questions_left)
            game_over_other = 1;
    }

    else if(strncmp(buf, "WAVE", strlen("WAVE")) == 0)
    {
        wave_recvd(buf);
    }

    else if(strncmp(buf, "MISSION_ACCOMPLISHED", strlen("MISSION_ACCOMPLISHED")) == 0)
    {
        game_over_won = 1;
    }

    else if(strncmp(buf, "PLAYER_LEFT", strlen("PLAYER_LEFT")) == 0)
    {
        player_left_recvd(buf);
    }

    else if(strncmp(buf, "comets_HALTED", strlen("comets_HALTED")) == 0)
    {
        comets_halted_recvd(buf);
    }

    else if(strncmp(buf, "LAN_INTERCEPTED", strlen("LAN_INTERCEPTED")) == 0)
    {
        /* Message handled within network.c - do nothing here */
    }

    else
    {
        DEBUGMSG(debug_game|debug_lan, "Unrecognized message from server: %s\n", buf);
    }  
}


int add_quest_recvd(char* buf)
{
    MC_FlashCard  fc;

    DEBUGMSG(debug_game|debug_lan, "Enter add_quest_recvd(), buf is: %s\n", buf);

    if(!buf)
    {
        fprintf(stderr, "NULL buf\n");
        return 0;
    }

    /* function call to parse buffer and receive question */
    if(!MC_MakeFlashcard(buf, &fc))
    {
        fprintf(stderr, "Unable to parse buffer into FlashCard\n");
        return 0;
    }

    /* If we have an open comet slot, put question in: */

    if(lan_add_comet(&fc))
    {
        if(num_attackers > 0)
            num_attackers--;
    }
    else
        fprintf(stderr, "add_quest_recvd() - was unable to add question from server\n");


    return 1;
}


/* Add a comet to a lan game: Note that in the lan game, the comets are added
 * immediately when a new question comes in.  It is up to the server to time
 * them appropriately - DSB.  */
int lan_add_comet(MC_FlashCard* fc)
{ 
    static int prev_city = -1; 
    int i;
    int com_found = -1;


    DEBUGCODE(debug_game|debug_lan)
    {
        fprintf(stderr, "Entering lan_add_comet(), card is\n");
        print_card(*fc);
        fprintf(stderr, "Existing questions are:\n");
        print_current_quests();
    }

    /* Make sure question is "sane" before we add it: */
    if(fc->answer > 999 || fc->answer < -999)
    {
        fprintf(stderr, "Warning, card with invalid answer encountered: %d\n",
                fc->answer);
        return 0;
    }


    /* Look for a free comet slot: */
    for (i = 0; i < MAX_MAX_COMETS; i++)
    {
        if (!comets[i].alive)
        {
            com_found = i;
            DEBUGMSG(debug_game|debug_lan, "lan_add_comet(): free comet slot found = %d\n", i);
            break;
        }
    }

    if (-1 == com_found)
    {
        /* free comet slot not found - no comet added: */
        DEBUGMSG(debug_game|debug_lan, "lan_add_comet() called but no free comet slot\n");
        DEBUGCODE(debug_game|debug_lan) print_current_quests();
        return 0;
    }



    /* Now we have a vacant comet slot at com_found and  */
    /* a valid question for it.                          */
    /* If we make it to here, create a new comet!*/
    MC_CopyCard(fc, &(comets[com_found].flashcard));
    comets[com_found].answer = fc->answer;
    comets[com_found].alive = 1;
    if(comets[com_found].formula_surf) SDL_FreeSurface(comets[com_found].formula_surf);
    if(comets[com_found].answer_surf) SDL_FreeSurface(comets[com_found].answer_surf);
    comets[com_found].formula_surf = T4K_BlackOutline(comets[com_found].flashcard.formula_string, comet_fontsize, &white);
    comets[com_found].answer_surf = T4K_BlackOutline(comets[com_found].flashcard.answer_string, comet_fontsize, &white);
    //  num_comets_alive++;

    /* Pick a city to attack that was not attacked last time */
    /* (so formulas are less likely to overlap). */
    do
    {
        i = rand() % NUM_CITIES;
    }
    while (i == prev_city);

    prev_city = i;

    /* Set in to attack that city: */
    comets[com_found].city = i;
    /* Start at the top, above the city in question: */
    comets[com_found].x = cities[i].x;
    comets[com_found].y = 0;
    comets[com_found].zapped = 0;
    /* Should it be a bonus comet? */
    comets[com_found].bonus = 0;

    /* Record the time at which this comet was created */
    comets[com_found].time_started = SDL_GetTicks();

    DEBUGMSG(debug_game|debug_lan, "lan_add_comet(): formula string is: %s\n", comets[com_found].flashcard.formula_string);

    /* No bonus comets in lan game for now: */
    //DEBUGMSG(debug_game|debug_lan, "bonus_comet_counter is %d\n",bonus_comet_counter);

    if (bonus_comet_counter == 1)
    {
        bonus_comet_counter = 0;
        //    comets[com_found].bonus = 1;
        //    playsound(SND_BONUS_COMET);
        //    DEBUGMSG(debug_game|debug_lan, "Created bonus comet");
    }


    /* No powerup comets in lan game for now: */
    //int t=-1;   
    //if(!powerup_comet_running)
    //{
    //  t = rand()%10;
    //  if( t < 1 )
    //  {
    //    powerup_add_comet();
    //  } 
    //}

    DEBUGCODE(debug_game|debug_lan)
    {
        fprintf(stderr, "Leaving lan_add_comet(), questions are:\n");
        print_current_quests();
    }

    /* comet slot found and question found so return successfully: */
    return 1;
}


int remove_quest_recvd(char* buf)
{
    int id = 0;
    int answered_by = -1;
    char* p = NULL;
    comet_type* zapped_comet;

    if(!buf)
    {
        DEBUGMSG(debug_game|debug_lan, "remove_quest_recvd() - returning because buf is NULL\n");
        return 0;
    }
    p = strchr(buf, '\t');
    if(!p)
    {
        DEBUGMSG(debug_game|debug_lan, "remove_quest_recvd() - returning because strchr() failed to find first tab char\n");
        return 0;
    }
    p++;
    id = atoi(p);
    //Now get index of player who answered it:
    p = strchr(p, '\t');
    if(!p)
    {
        DEBUGMSG(debug_game|debug_lan, "remove_quest_recvd() - returning because strchr() failed to find second tab char\n");
        return 0;
    }

    p++;
    answered_by = atoi(p);

    DEBUGMSG(debug_game|debug_lan, "remove_quest_recvd() for id = %d, answered by %d\n", id, answered_by);

    if(id < 1)  // The question_id can never be negative or zero
    {
        DEBUGMSG(debug_game|debug_lan, "remove_quest_recvd() - illegal question_id: %d\n", id);
        return 0;
    }

    if(answered_by == LAN_MyIndex()) //If we answered, it's already removed
    {
        DEBUGMSG(debug_game|debug_lan, "remove_quest_recvd() - answered and already removed by this client\n");
        return 0;
    }

    zapped_comet = search_comets_by_id(id);
    if(!zapped_comet)
    {
        DEBUGMSG(debug_game|debug_lan, "remove_quest_recvd() - could not find comet with id = %d\n", id);
        return 0;
    }

    DEBUGMSG(debug_game|debug_lan, "comet on screen found with question_id = %d\n", id);
    erase_comet_on_screen(zapped_comet, answered_by);

    return 1;
}





/* Receive notification of the current wave */
int wave_recvd(char* buf)
{
    int updated_wave = 0;
    char* p = NULL;

    if(buf == NULL)
        return 0;
    // get updated_wave:
    p = strchr(buf, '\t');
    if(!p)
        return 0;
    p++;
    updated_wave  = atoi(p);

    DEBUGMSG(debug_lan, "wave_score_recvd() - buf is: %s\n", buf);
    DEBUGMSG(debug_lan, "updated_wave is: %d\n", updated_wave);

    if(updated_wave != wave)
    {
        reset_level();
    }
    wave = updated_wave;
    return 1;
}


int player_left_recvd(char* buf)
{
    char _tmpbuf[512];
    char* name;
    //Adjust font size for resolution:
    int fontsize = (int)(DEFAULT_MENU_FONT_SIZE * get_scale());

    if(!buf)
        return 0;

    name = buf + strlen("PLAYER_LEFT\t");
    snprintf(_tmpbuf, sizeof(_tmpbuf), _("%s has left the game."), name);
    if(player_left_surf)
        SDL_FreeSurface(player_left_surf);
    player_left_surf = T4K_BlackOutline( _tmpbuf, fontsize, &white);
    player_left_time = SDL_GetTicks();
    player_left_pos.y = T4K_GetScreen()->h - player_left_surf->h;
    return 1;
}


int comets_halted_recvd(char* buf)
{
    comets_halted_by_server = 1;
    return 1;
}


comet_type* search_comets_by_id(int id)
{
    int i;
    for (i = 0; i < MAX_MAX_COMETS; i++)
    {
        if (comets[i].flashcard.question_id == id)
        {
            DEBUGMSG(debug_lan, "the question id is in slot %d\n", i);
            return &comets[i];
        }
    }
    // Didn't find it:
    return NULL;
}



int erase_comet_on_screen(comet_type* comet, int answered_by)
{
    if(!comet)
        return 0;
    //setting expl to 0 starts comet explosion animation
    comet->expl = 0;

    //TODO consider more elaborate sound or animation
    playsound(SND_SIZZLE);

    return 1;
}

/* For sorting of sorted_scores array */
int compare_scores(const void* p1, const void* p2)
{
    lan_player_type* lan1 = (lan_player_type*)p1;
    lan_player_type* lan2 = (lan_player_type*)p2;
    return (lan2->score - lan1->score);
}       

#endif  //HAVE_LIBSDL_NET


/* Print the current questions and the number of remaining questions: */
void print_current_quests(void)
{
    int i;
    fprintf(stderr, "\n------------  Current Questions:  -----------\n");
    for(i = 0; i < MAX_MAX_COMETS; i++)
    { 
        if(comets[i].alive == 1)
            fprintf(stderr, "Comet %d - question %d:\t%s\n", i, comets[i].flashcard.question_id, comets[i].flashcard.formula_string);

    }
    //fprintf(stderr, "--------------Question Queue-----------------\n");
    //for(i = 0; i < QUEST_QUEUE_SIZE; i++)
    //{
    //  if(quest_queue[i].question_id != -1)
    //    fprintf(stderr, "quest_queue %d - question %d:\t%s\n", i, quest_queue[i].question_id, quest_queue[i].formula_string);
    //  else
    //    fprintf(stderr, "quest_queue %d:\tEmpty\n", i);
    //}
    fprintf(stderr, "------------------------------------------\n");
}
