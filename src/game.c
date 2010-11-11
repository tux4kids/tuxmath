/*
   game.c: Contains Tux Math's main game loop!
 
   Copyright 2001, 2002, 2003, 2004, 2006, 2007, 2008, 2009, 2010.
   Authors: Bill Kendrick, David Bruce, Tim Holy, Brendan Luchen,
   Akash Gangil.
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org

game.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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

#include "transtruct.h"
#include "game.h"
#include "fileops.h"
#include "setup.h"
#include "mathcards.h"
#include "multiplayer.h"
#include "titlescreen.h"
#include "options.h"
#include "pixels.h"
#include "throttle.h"


#define FPS 15                     /* 15 frames per second */
#define MS_PER_FRAME (1000 / FPS)

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

#define PENGUIN_WALK_SPEED 3
#define SNOWFLAKE_SPEED 6
#define SNOWFLAKE_SEPARATION 3

const int SND_IGLOO_SIZZLE = SND_SIZZLE;
const int IMG_CITY_NONE = 0;

typedef struct comet_type {
  int alive;
  int expl;
  int city;
  float x, y;
  int answer;
  int bonus;
  int zapped;
  MC_FlashCard flashcard;
  Uint32 time_started;
} comet_type;

/* Local (to game.c) 'globals': */

static char* game_music_filenames[3] = {
  "game.mod",
  "game2.mod",
  "game3.mod",
};

static int gameover_counter;
static int game_status;
static int user_quit_received;
static int total_questions_left;
static int paused;
static int wave;
static int score;
static int pre_wave_score;
static int prev_wave_comets;
static int slowdown;
static int num_attackers;
static float speed;
static int demo_countdown;
static int tux_anim;
static int tux_anim_frame;
static int num_cities_alive;
static int tux_img;
static int old_tux_img;
static int frame;
static int neg_answer_picked;
static int tux_pressing;
static int doing_answer;
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
/* Feedback-related variables */
static int city_expl_height;
static int comet_feedback_number;
static float comet_feedback_height;
static float danger_level;

static int digits[MC_MAX_DIGITS];

static comet_type* comets = NULL;
static city_type* cities = NULL;
static penguin_type* penguins = NULL;
static steam_type* steam = NULL;

static cloud_type cloud;
static laser_type laser;
static SDL_Surface* bkgd = NULL; //640x480 background (windowed)
static SDL_Surface* scaled_bkgd = NULL; //native resolution (fullscreen)

static SDL_Surface* current_bkgd()
  { return screen->flags & SDL_FULLSCREEN ? scaled_bkgd : bkgd; }

static game_message s1, s2, s3, s4, s5;
static int start_message_chosen = 0;

/*****************************************************************/
MC_FlashCard quest_queue[QUEST_QUEUE_SIZE];    //current questions
int remaining_quests = 0;
static int comet_counter = 0;
static int lan_players = 0;
char lan_pnames[MAX_CLIENTS][NAME_SIZE];
int lan_pscores[MAX_CLIENTS];
/****************************************************************/

typedef struct {
  int x_is_blinking;
  int extra_life_is_blinking;
  int laser_enabled;
} help_controls_type;
static help_controls_type help_controls;

/* Local function prototypes: */
static int  game_initialize(void);
static void game_cleanup(void);
static void game_handle_help(void);
static void game_handle_user_events(void);
static void game_handle_demo(void);
static void game_handle_answer(void);
static void game_countdown(void);
static void game_handle_tux(void);
static void game_handle_comets(void);
static void game_handle_cities(void);
static void game_handle_penguins(void);
static void game_handle_steam(void);
static void game_handle_extra_life(void);
static void game_draw(void);
static void game_draw_background(void);
static void game_draw_comets(void);
static void game_draw_cities(void);
static void game_draw_misc(void);
static void game_handle_game_over(int game_status);

static int check_extra_life(void);
static int check_exit_conditions(void);

static void game_set_message(game_message*, const char* ,int x, int y);
static void game_clear_message(game_message*);
static void game_clear_messages(void);
void game_write_message(const game_message* msg);
static void game_write_messages(void);
static void draw_led_console(void);
static void draw_question_counter(void);
static void draw_console_image(int i);

static void reset_level(void);
static int add_comet(void);
static void add_score(int inc);
static void reset_comets(void);
static int num_comets_alive(void);

static void game_mouse_event(SDL_Event event);
static void game_key_event(SDLKey key, SDLMod mod);
static void free_on_exit(void);

static void help_add_comet(const char* formula_str, const char* ans_str);
static int help_renderframe_exit(void);
static void game_recalc_positions(int xres, int yres);

void putpixel(SDL_Surface* surface, int x, int y, Uint32 pixel);

/*****************************************************/
#ifdef HAVE_LIBSDL_NET
void game_handle_net_messages(void);
void game_handle_net_msg(char* buf);
int add_quest_recvd(char* buf);
int remove_quest_recvd(char* buf);
int connected_players_recvd(char* buf);
int update_score_recvd(char* buf);
int erase_comet_on_screen(comet_type* comet_ques);
MC_FlashCard* search_queue_by_id(int id);
comet_type* search_comets_by_id(int id);
/******************************************************/
#endif

void print_current_quests(void);

static void print_exit_conditions(void);
static void print_status(void);


/* --- MAIN GAME FUNCTION!!! --- */


int game(void)
{
  Uint32 timer = 0;

  DEBUGMSG(debug_game, "Entering game():\n");

  //see if the option matches the actual screen
  if (Opts_GetGlobalOpt(FULLSCREEN) == !(screen->flags & SDL_FULLSCREEN) )
  {
    ;//T4K_SwitchScreenMode();  //Huh??
  }


   /* most code moved into smaller functions (game_*()): */
  if (!game_initialize())
  {
    fprintf(stderr, "\ngame_initialize() failed!");
    return 0;
  }


  if (Opts_HelpMode()) {
    game_handle_help();
    game_cleanup();
    return GAME_OVER_OTHER;
  }
 


  /* --- MAIN GAME LOOP: --- */
  do
  {
    /* reset or increment various things with each loop: */
    frame++;
    old_tux_img = tux_img;
    tux_pressing = 0;

    if (laser.alive > 0)
    {
      laser.alive--;
    }

    /* Check for server messages if we are playing a LAN game: */
#ifdef HAVE_LIBSDL_NET
    if(Opts_LanMode())
    {
      game_handle_net_messages();
    }
#endif
    /* Most code now in smaller functions: */

    // 1. Check for user input
    game_handle_user_events();
    // 2. Update state of various game elements
    game_handle_demo();
    game_handle_answer();
    game_countdown();
    game_handle_tux();
    game_handle_comets();
    game_handle_cities();
    game_handle_penguins();
    game_handle_steam();
    game_handle_extra_life();
    // 3. Redraw:
    game_draw();
    // 4. Figure out if we should leave loop:
    game_status = check_exit_conditions(); 

    /* If we're in "PAUSE" mode, pause! */
    if (paused)
    {
      pause_game();
      paused = 0;
    }

      /* Keep playing music: */

#ifndef NOSOUND
    if (Opts_UsingSound())
    {
      if (!Mix_PlayingMusic())
      {
        T4K_AudioMusicLoad(game_music_filenames[(rand() % 3)], T4K_AUDIO_PLAY_ONCE);
      }
    }
#endif


    /* Pause (keep frame-rate event) */
    Throttle(MS_PER_FRAME, &timer);

  }
  while(GAME_IN_PROGRESS == game_status);
  /* END OF MAIN GAME LOOP! */

  game_handle_game_over(game_status);


  game_cleanup();

  /* Write post-game info to game summary file: */
  if (Opts_SaveSummary())
  {
    write_postgame_summary();
  }

  /* Save score in case needed for high score table: */
  Opts_SetLastScore(score);

  /* Return the chosen command: */
  if (GAME_OVER_WINDOW_CLOSE == game_status)
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
    return game_status;
  }
}






#ifdef HAVE_LIBSDL_NET
/*****************   Functions for LAN support  *****************/

/*Examines the network messages from the buffer and calls
  appropriate function accordingly*/

void game_handle_net_messages(void)
{
  char buf[NET_BUF_LEN];
  int done = 0;
  while(!done)
  {
    switch(LAN_NextMsg(buf))
    {
      case 1:   //Message received (e.g. a new question):
        game_handle_net_msg(buf);
        break;
      case 0:   //No more messages:
        done = 1;
        break;
      case -1:  //Error in networking or server:
        game_cleanup();
        game_status = GAME_OVER_ERROR;
      default:
        {}
    }
  }
}


