/*
  menu.c

  Functions responsible for loading, parsing and displaying game menu.

  Part of "Tux4Kids" Project
  http://www.tux4kids.com/

  Author: Boleslaw Kulbabinski <bkulbabinski@gmail.com>, (C) 2009

  (Functions responsible for running specific activities
   are moved from titlescreen.c)

  Copyright: See COPYING file that comes with this distribution.
*/

#include "menu.h"
#include "SDL_extras.h"
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
#include "network.h"
#include "server.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/* create string array of activities' names */
#define X(name) #name
char* activities[] = { ACTIVITIES };
#undef X

/* we may use a few separate menu trees */
typedef enum {
  MENU_MAIN,
  MENU_DIFFICULTY,
  MENU_LESSONS,
  MENU_LOGIN,
  N_OF_MENUS
} MenuType;

MenuNode* menus[N_OF_MENUS];

/* actions available while viewing the menu */
enum { NONE, CLICK, PAGEUP, PAGEDOWN, STOP_ESC, RESIZED };

/* stop button, left and right arrow positions do not
   depend on currently displayed menu */
SDL_Rect menu_rect, stop_rect, prev_rect, next_rect;
SDL_Surface *stop_button, *prev_arrow, *next_arrow, *prev_gray, *next_gray;

/*TODO: move these constants into a config file (maybe together with
  titlescreen paths and rects ? ) */
const float menu_pos[4] = {0.38, 0.23, 0.55, 0.72};
const float stop_pos[4] = {0.94, 0.0, 0.06, 0.06};
const float prev_pos[4] = {0.87, 0.93, 0.06, 0.06};
const float next_pos[4] = {0.94, 0.93, 0.06, 0.06};
const char* stop_path = "status/stop.svg";
const char* prev_path = "status/left.svg";
const char* next_path = "status/right.svg";
const char* prev_gray_path = "status/left_gray.svg";
const char* next_gray_path = "status/right_gray.svg";
const float button_gap = 0.2, text_h_gap = 0.4, text_w_gap = 0.5, button_radius = 0.27;
const int min_font_size = 8, default_font_size = 20, max_font_size = 40;

/* font size used in current resolution */
int curr_font_size;

/* menu title rect */
SDL_Rect menu_title_rect;

/* buffer size used when reading attributes or names */
const int buf_size = 128;



/* local functions */
MenuNode*       create_empty_node();
char*           get_attribute_name(const char* token);
char*           get_attribute_value(const char* token);
void            read_attributes(FILE* xml_file, MenuNode* node);
MenuNode*       load_menu_from_file(FILE* xml_file, MenuNode* parent);
void            free_menu(MenuNode* menu);
MenuNode*       create_one_level_menu(int items, char** item_names, char* title, char* trailer);

int             handle_activity(int act, int param);
int             run_academy(void);
int             run_arcade(int choice);
int             run_custom_game(void);
void            run_multiplayer(int mode, int difficulty);
int             run_factoroids(int choice);
int             run_lan_join(void);
int             run_lan_host(void);

int             run_menu(MenuNode* menu, bool return_choice);
SDL_Surface**   render_buttons(MenuNode* menu, bool selected);
void            prerender_menu(MenuNode* menu);
char*           find_longest_text(MenuNode* menu, int* length);
void            set_font_size();
void            prerender_all();



/*
  functions responsible for parsing menu files
  and creating menu trees
*/

/* creates new MenuNode struct with all fields set to NULL (or 0) */
MenuNode* create_empty_node()
{
  MenuNode* new_node = malloc(sizeof(MenuNode));
  new_node->parent = NULL;
  new_node->title = NULL;
  new_node->icon_name = NULL;
  new_node->icon = NULL;
  new_node->submenu_size = 0;
  new_node->submenu = NULL;
  new_node->activity = 0;
  new_node->param = 0;
  new_node->first_entry = 0;
  new_node->show_title = false;

  return new_node;
}

/* read attributes and fill appropriate node fields */
void read_attributes(FILE* xml_file, MenuNode* node)
{
  char attr_name[buf_size];
  char attr_val[buf_size];
  int i;

  /* read tokens until closing '>' is found */
  do
  {
    fscanf(xml_file, " %[^=\n]", attr_name);

    DEBUGMSG(debug_menu_parser, "read_attributes(): read attribute name: %s\n", attr_name);
    if(strchr(attr_name, '>'))
      break;

    fscanf(xml_file, "=\"%[^\"]\"", attr_val);
    DEBUGMSG(debug_menu_parser, "read_attributes(): read attribute value: %s\n", attr_val);

    if(strcmp(attr_name, "title") == 0)
      node->title = strdup(attr_val);
    else if(strcmp(attr_name, "entries") == 0)
      node->submenu_size = atoi(attr_val);
    else if(strcmp(attr_name, "param") == 0)
      node->param = atoi(attr_val);
    else if(strcmp(attr_name, "sprite") == 0)
      node->icon_name = strdup(attr_val);
    else if(strcmp(attr_name, "run") == 0)
    {
      for(i = 0; i < N_OF_ACTIVITIES; i++)
        if(strcmp(attr_val, activities[i]) == 0)
          node->activity = i;
    }
    else
      DEBUGMSG(debug_menu_parser, "read_attributes(): unknown attribute %s , omitting\n", attr_name);

  } while(strchr(attr_val, '>') == NULL);
}

/* recursively read and parse given xml menu file and create menu tree
   return NULL in case of problems */