void game_handle_net_msg(char* buf)
{
  DEBUGMSG(debug_lan, "Received server message: %s\n", buf);

  if(strncmp(buf, "PLAYER_MSG", strlen("PLAYER_MSG")) == 0)
  {
    DEBUGMSG(debug_lan, "buf is %s\n", buf);                                                  
  }

  else if(strncmp(buf, "ADD_QUESTION", strlen("ADD_QUESTION")) == 0)
  {
    if(!add_quest_recvd(buf))
      printf("ADD_QUESTION received but could not add question\n");
    else  
      DEBUGCODE(debug_game) print_current_quests();
  }

  else if(strncmp(buf, "REMOVE_QUESTION", strlen("REMOVE_QUESTION")) == 0)
  {
    if(!remove_quest_recvd(buf)) //remove the question with id in buf
      printf("REMOVE_QUESTION received but could not remove question\n");
    else 
      DEBUGCODE(debug_game) print_current_quests();
  }

  else if(strncmp(buf, "TOTAL_QUESTIONS", strlen("TOTAL_QUESTIONS")) == 0)
  {
    sscanf(buf,"%*s %d", &total_questions_left);
    if(!total_questions_left)
      game_over_other = 1;
  }

  else if(strncmp(buf, "CONNECTED_PLAYERS", strlen("CONNECTED_PLAYERS")) == 0)
  {
    connected_players_recvd(buf);
  }

  else if(strncmp(buf, "UPDATE_SCORE", strlen("UPDATE_SCORE")) == 0)
  {
    update_score_recvd(buf);
  }

  else if(strncmp(buf, "MISSION_ACCOMPLISHED", strlen("MISSION_ACCOMPLISHED")) == 0)
  {
    game_over_won = 1;
  }
  else
  {
    DEBUGMSG(debug_game, "Unrecognized message from server: %s\n", buf);
  }  
}


int add_quest_recvd(char* buf)
{
  /* Empty slots indicated by question_id == -1 */
  MC_FlashCard* fc = search_queue_by_id(-1);

  DEBUGMSG(debug_game, "Enter add_quest_recvd(), buf is: %s\n", buf);

  // if fc = NULL means no empty slot for question
  if(!buf)
  {
    printf("NULL buf\n");
    return 0;
  }

  if(!fc)
  {
    printf("NULL fc - no empty slot for question\n");
    return 0;
  }

  /* function call to parse buffer and receive question */
  if(!Make_Flashcard(buf, fc))
  {
    printf("Unable to parse buffer into FlashCard\n");
    return 0;
  }

  DEBUGCODE(debug_game) print_current_quests();

  /* If we have an open comet slot, put question in: */
  
  if(num_attackers > 0)
    if(add_comet())
      num_attackers--;

  return 1;
}


int remove_quest_recvd(char* buf)
{
  int id = 0;
  char* p = NULL;
  MC_FlashCard* fc = NULL;
  comet_type* comet_screen;

  if(!buf)
    return 0;

  p = strchr(buf, '\t');
  if(!p)
    return 0;

  p++;
  id = atoi(p);

  DEBUGMSG(debug_game, "remove_quest_recvd() for id = %d\n", id);

  if(id < 1)  // The question_id can never be negative or zero
    return 0;

  comet_screen = search_comets_by_id(id);
  fc = search_queue_by_id(id);
  if(!comet_screen && !fc)
    return 0;

  if(comet_screen)
  {
    DEBUGMSG(debug_game, "comet on screen found with question_id = %d\n", id);
    erase_comet_on_screen(comet_screen);
  }

  //NOTE: normally the question should no longer be in the queue,
  //so the next statement should not get executed:
  if(fc)
  {
    DEBUGMSG(debug_game,
             "Note - request to erase question still in queue: %s\n",
             fc->formula_string);
    MC_ResetFlashCard(fc);
  }

  return 1;
}


/* Here we have been told how many LAN players are still    */
/* in the game. This should always be followed by a series  */
/* of UPDATE_SCORE messages, each with the name and score  */
/* of a player. We clear out the array to get rid of anyone */
/* who has disconnected.                                    */
int connected_players_recvd(char* buf)
{
  int n = 0;
  int i = 0;
  char* p = NULL;

  if(!buf)
    return 0;

  p = strchr(buf, '\t');
  if(!p)
    return 0;
  p++;
  n = atoi(p);

  DEBUGMSG(debug_game, "connected_players_recvd() for n = %d\n", n);

  if(n < 0 || n > MAX_CLIENTS)
  {
    fprintf(stderr, "connected_players_recvd() - illegal value: %d\n", n);
    return -1;
  }
  lan_players = n;

  /* Reset array - we should be getting new values in immediately */
  /* following messages.                                          */
  for(i = 0; i < MAX_CLIENTS; i++)
  {
    lan_pnames[i][0] = '\0';
    lan_pscores[i] = -1;
  }
  return n;
}

/* Receive the name and current score of a currently-connected */
/* LAN player.                                                 */
int update_score_recvd(char* buf)
{
  int i = 0;
  char* p = NULL;

  if(buf == NULL)
    return 0;
  // get i:
  p = strchr(buf, '\t');
  if(!p)
    return 0;
  p++;
  i = atoi(p);

  //get name:
  p = strchr(p, '\t');
  if(!p)
    return 0;
  p++;
  strncpy(lan_pnames[i], p, NAME_SIZE);
  //This has most likely copied the score field as well, so replace the
  //tab delimiter with a null to terminate the string:
  {
    char* p2 = strchr(lan_pnames[i], '\t');
    if (p2)
      *p2 = '\0';
  }

  //Now get score:
  p = strchr(p, '\t');
  if(p)
    lan_pscores[i] = atoi(p);

  DEBUGMSG(debug_lan, "update_score_recvd() - buf is: %s\n", buf);
  DEBUGMSG(debug_lan, "i is: %d\tname is: %s\tscore is: %d\n", 
           i, lan_pnames[i], lan_pscores[i]);

  return 1;
}

/* Return a pointer to an empty comet slot, */
/* returning NULL if no vacancy found:      */

MC_FlashCard* search_queue_by_id(int id)
{
  int i = 0;
  for(i = 0; i < QUEST_QUEUE_SIZE; i++)
  {
    if(quest_queue[i].question_id == id)
      return &quest_queue[i];
  }
  //if we don't find a match:
  return NULL;
}


comet_type* search_comets_by_id(int id)
{
  int i;
  for (i = 0; i < Opts_MaxComets(); i++)
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



int erase_comet_on_screen(comet_type* comet)
{
  if(!comet)
    return 0;
  //setting expl to 0 starts comet explosion animation
  comet->expl = 0;

  //TODO consider more elaborate sound or animation
  playsound(SND_SIZZLE);

  return 1;
}

#endif

/* Print the current questions and the number of remaining questions: */
void print_current_quests(void)
{
  int i;
  printf("\n------------  Current Questions:  -----------\n");
  for(i = 0; i < Opts_MaxComets(); i++)
  { 
    if(comets[i].alive == 1)
     printf("Comet %d - question %d:\t%s\n", i, comets[i].flashcard.question_id, comets[i].flashcard.formula_string);

  }
  printf("--------------Question Queue-----------------\n");
  for(i = 0; i < QUEST_QUEUE_SIZE; i++)
  {
    if(quest_queue[i].question_id != -1)
      printf("quest_queue %d - question %d:\t%s\n", i, quest_queue[i].question_id, quest_queue[i].formula_string);
    else
      printf("quest_queue %d:\tEmpty\n", i);
  }
  printf("------------------------------------------\n");
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



int game_initialize(void)
{
  int i,img;
  
  DEBUGMSG(debug_game,"Entering game_initialize()\n");
  
  /* Clear window: */
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
  SDL_Flip(screen);

  game_status = GAME_IN_PROGRESS;
  gameover_counter = -1;
  user_quit_received = 0;
  network_error = 0;

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
    if (!MC_StartGame())
    {
      fprintf(stderr, "\nMC_StartGame() failed!");
      return 0;
    } 
    DEBUGMSG(debug_mathcards | debug_game,"MC_StartGame() finished.\n")
  }
  else  
  {
    /* Reset question queue and player name/score lists: */
    int i;

    for(i = 0; i < QUEST_QUEUE_SIZE; i ++)
      MC_ResetFlashCard(&(quest_queue[i]));

    for(i = 0; i < MAX_CLIENTS; i++)
    {
      lan_pnames[i][0] = '\0';
      lan_pscores[i] = -1;
    }
  }

  /* Allocate memory */
  comets = NULL;  // set in case allocation fails partway through
  cities = NULL;
  penguins = NULL;
  steam = NULL;
  comets = (comet_type *) malloc(MAX_MAX_COMETS * sizeof(comet_type));
  if (comets == NULL)
  {
    printf("Allocation of comets failed");
    return 0;
  }

  cities = (city_type *) malloc(NUM_CITIES * sizeof(city_type));
  if (cities == NULL)
  {
    printf("Allocation of cities failed");
    return 0;
  }

  penguins = (penguin_type *) malloc(NUM_CITIES * sizeof(penguin_type));
  if (penguins == NULL)
  {
    printf("Allocation of penguins failed");
    return 0;
  }

  steam = (steam_type *) malloc(NUM_CITIES * sizeof(steam_type));
  if (steam == NULL)
  {
    printf("Allocation of steam failed");
    return 0;
  }

 
  /* Write pre-game info to game summary file: */
  if (Opts_SaveSummary())
  {
    write_pregame_summary();
  }

  /* Prepare to start the game: */
  city_expl_height = screen->h - images[IMG_CITY_BLUE]->h;

  /* Initialize feedback parameters */
  comet_feedback_number = 0;
  comet_feedback_height = 0;
  danger_level = Opts_DangerLevel();

  wave = 1;
  num_attackers = 2;
  prev_wave_comets = Opts_StartingComets();
  speed = Opts_Speed();
  slowdown = 0;
  score = 0;
  demo_countdown = 2000;
  total_questions_left = 0;
  level_start_wait = LEVEL_START_WAIT_START;
  neg_answer_picked = 0;

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
  laser.alive = 0;

  /* Reset remaining stuff: */

  bkgd = scaled_bkgd = NULL;
  last_bkgd = -1;
  reset_level();
  reset_comets();

  frame = 0;
  paused = 0;
  doing_answer = 0;
  tux_pressing = 0;
  tux_img = IMG_TUX_RELAX1;
  tux_anim = -1;
  tux_anim_frame = 0;

  // Initialize the messages
  game_clear_message(&s5);
  if (!start_message_chosen)
  {
    game_clear_message(&s1);
    game_clear_message(&s2);
    game_clear_message(&s3);
    game_clear_message(&s4);
  }

  help_controls.x_is_blinking = 0;
  help_controls.extra_life_is_blinking = 0;
  help_controls.laser_enabled = 1;

  T4K_OnResolutionSwitch(game_recalc_positions);
  
  DEBUGMSG(debug_game,"Exiting game_initialize()\n");

  return 1;
}


void game_cleanup(void)
{
#ifdef HAVE_LIBSDL_NET  
  LAN_Cleanup();
#endif

  /* Free background: */
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
  
  /* clear start message */
  start_message_chosen = 0;
  
  /* Free dynamically-allocated items */
  free_on_exit();

  /* Stop music: */
#ifndef NOSOUND
  if (Opts_UsingSound())
  {
    if (Mix_PlayingMusic())
    {
      Mix_HaltMusic();
    }
  }
#endif

  DEBUGMSG(debug_game, "Leaving game_cleanup():\n");
}


void game_handle_help(void)
{
  const int left_edge = 140;
  int frame_start;
  int quit_help = 0;

  help_controls.laser_enabled = 0;
  help_controls.x_is_blinking = 0;
  help_controls.extra_life_is_blinking = 0;

  // Here are some things that have to happen before we can safely
  // draw the screen
  tux_img = IMG_TUX_CONSOLE1;
  old_tux_img = tux_img;
  tux_pressing = 0;
  frame = 0;

  // Write the introductory text
  game_set_message(&s1,_("Welcome to TuxMath!"),-1,50);

#ifndef NOSOUND
  if (Opts_UsingSound())
  {
    if (!Mix_PlayingMusic())
    {
      Mix_PlayMusic(musics[MUS_GAME], 0);
    }
  }
#endif

  // Wait 2 seconds while rendering frames
  while (frame < 2*FPS && !(quit_help = help_renderframe_exit()));
  if (quit_help)
    return;

  game_set_message(&s2,_("Your mission is to save your"), left_edge, 100);
  game_set_message(&s3,_("penguins' igloos from the"), left_edge, 135);
  game_set_message(&s4,_("falling comets."), left_edge, 170);

  frame_start = frame;
  while (frame-frame_start < 5*FPS && !(quit_help = help_renderframe_exit()));  // wait 5 more secs
  if (quit_help)
    return;

  // Bring in a comet
  speed = 2;
  help_add_comet("2 + 1 = ?", "3");
  help_controls.laser_enabled = 1;
  level_start_wait = 0;

  frame_start = frame;
  while (comets[0].alive && frame-frame_start < 100 && !(quit_help = help_renderframe_exit())); // advance comet
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
  game_clear_message(&s2);
  game_clear_message(&s3);
  game_clear_message(&s4);

  help_controls.laser_enabled = 0;
  frame_start = frame;
  while (frame-frame_start < 3*FPS && !(quit_help = help_renderframe_exit()));  // wait 3 secs

  speed = 2;
  game_set_message(&s1,_("If an igloo gets hit by a comet,"),left_edge,100);
  game_set_message(&s2,_("it melts. But don't worry: the"),left_edge,135);
  game_set_message(&s3,_("penguin is OK!"),left_edge,170);
  game_set_message(&s4,_("Just watch what happens:"),left_edge,225);
  game_set_message(&s5,_("(Press a key to start)"),left_edge,260);

  key_pressed = 0;
  while (!key_pressed && !(quit_help = help_renderframe_exit()));
  if (quit_help)
    return;
  game_clear_message(&s5);

  help_add_comet("3 * 3 = ?", "9");
  comets[0].y = 2*(screen->h)/3;   // start it low down
  while ((comets[0].expl == -1) && !(quit_help = help_renderframe_exit()));  // wait 3 secs
  if (quit_help)
    return;
  game_set_message(&s4,_("Notice the answer"),left_edge,comets[0].y-100);
  help_renderframe_exit();
  SDL_Delay(4000);
  game_clear_message(&s4);

  frame_start = frame;
  while (frame-frame_start < 5*FPS && !(quit_help = help_renderframe_exit()));  // wait 5 secs
  if (quit_help)
    return;

  game_set_message(&s1,_("If it gets hit again, the"),left_edge,100);
  game_set_message(&s2,_("penguin leaves."),left_edge,135);
  game_set_message(&s3,_("(Press a key when ready)"),left_edge,200);

  key_pressed = 0;
  while (!key_pressed && !(quit_help = help_renderframe_exit()));
  if (quit_help)
    return;
  game_clear_message(&s3);

  help_add_comet("56 / 8 = ?", "7");
  comets[0].y = 2*(screen->h)/3;   // start it low down

  while (comets[0].alive && !(quit_help = help_renderframe_exit()));

  if (quit_help)
    return;
  frame_start = frame;

  while ((frame-frame_start < 3*FPS) && !(quit_help = help_renderframe_exit()));

  if (quit_help)
    return;

  help_controls.laser_enabled = 1;
  game_set_message(&s1,_("You can fix the igloos"), left_edge,100);
  game_set_message(&s2,_("by stopping bonus comets."), left_edge,135);
  help_add_comet("2 + 2 = ?", "4");
  comets[0].bonus = 1;
  frame_start = frame;

  while (comets[0].alive && (frame-frame_start < 50) && !(quit_help = help_renderframe_exit()));

  if (quit_help)
    return;
  if (comets[0].alive)
    speed = 0;
  game_set_message(&s3,_("Zap it now!"),left_edge,225);

  while (comets[0].alive && !(quit_help = help_renderframe_exit()));

  if (quit_help)
    return;
  game_set_message(&s1,_("Great job!"),left_edge,100);
  game_clear_message(&s2);
  game_clear_message(&s3);
  frame_start = frame;

  while ((frame-frame_start < 2*FPS) && !(quit_help = help_renderframe_exit()));

  if (quit_help)
    return;
  check_extra_life();
  frame_start = frame;

  while ((frame-frame_start < 10*FPS) && !(quit_help = help_renderframe_exit()));

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
  static Uint32 last_time = 0;
  static Uint32 now_time;

  if (last_time == 0)
    last_time = SDL_GetTicks(); // Initialize...

  tux_pressing = 0;
  if (laser.alive > 0)
      laser.alive--;
  game_handle_user_events();
  game_handle_answer();
  game_handle_tux();
  game_handle_comets();
  game_handle_cities();
  game_handle_penguins();
  game_handle_steam();
  game_handle_extra_life();
  game_draw();
  game_status = check_exit_conditions();

  // Delay to keep frame rate constant. Do this in a way
  // that won't cause a freeze if the timer wraps around.
  now_time = SDL_GetTicks();
  if (now_time >= last_time && now_time < last_time + MS_PER_FRAME)
    SDL_Delay((last_time+MS_PER_FRAME) - now_time);
  last_time = now_time;

  frame++;

  return (game_status != GAME_IN_PROGRESS);
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

  strncpy(comets[0].flashcard.formula_string,formula_str,MC_MaxFormulaSize() );
  strncpy(comets[0].flashcard.answer_string,ans_str,MC_MaxAnswerSize() );
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

void game_clear_message(game_message *msg)
{
  game_set_message(msg,"",0,0);
}

void game_clear_messages()
{
  game_clear_message(&s1);
  game_clear_message(&s2);
  game_clear_message(&s3);
  game_clear_message(&s4);
  game_clear_message(&s5);
}

void game_write_message(const game_message *msg)
{
  SDL_Surface* surf;
  SDL_Rect rect;

  if (strlen(msg->message) > 0)
  {
    surf = T4K_BlackOutline( _(msg->message), DEFAULT_HELP_FONT_SIZE, &white);
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
    //SDL_UpdateRect(screen, rect.x, rect.y, rect.w, rect.h);
  }
}

void game_write_messages(void)
{
  game_write_message(&s1);
  game_write_message(&s2);
  game_write_message(&s3);
  game_write_message(&s4);
  game_write_message(&s5);
}

void game_handle_user_events(void)
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
      game_key_event(key, mod);
    }
    else if (event.type == SDL_MOUSEBUTTONDOWN)
    {
      game_mouse_event(event);
    }
  }
}

void game_handle_demo(void)
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
    if (picked_comet != -1 && (frame % 5) == 0 && (rand() % 10) < 8)
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