MenuNode* load_menu_from_file(FILE* xml_file, MenuNode* parent)
{
  MenuNode* new_node = create_empty_node();
  char buffer[buf_size];
  int i;

  new_node->parent = parent;

  DEBUGMSG(debug_menu_parser, "entering load_menu_from_file()\n");
  fscanf(xml_file, " < %s", buffer);

  if(strcmp(buffer, "menu") == 0)
  {
    read_attributes(xml_file, new_node);
    if(new_node->title == NULL)
    {
      DEBUGMSG(debug_menu_parser, "load_menu_from_file(): no title attribute, exiting\n");
      return NULL;
    }

    if(new_node->submenu_size > 0)
    {
      new_node->submenu = malloc(new_node->submenu_size * sizeof(MenuNode));
      for(i = 0; i < new_node->submenu_size; i++)
        new_node->submenu[i] = load_menu_from_file(xml_file, new_node);
    }

    fscanf(xml_file, " </%[^>\n]> ", buffer);
    if(strcmp(buffer, "menu") != 0)
      DEBUGMSG(debug_menu_parser, "load_menu_from_file(): warning - no closing menu tag, found %s instead\n", buffer);
  }
  else if(strcmp(buffer, "item") == 0)
  {
    read_attributes(xml_file, new_node);
    if(new_node->title == NULL)
    {
      DEBUGMSG(debug_menu_parser, "load_menu_from_file(): no title attribute, exiting\n");
      return NULL;
    }
  }
  else
  {
    DEBUGMSG(debug_menu_parser, "load_menu_from_file(): unknown tag: %s\n, exiting\n", buffer);
    return NULL;
  }

  DEBUGMSG(debug_menu_parser, "load_menu_from_file(): node loaded successfully\n");
  return new_node;
}

/* recursively free all non-NULL pointers in a menu tree */
void free_menu(MenuNode* menu)
{
  int i;

  DEBUGMSG(debug_menu, "entering free_menu()\n");
  if(menu != NULL)
  {
    if(menu->title != NULL)
      free(menu->title);
    if(menu->icon_name != NULL)
      free(menu->icon_name);
    if(menu->icon != NULL)
      FreeSprite(menu->icon);

    if(menu->submenu != NULL)
    {
      for(i = 0; i < menu->submenu_size; i++)
        if(menu->submenu[i] != NULL)
        {
          free_menu(menu->submenu[i]);
          menu->submenu[i] = NULL;
        }
      free(menu->submenu);
    }

    free(menu);
  }
}

/* create a simple one-level menu without sprites.
   all given strings are copied */
MenuNode* create_one_level_menu(int items, char** item_names, char* title, char* trailer)
{
  MenuNode* menu = create_empty_node();
  int i;

  if(title)
  {
    menu->title = strdup(title);
    menu->show_title = true;
  }
  menu->submenu_size = items + (trailer ? 1 : 0);
  menu->submenu = (MenuNode**) malloc(menu->submenu_size * sizeof(MenuNode*));
  for(i = 0; i < items; i++)
  {
    menu->submenu[i] = create_empty_node();
    menu->submenu[i]->title = strdup(item_names[i]);
    menu->submenu[i]->activity = i;
  }

  if(trailer)
  {
    menu->submenu[items] = create_empty_node();
    menu->submenu[items]->title = strdup(trailer);
    menu->submenu[items]->activity = items;
  }

  return menu;
}

/*
  handlers for specific game activities
*/

/* return QUIT if user decided to quit the application while running an activity
   return 0 otherwise */
int handle_activity(int act, int param)
{
  DEBUGMSG(debug_menu, "entering handle_activity()\n");

  switch(act)
  {
    case RUN_CAMPAIGN:
      start_campaign();
      break;

    case RUN_ACADEMY:
      if(run_academy() == QUIT)
        return QUIT;
      break;

    case RUN_ARCADE:
      run_arcade(param);
      break;

    case RUN_LAN_HOST:
      run_lan_host();
      break;

    case RUN_LAN_JOIN:
      run_lan_join();
      break;

    case RUN_CUSTOM:
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
        {audioMusicUnload();}
      game();
      if (Opts_GetGlobalOpt(MENU_MUSIC)) //Turn menu music back on
        audioMusicLoad( "tuxi.ogg", -1 );
      Opts_SetHelpMode(0);
      break;

    case RUN_FACTORS:
      run_factoroids(0);
      break;

    case RUN_FRACTIONS:
      run_factoroids(1);
      break;

    case RUN_DEMO:
      if(read_named_config_file("demo"))
      {
        audioMusicUnload();
        game();
        if (Opts_GetGlobalOpt(MENU_MUSIC))
          audioMusicLoad( "tuxi.ogg", -1 );
      }
      else
        fprintf(stderr, "\nCould not find demo config file\n");
      break;

    case RUN_INFO:
      ShowMessage(DEFAULT_MENU_FONT_SIZE,
                  _("TuxMath is free and open-source!"),
                  _("You can help make it better."),
                  _("Suggestions, artwork, and code are all welcome!"),
                  _("Discuss TuxMath at tuxmath-devel@lists.sourceforge.net"));
      break;

    case RUN_CREDITS:
      credits();
      break;

    case RUN_QUIT:
      return QUIT;
  }

  DEBUGMSG(debug_menu, "Leaving handle_activity\n");

  return 0;
}