void game_handle_answer(void)
{
  int i, j, lowest, lowest_y;
  char ans[MC_MAX_DIGITS + 2]; //extra space for negative, and for final '\0'
  Uint32 ctime;

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
  


  /*  Pick the lowest comet which has the right answer: */
  /*  FIXME: do we want it to prefer bonus comets to regular comets? */
  lowest_y = 0;
  lowest = -1;

  for (i = 0; i < Opts_MaxComets(); i++)
  {
    if (comets[i].alive &&
        comets[i].expl == -1 &&
        //comets[i].answer == num &&
        0 == strncmp(comets[i].flashcard.answer_string, ans, MC_MAX_DIGITS+1) &&
        comets[i].y > lowest_y)
    {
      lowest = i;
      lowest_y = comets[i].y;
    }
  }

  /* If there was a comet with this answer, destroy it! */
  if (lowest != -1)  /* -1 means no comet had this answer */
  {
    float t;
    /* Store the time the question was present on screen (do this */
    /* in a way that avoids storing it if the time wrapped around */
    ctime = SDL_GetTicks();
    if (ctime > comets[lowest].time_started)
      t = ((float)(ctime - comets[lowest].time_started)/1000);
    else
      t = -1;   //Mathcards will ignore t == -1
    /* Tell Mathcards or the server that we answered correctly: */
    if(Opts_LanMode())
#ifdef HAVE_LIBSDL_NET
      LAN_AnsweredCorrectly(comets[lowest].flashcard.question_id, t);
#else
      {}  // Needed for compiler, even though this path can't occur
#endif      
    else
      MC_AnsweredCorrectly(comets[lowest].flashcard.question_id, t);


    /* Destroy comet: */
    comets[lowest].expl = 0;
    comets[lowest].zapped = 1;
    /* Fire laser: */
    laser.alive = LASER_START;
    laser.x1 = screen->w / 2;
    laser.y1 = screen->h;
    laser.x2 = comets[lowest].x;
    laser.y2 = comets[lowest].y;
    playsound(SND_LASER);
    playsound(SND_SIZZLE);

    /* Record data for feedback */
    if (Opts_UseFeedback())
    {
      comet_feedback_number++;
      comet_feedback_height += comets[lowest].y/city_expl_height;

#ifdef FEEDBACK_DEBUG
      printf("Added comet feedback with height %g\n",comets[lowest].y/city_expl_height);
#endif
    }

    /* Pick Tux animation: */
    /* 50% of the time.. */
    if ((rand() % 10) < 5)
    {
      /* ... pick an animation to play: */
      if ((rand() % 10) < 5)
        tux_anim = IMG_TUX_YES1;
      else
        tux_anim = IMG_TUX_YAY1;
      tux_anim_frame = ANIM_FRAME_START;
    }

    /* Increment score: */

    /* [ add = 25, sub = 50, mul = 75, div = 100 ] */
    /* [ the higher the better ] */
    /* FIXME looks like it might score a bit differently based on screen mode? */
    add_score(25 * comets[lowest].flashcard.difficulty *
              (screen->h - comets[lowest].y + 1) /
               screen->h);
  }
  else
  {
    /* Didn't hit anything! */
    laser.alive = LASER_START;
    laser.x1 = screen->w / 2;
    laser.y1 = screen->h;
    laser.x2 = laser.x1;
    laser.y2 = 0;
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

void game_countdown(void)
{
  if (level_start_wait <= 0)
  {
    game_clear_messages();
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

void game_handle_tux(void)
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
//rather than absolute position, and determine the latter in game_draw_comets()
void game_handle_comets(void)
{
  /* Handle comets. Since the comets also are the things that trigger
     changes in the cities, we set some flags in them, too. */
  int i, this_city;
  Uint32 ctime;

//  num_comets_alive = 0;

  /* Clear the threatened flag on each city */
  for (i = 0; i < NUM_CITIES; i++)
    cities[i].threatened = 0;

  for (i = 0; i < Opts_MaxComets(); i++)
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
        comets[i].y += speed * Opts_BonusSpeedRatio() *
                       city_expl_height / (screen->h - images[IMG_CITY_BLUE]->h);
      }
      else /* Regular comet: */
      {
        comets[i].y += speed *
                       city_expl_height / (screen->h - images[IMG_CITY_BLUE]->h);
      }

      /* Does it threaten a city? */
      if (comets[i].y > 3 * screen->h / 4)
        cities[this_city].threatened = 1;

      /* Did it hit a city? */
      if (comets[i].y >= city_expl_height &&
          comets[i].expl == -1)
      {
        /* Tell MathCards about it - question not answered correctly: */
        if(Opts_LanMode())
#ifdef HAVE_LIBSDL_NET
          LAN_NotAnsweredCorrectly(comets[i].flashcard.question_id);
#else
          {}
#endif
        else
          MC_NotAnsweredCorrectly(comets[i].flashcard.question_id);

        /* Store the time the question was present on screen (do this */
        /* in a way that avoids storing it if the time wrapped around */
        ctime = SDL_GetTicks();
        if (ctime > comets[i].time_started) {
          MC_AddTimeToList((float)(ctime - comets[i].time_started)/1000);
        }

        /* Record data for speed feedback */
        /* Do this only for cities that are alive; dead cities */
        /* might not get much protection from the player */
        if (Opts_UseFeedback() && cities[this_city].hits_left) {
          comet_feedback_number++;
          comet_feedback_height += 1.0 + Opts_CityExplHandicap();

#ifdef FEEDBACK_DEBUG
           printf("Added comet feedback with height %g\n",
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
          speed = Opts_Speed();
          slowdown = 1;
        }

        tux_anim = IMG_TUX_FIST1;
        tux_anim_frame = ANIM_FRAME_START;

        /* Destroy comet: */
        comets[i].expl = 0;
      }

      /* Handle comet explosion animation: */
      if (comets[i].expl >= 0)
      {
        comets[i].expl++;
        if (comets[i].expl >= sprites[IMG_COMET_EXPL]->num_frames * 2) {
          comets[i].alive = 0;
          comets[i].expl = -1;
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

  /* FIXME for the LAN game, the adding of comets needs to take place in  */
  /* check_messages() when new questions come in from the server.  For    */
  /* ease of understanding, we should do it at the same place in the game */
  /* loop for the non-LAN (i.e. local MC_*() functions) game - DSB        */
  /* add more comets if needed: */
  if (!Opts_HelpMode() && level_start_wait == 0) //&&
     // (frame % 20) == 0)
  {
    /* num_attackers is how many comets are left in wave */
    if (num_attackers > 0)
    {
//      if ((rand() % 2) == 0 || num_comets_alive() == 0)  NOTE also caused timing issue
      {
        if (add_comet())
        {
          num_attackers--;
        }
      }
    }
    else
    {
      if (num_comets_alive() == 0)
      {
        if (!check_extra_life())
        {
          /* Time for the next wave! */
          wave++;
          reset_level();
        }
      }
    }
  }
}



void game_handle_cities(void)
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


void game_handle_penguins(void)
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
      penguins[i].x += direction*PENGUIN_WALK_SPEED;
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
      penguins[i].x += direction*PENGUIN_WALK_SPEED;
      if (direction < 0) {
        if (penguins[i].x + images[IMG_PENGUIN_WALK_OFF1]->w/2 < 0)
          penguins[i].status = PENGUIN_OFFSCREEN;
      } else {
        if (penguins[i].x - images[IMG_PENGUIN_WALK_OFF1]->w/2 > screen->w)
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

void game_handle_steam(void)
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


void game_handle_extra_life(void)
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
      cloud.x += direction*PENGUIN_WALK_SPEED;
    }
    else {
      // Cloud is "parked," handle the snowfall and igloo rebuilding
      cities[cloud.city].status = CITY_REBUILDING;
      igloo_top = screen->h - igloo_vertical_offset
        - images[IMG_IGLOO_INTACT]->h;
      for (i = 0, num_below_igloo = 0; i < NUM_SNOWFLAKES; i++) {
        cloud.snowflake_y[i] += SNOWFLAKE_SPEED;
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

void game_draw(void)
{
  SDL_Rect dest;

  /* Clear screen: */
  game_draw_background();

  /* Draw miscellaneous informational items */
  game_draw_misc();

  /* Draw cities/igloos and (if applicable) penguins: */
  game_draw_cities();

  /* Draw normal comets first, then bonus comets */
  game_draw_comets();


  /* Draw laser: */
  if (laser.alive)
  {
    draw_line(laser.x1, laser.y1, laser.x2, laser.y2,
                  255 / ((LASER_START + 1) - laser.alive),
                  192 / ((LASER_START + 1) - laser.alive),
                  64);
  }

  /* Draw numeric keypad: */
  if (Opts_GetGlobalOpt(USE_KEYPAD))
  {
    /* pick image to draw: */
    int keypad_image;
    if (MC_GetOpt(ALLOW_NEGATIVES) )
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
  draw_led_console();
  draw_console_image(tux_img);

  /* Draw any messages on the screen (used for the help mode) */
  game_write_messages();

  /* Swap buffers: */
  SDL_Flip(screen);
}

void game_draw_background(void)
{
  static int old_wave = 0; //update wave immediately
  static Uint32 bgcolor, fgcolor = 0;
  SDL_Rect dest;

  if (fgcolor == 0)
    fgcolor = SDL_MapRGB(screen->format, 64, 96, 64);
  if (old_wave != wave)
  {
    DEBUGMSG(debug_game,"Wave %d\n", wave);
    old_wave = wave;
    bgcolor = SDL_MapRGB(screen->format,
                         64,
                         64 + ((wave * 32) % 192),
                         128 - ((wave * 16) % 128) );
    DEBUGMSG(debug_game,"Filling screen with color %d\n", bgcolor);
  }

  if (current_bkgd() == NULL || (current_bkgd()->w != screen->w && 
                                 current_bkgd()->h != screen->h) )
  {
    dest.x = 0;
    dest.y = 0;
    dest.w = screen->w;
    dest.h = ((screen->h) / 4) * 3;

    SDL_FillRect(screen, &dest, bgcolor);


    dest.y = ((screen->h) / 4) * 3;
    dest.h = (screen->h) / 4;

    SDL_FillRect(screen, &dest, fgcolor);
  }

  if (current_bkgd())
  {
    dest.x = (screen->w - current_bkgd()->w) / 2;
    dest.y = (screen->h - current_bkgd()->h) / 2;
    SDL_BlitSurface(current_bkgd(), NULL, screen, &dest);
  }
}

/* Draw comets: */
/* NOTE bonus comets split into separate pass to make them */
/* draw last (i.e. in front), as they can overlap          */
void game_draw_comets(void)
{

  int i;
  SDL_Surface* img = NULL;
  SDL_Rect dest;
  char* comet_str;

   /* First draw regular comets: */
  for (i = 0; i < Opts_MaxComets(); i++)
  {
    if (comets[i].alive && !comets[i].bonus)
    {
      if (comets[i].expl == -1)
      {
        /* Decide which image to display: */
        img = sprites[IMG_COMET]->frame[(frame + i) % sprites[IMG_COMET]->num_frames];
        /* Display the formula (flashing, in the bottom half
                   of the screen) */
        if (comets[i].y < screen->h / 2 || frame % 8 < 6)
        {
          comet_str = comets[i].flashcard.formula_string;
        }
        else
        {
          comet_str = NULL;
        }
      }
      else
      {
        /* show each frame of explosion twice */
        img = sprites[IMG_COMET_EXPL]->frame[comets[i].expl / 2];
        comet_str = comets[i].flashcard.answer_string;
      }

      /* Draw it! */
      dest.x = comets[i].x - (img->w / 2);
      dest.y = comets[i].y - img->h;
      dest.w = img->w;
      dest.h = img->h;

      SDL_BlitSurface(img, NULL, screen, &dest);
      if (comet_str != NULL)
      {
        draw_nums(comet_str, comets[i].x, comets[i].y);
      }
    }
  }

  /* Now draw any bonus comets: */
  for (i = 0; i < Opts_MaxComets(); i++)
  {
    if (comets[i].alive && comets[i].bonus)
    {
      if (comets[i].expl == -1)
      {
        /* Decide which image to display: */
        img = sprites[IMG_BONUS_COMET]->frame[(frame + i) % sprites[IMG_BONUS_COMET]->num_frames];
        /* Display the formula (flashing, in the bottom half
                   of the screen) */
        if (comets[i].y < screen->h / 2 || frame % 8 < 6)
        {
          comet_str = comets[i].flashcard.formula_string;
        }
        else
        {
          comet_str = NULL;
        }
      }
      else
      {
        img = sprites[IMG_BONUS_COMET_EXPL]->frame[comets[i].expl / 2];
        comet_str = comets[i].flashcard.answer_string;
      }

      /* Draw it! */
      dest.x = comets[i].x - (img->w / 2);
      dest.y = comets[i].y - img->h;
      dest.w = img->w;
      dest.h = img->h;
      SDL_BlitSurface(img, NULL, screen, &dest);
      if (comet_str != NULL)
      {
        draw_nums(comet_str, comets[i].x, comets[i].y);
      }
    }
  }
}



void game_draw_cities(void)
{
  int i, j, current_layer, max_layer;
  SDL_Rect src, dest;
  SDL_Surface* this_image;

  if (Opts_GetGlobalOpt(USE_IGLOOS)) {
    /* We have to draw respecting layering */
    current_layer = 0;
    max_layer = 0;
    do {
      for (i = 0; i < NUM_CITIES; i++) {
        if (cities[i].status != CITY_GONE && cities[i].layer > max_layer)
          max_layer = cities[i].layer;
        if (penguins[i].status != PENGUIN_OFFSCREEN && penguins[i].layer > max_layer)
          max_layer = penguins[i].layer;
        if (steam[i].status == STEAM_ON && steam[i].layer > max_layer)
          max_layer = steam[i].layer;
        if (cities[i].layer == current_layer &&
            cities[i].img != IMG_CITY_NONE) {
          // Handle the blended igloo images, which are encoded
          // (FIXME) with a negative image number
          if (cities[i].img <= 0)
            this_image = blended_igloos[-cities[i].img];
          else
            this_image = images[cities[i].img];
          //this_image = blended_igloos[frame % NUM_BLENDED_IGLOOS];
          dest.x = cities[i].x - (this_image->w / 2);
          dest.y = (screen->h) - (this_image->h) - igloo_vertical_offset;
          if (cities[i].img == IMG_IGLOO_MELTED3 ||
              cities[i].img == IMG_IGLOO_MELTED2)
            dest.y -= (images[IMG_IGLOO_MELTED1]->h - this_image->h)/2;
          dest.w = (this_image->w);
          dest.h = (this_image->h);
          SDL_BlitSurface(this_image, NULL, screen, &dest);
        }
        if (penguins[i].layer == current_layer &&
            penguins[i].status != PENGUIN_OFFSCREEN) {
          this_image = images[penguins[i].img];
          if (penguins[i].status == PENGUIN_WALKING_OFF ||
              penguins[i].status == PENGUIN_WALKING_ON) {
            /* With walking penguins, we have to use flipped images
               when it's walking left. The other issue is that the
               images are of different widths, so aligning on the
               center produces weird forward-backward walking. The
               reliable way is the align them all on the tip of the
               beak (the right border of the unflipped image) */
            dest.x = penguins[i].x - (this_image->w / 2);
            dest.y = (screen->h) - (this_image->h);
            if ((i<NUM_CITIES/2 && penguins[i].status==PENGUIN_WALKING_OFF) ||
                (i>=NUM_CITIES/2 && penguins[i].status==PENGUIN_WALKING_ON)) {
              /* walking left */
              this_image = flipped_images[flipped_img_lookup[penguins[i].img]];
              dest.x = penguins[i].x - images[IMG_PENGUIN_WALK_OFF2]->w/2;
            } else
              dest.x = penguins[i].x - this_image->w
                + images[IMG_PENGUIN_WALK_OFF2]->w/2;   /* walking right */
          }
          else {
            dest.x = penguins[i].x - (this_image->w / 2);
            dest.y = (screen->h) - (5*(this_image->h))/4 - igloo_vertical_offset;
          }
          dest.w = (this_image->w);
          dest.h = (this_image->h);
          SDL_BlitSurface(this_image, NULL, screen, &dest);
        }
        if (steam[i].layer == current_layer &&
            steam[i].status == STEAM_ON) {
          this_image = images[steam[i].img];
          dest.x = cities[i].x - (this_image->w / 2);
          dest.y = (screen->h) - this_image->h - ((4 * images[IMG_IGLOO_INTACT]->h) / 7);
          dest.w = (this_image->w);
          dest.h = (this_image->h);
          SDL_BlitSurface(this_image, NULL, screen, &dest);
        }
      }
      current_layer++;
    } while (current_layer <= max_layer);
    if (cloud.status == EXTRA_LIFE_ON) {
      /* Render cloud & snowflakes */
      for (i = 0; i < NUM_SNOWFLAKES; i++) {
        if (cloud.snowflake_y[i] > cloud.y &&
            cloud.snowflake_y[i] < screen->h - igloo_vertical_offset) {
          this_image = images[IMG_SNOW1+cloud.snowflake_size[i]];
          dest.x = cloud.snowflake_x[i] - this_image->w/2 + cloud.x;
          dest.y = cloud.snowflake_y[i] - this_image->h/2;
          dest.w = this_image->w;
          dest.h = this_image->h;
          SDL_BlitSurface(this_image, NULL, screen, &dest);
        }
      }
      this_image = images[IMG_CLOUD];
      dest.x = cloud.x - this_image->w/2;
      dest.y = cloud.y - this_image->h/2;
      dest.w = this_image->w;
      dest.h = this_image->h;
      SDL_BlitSurface(this_image, NULL, screen, &dest);
    }
  }
  else {
    /* We're drawing original city graphics, for which there are no
       layering issues, but has special handling for the shields */
    for (i = 0; i < NUM_CITIES; i++) {
      this_image = images[cities[i].img];
      dest.x = cities[i].x - (this_image->w / 2);
      dest.y = (screen->h) - (this_image->h);
      dest.w = (this_image->w);
      dest.h = (this_image->h);
      SDL_BlitSurface(this_image, NULL, screen, &dest);

      /* Draw sheilds: */
      if (cities[i].hits_left > 1) {
        for (j = (frame % 3); j < images[IMG_SHIELDS]->h; j = j + 3) {
          src.x = 0;
          src.y = j;
          src.w = images[IMG_SHIELDS]->w;
          src.h = 1;

          dest.x = cities[i].x - (images[IMG_SHIELDS]->w / 2);
          dest.y = (screen->h) - (images[IMG_SHIELDS]->h) + j;
          dest.w = src.w;
          dest.h = src.h;

          SDL_BlitSurface(images[IMG_SHIELDS], &src, screen, &dest);
        }
      }
    }
  }


}



void game_draw_misc(void)
{
  int i;
  int offset;
  SDL_Rect dest;
  char str[64];

  /* Draw "Demo" */
  if (Opts_DemoMode())
  {
    dest.x = (screen->w - images[IMG_DEMO]->w) / 2;
    dest.y = (screen->h - images[IMG_DEMO]->h) / 2;
    dest.w = images[IMG_DEMO]->w;
    dest.h = images[IMG_DEMO]->h;

    SDL_BlitSurface(images[IMG_DEMO], NULL, screen, &dest);
  }

  /* If we are playing through a defined list of questions */
  /* without "recycling", display number of remaining questions: */
  if (MC_GetOpt(PLAY_THROUGH_LIST) )
  {
    draw_question_counter();
  }

  if (extra_life_earned) {
    /* Draw extra life earned icon */
    dest.x = 0;
    dest.y = 0;
    dest.w = images[IMG_EXTRA_LIFE]->w;
    dest.h = images[IMG_EXTRA_LIFE]->h;
    SDL_BlitSurface(images[IMG_EXTRA_LIFE], NULL, screen, &dest);
  } else if (bonus_comet_counter) {
    /* Draw extra life progress bar */
    dest.x = 0;
    dest.y = images[IMG_EXTRA_LIFE]->h/4;
    dest.h = images[IMG_EXTRA_LIFE]->h/2;
    dest.w = ((Opts_BonusCometInterval() + 1 - bonus_comet_counter)
              * images[IMG_EXTRA_LIFE]->w) / Opts_BonusCometInterval();
    SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 255, 0));
  }

  /* Draw wave: */
  if (Opts_BonusCometInterval())
    offset = images[IMG_EXTRA_LIFE]->w + 5;
  else
    offset = 0;

  dest.x = offset;
  dest.y = glyph_offset;
  dest.w = images[IMG_WAVE]->w;
  dest.h = images[IMG_WAVE]->h;

  SDL_BlitSurface(images[IMG_WAVE], NULL, screen, &dest);

  sprintf(str, "%d", wave);
  draw_numbers(str, offset+images[IMG_WAVE]->w + (images[IMG_NUMBERS]->w / 10), 0);

  if (Opts_KeepScore())
  {
    /* Draw "score" label: */
    dest.x = (screen->w - ((images[IMG_NUMBERS]->w / 10) * 7) -
                  images[IMG_SCORE]->w -
                  images[IMG_STOP]->w - 5);
    dest.y = glyph_offset;
    dest.w = images[IMG_SCORE]->w;
    dest.h = images[IMG_SCORE]->h;
    SDL_BlitSurface(images[IMG_SCORE], NULL, screen, &dest);
  
    /* In LAN mode, we show the server-generated score: */
    //FIXME we need to figure out which player we are
//    if(Opts_LanMode())
//      sprintf(str, "%.6d", lan_pscores[0]);
//    else
      sprintf(str, "%.6d", score);

    /* Draw score numbers: */
    draw_numbers(str,
                 screen->w - ((images[IMG_NUMBERS]->w / 10) * 6) - images[IMG_STOP]->w - 5,
                 0);
  }

  /* Draw other players' scores (turn-based single machine multiplayer) */
  if (mp_get_parameter(PLAYERS) && mp_get_parameter(MODE) == SCORE_SWEEP )
  {
    int i;
    SDL_Surface* score = NULL;
    SDL_Rect loc;
    for (i = 0; i < mp_get_parameter(PLAYERS); ++i)
    {
      snprintf(str, 64, "%s: %d", mp_get_player_name(i),mp_get_player_score(i));
      score = T4K_BlackOutline(str, DEFAULT_MENU_FONT_SIZE, &white);
      if(score)
      {
        loc.w = screen->w - score->w;
        loc.h = score->h * (i + 2);
        loc.x = 0;
        loc.y = 0;
        SDL_BlitSurface(score, NULL, screen, &loc);
	SDL_FreeSurface(score);
	score = NULL;
      }
    }
  }

   /* Draw other players' scores (LAN game) */
  if (Opts_LanMode())
  {
    int entries = 0;
    SDL_Surface* score = NULL;
    SDL_Rect loc;

    for (i = 0; i < MAX_CLIENTS; i++)
    {
      if(lan_pscores[i] >= 0)
      {
        snprintf(str, 64, "%s: %d", lan_pnames[i], lan_pscores[i]);
        score = T4K_BlackOutline(str, DEFAULT_MENU_FONT_SIZE, &white);
        if(score)
        {
          loc.w = score->w;
          loc.h = score->h;
          loc.x = 0;
          loc.y = score->h * (entries + 2);
          SDL_BlitSurface(score, NULL, screen, &loc);
          entries++;
	  SDL_FreeSurface(score);
	  score = NULL;
        }
      }
    }
  }
  
  /* Draw stop button: */
  if (!help_controls.x_is_blinking || (frame % 10 < 5)) {
    dest.x = (screen->w - images[IMG_STOP]->w);
    dest.y = 0;
    dest.w = images[IMG_STOP]->w;
    dest.h = images[IMG_STOP]->h;

    SDL_BlitSurface(images[IMG_STOP], NULL, screen, &dest);
  }
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
  }
  else
  {
    if (MC_MissionAccomplished())
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
    if(!MC_TotalQuestionsLeft())
      return GAME_OVER_OTHER;
  }


  //NOTE can't use this check in LAN mode because we don't know if the server has 
  //questions left
 
  /* Need to get out if no comets alive and MathCards has no questions left in list, */
  /* even though MathCards thinks there are still questions "in play".  */
  /* This SHOULD NOT HAPPEN and means we have a bug somewhere. */
  if (!Opts_LanMode())
  {
    if (!MC_ListQuestionsLeft() && !num_comets_alive())
    {
      fprintf(stderr, "Error - no questions left but game not over\n");
      DEBUGMSG(debug_game, "ListQuestionsLeft() = %d ", MC_ListQuestionsLeft());
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
  printf("\ngame_status:\t");
  switch (game_status)
  {
    case GAME_IN_PROGRESS:
    {
      printf("GAME_IN_PROGRESS\n");
      break;
    }

    case GAME_OVER_WON:
    {
      printf("GAME_OVER_WON\n");
      break;
    }
    case GAME_OVER_LOST:
    {
      printf("GAME_OVER_LOST\n");
      break;
    }
    case GAME_OVER_OTHER:
    {
      printf("GAME_OVER_OTHER\n");
      break;
    }
    case GAME_OVER_ESCAPE:
    {
      printf("GAME_OVER_ESCAPE\n");
      print_status();
      break;
    }
    case GAME_OVER_WINDOW_CLOSE:
    {
      printf("GAME_OVER_WINDOW_CLOSE\n");
      break;
    }
    case GAME_OVER_ERROR:
    {
      printf("GAME_OVER_ERROR\n");
      break;
    }
    default:
    {
      printf("Unrecognized value\n");
      break;
    }
  }
}


void game_handle_game_over(int game_status)
{
  Uint32 timer = 0;

  DEBUGCODE(debug_game) print_exit_conditions();

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
        frame++;

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
        if (((frame / 2) % 4))
        {
          SDL_BlitSurface(images[IMG_GAMEOVER_WON], NULL, screen, &dest_message);
        }

        /* draw dancing tux: */
        draw_console_image(IMG_CONSOLE_BASH);
        /* walk tux back and forth */
        tux_offset += tux_step;
        /* select tux_egypt images according to which way tux is headed: */
        if (tux_step < 0)
          tux_img = IMG_TUX_EGYPT1 + ((frame / 3) % 2);
        else
          tux_img = IMG_TUX_EGYPT3 + ((frame / 3) % 2);

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
        Throttle(MS_PER_FRAME, &timer);
      }
      while (looping);
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
        frame++;

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

        Throttle(MS_PER_FRAME, &timer);
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

  for (i = 0; i < Opts_MaxComets(); i++)
  {
    DEBUGCODE(debug_game)
    {
      if(comets[i].alive)
        printf("Warning - resetting comets but comet[%d| still alive\n", i);
    }
    comets[i].alive = 0;
  }

  /* Clear LED F: */

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

  /* Set number of attackers for this wave: */

  /* On first wave or if slowdown flagged due to wrong answer: */
  if (wave == 1 || slowdown)
  {
    next_wave_comets = Opts_StartingComets();
    speed = Opts_Speed();
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
        printf("Evaluating feedback...\n  old danger level = %g,",danger_level);
        #endif

        /* Update our danger level, i.e., the target height */
        danger_level = 1 - (1-danger_level) /
                           Opts_DangerLevelSpeedup();
        if (danger_level > Opts_DangerLevelMax())
          danger_level = Opts_DangerLevelMax();

        #ifdef FEEDBACK_DEBUG
        printf(" new danger level = %g.\n",danger_level);
        #endif

        /* Check to see whether we have any feedback data. If not, skip it. */
        if (comet_feedback_number == 0)
        {
          use_feedback = 0;  /* No comets above living cities, skip feedback */

          #ifdef FEEDBACK_DEBUG
          printf("No feedback data available, aborting.\n\n");
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
          printf("  comet average height = %g, height differential = %g.\n",
                 comet_avg_height, height_differential);
          printf("  old speed = %g,",speed);
          #endif

          speed *= (1 - height_differential/danger_level/2);

          /* Enforce bounds on speed */
          if (speed < MINIMUM_SPEED)
            speed = MINIMUM_SPEED;
          if (speed > Opts_MaxSpeed())
            speed = Opts_MaxSpeed();

          #ifdef FEEDBACK_DEBUG
          printf(" new speed = %g.\n",speed);
          printf("Feedback evaluation complete.\n\n");
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
  num_attackers = prev_wave_comets;
}



/* Add a comet to the game (if there's room): */
int add_comet(void)
{
  static int prev_city = -1;
  int i;
  float y_spacing;

  int com_found = -1;
  int q_found = -1;  

  y_spacing = (images[IMG_NUMS]->h) * 1.5;

  /* Return if any previous comet too high up to create another one yet: */
  for (i = 0; i < Opts_MaxComets(); i++)
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
  for (i = 0; i < Opts_MaxComets(); i++)
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


  /* If playing in LAN mode, see if we have a question ready  in  */
  /* our local queue:                                             */
   
  if(Opts_LanMode())
  {
    DEBUGCODE(debug_game) print_current_quests();
    for (i = 0; i < QUEST_QUEUE_SIZE; i++)
    {
      if(quest_queue[i].question_id != -1)
      {
        DEBUGMSG(debug_game, "Found question_id %d, %s\n", 
                  quest_queue[i].question_id,
                  quest_queue[i].formula_string);
        q_found = i;
        break;
      }
    }

    if(q_found == -1)
    {
      DEBUGMSG(debug_game, "add_comet() called but no question available in queue\n");
      DEBUGCODE(debug_game) print_current_quests();
      return 0;
    } 
  }

  /* Now we have a vacant comet slot at com_found and (if in LAN mode) */
  /* a question for it at q_found.  Now just copy:                     */

  if(Opts_LanMode())
  {
    MC_CopyCard(&(quest_queue[q_found]), &(comets[com_found].flashcard));
    MC_ResetFlashCard(&(quest_queue[q_found]));
  }
  else // Not LAN mode - just get question with direct call:
  {
    if (!MC_NextQuestion(&(comets[com_found].flashcard)))
    {
      /* no more questions available - cannot create comet.  */
      return 0;
    }
  }

  DEBUGCODE(debug_game)
  {
    printf("In add_comet(), card is\n");
    print_card(comets[com_found].flashcard);
  }
  
  /* Make sure question is "sane" before we add it: */
  if( (comets[com_found].flashcard.answer > 999)
    ||(comets[com_found].flashcard.answer < -999))
  {
    printf("Warning, card with invalid answer encountered: %d\n",
           comets[com_found].flashcard.answer);
    MC_ResetFlashCard(&(comets[com_found].flashcard));
    return 0;
  }

  /* If we make it to here, create a new comet!*/
  comets[com_found].answer = comets[com_found].flashcard.answer;
  comets[com_found].alive = 1;
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
   
  /* comet slot found and question found so return successfully: */
  return 1;
}



/* Draw numbers/symbols over the attacker: */
/* This draws the numbers related to the comets */
void draw_nums(const char* str, int x, int y)
{
  int i, j, cur_x, c;
  int str_length, char_width, image_length;

  SDL_Rect src, dest;

  /* avoid some recalculation and repeated function calls: */
  str_length = strlen(str);
  /* IMG_NUMS now consists of 10 digit graphics, NUM_OPERS (i.e. 4) */
  /* operation symbols, and the '=' and '?' symbols, all side by side. */
  /* char_width is the width of a single symbol.                     */
  char_width = (images[IMG_NUMS]->w / (16));
  /* Calculate image_length, taking into account that the string will */
  /* usually have four empty spaces that are only half as wide:       */
  image_length = str_length * char_width - (char_width * 0.5 * 4);
  /* Center around the shape */
  cur_x = x - (image_length) / 2;

  /* the following code keeps the formula at least 8 pixels inside the window: */
  if (cur_x < 8)
    cur_x = 8;
  if (cur_x + (image_length) >= (screen->w - 8))
    cur_x = ((screen->w - 8) - (image_length));

  /* Draw each character: */
  for (i = 0; i < str_length; i++)
  {
    c = -1;

    /* Determine which character to display: */

    if (str[i] >= '0' && str[i] <= '9')
    {
      c = str[i] - '0';
    }
    else if ('=' == str[i])
    {
      c = 14;  /* determined by layout of nums.png image */
    }
    else if ('?' == str[i])
    {
      c = 15;  /* determined by layout of nums.png image */
    }
    else  /* [ THIS COULD CAUSE SLOWNESS... ] */
    {
      for (j = 0; j < 4; j++)
      {
        if (str[i] == operchars[j])
        {
          c = 10 + j;
        }
      }
    }

    /* Display this character! */
    if (c != -1)
    {
      src.x = c * char_width;
      src.y = 0;
      src.w = char_width;
      src.h = images[IMG_NUMS]->h;

      dest.x = cur_x;
      dest.y = y - images[IMG_NUMS]->h;
      dest.w = src.w;
      dest.h = src.h;

      SDL_BlitSurface(images[IMG_NUMS], &src,
                          screen, &dest);
      /* Move the 'cursor' one character width: */
      cur_x = cur_x + char_width;
    }
    /* If char is a blank, no drawing to do but still move the cursor: */
    /* NOTE: making spaces only half as wide seems to look better.     */
    if (' ' == str[i])
    {
      cur_x = cur_x + (char_width * 0.5);
    }
  }
}


/* Draw status numbers: */
void draw_numbers(const char* str, int x, int y)
{
  int i, cur_x, c;
  SDL_Rect src, dest;

  cur_x = x;

  /* Draw each character: */

  for (i = 0; i < strlen(str); i++)
  {
    c = -1;

    /* Determine which character to display: */
    if (str[i] >= '0' && str[i] <= '9')
      c = str[i] - '0';

    /* Display this character! */
    if (c != -1)
    {
      src.x = c * (images[IMG_NUMBERS]->w / 10);
      src.y = 0;
      src.w = (images[IMG_NUMBERS]->w / 10);
      src.h = images[IMG_NUMBERS]->h;

      dest.x = cur_x;
      dest.y = y;
      dest.w = src.w;
      dest.h = src.h;

      SDL_BlitSurface(images[IMG_NUMBERS], &src,
                          screen, &dest);

      /* Move the 'cursor' one character width: */
      cur_x = cur_x + (images[IMG_NUMBERS]->w / 10);
    }
  }
}


/* Pause loop: */

int pause_game(void)
{
  /* NOTE - done and quit changed to pause_done and pause_quit */
  /* due to potentially confusing name collision */
  int pause_done, pause_quit;
  SDL_Event event;
  SDL_Rect dest;

  /* Only pause if pause allowed: */
  if (!Opts_AllowPause())
  {
    fprintf(stderr, "Pause requested but not allowed by Opts!\n");
    return 0;
  }

  pause_done = 0;
  pause_quit = 0;

  dest.x = (screen->w - images[IMG_PAUSED]->w) / 2;
  dest.y = (screen->h - images[IMG_PAUSED]->h) / 2;
  dest.w = images[IMG_PAUSED]->w;
  dest.h = images[IMG_PAUSED]->h;

  T4K_DarkenScreen(1);  // cut all channels by half
  SDL_BlitSurface(images[IMG_PAUSED], NULL, screen, &dest);
  SDL_UpdateRect(screen, 0, 0, 0, 0);

#ifndef NOSOUND
  if (Opts_UsingSound())
    Mix_PauseMusic();
#endif

  do
  {
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_KEYDOWN)
        pause_done = 1;
      else if (event.type == SDL_QUIT)
      {
        user_quit_received = GAME_OVER_WINDOW_CLOSE;
        pause_quit = 1;
      }
    }

    SDL_Delay(100);
  }
  while (!pause_done && !pause_quit);

#ifndef NOSOUND
  if (Opts_UsingSound())
    Mix_ResumeMusic();
#endif

  return (pause_quit);
}



/* FIXME these ought to be in SDL_extras - DSB */

/* Draw a line: */
void draw_line(int x1, int y1, int x2, int y2, int red, int grn, int blu)
{
  int dx, dy, tmp;
  float m, b;
  Uint32 pixel;
  SDL_Rect dest;

  pixel = SDL_MapRGB(screen->format, red, grn, blu);

  dx = x2 - x1;
  dy = y2 - y1;

  putpixel(screen, x1, y1, pixel);

  if (dx != 0)
  {
    m = ((float) dy) / ((float) dx);
    b = y1 - m * x1;

    if (x2 > x1)
      dx = 1;
    else
      dx = -1;

    while (x1 != x2)
    {
      x1 = x1 + dx;
      y1 = m * x1 + b;

      putpixel(screen, x1, y1, pixel);
    }
  }
  else
  {
    if (y1 > y2)
    {
      tmp = y1;
      y1 = y2;
      y2 = tmp;
    }

    dest.x = x1;
    dest.y = y1;
    dest.w = 3;
    dest.h = y2 - y1;

    SDL_FillRect(screen, &dest, pixel);
  }
}


/* Draw a single pixel into the surface: */

void putpixel(SDL_Surface* surface, int x, int y, Uint32 pixel)
{
#ifdef PUTPIXEL_RAW
  int bpp;
  Uint8* p;

  /* Determine bytes-per-pixel for the surface in question: */

  bpp = surface->format->BytesPerPixel;


  /* Set a pointer to the exact location in memory of the pixel
     in question: */

  p = (Uint8 *) (surface->pixels +       /* Start at beginning of RAM */
                 (y * surface->pitch) +  /* Go down Y lines */
                 (x * bpp));             /* Go in X pixels */


  /* Assuming the X/Y values are within the bounds of this surface... */

  if (x >= 0 && y >= 0 && x < surface->w && y < surface->h)
  {
      /* Set the (correctly-sized) piece of data in the surface's RAM
         to the pixel value sent in: */

    if (bpp == 1)
      *p = pixel;
    else if (bpp == 2)
      *(Uint16 *)p = pixel;
    else if (bpp == 3)
    {
      if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
      {
        p[0] = (pixel >> 16) & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = pixel & 0xff;
      }
      else
      {
        p[0] = pixel & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = (pixel >> 16) & 0xff;
      }
    }
    else if (bpp == 4)
    {
      *(Uint32 *)p = pixel;
    }
  }
#else
  SDL_Rect dest;

  dest.x = x;
  dest.y = y;
  dest.w = 3;
  dest.h = 4;

  SDL_FillRect(surface, &dest, pixel);
#endif
}


/* Draw image at lower center of screen: */
void draw_console_image(int i)
{
  SDL_Rect dest;

  dest.x = (screen->w - images[i]->w) / 2;
  dest.y = (screen->h - images[i]->h);
  dest.w = images[i]->w;
  dest.h = images[i]->h;

  SDL_BlitSurface(images[i], NULL, screen, &dest);
}


void draw_question_counter(void)
{
  int questions_left;
  int comet_img;
  int nums_width;
  int nums_x;
  int comet_width;
  int comet_x;

  char str[64];
  SDL_Rect dest;

  /* Calculate placement based on image widths: */
  nums_width = (images[IMG_NUMBERS]->w / 10) * 4; /* displaying 4 digits */
  comet_width = images[IMG_MINI_COMET1]->w;
  comet_x = (screen->w)/2 - (comet_width + nums_width)/2;
  nums_x = comet_x + comet_width;

  /* Draw mini comet symbol:            */
  /* Decide which image to display: */
  comet_img = IMG_MINI_COMET1 + (frame % 3);
  /* Draw it! */
  dest.x = comet_x;
  dest.y = 0;
  dest.w = comet_width;
  dest.h = images[comet_img]->h;

  SDL_BlitSurface(images[comet_img], NULL, screen, &dest);

  /* draw number of remaining questions: */
  if(Opts_LanMode())
    questions_left = total_questions_left;
  else
    questions_left = MC_TotalQuestionsLeft();

  sprintf(str, "%.4d", questions_left);
  draw_numbers(str, nums_x, 0);
}

/* FIXME very confusing having this function draw console */
void draw_led_console(void)
{
  int i;
  SDL_Rect src, dest;
  int y;

  /* draw new console image with "monitor" for LED numbers: */
  draw_console_image(IMG_CONSOLE_LED);
  /* set y to draw LED numbers into Tux's "monitor": */
  y = (screen->h
     - images[IMG_CONSOLE_LED]->h
     + 4);  /* "monitor" has 4 pixel margin */

  /* begin drawing so as to center display depending on whether minus */
  /* sign needed (4 digit slots) or not (3 digit slots) DSB */
  if (MC_GetOpt(ALLOW_NEGATIVES) )
    dest.x = ((screen->w - ((images[IMG_LEDNUMS]->w) / 10) * 4) / 2);
  else
    dest.x = ((screen->w - ((images[IMG_LEDNUMS]->w) / 10) * 3) / 2);

  for (i = -1; i < MC_MAX_DIGITS; i++) /* -1 is special case to allow minus sign */
                              /* with minimal modification of existing code DSB */
  {
    if (-1 == i)
    {
      if (MC_GetOpt(ALLOW_NEGATIVES))
      {
        if (neg_answer_picked)
          src.x =  (images[IMG_LED_NEG_SIGN]->w) / 2;
        else
          src.x = 0;

        src.y = 0;
        src.w = (images[IMG_LED_NEG_SIGN]->w) / 2;
        src.h = images[IMG_LED_NEG_SIGN]->h;

        dest.y = y;
        dest.w = src.w;
        dest.h = src.h;

        SDL_BlitSurface(images[IMG_LED_NEG_SIGN], &src, screen, &dest);
        /* move "cursor" */
        dest.x += src.w;
      }
    }
    else
    {
      src.x = digits[i] * ((images[IMG_LEDNUMS]->w) / 10);
      src.y = 0;
      src.w = (images[IMG_LEDNUMS]->w) / 10;
      src.h = images[IMG_LEDNUMS]->h;

      /* dest.x already set */
      dest.y = y;
      dest.w = src.w;
      dest.h = src.h;

      SDL_BlitSurface(images[IMG_LEDNUMS], &src, screen, &dest);
      /* move "cursor" */
      dest.x += src.w;
    }
  }
}

/* Translates mouse events into keyboard events when on-screen keypad used */
/* or when exit button clicked.                                            */
void game_mouse_event(SDL_Event event)
{
  int keypad_w, keypad_h, x, y, row, column;
  SDLKey key = SDLK_UNKNOWN;
  SDLMod mod = event.key.keysym.mod;
  keypad_w = 0;
  keypad_h = 0;

  /* Check to see if user clicked exit button: */
  /* The exit button is in the upper right corner of the screen: */
  if ((event.button.x >= (screen->w - images[IMG_STOP]->w))
    &&(event.button.y <= images[IMG_STOP]->h))
  {
    key = SDLK_ESCAPE;
    game_key_event(key, mod);
    return;
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
  if (MC_GetOpt(ALLOW_NEGATIVES))
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
      printf("\nIllegal row or column value!\n");
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
    game_key_event(key, mod);
  }
}

/* called by either key presses or mouse clicks on */
/* on-screen keypad */
void game_key_event(SDLKey key, SDLMod mod)
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
        && MC_GetOpt(ALLOW_NEGATIVES) )  /* do nothing unless neg answers allowed */
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
	    MC_GetOpt(ALLOW_NEGATIVES)
          )  /* do nothing unless neg answers allowed */
  {
    /* allow player to make answer positive: */
	  printf("SDKL_PLUS received\n");
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
}

/* Increment score: */

void add_score(int inc)
{
  score += inc;
  DEBUGMSG(debug_game,"Score is now: %d\n", score);
}



void reset_comets(void)
{
  int i = 0;
  comet_counter = 0;

  for (i = 0; i < Opts_MaxComets(); i++)
  {
    comets[i].alive = 0;
    comets[i].expl = -1;
    comets[i].city = 0;
    comets[i].x = 0;
    comets[i].y = 0;
    comets[i].answer = 0;
    MC_ResetFlashCard(&(comets[i].flashcard) );
    comets[i].bonus = 0;
  }
}


void print_status(void)
{
  int i;

  printf("\nCities:");
  printf("\nHits left: ");
  for (i = 0; i < NUM_CITIES; i++)
    printf("%02d ",cities[i].hits_left);
  printf("\nStatus:    ");
  for (i = 0; i < NUM_CITIES; i++)
    printf("%02d ",cities[i].status);

  printf("\nPenguins:");
  printf("\nStatus:    ");
  for (i = 0; i < NUM_CITIES; i++)
    printf("%02d ",penguins[i].status);

  printf("\nCloud:");
  printf("\nStatus:    %d",cloud.status);
  printf("\nCity:      %d",cloud.city);
  printf("\n");
}


void free_on_exit(void)
{
  int i;
  for (i = 0; i < MAX_MAX_COMETS; ++i)
    MC_FreeFlashcard(&(comets[i].flashcard));
  free(comets);
  free(cities);
  free(penguins);
  free(steam);
}

/* Recalculate on-screen city & comet locations when screen dimensions change */
void game_recalc_positions(int xres, int yres)
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

  city_expl_height = yres - images[IMG_CITY_BLUE]->h;
  //move comets to a location 'equivalent' to where they were
  //i.e. with the same amount of time left before impact
  for (i = 0; i < Opts_MaxComets(); ++i)
  {
    if (!comets[i].alive)
      continue;

    comets[i].x = cities[comets[i].city].x;
    //if (Opts_GetGlobalOpt(FULLSCREEN) )
      comets[i].y = comets[i].y * city_expl_height / old_city_expl_height;
    //else
    //  comets[i].y = comets[i].y * RES_Y / screen->h;
  }
}

static int num_comets_alive()
{
  int i = 0;
  int living = 0;
  for(i = 0; i < Opts_MaxComets(); i++)
    if(comets[i].alive)
      living++;
  return living;
}