int run_academy(void)
{
  int chosen_lesson = -1;

  chosen_lesson = run_menu(menus[MENU_LESSONS], true);
  while (chosen_lesson >= 0)
  {
    if (Opts_GetGlobalOpt(MENU_SOUND))
      playsound(SND_POP);

    /* Re-read global settings first in case any settings were */
    /* clobbered by other lesson or arcade games this session: */
    read_global_config_file();
    /* Now read the selected file and play the "mission": */
    if (read_named_config_file(lesson_list_filenames[chosen_lesson]))
    {
      if (Opts_GetGlobalOpt(MENU_MUSIC))  //Turn menu music off for game
        {audioMusicUnload();}

      game();

      /* If successful, display Gold Star for this lesson! */
      if (MC_MissionAccomplished())
      {
        lesson_list_goldstars[chosen_lesson] = 1;
       /* and save to disk: */
        write_goldstars();
      }

      if (Opts_GetGlobalOpt(MENU_MUSIC)) //Turn menu music back on
        {audioMusicLoad("tuxi.ogg", -1);}
    }
    else  // Something went wrong - could not read lesson config file:
    {
      fprintf(stderr, "\nCould not find file: %s\n", lesson_list_filenames[chosen_lesson]);
      chosen_lesson = -1;
    }
    // Let the user choose another lesson; start with the screen and
    // selection that we ended with
    chosen_lesson = run_menu(menus[MENU_LESSONS], true);
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
    if (read_named_config_file(arcade_config_files[choice]))
    {
      audioMusicUnload();
      game();
      RenderTitleScreen();
      if (Opts_GetGlobalOpt(MENU_MUSIC))
        audioMusicLoad( "tuxi.ogg", -1 );
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
  const char *s1, *s2, *s3, *s4;
  s1 = _("Edit 'options' file in your home directory");
  s2 = _("to create customized game!");
  s3 = _("Press a key or click your mouse to start game.");
  s4 = _("See README.txt for more information");
  ShowMessage(DEFAULT_MENU_FONT_SIZE, s1, s2, s3, s4);

  if (read_user_config_file()) {
    if (Opts_GetGlobalOpt(MENU_MUSIC))
      audioMusicUnload();

    game();
    write_user_config_file();

    if (Opts_GetGlobalOpt(MENU_MUSIC))
      audioMusicLoad( "tuxi.ogg", -1 );
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
                     _("(Between 2 and 4 players)"));
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

  audioMusicUnload();
  if(choice == 0)
    factors();
  else
    fractions();

  if (Opts_GetGlobalOpt(MENU_MUSIC))
    audioMusicLoad( "tuxi.ogg", -1 );

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
  char buf[256];
  char server_name[150];
  char* argv[3];
  int chosen_lesson = -1;

  /* For now, only allow one server instance: */
  if(ServerRunning())
  {
    ShowMessage(DEFAULT_MENU_FONT_SIZE, _("The server is already running"),
                NULL, NULL, NULL);
    return 0;
  }

  NameEntry(server_name, _("Enter Server Name:"), _("(limit 50 characters)"));
  argv[0] = "tuxmathserver";
  argv[1] = "--name";
  snprintf(buf, 256, "\"%s\"", server_name);
  argv[2] = buf;


  /* If we have POSIX threads available (Linux), we launch server in a thread within  */
  /* our same process. The server will use the currently selected Mathcards settings, */
  /* so we can let the user select the lesson for the server to use.                  */

#ifdef HAVE_PTHREAD_H

  ShowMessage(DEFAULT_MENU_FONT_SIZE, 
              _("Click or press key to select server lesson file"),
              NULL, NULL, NULL);

  {
    chosen_lesson = run_menu(menus[MENU_LESSONS], true);

    while (chosen_lesson >= 0)
    {
      if (Opts_GetGlobalOpt(MENU_SOUND))
        playsound(SND_POP);

      /* Re-read global settings first in case any settings were */
      /* clobbered by other lesson or arcade games this session: */
      read_global_config_file();
      /* Now read the selected file and play the "mission": */
      if (read_named_config_file(lesson_list_filenames[chosen_lesson]))
        break;
      else    
      {  // Something went wrong - could not read lesson config file:
        fprintf(stderr, "\nCould not find file: %s\n", lesson_list_filenames[chosen_lesson]);
        chosen_lesson = -1;
      }
      // Let the user choose another lesson; start with the screen and
      // selection that we ended with
      chosen_lesson = run_menu(menus[MENU_LESSONS], true);
    }
  }

  ShowMessage(DEFAULT_MENU_FONT_SIZE,
              _("Server Name:"),
              server_name,
              _("Selected Lesson:"),
              lesson_list_titles[chosen_lesson]);

  RunServer_pthread(3, argv);


  /* Without pthreads, we just launch standalone server, which for now only     */
  /* supports the hardcoded default settings.                                   */
#else
  RunServer_prog(3, argv);
#endif

#endif
  return 0;
}



int run_lan_join(void)
{
#ifdef HAVE_LIBSDL_NET
  if(detecting_servers(_("Detecting servers"), _("Please wait")))
  {
    int stdby;
    char buf[256];
    char player_name[HIGH_SCORE_NAME_LENGTH * 3];

    snprintf(buf, 256, _("Connected to server: %s"), LAN_ConnectedServerName());
    NameEntry(player_name, buf, _("Enter your name:"));
    LAN_SetName(player_name);
    Ready(_("Click when ready"));
    LAN_StartGame();
    stdby = Standby(_("Waiting for other players"), NULL);
    if (stdby == 1)
    {
      audioMusicUnload();
      Opts_SetLanMode(1);  // Tells game() we are playing over network
      game();
      Opts_SetLanMode(0);  // Go back to local play
      if (Opts_GetGlobalOpt(MENU_MUSIC))
          audioMusicLoad( "tuxi.ogg", -1 );
    }
    else
    {
      ShowMessage(DEFAULT_MENU_FONT_SIZE,
                  NULL, _("Sorry, game already in progress."), NULL, NULL);
      printf(_("Sorry, game already in progress.\n"));
    }  
  }
  else
  {
    ShowMessage(DEFAULT_MENU_FONT_SIZE, 
                NULL, _("Sorry, no server could be found."), NULL, NULL);
    printf(_("Sorry, no server could be found.\n"));
  }
#else
  ShowMessage(DEFAULT_MENU_FONT_SIZE, 
              NULL, _("Sorry, this version built without network support"), NULL, NULL);
  printf( _("Sorry, this version built without network support.\n"));
#endif

  DEBUGMSG(debug_menu, "Leaving run_lan_join()\n"); 
  return 0;
}


/* Display the menu and run the event loop.
   if return_choice = true then return chosen value instead of
   running handle_activity()
   this function is a modified copy of choose_menu_item() */
int run_menu(MenuNode* root, bool return_choice)
{
  SDL_Surface** menu_item_unselected = NULL;
  SDL_Surface** menu_item_selected = NULL;
  SDL_Surface* title_surf;
  SDL_Event event;
  MenuNode* menu = root;
  MenuNode* tmp_node;

  SDL_Rect tmp_rect;
  sprite* tmp_sprite;
  int i;
  int stop = 0;
  int items;

  int action = NONE;

  Uint32 frame_start = 0;       //For keeping frame rate constant
  Uint32 frame_now = 0;
  Uint32 frame_counter = 0;
  int loc = -1;                  //The currently selected menu item
  int old_loc = -1;
  int click_flag = 1;

  for(;;) /* one loop body execution for one menu page */
  {
    DEBUGMSG(debug_menu, "run_menu(): drawing whole new menu page\n");

    DrawTitleScreen();
    /* render buttons for current menu page */
    menu_item_unselected = render_buttons(menu, false);
    menu_item_selected = render_buttons(menu, true);
    items = min(menu->entries_per_screen, menu->submenu_size - menu->first_entry);

    DEBUGMSG(debug_menu, "run_menu(): drawing %d buttons\n", items);
    for(i = 0; i < items; i++)
    {
      if(loc == i)
        SDL_BlitSurface(menu_item_selected[i], NULL, screen, &menu->submenu[menu->first_entry + i]->button_rect);
      else
        SDL_BlitSurface(menu_item_unselected[i], NULL, screen, &menu->submenu[menu->first_entry + i]->button_rect);
      if(menu->submenu[menu->first_entry + i]->icon)
        SDL_BlitSurface(menu->submenu[menu->first_entry + i]->icon->default_img, NULL, screen, &menu->submenu[menu->first_entry + i]->icon_rect);
    }

    SDL_BlitSurface(stop_button, NULL, screen, &stop_rect);

    if(menu->entries_per_screen < menu->submenu_size)
    {
      /* display arrows */
      if(menu->first_entry > 0)
        SDL_BlitSurface(prev_arrow, NULL, screen, &prev_rect);
      else
        SDL_BlitSurface(prev_gray, NULL, screen, &prev_rect);
      if(menu->first_entry + items < menu->submenu_size)
        SDL_BlitSurface(next_arrow, NULL, screen, &next_rect);
      else
        SDL_BlitSurface(next_gray, NULL, screen, &next_rect);
    }

    if(menu->show_title)
    {
      menu_title_rect = menu->submenu[0]->button_rect;
      menu_title_rect.y = menu_rect.y - menu_title_rect.h;
      title_surf = BlackOutline(_(menu->title), curr_font_size, &red);
      SDL_BlitSurface(title_surf, NULL, screen, &menu_title_rect);
      SDL_FreeSurface(title_surf);
    }
    SDL_UpdateRect(screen, 0, 0, 0, 0);

    SDL_WM_GrabInput(SDL_GRAB_OFF);

    while (SDL_PollEvent(&event));  // clear pending events

    /******** Main loop: *********/
    stop = false;
    DEBUGMSG(debug_menu, "run_menu(): entering menu loop\n");
    while (!stop)
    {
      frame_start = SDL_GetTicks();         /* For keeping frame rate constant.*/

      action = NONE;
      while (!stop && SDL_PollEvent(&event))
      {
        switch (event.type)
        {
          case SDL_QUIT:
          {
            FreeSurfaceArray(menu_item_unselected, items);
            FreeSurfaceArray(menu_item_selected, items);
            return QUIT;
          }

          case SDL_MOUSEMOTION:
          {
            loc = -1;
            for (i = 0; i < items; i++)
            {
              if (inRect(menu->submenu[menu->first_entry + i]->button_rect, event.motion.x, event.motion.y))
              {
                if (Opts_GetGlobalOpt(MENU_SOUND) && old_loc != i)
                  playsound(SND_TOCK);
                loc = i;
                break;   /* from for loop */
              }
            }

            /* "Left" button - make click if button active: */
            if(inRect(prev_rect, event.motion.x, event.motion.y)
               && menu->first_entry > 0 && Opts_GetGlobalOpt(MENU_SOUND))
            {
              if(click_flag)
              {
                playsound(SND_TOCK);
                click_flag = 0;
              }
            }

            /* "Right" button - make click if button active: */
            else if(inRect(next_rect, event.motion.x, event.motion.y)
               && menu->first_entry + items < menu->submenu_size
               && Opts_GetGlobalOpt(MENU_SOUND))
            {
              if(click_flag)
              {
                playsound(SND_TOCK);
                click_flag = 0;
              }
            }

            /* "stop" button */
            else if (inRect(stop_rect, event.motion.x, event.motion.y )
               && Opts_GetGlobalOpt(MENU_SOUND))
            {
              if(click_flag)
              {
                playsound(SND_TOCK);
                click_flag = 0;
              }
            }

            else  // Mouse outside of arrow rects - re-enable click sound:
              click_flag = 1;

            break;
          }

          case SDL_MOUSEBUTTONDOWN:
          {
            loc = -1;  // By default, don't be in any entry
            for (i = 0; i < items; i++)
            {
              if (inRect(menu->submenu[menu->first_entry + i]->button_rect, event.motion.x, event.motion.y))
              {
                // Play sound if loc is being changed:
                if (Opts_GetGlobalOpt(MENU_SOUND))
                  playsound(SND_POP);
                loc = i;
                action = CLICK;
                break;   /* from for loop */
              }
            }

            /* "Left" button */
            if (inRect(prev_rect, event.motion.x, event.motion.y)
               && menu->first_entry > 0)
            {
              if (Opts_GetGlobalOpt(MENU_SOUND))
                playsound(SND_POP);
              action = PAGEUP;
            }

            /* "Right" button - go to next page: */
            else if (inRect(next_rect, event.motion.x, event.motion.y )
               && menu->first_entry + items < menu->submenu_size)
            {
              if (Opts_GetGlobalOpt(MENU_SOUND))
                playsound(SND_POP);
              action = PAGEDOWN;
            }

            /* "Stop" button - go to main menu: */
            else if (inRect(stop_rect, event.button.x, event.button.y ))
            {
              if (Opts_GetGlobalOpt(MENU_SOUND))
                playsound(SND_POP);
              action = STOP_ESC;
            }

            break;
          } /* End of case SDL_MOUSEDOWN */

          case SDL_KEYDOWN:
          {
            /* Proceed according to particular key pressed: */
            switch (event.key.keysym.sym)
            {
              case SDLK_ESCAPE:
              {
                action = STOP_ESC;
                break;
              }

              case SDLK_RETURN:
              case SDLK_SPACE:
              case SDLK_KP_ENTER:
              {
                if (Opts_GetGlobalOpt(MENU_SOUND))
                  playsound(SND_POP);
                action = CLICK;
                break;
              }

              /* Go to previous page, if present: */
              case SDLK_LEFT:
              case SDLK_PAGEUP:
              {
                if (Opts_GetGlobalOpt(MENU_SOUND))
                  playsound(SND_TOCK);
                if (menu->first_entry > 0)
                  action = PAGEUP;
                break;
              }

              /* Go to next page, if present: */
              case SDLK_RIGHT:
              case SDLK_PAGEDOWN:
              {
                if (Opts_GetGlobalOpt(MENU_SOUND))
                  playsound(SND_TOCK);
                if (menu->first_entry + items < menu->submenu_size)
                  action = PAGEDOWN;
                break;
              }

              /* Go up one entry, if present: */
              case SDLK_UP:
              {
                if (Opts_GetGlobalOpt(MENU_SOUND))
                  playsound(SND_TOCK);
                if (loc > 0)
                  loc--;
                else if (menu->submenu_size <= menu->entries_per_screen) 
                  loc = menu->submenu_size - 1;  // wrap around if only 1 screen
                else if (menu->first_entry > 0)
                {
                  loc = menu->entries_per_screen - 1;
                  action = PAGEUP;
                }
                break;
              }

              case SDLK_DOWN:
              {
                if (Opts_GetGlobalOpt(MENU_SOUND))
                  playsound(SND_TOCK);
                if (loc + 1 < min(menu->submenu_size, menu->entries_per_screen))
                  loc++;
                else if (menu->submenu_size <= menu->entries_per_screen) 
                  loc = 0;  // wrap around if only 1 screen
                else if (menu->first_entry + menu->entries_per_screen < menu->submenu_size)
                {
                  loc = 0;
                  action = PAGEDOWN;
                }
                break;
              }

              /* Change window size (used only to debug) */
              case SDLK_F5:
              case SDLK_F6:
              case SDLK_F7:
              case SDLK_F8:
              {
                /* these keys are available only if in debug mode */
                DEBUGCODE(debug_titlescreen | debug_menu)
                {
                  switch(event.key.keysym.sym)
                  {
                    case SDLK_F5:
                    {
                      /* decrease screen width */
                      ChangeWindowSize(win_res_x - 50, win_res_y);
                      break;
                    }
                    case SDLK_F6:
                    {
                      /* increase screen width */
                      ChangeWindowSize(win_res_x + 50, win_res_y);
                      break;
                    }
                    case SDLK_F7:
                    {
                      /* decrease screen height */
                      ChangeWindowSize(win_res_x, win_res_y - 50);
                      break;
                    }
                    case SDLK_F8:
                    {
                      /* increase screen height */
                      ChangeWindowSize(win_res_x, win_res_y + 50);
                      break;
                    }
                    default:
                      break;
                  }
                  action = RESIZED;
                }
                break;
              }

              /* Toggle screen mode: */
              case SDLK_F10:
              {
                SwitchScreenMode();
                action = RESIZED;
                break;
              }

              /* Toggle menu music: */
              case SDLK_F11:
              {
                if (Opts_GetGlobalOpt(MENU_MUSIC))
                {
                  audioMusicUnload( );
                  Opts_SetGlobalOpt(MENU_MUSIC, 0);
                }
                else
                {
                  Opts_SetGlobalOpt(MENU_MUSIC, 1);
                  audioMusicLoad("tuxi.ogg", -1);
                }
                break;
              }

              default:
              {
                /* Some other key - do nothing. */
              }

              break;  /* To get out of _outer_ switch/case statement */
            }  /* End of key switch statement */
          }  // End of case SDL_KEYDOWN in outer switch statement
        }  // End event switch statement

        if (old_loc != loc) {
          DEBUGMSG(debug_menu, "run_menu(): changed button focus, old=%d, new=%d\n", old_loc, loc);
          if(old_loc >= 0 && old_loc < items)
          {
            tmp_rect = menu->submenu[old_loc + menu->first_entry]->button_rect;
            SDL_BlitSurface(menu_item_unselected[old_loc], NULL, screen, &tmp_rect);
            if(menu->submenu[menu->first_entry + old_loc]->icon)
              SDL_BlitSurface(menu->submenu[menu->first_entry + old_loc]->icon->default_img, NULL, screen, &menu->submenu[menu->first_entry + old_loc]->icon_rect);
            SDL_UpdateRect(screen, tmp_rect.x, tmp_rect.y, tmp_rect.w, tmp_rect.h);
          }
          if(loc >= 0 && loc < items)
          {
            tmp_rect = menu->submenu[loc + menu->first_entry]->button_rect;
            SDL_BlitSurface(menu_item_selected[loc], NULL, screen, &tmp_rect);
            if(menu->submenu[menu->first_entry + loc]->icon)
            {
              SDL_BlitSurface(menu->submenu[menu->first_entry + loc]->icon->default_img, NULL, screen, &menu->submenu[menu->first_entry + loc]->icon_rect);
              menu->submenu[menu->first_entry + loc]->icon->cur = 0;
            }
            SDL_UpdateRect(screen, tmp_rect.x, tmp_rect.y, tmp_rect.w, tmp_rect.h);
          }
          old_loc = loc;
        }

        if(HandleTitleScreenEvents(&event))
          stop = true;

        switch(action)
        {
          case RESIZED:
            RenderTitleScreen();
            menu->first_entry = 0;
            prerender_all();
            stop = true;
            break;

          case CLICK:
            if(loc < 0 || loc >= items)
            {
              DEBUGMSG(debug_menu, "run_menu(): incorrect location for CLICK action (%d) !\n", loc);
            }
            else
            {
              tmp_node = menu->submenu[menu->first_entry + loc];
              if(tmp_node->submenu_size == 0)
              {
                if(return_choice)
                {
                  FreeSurfaceArray(menu_item_unselected, items);
                  FreeSurfaceArray(menu_item_selected, items);
                  return tmp_node->activity;
                }
                else
                {
                  if(tmp_node->activity == RUN_MAIN_MENU)
                  {
                    /* go back to the root of this menu */
                    menu = root;
                  }
                  else
                  {
                    if(handle_activity(tmp_node->activity, tmp_node->param) == QUIT)
                    {
                      DEBUGMSG(debug_menu, "run_menu(): handle_activity() returned QUIT message, exiting.\n");
                      FreeSurfaceArray(menu_item_unselected, items);
                      FreeSurfaceArray(menu_item_selected, items);
                      return QUIT;
                    }
                  }
                }
              }
              else
              {
                menu->first_entry = 0;
                menu = tmp_node;
                menu->first_entry = 0;
              }
              stop = true;
            }
            break;

          case STOP_ESC:
            if(menu->parent == NULL)
            {
              FreeSurfaceArray(menu_item_unselected, items);
              FreeSurfaceArray(menu_item_selected, items);
              return STOP;
            }
            else
              menu = menu->parent;
            stop = true;
            break;

          case PAGEUP:
            menu->first_entry -= menu->entries_per_screen;
            stop = true;
            break;

          case PAGEDOWN:
            menu->first_entry += menu->entries_per_screen;
            stop = true;
            break;
        }

      }  // End of SDL_PollEvent while loop

      if(stop)
        break;

/* FIXME look at this more closely - the sprites have not been displaying */
/* except for the default frame:                                          */

      if(!stop && frame_counter % 5 == 0 && loc >= 0 && loc < items)
      {
        tmp_sprite = menu->submenu[menu->first_entry + loc]->icon;
        if(tmp_sprite)
        {
          DEBUGMSG(debug_menu, "run_menu(): incrementing sprite for entry %d\n", loc);

          SDL_BlitSurface(menu_item_selected[loc], NULL, screen,
                          &menu->submenu[menu->first_entry + loc]->icon_rect);
          SDL_BlitSurface(tmp_sprite->frame[tmp_sprite->cur], NULL, screen,
                          &menu->submenu[menu->first_entry + loc]->icon_rect);
          UpdateRect(screen, &menu->submenu[menu->first_entry + loc]->icon_rect);
          NextFrame(tmp_sprite);
        }
        else
          DEBUGMSG(debug_menu, "run_menu(): entry %d has no valid sprite\n", loc);
      }

      HandleTitleScreenAnimations();

      /* Wait so we keep frame rate constant: */
      frame_now = SDL_GetTicks();
      if (frame_now < frame_start)
        frame_start = frame_now;  // in case the timer wraps around
      if((frame_now - frame_start) < 1000 / MAX_FPS)
        SDL_Delay(1000 / MAX_FPS - (frame_now - frame_start));

      frame_counter++;
    } // End of while(!stop) loop

    /* free button surfaces */
    DEBUGMSG(debug_menu, "run_menu(): freeing %d button surfaces\n", items);
    FreeSurfaceArray(menu_item_unselected, items);
    FreeSurfaceArray(menu_item_selected, items);
  }

  return QUIT;
}

/* return button surfaces that are currently displayed (without sprites) */
SDL_Surface** render_buttons(MenuNode* menu, bool selected)
{
  SDL_Surface** menu_items = NULL;
  SDL_Rect curr_rect;
  SDL_Surface* tmp_surf = NULL;
  int i;
  int items = min(menu->entries_per_screen, menu->submenu_size - menu->first_entry);

  menu_items = (SDL_Surface**) malloc(items * sizeof(SDL_Surface*));
  if(NULL == menu_items)
  {
    DEBUGMSG(debug_menu, "render_buttons(): failed to allocate memory for buttons!\n");
    return NULL;  // error
  }

  for (i = 0; i < items; i++)
  {
    curr_rect = menu->submenu[menu->first_entry + i]->button_rect;
    menu_items[i] = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
                                          curr_rect.w,
                                          curr_rect.h,
                                          32,
                                          rmask, gmask, bmask, amask);

    SDL_BlitSurface(screen, &curr_rect, menu_items[i], NULL);
    /* button */
    if(selected)
      tmp_surf = CreateButton(curr_rect.w, curr_rect.h, button_radius * curr_rect.h, SEL_RGBA);
    else
      tmp_surf = CreateButton(curr_rect.w, curr_rect.h, button_radius * curr_rect.h, REG_RGBA);

    SDL_BlitSurface(tmp_surf, NULL, menu_items[i], NULL);
    SDL_FreeSurface(tmp_surf);

    /* text */
    DEBUGMSG(debug_menu, "render_buttons(): English string is: %s\n",
             menu->submenu[menu->first_entry + i]->title);
    DEBUGMSG(debug_menu, "render_buttons(): NLS string is: %s\n",
             _(menu->submenu[menu->first_entry + i]->title));

    tmp_surf = BlackOutline(_(menu->submenu[menu->first_entry + i]->title),
                            curr_font_size, selected ? &yellow : &white);
    SDL_BlitSurface(tmp_surf, NULL, menu_items[i], &menu->submenu[menu->first_entry + i]->text_rect);
    SDL_FreeSurface(tmp_surf);
  }

  return menu_items;
}

/* recursively load sprites and calculate button rects
   to fit into current screen */
void prerender_menu(MenuNode* menu)
{
  SDL_Surface* temp_surf;
  MenuNode* curr_node;
  int i, imod, max_text_h = 0, max_text_w = 0;
  int button_h, button_w;
  bool found_icons = false;
  char filename[buf_size];

  if(NULL == menu)
  {
    DEBUGMSG(debug_menu, "prerender_menu(): NULL pointer, exiting !\n");
    return;
  }

  if(0 == menu->submenu_size)
  {
    DEBUGMSG(debug_menu, "prerender_menu(): no submenu, exiting.\n");
    return;
  }

  for(i = 0; i < menu->submenu_size; i++)
  {
    if(menu->submenu[i]->icon_name)
      found_icons = true;
    temp_surf = NULL;
    temp_surf = SimpleText(_(menu->submenu[i]->title), curr_font_size, &black);
    if(temp_surf)
    {
      max_text_h = max(max_text_h, temp_surf->h);
      max_text_w = max(max_text_w, temp_surf->w);
      SDL_FreeSurface(temp_surf);
    }
  }

  button_h = (1.0 + 2.0 * text_h_gap) * max_text_h;
  button_w = max_text_w + ( (found_icons ? 1.0 : 0.0) + 2.0 * text_w_gap) * button_h;

  menu->entries_per_screen = (int) ( (menu_rect.h - button_gap * button_h) /
                                   ( (1.0 + button_gap) * button_h ) );

  for(i = 0; i < menu->submenu_size; i++)
  {
    curr_node = menu->submenu[i];
    curr_node->button_rect.x = menu_rect.x;
    imod = i % menu->entries_per_screen;
    curr_node->button_rect.y = menu_rect.y + imod * button_h + (imod + 1) * button_gap * button_h;
    curr_node->button_rect.w = button_w;
    curr_node->button_rect.h = button_h;

    curr_node->icon_rect = curr_node->button_rect;
    curr_node->icon_rect.w = curr_node->icon_rect.h;

    curr_node->text_rect.x = ( (found_icons ? 1.0 : 0.0) + text_w_gap) * curr_node->icon_rect.w;
    curr_node->text_rect.y = text_h_gap * max_text_h;
    curr_node->text_rect.h = max_text_h;
    curr_node->text_rect.w = max_text_w;

    if(curr_node->icon)
      FreeSprite(curr_node->icon);

    if(curr_node->icon_name)
    {
      sprintf(filename, "sprites/%s", curr_node->icon_name);
      DEBUGMSG(debug_menu, "prerender_menu(): loading sprite %s for item #%d.\n", filename, i);
      curr_node->icon = LoadSpriteOfBoundingBox(filename, IMG_ALPHA, button_h, button_h);
    }
    else
      DEBUGMSG(debug_menu, "prerender_menu(): no sprite for item #%d.\n", i);

    prerender_menu(menu->submenu[i]);
  }
}

char* find_longest_text(MenuNode* menu, int* length)
{
  SDL_Surface* text = NULL;
  char *ret = NULL, *temp = NULL;
  int i;

  if(menu->submenu_size == 0)
  {
    text = SimpleText(_(menu->title), curr_font_size, &black);
    if(text->w > *length)
    {
      *length = text->w;
      ret = menu->title;
    }
    SDL_FreeSurface(text);
  }
  else
  {
    for(i = 0; i < menu->submenu_size; i++)
    {
      temp = find_longest_text(menu->submenu[i], length);
      if(temp)
        ret = temp;
    }
  }
  return ret;
}

/* find the longest text in all existing menus and binary search
   for the best font size */
void set_font_size()
{
  char* longest = NULL;
  char* temp;
  SDL_Surface* surf;
  int length = 0, i, min_f, max_f, mid_f;

  curr_font_size = default_font_size;

  for(i = 0; i < N_OF_MENUS; i++)
  {
    if(menus[i])
    {
      temp = find_longest_text(menus[i], &length);
      if(temp)
        longest = temp;
    }
  }

  if(!longest)
    return;

  min_f = min_font_size;
  max_f = max_font_size;

  while(min_f < max_f)
  {
    mid_f = (min_f + max_f) / 2;
    surf = SimpleText(_(longest), mid_f, &black);
    if(surf->w + (1.0 + 2.0 * text_w_gap) * (1.0 + 2.0 * text_h_gap) * surf->h < menu_rect.w)
      min_f = mid_f + 1;
    else
      max_f = mid_f;
    SDL_FreeSurface(surf);
  }

  curr_font_size = min_f;
}

/* prerender arrows, stop button and all non-NULL menus from menus[] array
   this function should be invoked after every resolution change */
void prerender_all()
{
  int i;

  SetRect(&menu_rect, menu_pos);

  SetRect(&stop_rect, stop_pos);
  if(stop_button)
    SDL_FreeSurface(stop_button);
  stop_button = LoadImageOfBoundingBox(stop_path, IMG_ALPHA, stop_rect.w, stop_rect.h);
  /* move button to the right */
  stop_rect.x = screen->w - stop_button->w;

  SetRect(&prev_rect, prev_pos);
  if(prev_arrow)
    SDL_FreeSurface(prev_arrow);
  prev_arrow = LoadImageOfBoundingBox(prev_path, IMG_ALPHA, prev_rect.w, prev_rect.h);
  if(prev_gray)
    SDL_FreeSurface(prev_gray);
  prev_gray = LoadImageOfBoundingBox(prev_gray_path, IMG_ALPHA, prev_rect.w, prev_rect.h);
  /* move button to the right */
  prev_rect.x += prev_rect.w - prev_arrow->w;

  SetRect(&next_rect, next_pos);
  if(next_arrow)
    SDL_FreeSurface(next_arrow);
  next_arrow = LoadImageOfBoundingBox(next_path, IMG_ALPHA, next_rect.w, next_rect.h);
  if(next_gray)
    SDL_FreeSurface(next_gray);
  next_gray = LoadImageOfBoundingBox(next_gray_path, IMG_ALPHA, next_rect.w, next_rect.h);

  set_font_size();

  for(i = 0; i < N_OF_MENUS; i++)
    if(menus[i])
      prerender_menu(menus[i]);
}

/* load menu trees from disk and prerender them */
void LoadMenus(void)
{
  FILE* menu_file = NULL;
  int i;

  for(i = 0; i < N_OF_MENUS; i++)
    menus[i] = NULL;

  /* main menu */
  menu_file = fopen(DATA_PREFIX "/menus/main_menu.xml", "r");
  if(menu_file == NULL)
  {
    DEBUGMSG(debug_menu, "LoadMenus(): Could not load main menu file !\n");
  }
  else
  {
    menus[MENU_MAIN] = load_menu_from_file(menu_file, NULL);
    fclose(menu_file);
  }

  /* difficulty menu */
  menu_file = fopen(DATA_PREFIX "/menus/level_menu.xml", "r");
  if(menu_file == NULL)
  {
    DEBUGMSG(debug_menu, "LoadMenus(): Could not load level menu file !\n");
  }
  else
  {
    menus[MENU_DIFFICULTY] = load_menu_from_file(menu_file, NULL);
    fclose(menu_file);
  }

  prerender_all();
}



/* create login menu tree, run it and set the user home directory
   -1 indicates that the user wants to quit without logging in,
    0 indicates that a choice has been made. */
int RunLoginMenu(void)
{
  int n_login_questions = 0;
  char **user_login_questions = NULL;
  char *title = NULL;
  int n_users = 0;
  char **user_names = NULL;
  int chosen_login = -1;
  int level;
  int i;
  char *trailer_quit = "Quit";
  char *trailer_back = "Back";
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

  menus[MENU_LOGIN] = create_one_level_menu(n_users, user_names, title, trailer_quit);

  while (n_users) {
    // Get the user choice
    set_font_size();
    prerender_menu(menus[MENU_LOGIN]);
    chosen_login = run_menu(menus[MENU_LOGIN], true);
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
        free_menu(menus[MENU_LOGIN]);
        menus[MENU_LOGIN] = NULL;
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
    free_menu(menus[MENU_LOGIN]);
    menus[MENU_LOGIN] = create_one_level_menu(n_users, user_names, title, trailer);
  }

  // The user home directory is set, clean up remaining memory
  for (i = 0; i < n_login_questions; i++)
    free(user_login_questions[i]);
  free(user_login_questions);
  free_menu(menus[MENU_LOGIN]);
  menus[MENU_LOGIN] = NULL;

  // Signal success
  return 0;
}

/* run main menu. If this function ends it means that tuxmath is going to quit */
void RunMainMenu(void)
{
  int i;
  MenuNode* tmp_node;
  DEBUGMSG(debug_menu, "Entering RunMainMenu()\n");

   /* lessons menu */
  DEBUGMSG(debug_menu, "LoadMenus(): Generating lessons submenu. (%d lessons)\n", num_lessons);

  tmp_node = create_empty_node();
  tmp_node->submenu_size = num_lessons;
  tmp_node->submenu = (MenuNode**) malloc(num_lessons * sizeof(MenuNode*));
  for(i = 0; i < num_lessons; i++)
  {
    tmp_node->submenu[i] = create_empty_node();
    tmp_node->submenu[i]->icon_name = strdup(lesson_list_goldstars[i] ? "goldstar" : "no_goldstar");
    tmp_node->submenu[i]->title = (char*) malloc( (strlen(lesson_list_titles[i]) + 1) * sizeof(char) );
    strcpy(tmp_node->submenu[i]->title, lesson_list_titles[i]);
    tmp_node->submenu[i]->activity = i;
  }
  menus[MENU_LESSONS] = tmp_node;
  set_font_size();
  prerender_menu(menus[MENU_LESSONS]);
  //prerender_all();

  run_menu(menus[MENU_MAIN], false);
  DEBUGMSG(debug_menu, "Leaving RunMainMenu()\n");
}

/* free all loaded menu trees */
void UnloadMenus(void)
{
  int i;

  DEBUGMSG(debug_menu, "entering UnloadMenus()\n");

  if(stop_button)
  {
    SDL_FreeSurface(stop_button);
    stop_button = NULL;
  }

  if(prev_arrow)
  {
    SDL_FreeSurface(prev_arrow);
    prev_arrow = NULL;
  }

  if(next_arrow)
  {
    SDL_FreeSurface(next_arrow);
    next_arrow = NULL;
  }

  for(i = 0; i < N_OF_MENUS; i++)
    if(menus[i] != NULL)
    {
      DEBUGMSG(debug_menu, "UnloadMenus(): freeing menu #%d\n", i);
      free_menu(menus[i]);
    }

  DEBUGMSG(debug_menu, "leaving UnloadMenus()\n");
}

