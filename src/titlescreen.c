/***************************************************************************
 -  file: titlescreen.c
 -  description: splash, title and menu screen functionality 
                            ------------------
    begin                : Thur May 4 2000
    copyright            : (C) 2000 by Sam Hart
                         : (C) 2003 by Jesse Andrews
    email                : tuxtype-dev@tux4kids.net

    Modified for use in tuxmath by David Bruce - 2006-2007.
    email                : <dbruce@tampabay.rr.com>
                           <tuxmath-devel@lists.sourceforge.net>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


// titlescreen.h has all of the tuxtype-related stuff:
#include "titlescreen.h"

// tuxmath includes:
#include "tuxmath.h"
#include "options.h"
#include "fileops.h"
#include "game.h"
#include "mathcards.h"
#include "setup.h"     //for cleanup()
#include "credits.h"
#include "highscore.h"
#include "ConvertUTF.h" // for wide char to UTF-8 conversion


/* --- Data Structure for Dirty Blitting --- */
SDL_Rect srcupdate[MAX_UPDATES];
SDL_Rect dstupdate[MAX_UPDATES];
int numupdates = 0; // tracks how many blits to be done

// Type needed for TransWipe():
struct blit {
    SDL_Surface *src;
    SDL_Rect *srcrect;
    SDL_Rect *dstrect;
    unsigned char type;
} blits[MAX_UPDATES];

// Lessons available for play
lesson_entry *lesson_list = NULL;
int num_lessons = 0;

// globals from tuxtype's globals.h defined outside of titlescreen.c (in tuxtype):
//int show_tux4kids;
int debugOn; //FIXME switch to TUXMATH_DEBUG




/* --- media for menus --- */


/* FIXME Instead of six parallel arrays, make struct with six fields and create a single array of the struct? */

/* --- define menu structure --- */
/* (these values are all in the Game_Type enum in titlescreen.h) */
/* They are the "commands" associated with the menu items.   */
const int menu_item[TITLE_MENU_ITEMS + 1][TITLE_MENU_DEPTH + 1] =
{{0,  0,            0,              0,                  0           },
 {0,  LESSONS,      ARCADE_CADET,   INTERFACE_OPTIONS,  NOT_CODED   },
 {0,  ARCADE,       ARCADE_SCOUT,   HELP,               FREETYPE    },
 {0,  OPTIONS,      ARCADE_RANGER,  CREDITS,            PROJECT_INFO},
 {0,  GAME_OPTIONS, ARCADE_ACE,     PROJECT_INFO,       SET_LANGUAGE},
 {0,  QUIT_GAME,    HIGH_SCORES,    MAIN,               MAIN        }};

/* --- menu text --- */
const unsigned char* menu_text[TITLE_MENU_ITEMS + 1][TITLE_MENU_DEPTH + 1] = 
/*    Main Menu                                       'Arcade' Games                    Options                     Game Options            */
{{(const unsigned char*)"",     /* NOTE the casts to 'const unsigned char*' are all to */
  (const unsigned char*)"",     /* prevent compiler warnings.                          */
  (const unsigned char*)"",
  (const unsigned char*)"",
  (const unsigned char*)""},
 {(const unsigned char*)"",
  (const unsigned char*)N_("Math Command Training Academy"),   /* Top entry, first page */ 
  (const unsigned char*)N_("Space Cadet"),                     /* Top entry, second page */ 
  (const unsigned char*)N_("Settings"),
  (const unsigned char*)N_("Speed")},
 {(const unsigned char*)"",
  (const unsigned char*)N_("Play Arcade Game"),                /* Second entry, first page */
  (const unsigned char*)N_("Scout"),
  (const unsigned char*)N_("Help"),
  (const unsigned char*)N_("Sound")},
 {(const unsigned char*)"",
  (const unsigned char*)N_("Play Custom Game"),                /* Third entry, first page */
  (const unsigned char*)N_("Ranger"),
  (const unsigned char*)N_("Credits"),
  (const unsigned char*)N_("Graphics")},
 {(const unsigned char*)"",
  (const unsigned char*)N_("More Options"),                   /* Fourth entry, first page */
  (const unsigned char*)N_("Ace"),
  (const unsigned char*)N_("Project Info"),
  (const unsigned char*)N_("Advanced Options")},
 {(const unsigned char*)"",
  (const unsigned char*)N_("Quit"),                            /* Bottom entry, first page */
  (const unsigned char*)N_("Hall Of Fame"),
  (const unsigned char*)N_("Main Menu"),
  (const unsigned char*)N_("Main Menu")}};


/* These are the filenames of the images used in the animated menu icons: */
/* --- menu icons --- */
const unsigned char* menu_sprite_files[TITLE_MENU_ITEMS + 1][TITLE_MENU_DEPTH + 1] = 
{{"", "",                 "",                  "",        ""        },
 {"", "lesson",           "tux_helmet_yellow", "grade1_", "list"    },
 {"", "comet",            "tux_helmet_green",  "grade2_", "practice"},
 {"", "tux_config",       "tux_helmet_blue",   "grade3_", "keyboard"},
 {"", "tux_config_brown", "tux_helmet_red",    "grade4_", "lang"    },
 {"", "quit",             "main",              "main",    "main"   }};


/* this will contain pointers to all of the menu 'icons' */
sprite* menu_sprites[TITLE_MENU_ITEMS + 1][TITLE_MENU_DEPTH + 1] = {{NULL}};
/* images of regular and selected text of menu items: */
SDL_Surface* reg_text[TITLE_MENU_ITEMS + 1][TITLE_MENU_DEPTH + 1] = {{NULL}};
SDL_Surface* sel_text[TITLE_MENU_ITEMS + 1][TITLE_MENU_DEPTH + 1] = {{NULL}};

/* reg and sel are used to create the translucent button backgrounds. */
sprite* reg = NULL;
sprite* sel = NULL;
sprite* Tux = NULL;

/* keep track of the width of each menu: */
int menu_width[TITLE_MENU_DEPTH + 1];

/* NOTE for 'depth', think pages like a restaurant menu, */
/* not heirarchical depth - choice of term is misleading */
int menu_depth; // how deep we are in the menu
settings localsettings;

SDL_Event event;

/* --- locations we need --- */
SDL_Rect text_dst[TITLE_MENU_ITEMS + 1];     // location of text for menu
SDL_Rect menu_sprite_dest[TITLE_MENU_ITEMS + 1]; // location of animated icon
/* These are the rectangular mouse event "buttons" for each menu item */
SDL_Rect menu_button[TITLE_MENU_ITEMS + 1];  // size of "button"

SDL_Rect dest,
	 Tuxdest,
	 Titledest,
         stopRect,
	 cursor;

/* Local function prototypes: */
void draw_button(SDL_Rect* target_rect, int width, sprite* s);
void TitleScreen_load_menu(void);
void TitleScreen_unload_menu(void);
void TitleScreen_load_media(void);
void TitleScreen_unload_media(void);
void NotImplemented(void);
void HighScoreScreen(void);
void HighScoreNameEntry(unsigned char* name_buf);
void TransWipe(SDL_Surface* newbkg, int type, int var1, int var2);
void UpdateScreen(int* frame);
void AddRect(SDL_Rect* src, SDL_Rect* dst);
void InitEngine(void);
void ShowMessage(char* str1, char* str2, char* str3, char* str4);

/***********************************************************/
/*                                                         */
/*       "Public functions" (callable throughout program)  */
/*                                                         */
/***********************************************************/



/****************************************
* TitleScreen: Display the title screen *
****************************************/

/* display title screen, get input */

void TitleScreen(void)
{

  Uint32 frame = 0;
  Uint32 start = 0;

  int i, j; 
  int tux_frame = 0;
  int done = 0;
  int firstloop = 1;
  int menu_opt = NONE;
  int update_locs = 1;
  int redraw = 1;
  int key_menu = 1;
  int old_key_menu = 5;


  if (Opts_UsingSound())
  {
    Opts_SetMenuSound(1);
    Opts_SetMenuMusic(1);
//    menu_music = localsettings.menu_music;
  }

  InitEngine();  //set up pointers for blitting structure.


  /* --- setup colors we use --- */
  black.r       = 0x00; black.g       = 0x00; black.b       = 0x00;
  gray.r        = 0x80; gray.g        = 0x80; gray.b        = 0x80;
  dark_blue.r   = 0x00; dark_blue.g   = 0x00; dark_blue.b   = 0x60; 
  red.r         = 0xff; red.g         = 0x00; red.b         = 0x00;
  white.r       = 0xff; white.g       = 0xff; white.b       = 0xff;
  yellow.r      = 0xff; yellow.g      = 0xff; yellow.b      = 0x00; 


  start = SDL_GetTicks();


  /* StandbyScreen: Display the Standby screen: */
  if (images[IMG_STANDBY])
  {  
    // Center horizontally
    dest.x = ((screen->w) / 2) - (images[IMG_STANDBY]->w) / 2;
    // Center vertically
    dest.y = ((screen->h) / 2) - (images[IMG_STANDBY]->h) / 2;
    dest.w = images[IMG_STANDBY]->w;
    dest.h = images[IMG_STANDBY]->h;

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_BlitSurface(images[IMG_STANDBY], NULL, screen, &dest);
    SDL_UpdateRect(screen, 0, 0, 0, 0);
  }
 

  /* --- wait  --- */

  while ((SDL_GetTicks() - start) < 2000)
  {
    /* Check to see if user pressed escape */
    if (SDL_PollEvent(&event)
     && event.type==SDL_KEYDOWN
     && event.key.keysym.sym == SDLK_ESCAPE)
    {
      return;
    } 
    SDL_Delay(50);
  }

  SDL_WM_GrabInput(SDL_GRAB_ON); // User input goes to TuxMath, not window manager
  SDL_ShowCursor(1);


  /***************************
  * Tux and Title animations *
  ***************************/

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "->Now Animating Tux and Title onto the screen\n" );
#endif

  /* Load media and menu data: */
  /* FIXME should get out if needed media not loaded OK */
  TitleScreen_load_media();

  /* Draw background, if it loaded OK: */
  if (images[IMG_MENU_BKG])
  {
    /* FIXME not sure TransWipe() works in Windows: */
    TransWipe(images[IMG_MENU_BKG], RANDOM_WIPE, 10, 20);
    /* Make sure background gets drawn (since TransWipe() doesn't */
    /* seem to work reliably as of yet):                          */
    SDL_BlitSurface(images[IMG_MENU_BKG], NULL, screen, NULL);
 
  }
  /* Red "Stop" circle in upper right corner to go back to main menu: */
  if (images[IMG_STOP])
  {
    stopRect.w = images[IMG_STOP]->w;
    stopRect.h = images[IMG_STOP]->h;
    stopRect.x = screen->w - images[IMG_STOP]->w;
    stopRect.y = 0;
    SDL_BlitSurface(images[IMG_STOP], NULL, screen, &stopRect);
  }
  SDL_UpdateRect(screen, 0, 0, 0, 0);

  /* --- Pull tux & logo onscreen --- */
  /* NOTE we wind up with Tuxdest.y == (screen->h)  - (Tux->frame[0]->h), */
  /* and Titledest.x == 0.                                                */
  if (images[IMG_MENU_BKG]
   && images[IMG_MENU_TITLE]
   && images[IMG_STOP]
   && Tux && Tux->frame[0])
  {
    Tuxdest.x = 0;
    Tuxdest.y = screen->h;
    Tuxdest.w = Tux->frame[0]->w;
    Tuxdest.h = Tux->frame[0]->h;

    Titledest.x = screen->w;
    Titledest.y = 10;
    Titledest.w = images[IMG_MENU_TITLE]->w;
    Titledest.h = images[IMG_MENU_TITLE]->h;


    for (i = 0; i < (PRE_ANIM_FRAMES * PRE_FRAME_MULT); i++)
    {
      start = SDL_GetTicks();
      SDL_BlitSurface(images[IMG_MENU_BKG], &Tuxdest, screen, &Tuxdest);
      SDL_BlitSurface(images[IMG_MENU_BKG], &Titledest, screen, &Titledest);

      Tuxdest.y -= Tux->frame[0]->h / (PRE_ANIM_FRAMES * PRE_FRAME_MULT);
      Titledest.x -= (screen->w) / (PRE_ANIM_FRAMES * PRE_FRAME_MULT);

      SDL_BlitSurface(Tux->frame[0], NULL, screen, &Tuxdest);
      SDL_BlitSurface(images[IMG_MENU_TITLE], NULL, screen, &Titledest);
      SDL_BlitSurface(images[IMG_STOP], NULL, screen, &stopRect);
 
      SDL_UpdateRect(screen, Tuxdest.x, Tuxdest.y, Tuxdest.w, Tuxdest.h);
      SDL_UpdateRect(screen, Titledest.x, Titledest.y, Titledest.w + 40,
                     Titledest.h);
      SDL_UpdateRect(screen, stopRect.x, stopRect.y, stopRect.w, stopRect.h);

      while ((SDL_GetTicks() - start) < 33) 
      {
        SDL_Delay(2);
      }
    }


  }

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "Tux and Title are in place now\n");
#endif

  /* Start playing menu music if desired: */
  if (Opts_MenuMusic())
  {
    audioMusicLoad("tuxi.ogg", -1);
  }

  /* Move mouse to top button: */
  cursor.x = menu_button[1].x + (menu_button[1].w / 2);
  cursor.y = menu_button[1].y + (3 * menu_button[1].h / 4);
  SDL_WarpMouse(cursor.x, cursor.y);
  SDL_WM_GrabInput(SDL_GRAB_OFF);


  /****************************
  * Main Loop Starts Here ... *
  ****************************/


  menu_depth = 1;
  firstloop = 1;
  if (Tux && Tux->frame[0])
  {
    Tuxdest.y = screen->h - Tux->frame[0]->h;
  }

  while (!done) 
  {
    start = SDL_GetTicks();

    /* ---process input queue --- */

    menu_opt = NONE; // clear the option so we don't change twice!

    old_key_menu = key_menu;

    /* Retrieve any user interface events: */
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_MOUSEMOTION:
        {
          cursor.x = event.motion.x;
          cursor.y = event.motion.y;
          break;
        }

        /* Handle mouse clicks based on mouse location: */
        case SDL_MOUSEBUTTONDOWN:
        {
          cursor.x = event.motion.x;
          cursor.y = event.motion.y;

          for (j = 1; j <= TITLE_MENU_ITEMS; j++)
          {
            if (inRect(menu_button[j], cursor.x, cursor.y))
            {
              menu_opt = menu_item[j][menu_depth];
              if (Opts_MenuSound())
              {
                tuxtype_playsound(sounds[SND_POP]);
              }

#ifdef TUXMATH_DEBUG 
              fprintf(stderr, "->>BUTTON CLICK menu_opt = %d\n", menu_opt);
              fprintf(stderr, "->J = %d menu_depth=%d\n", j, menu_depth);
#endif
              break; // from for loop (optimization)
            }
          }

  
          /* Stop button is equivalent to Esc key: */
          if (inRect(stopRect, cursor.x, cursor.y))
          {
            /* Go to main menu (if in submenu) or quit: */
            if (menu_depth != 1) 
              menu_opt = MAIN;
            else
              menu_opt = QUIT_GAME;

            if (Opts_MenuSound())
              tuxtype_playsound(sounds[SND_POP]);
          }

          break;
        }


        case SDL_QUIT:
        {
          menu_opt = QUIT_GAME;
          break;
        }


        /* Handle key press events based on key value: */
        case SDL_KEYDOWN:
        {
          switch (event.key.keysym.sym)
          {
            case SDLK_ESCAPE:
            {
              /* Go to main menu (if in submenu) or quit: */
              if (menu_depth != 1) 
                menu_opt = MAIN;
              else
                menu_opt = QUIT_GAME;

              if (Opts_MenuSound())
                tuxtype_playsound(sounds[SND_POP]);
              break;
            }

            /* Toggle screen mode: */
            case SDLK_F10: 
            {
              switch_screen_mode();
              redraw = 1;
              break;
            }

            /* Toggle menu music: */
            case SDLK_F11:
            {
              if (Opts_MenuMusic())
              {
                audioMusicUnload( );
                Opts_SetMenuMusic(0);
              }
              else
              {
                Opts_SetMenuMusic(1);
                audioMusicLoad("tuxi.ogg", -1);
              }
              //redraw = 1;
              break;
            }

            case SDLK_UP:
            {
              if (Opts_MenuSound())
                tuxtype_playsound(sounds[SND_TOCK]);
              key_menu--;
              if (key_menu < 1)
                key_menu = 5;
              break;
            }

            case SDLK_DOWN:
            {
              key_menu++;
              if (Opts_MenuSound())
                tuxtype_playsound(sounds[SND_TOCK]);
              if (key_menu > 5)
                key_menu = 1;
              break;
            }

	    case SDLK_KP_ENTER:
            case SDLK_SPACE:
            case SDLK_RETURN:
            {
              if (key_menu)
              {
                menu_opt = menu_item[key_menu][menu_depth];

#ifdef TUXMATH_DEBUG
                fprintf(stderr, "In TitleScreen() after keypress, key_menu = %d\t"
                                "menu_depth = %d\t, menu_opt = %d\n", 
                                key_menu, menu_depth, menu_opt);
#endif

                if (Opts_MenuSound())
                  tuxtype_playsound(sounds[SND_POP]);
              }
              break;
            }


            default:     /* Some other key pressed - do nothing: */
            {
              break;
            }
          }             /* End of switch(event.key.keysym.sym) statement */
        }               /* End of case: SDL_KEYDOWN: */


        default:        /* Some other type of SDL event - do nothing;    */
        {
          break;
        }
      }                 /* End of switch(event.type) statement           */
    }	              /* End of while (SDL_PollEvent(&event)) loop     */



    /* --- warp mouse to follow keyboard input --- */

    if (old_key_menu != key_menu)
    {
      cursor.x = menu_button[key_menu].x + (menu_button[key_menu].w / 2);
      cursor.y = menu_button[key_menu].y + (3 * menu_button[key_menu].h / 4);
      SDL_WarpMouse(cursor.x, cursor.y);
    }


    /* --- do menu processing --- */
    switch (menu_opt)
    {
      /* First page:-----------------------------------------------*/
    
      case LESSONS: /* Go to 'lessons' menu: */
                             /* i.e. Math Command Training Academy */
      {
        /* choose_config_file() returns after selected lessons are  */
        /* done - game() called from there.                         */
        if (choose_config_file())  
        {
          if (Opts_MenuMusic())  // Restart music after game
            audioMusicLoad( "tuxi.ogg", -1 );
        }
        redraw = 1;
        break;
      }


      case ARCADE:   /* Go to Arcade submenu */
      {
        menu_depth = ARCADE_SUBMENU; /* i.e. 2 */
        update_locs = 1;
        redraw = 1;
        break;
      }


      case OPTIONS:
      {
        char *s1, *s2, *s3, *s4;
        s1 = _("Edit 'options' file in your home directory");
        s2 = _("to create customized game!");
        s3 = _("Press a key or click your mouse to start game.");
        s4 = N_("See README.txt for more information");
        ShowMessage(s1, s2, s3, s4);

        if (read_user_config_file())
        {
          if (Opts_MenuMusic())
          {
            audioMusicUnload();
          }

          game();
          write_user_config_file();

          if (Opts_MenuMusic())
          {
            audioMusicLoad( "tuxi.ogg", -1 );
          }
        }

        redraw = 1;
        break;
      }


      case GAME_OPTIONS: /* Go to page three of menu system */
      {
        menu_depth = GAME_OPTIONS_SUBMENU;  /* i.e. 3 */
        update_locs = 1;
        redraw=1;
        break;
      }


      case QUIT_GAME:
      {
        done = 1;
        break;
      }

      /* Second (Arcade) page:-----------------------------------------------*/

      /* Play game of selected difficulty:                                   */
      /* TODO save high scores for each difficulty level.                    */
      /* TODO display brief description of type of questions at each level.  */
      case ARCADE_CADET:
      {
#ifdef TUXMATH_DEBUG
        fprintf(stderr, "menu_opt == ARCADE_CADET");
#endif
        if (read_named_config_file("arcade/space_cadet"))
        {
          audioMusicUnload();
          game();
          /* See if player made high score list!                        */
          if (check_score_place(ACE_HIGH_SCORE, Opts_LastScore()) < HIGH_SCORES_SAVED)
          {
            /* (Get name string from player) */
            insert_score("Cadet (temporary)", CADET_HIGH_SCORE, Opts_LastScore());
            write_high_scores();
#ifdef TUXMATH_DEBUG
            print_high_scores(stderr);
#endif 
          }
        }
        else
        {
          fprintf(stderr, "\nCould not find arcade space_cadet config file\n");
        }

        if (Opts_MenuMusic())
        {
          audioMusicLoad( "tuxi.ogg", -1 );
        }
        redraw = 1;
        break;
      }


      case ARCADE_SCOUT:
      {
#ifdef TUXMATH_DEBUG
        fprintf(stderr, "menu_opt == ARCADE_SCOUT");
#endif
        if (read_named_config_file("arcade/scout"))
        {
          audioMusicUnload();
          game();
          /* See if player made high score list!                        */
          if (check_score_place(ACE_HIGH_SCORE, Opts_LastScore()) < HIGH_SCORES_SAVED)
          {
            /* (Get name string from player) */
            insert_score("Scout (temporary)", SCOUT_HIGH_SCORE, Opts_LastScore());
            write_high_scores();
#ifdef TUXMATH_DEBUG
            print_high_scores(stderr);
#endif 
          }

        }
        else
        {
          fprintf(stderr, "\nCould not find arcade scout config file\n");
        }

        if (Opts_MenuMusic())
        {
          audioMusicLoad( "tuxi.ogg", -1 );
        }
        redraw = 1;
        break;
      }


      case ARCADE_RANGER:
      {
#ifdef TUXMATH_DEBUG
        fprintf(stderr, "menu_opt == ARCADE_RANGER");
#endif
        if (read_named_config_file("arcade/ranger"))
        {
          audioMusicUnload();
          game();
          /* See if player made high score list!                        */
          if (check_score_place(ACE_HIGH_SCORE, Opts_LastScore()) < HIGH_SCORES_SAVED)
          {
            /* (Get name string from player) */
            insert_score("Ranger (temporary)", RANGER_HIGH_SCORE, Opts_LastScore());
            write_high_scores();
#ifdef TUXMATH_DEBUG
            print_high_scores(stderr);
#endif 
          }
        }
        else
        {
          fprintf(stderr, "\nCould not find arcade ranger config file\n");
        }

        if (Opts_MenuMusic())
        {
          audioMusicLoad( "tuxi.ogg", -1 );
        }
        redraw = 1;
        break;
      }



      case ARCADE_ACE:
      {
#ifdef TUXMATH_DEBUG
        fprintf(stderr, "menu_opt == ARCADE_ACE");
#endif
        if (read_named_config_file("arcade/ace"))
        {
          audioMusicUnload();
          game();
          /* The 'Ace' mission sets this to 0.1 - put back to 1 in case */
          /* next mission file forgets to specify it:                   */
          MC_SetFractionToKeep(1.0);
          /* See if player made high score list!                        */
          if (check_score_place(ACE_HIGH_SCORE, Opts_LastScore()) < HIGH_SCORES_SAVED)
          {
            /* (Get name string from player) */
            insert_score("Ace (temporary)", ACE_HIGH_SCORE, Opts_LastScore());
            write_high_scores();
#ifdef TUXMATH_DEBUG
            print_high_scores(stderr);
#endif 
          }
        }
        else
        {
          fprintf(stderr, "\nCould not find arcade ace config file\n");
        }

        if (Opts_MenuMusic())
        {
          audioMusicLoad( "tuxi.ogg", -1 );
        }
        redraw = 1;
        break; 
      }

      /* Go back to main menu: */
      case HIGH_SCORES:
      {
        HighScoreScreen();
        redraw = 1;
        break;
      }


    /* Third (Game Options) page:----------*/

      case INTERFACE_OPTIONS:
      {
        NotImplemented();
        redraw = 1;
        break;
      }


      case HELP:
      {
        NotImplemented();
        redraw = 1;
        break;
      }


      case CREDITS:
      {
        TitleScreen_unload_media();
        credits();
        TitleScreen_load_media();
        redraw = 1;
        break;
      }


      case PROJECT_INFO:
      {
        NotImplemented();
//      projectInfo();
        redraw = 1;
        break;
      }

      /* Go back to main menu: */
      case MAIN:
      {
        menu_depth = ROOTMENU;
        update_locs = 1;
        redraw = 1;
        break;
      }

      default:
      {
        /* Do nothing */
      }
    } /* End of menu_opts switch statement. */


    /* Rest of menu_opts are not currently used: */
/*
    if (menu_opt == SET_LANGUAGE)
    {
      TitleScreen_unload_media();
      chooseTheme();
      LoadLang();
      LoadKeyboard();
      TitleScreen_load_media();
      redraw = 1;

      if (Opts_MenuMusic())
      {
        audioMusicLoad( "tuxi.ogg", -1 );
      }
    }
*/

    /* ------ End menu_opt processing ----------- */



    if (redraw)
    {
      if (images[IMG_MENU_BKG])
        SDL_BlitSurface(images[IMG_MENU_BKG], NULL, screen, NULL); 
      if (images[IMG_MENU_TITLE])
        SDL_BlitSurface(images[IMG_MENU_TITLE], NULL, screen, &Titledest);
      if (images[IMG_STOP])
        SDL_BlitSurface(images[IMG_STOP], NULL, screen, &stopRect);

      SDL_UpdateRect(screen, 0, 0, 0, 0);
      frame = redraw = 0;   // so we redraw tux
      update_locs = 1;      // so we redraw menu
      firstloop = 1;
    }


    /* --- create new menu screen when needed --- */
    if (update_locs)
    {
      /* --- erase the last menu --- */
      for (i = 1; i <= TITLE_MENU_ITEMS; i++)
      {
        text_dst[i].x = 290;
        text_dst[i].w = reg_text[i][menu_depth]->w;
        text_dst[i].h = reg_text[i][menu_depth]->h;
        SDL_BlitSurface(images[IMG_MENU_BKG], &menu_button[i], screen, &menu_button[i]);
        menu_button[i].w = menu_width[menu_depth] + (2 * reg->frame[2]->w);
      }

      /* --- draw the full menu --- */

      for (j = 1; j <= TITLE_MENU_ITEMS; j++)
      {
        draw_button(&menu_button[j], menu_width[menu_depth], reg);
        SDL_BlitSurface(reg_text[j][menu_depth], NULL, screen, &text_dst[j]);
        SDL_BlitSurface(menu_sprites[j][menu_depth]->default_img, NULL, screen, &menu_sprite_dest[j]);
      }

      SDL_UpdateRect(screen, 0, 0, 0, 0); 
      update_locs = 0;
    }



    /* --- make tux blink --- */

    switch (frame % TUX6)
    {
      case 0:    tux_frame = 1; break;
      case TUX1: tux_frame = 2; break;
      case TUX2: tux_frame = 3; break;
      case TUX3: tux_frame = 4; break;			
      case TUX4: tux_frame = 3; break;
      case TUX5: tux_frame = 2; break;
      default: tux_frame = 0;
    }

    if (tux_frame)
    {
      SDL_BlitSurface(images[IMG_MENU_BKG], &Tuxdest, screen, &Tuxdest);
      SDL_BlitSurface(Tux->frame[tux_frame - 1], NULL, screen, &Tuxdest);
    }


    /* --- check if mouse is in a menu option --- */

    key_menu = 0;

    for (j = 1; j <= TITLE_MENU_ITEMS; j++)
    {
      if ((cursor.x >= menu_button[j].x && cursor.x <= (menu_button[j].x + menu_button[j].w)) &&
          (cursor.y >= menu_button[j].y && cursor.y <= (menu_button[j].y + menu_button[j].h)))
      {
        key_menu = j; // update menu to point
        break;        // Don't need to check rest of menu
      }
    }


    /* --- return old selection to unselected state --- */

    if (old_key_menu && (key_menu != old_key_menu))
    {
      SDL_BlitSurface(images[IMG_MENU_BKG], &menu_button[old_key_menu], screen, &menu_button[old_key_menu]);
      draw_button(&menu_button[old_key_menu], menu_width[menu_depth], reg );
      SDL_BlitSurface(reg_text[old_key_menu][menu_depth], NULL, screen, &text_dst[old_key_menu]);
      SDL_BlitSurface(menu_sprites[old_key_menu][menu_depth]->default_img, NULL, screen, &menu_sprite_dest[old_key_menu]);
    }


    /* --- draw current selection --- */

    if ((key_menu != 0) &&
       ((old_key_menu != key_menu) || (frame % 5 == 0))) // Redraw every fifth frame?
    {
      if (key_menu != old_key_menu)
      {
        rewind(menu_sprites[key_menu][menu_depth]);
        tuxtype_playsound(sounds[SND_TOCK]);
      }

      SDL_BlitSurface(images[IMG_MENU_BKG], &menu_button[key_menu], screen, &menu_button[key_menu]);
      draw_button(&menu_button[key_menu], menu_width[menu_depth], sel );
      SDL_BlitSurface(sel_text[key_menu][menu_depth], NULL, screen, &text_dst[key_menu]);
      SDL_BlitSurface(menu_sprites[key_menu][menu_depth]->frame[menu_sprites[key_menu][menu_depth]->cur], NULL, screen, &menu_sprite_dest[key_menu]);

      next_frame(menu_sprites[key_menu][menu_depth]);
    }


    // HACK This is still more than we need to update every frame but
    // it cuts cpu on my machine %60 so it seems better...

    for ( i=1; i<6; i++ )
    {
      SDL_UpdateRect(screen, menu_button[i].x, menu_button[i].y, menu_button[i].w, menu_button[i].h);
    }

    if (tux_frame)
    {
//      SDL_UpdateRect(screen, Tuxdest.x+37, Tuxdest.y+40, 70, 45);
      SDL_UpdateRect(screen, 0, 0, 0, 0);

    }

    if (firstloop)
    {
      SDL_UpdateRect(screen, Tuxdest.x, Tuxdest.y, Tuxdest.w, Tuxdest.h);
    }

    firstloop = 0;

    /* Wait so we keep frame rate constant: */
    while ((SDL_GetTicks() - start) < 33)
    {
      SDL_Delay(20);
    }

    frame++;
  } /* ----------- End of 'while(!done)' loop ------------  */


#ifdef TUXMATH_DEBUG
  fprintf(stderr, "->>Freeing title screen images\n");
#endif

  localsettings.menu_music = Opts_MenuMusic();
  TitleScreen_unload_media();

#ifdef TUXMATH_DEBUG
  fprintf(stderr,"->TitleScreen():END \n");
#endif

}


void switch_screen_mode(void)
{
  SDL_Surface *tmp;
  SDL_Rect src, dst;

  int window = 0;

  src.x = 0;
  src.y = 0;
  src.w = RES_X;
  src.h = RES_Y;
  dst.x = 0;
  dst.y = 0;

  tmp = SDL_CreateRGBSurface(
      SDL_SWSURFACE,
      RES_X,
      RES_Y,
      BPP,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
      0xff000000,
      0x00ff0000,
      0x0000ff00,
      0x000000ff
#else
      0x000000ff,
      0x0000ff00,
      0x00ff0000,
      0xff000000
#endif
      );

  if (screen->flags & SDL_FULLSCREEN)
  {
    window = 1;
  }

  SDL_BlitSurface(screen,&src,tmp,&dst);
  SDL_UpdateRect(tmp,0,0,RES_X,RES_Y);
  SDL_FreeSurface(screen);
  screen = NULL;

  if (window)
  {
    screen = SDL_SetVideoMode(RES_X,RES_Y,BPP, SDL_SWSURFACE|SDL_HWPALETTE);
  }
  else
  {
    screen = SDL_SetVideoMode(RES_X,RES_Y,BPP, SDL_SWSURFACE|SDL_HWPALETTE|SDL_FULLSCREEN);
  }

  SDL_BlitSurface(tmp,&src,screen,&dst);
  SDL_UpdateRect(tmp,0,0,RES_X,RES_Y);
  SDL_FreeSurface(tmp);
}



/***********************************************************/
/*                                                         */
/*    "Private functions" (callable only from this file)   */
/*                                                         */
/***********************************************************/


void TitleScreen_load_media( void )
{
  /* --- load sounds --- */

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "Entering TitleScreen_load_media():\n");
#endif

/*  if (Opts_MenuSound())
  {
    snd_move = LoadSound("tock.wav");
    snd_select = LoadSound("pop.wav");
  }*/
 
  /* --- load graphics --- */

//  titlepic = LoadImage("title/title1.png", IMG_ALPHA);
//  bkg = LoadImage( "title/main_bkg.jpg", IMG_REGULAR );
  sel = LoadSprite("sprites/sel", IMG_ALPHA);
  reg = LoadSprite("sprites/reg", IMG_ALPHA);
  Tux = LoadSprite("tux/bigtux", IMG_ALPHA);
  //font = LoadFont(menu_font, menu_font_size);

  /* Should we call this directly from TitleScreen()? */
  TitleScreen_load_menu();
}


void TitleScreen_load_menu(void)
{
  char fn[PATH_MAX];
  int max, i, j;

  SDL_ShowCursor(1);

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "loading & parsing menu\n");
#endif

  for (j = 1; j <= TITLE_MENU_DEPTH; j++)  /* Each 'depth' is a different menu */
  {
    max = 0;  // max will be width of widest text entry of this menu

    for (i = 1; i <= TITLE_MENU_ITEMS; i++)
    {
      /* --- create text surfaces --- */
      reg_text[i][j] = black_outline( _((unsigned char*)menu_text[i][j]), default_font, &white);
      sel_text[i][j] = black_outline( _((unsigned char*)menu_text[i][j]), default_font, &yellow);

      /* Make sure we don't try to dereference NULL ptr: */
      if (sel_text[i][j] && sel_text[i][j]->w > max)
      {
        max = sel_text[i][j]->w;
      }

      /* --- load animated icon for menu item --- */
      sprintf(fn, "sprites/%s", menu_sprite_files[i][j]);
      menu_sprites[i][j] = LoadSprite(fn, IMG_ALPHA);
    }
    menu_width[j] = max + 20 + 40; // 40 is width of sprite, 20 is gap
  }

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "done creating graphics, now setting positions\n");
#endif

  /* --- setup menu item destinations --- */

  menu_button[1].x = 240;    // center of top button hardcoded to (240, 100)
  menu_button[1].y = 100;
  menu_button[1].w = menu_width[1];  //calc from width of widest menu item

  /* Must be sure sel has been loaded before checking height: */
  if (sel && sel->frame[1])
    menu_button[1].h = sel->frame[1]->h; //height of button shading

  menu_sprite_dest[1].x = menu_button[1].x + 6; // inset graphic by (6, 4) */
  menu_sprite_dest[1].y = menu_button[1].y + 4;
  menu_sprite_dest[1].w = 40;      // maybe should be MENU_SPRITE_WIDTH?
  menu_sprite_dest[1].h = 50;      //   "      "    " MENU_SPRITE_HEIGHT?

  text_dst[1].y = menu_button[1].y+15;

  /* FIXME each menu item drawn hardcoded 60 pixels below last - */
  /* perhaps increment should be "menu_button[j-1].h + MENU_ITEM_GAP" */
  for (j = 2; j < 6; j++) 
  {
    /* --- setup vertical location of button text --- */
    text_dst[j].y = text_dst[j - 1].y + 60;

    /* --- setup location of button background --- */
    menu_button[j].x = menu_button[j - 1].x;
    menu_button[j].y = menu_button[j - 1].y + 60;
    menu_button[j].w = menu_button[j - 1].w;
    menu_button[j].h = menu_button[j - 1].h;

    /* --- setup location of animated icon --- */
    menu_sprite_dest[j].x = menu_sprite_dest[j - 1].x;
    menu_sprite_dest[j].y = menu_sprite_dest[j - 1].y + 60;
    menu_sprite_dest[j].w = menu_sprite_dest[j - 1].w;
    menu_sprite_dest[j].h = menu_sprite_dest[j - 1].h;
  }
}

/* draw_button() creates and draws a translucent button with */
/* rounded ends.  The location and size are taken from the */
/* SDL_Rect* and width arguments.  The sprite is used to   */
/* fill in the rect with the desired translucent color and */
/* give it nice, rounded ends.                             */
/* FIXME make it match target_rect more precisely          */
void draw_button(SDL_Rect* target_rect, int width, sprite* s )
{
  SDL_Rect button;

  /* Segfault prevention: */
  if (! target_rect 
   || !s
   || !s->frame[0]
   || !s->frame[1])
  {
    return;
  }

  button.x = target_rect->x;
  button.y = target_rect->y;
  button.w = s->frame[0]->w;
  button.h = s->frame[0]->h;

  SDL_BlitSurface(s->frame[0], NULL, screen, &button);
  button.w = s->frame[1]->w;

  for (button.x += s->frame[0]->w;
       button.x < (target_rect->x + width);
       button.x += s->frame[1]->w)

  { 
     SDL_BlitSurface(s->frame[1], NULL, screen, &button);
  }

  button.w = s->frame[2]->w;
  SDL_BlitSurface(s->frame[2], NULL, screen, &button);
}




void TitleScreen_unload_menu(void)
{
  int i,j;

  for (i = 1; i <= TITLE_MENU_ITEMS; i++)
  {
    for (j = 1; j <= TITLE_MENU_DEPTH; j++)
    {
      SDL_FreeSurface(reg_text[i][j]);
      SDL_FreeSurface(sel_text[i][j]);
      FreeSprite(menu_sprites[i][j]);
    }
  }
}







void TitleScreen_unload_media( void ) {

	/* --- unload sounds --- */

/*	if (Opts_MenuSound()){
	    Mix_FreeChunk(snd_move);
	    Mix_FreeChunk(snd_select);
	}*/

	/* --- unload graphics --- */

//	SDL_FreeSurface(titlepic);
//	SDL_FreeSurface(bkg);

	FreeSprite(sel);
	FreeSprite(reg);

	FreeSprite(Tux);

	//TTF_CloseFont(font);
	TitleScreen_unload_menu();
}

void NotImplemented(void)
{
  char *s1, *s2, *s3, *s4;

  s1 = _("Work In Progress!");
  s2 = _("This feature is not ready yet");
  s3 = _("Discuss the future of TuxMath at");
  s4 = N_("tuxmath-devel@lists.sourceforge.net");

  ShowMessage(s1, s2, s3, s4);
}



/* Display high scores: */
void HighScoreScreen(void)
{
  int i = 0;
  int finished = 0;
  int tux_frame = 0;
  Uint32 frame = 0;
  Uint32 start = 0;

  int diff_level = CADET_HIGH_SCORE;
  int old_diff_level = SCOUT_HIGH_SCORE; //So table gets refreshed first time through
  char* diff_level_name = _("Space Cadet");
  /* Surfaces, char buffers, and rects for table: */
  SDL_Surface* score_entries[HIGH_SCORES_SAVED] = {NULL};
  /* 20 spaces should be enough room for place and score on each line: */
  char score_texts[HIGH_SCORES_SAVED][HIGH_SCORE_NAME_LENGTH + 20] = {{'\0'}};
  unsigned char player_name[HIGH_SCORE_NAME_LENGTH * 3] = {'\0'};

  SDL_Rect score_rects[HIGH_SCORES_SAVED];
  SDL_Rect leftRect, rightRect;

  int score_table_x = 240;
  int score_table_y = 100;


  /* Draw background & title: */
  if (images[IMG_MENU_BKG])
    SDL_BlitSurface( images[IMG_MENU_BKG], NULL, screen, NULL );
  if (images[IMG_MENU_TITLE])
    SDL_BlitSurface(images[IMG_MENU_TITLE], NULL, screen, &Titledest);

  /* Put arrow buttons in right lower corner, inset by 20 pixels */
  /* with a 10 pixel space between:                              */
  if (images[IMG_RIGHT])
  {
    rightRect.w = images[IMG_RIGHT]->w;
    rightRect.h = images[IMG_RIGHT]->h;
    rightRect.x = screen->w - images[IMG_RIGHT]->w - 20;
    rightRect.y = screen->h - images[IMG_RIGHT]->h - 20;
    SDL_BlitSurface(images[IMG_RIGHT], NULL, screen, &rightRect);
  }

  if (images[IMG_LEFT])
  {
    leftRect.w = images[IMG_LEFT]->w;
    leftRect.h = images[IMG_LEFT]->h;
    leftRect.x = rightRect.x - 10 - images[IMG_LEFT]->w;
    leftRect.y = screen->h - images[IMG_LEFT]->h - 20;
    SDL_BlitSurface(images[IMG_LEFT_GRAY], NULL, screen, &leftRect);
  }

  /* Red "Stop" circle in upper right corner to go back to main menu: */
  if (images[IMG_STOP])
  {
    stopRect.w = images[IMG_STOP]->w;
    stopRect.h = images[IMG_STOP]->h;
    stopRect.x = screen->w - images[IMG_STOP]->w;
    stopRect.y = 0;
    SDL_BlitSurface(images[IMG_STOP], NULL, screen, &stopRect);
  }

  if (Tux && Tux->num_frames) /* make sure sprite has at least one frame */
  {
    SDL_BlitSurface(Tux->frame[0], NULL, screen, &Tuxdest);
  }
  SDL_UpdateRect(screen, 0, 0, 0, 0);

  while (!finished)
  {
    start = SDL_GetTicks();

    /* Check for user events: */
    while (SDL_PollEvent(&event)) 
    {
      switch (event.type)
      {
        case SDL_QUIT:
        {
          cleanup();
        }

        case SDL_MOUSEBUTTONDOWN:
        /* "Stop" button - go to main menu: */
        { 
          if (inRect(stopRect, event.button.x, event.button.y ))
          {
            finished = 1;
            tuxtype_playsound(sounds[SND_TOCK]);
          }

          /* "Left" button - go to previous page: */
          if (inRect(leftRect, event.button.x, event.button.y))
          {
            if (diff_level > CADET_HIGH_SCORE)
            {
              diff_level--;
              if (Opts_MenuSound())
              {
                tuxtype_playsound(sounds[SND_TOCK]);
              }
            }
          }

          /* "Right" button - go to next page: */
          if (inRect( rightRect, event.button.x, event.button.y ))
          {
            if (diff_level < ACE_HIGH_SCORE)
            {
              diff_level++;
              if (Opts_MenuSound())
              {
                tuxtype_playsound(sounds[SND_TOCK]);
              }
            }
          }
          break;
        }


        case SDL_KEYDOWN:
        {
          finished = 1;
          tuxtype_playsound(sounds[SND_TOCK]);
        }
      }
    }


    /* If needed, redraw: */
    if (diff_level != old_diff_level)
    {
      /* Draw background & title: */
      if (images[IMG_MENU_BKG])
        SDL_BlitSurface( images[IMG_MENU_BKG], NULL, screen, NULL );
      if (images[IMG_MENU_TITLE])
        SDL_BlitSurface(images[IMG_MENU_TITLE], NULL, screen, &Titledest);
      /* Draw Tux: */
      if (Tux && Tux->num_frames) /* make sure sprite has at least one frame */
        SDL_BlitSurface(Tux->frame[0], NULL, screen, &Tuxdest);
      /* Draw controls: */
      if (images[IMG_STOP])
        SDL_BlitSurface(images[IMG_STOP], NULL, screen, &stopRect);
      /* Draw regular or grayed-out left arrow: */
      if (diff_level == CADET_HIGH_SCORE)
      {
        if (images[IMG_LEFT_GRAY])
          SDL_BlitSurface(images[IMG_LEFT_GRAY], NULL, screen, &leftRect);
      }
      else
      {
        if (images[IMG_LEFT])
          SDL_BlitSurface(images[IMG_LEFT], NULL, screen, &leftRect);
      }
      /* Draw regular or grayed-out right arrow: */
      if (diff_level == ACE_HIGH_SCORE)
      {
        if (images[IMG_RIGHT_GRAY])
          SDL_BlitSurface(images[IMG_RIGHT_GRAY], NULL, screen, &rightRect);
      }
      else
      {
        if (images[IMG_RIGHT])
          SDL_BlitSurface(images[IMG_RIGHT], NULL, screen, &rightRect);
      }

      /* Generate and draw desired table: */
      for (i = 0; i < HIGH_SCORES_SAVED; i++)
      {
        /* Get data for entries: */
        sprintf(score_texts[i],
                "%d.\t%d\t%s",
                i + 1,                  /* Add one to get common-language place number */
                HS_Score(diff_level, i),
                HS_Name(diff_level, i));

        /* Clear out old surfaces and update: */
        if (score_entries[i])
          SDL_FreeSurface(score_entries[i]);

        score_entries[i] = black_outline(N_(score_texts[i]), default_font, &white);

        /* Get out if black_outline() fails: */
        if (!score_entries[i])
          continue;
         
        /* Set up entries in vertical column: */
        if (0 == i)
          score_rects[i].y = score_table_y;
        else
          score_rects[i].y = score_rects[i -1].y + score_rects[i -1].h;

        score_rects[i].x = score_table_x;
        score_rects[i].h = score_entries[i]->h;
        score_rects[i].w = score_entries[i]->w;

        SDL_BlitSurface(score_entries[i], NULL, screen, &score_rects[i]);
      }
      /* Update screen: */
      SDL_UpdateRect(screen, 0, 0, 0, 0);

      old_diff_level = diff_level;
    }


    /* --- make tux blink --- */
    switch (frame % TUX6)
    {
      case 0:    tux_frame = 1; break;
      case TUX1: tux_frame = 2; break;
      case TUX2: tux_frame = 3; break;
      case TUX3: tux_frame = 4; break;			
      case TUX4: tux_frame = 3; break;
      case TUX5: tux_frame = 2; break;
      default: tux_frame = 0;
    }

    if (Tux && tux_frame)
    {
      SDL_BlitSurface(Tux->frame[tux_frame - 1], NULL, screen, &Tuxdest);
      SDL_UpdateRect(screen, Tuxdest.x+37, Tuxdest.y+40, 70, 45);
    }


    /* Wait so we keep frame rate constant: */
    while ((SDL_GetTicks() - start) < 33)
    {
      SDL_Delay(20);
    }
    frame++;
  }  // End of while (!finished) loop
  HighScoreNameEntry(player_name);
}

/* Display screen to allow player to enter name for high score table:     */
/* The name_buf argument *must* point to a validly allocated string array */
/* at least three times HIGH_SCORE_NAME_LENGTH because UTF-8 is a         */
/* multibyte encoding.                                                    */
void HighScoreNameEntry(unsigned char* name_buf)
{
  SDL_Surface *s1, *s2, *s3, *s4;
  SDL_Rect loc;
  SDL_Rect redraw_rect;
  int redraw = 0;
  int first_draw = 1;
  int finished = 0;
  int tux_frame = 0;
  Uint32 frame = 0;
  Uint32 start = 0;
  char* str1, *str2, *str3, *str4;
  wchar_t buf[HIGH_SCORE_NAME_LENGTH + 1] = {'\0'};
  unsigned char player_name[HIGH_SCORE_NAME_LENGTH * 3] = {'\0'};

  if (!name_buf)
    return;
  
  s1 = s2 = s3 = s4 = NULL;
  str1 = str2  = str3 = str4 = NULL;

  /* We need to get Unicode vals from SDL keysyms */
  SDL_EnableUNICODE(SDL_ENABLE);

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "\nEnter HighScoreNameEntry()\n" );
#endif

  str1 = _("Great Score - You Are In The Hall of Fame!");
  str2 = _("Enter Your Name:");

  if (str1)
    s1 = black_outline(str1, default_font, &white);
  if (str2)
    s2 = black_outline(str2, default_font, &white);
  if (str3)
    s3 = black_outline(str3, default_font, &white);
  /* When we get going with i18n may need to modify following - see below: */
  if (str4)
    s4 = black_outline(str4, default_font, &white);


  /* Redraw background: */
  if (images[IMG_MENU_BKG])
    SDL_BlitSurface( images[IMG_MENU_BKG], NULL, screen, NULL );

  /* Red "Stop" circle in upper right corner to go back to main menu: */
  if (images[IMG_STOP])
  {
    stopRect.w = images[IMG_STOP]->w;
    stopRect.h = images[IMG_STOP]->h;
    stopRect.x = screen->w - images[IMG_STOP]->w;
    stopRect.y = 0;
    SDL_BlitSurface(images[IMG_STOP], NULL, screen, &stopRect);
  }

  if (Tux && Tux->num_frames) /* make sure sprite has at least one frame */
  {
    SDL_BlitSurface(Tux->frame[0], NULL, screen, &Tuxdest);
  }

  /* Draw lines of text (do after drawing Tux so text is in front): */
  if (s1)
  {
    loc.x = 320 - (s1->w/2); loc.y = 10;
    SDL_BlitSurface( s1, NULL, screen, &loc);
  }
  if (s2)
  {
    loc.x = 320 - (s2->w/2); loc.y = 60;
    SDL_BlitSurface( s2, NULL, screen, &loc);
  }
  if (s3)
  {
    loc.x = 320 - (s3->w/2); loc.y = 300;
    SDL_BlitSurface( s3, NULL, screen, &loc);
  }
  if (s4)
  {
    loc.x = 320 - (s4->w/2); loc.y = 340;
    SDL_BlitSurface( s4, NULL, screen, &loc);
  }

  /* and update: */
  SDL_UpdateRect(screen, 0, 0, 0, 0);



  while (!finished)
  {
    start = SDL_GetTicks();

    while (SDL_PollEvent(&event)) 
    {
      switch (event.type)
      {
        case SDL_QUIT:
        {
          cleanup();
        }

        case SDL_MOUSEBUTTONDOWN:
        /* "Stop" button - go to main menu: */
        { 
          if (inRect(stopRect, event.button.x, event.button.y ))
          {
            finished = 1;
            tuxtype_playsound(sounds[SND_TOCK]);
            break;
          }
        }
        case SDL_KEYDOWN:
        {
#ifdef TUXMATH_DEBUG
          fprintf(stderr, "Before keypress, string is %S\tlength = %d\n",
                  buf, (int)wcslen(buf));
#endif
          switch (event.key.keysym.sym)
          {
            case SDLK_ESCAPE:
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
            {
              finished = 1;
              tuxtype_playsound(sounds[SND_TOCK]);
              break;
            }
            case SDLK_BACKSPACE:
            {
              if (wcslen(buf) > 0)
                buf[(int)wcslen(buf) - 1] = '\0';
              redraw = 1;
              break;
            }


            /* For any other keys, if the key has a Unicode value, */
            /* we add it to our string:                            */
            default:
            {
              if ((event.key.keysym.unicode > 0)
              && (wcslen(buf) < HIGH_SCORE_NAME_LENGTH)) 
              {
                buf[(int)wcslen(buf)] = event.key.keysym.unicode;
                redraw = 1;
              } 
            }
          }  /* end  'switch (event.key.keysym.sym)'  */

#ifdef TUXMATH_DEBUG
          fprintf(stderr, "After keypress, string is %S\tlength = %d\n",
                    buf, (int)wcslen(buf));
#endif
            /* Now draw name, if needed: */
          if (redraw)
          {
            redraw = 0;
            /* Redraw background in area where we drew text last time: */ 
            if (!first_draw)
            {
              SDL_BlitSurface(images[IMG_MENU_BKG], &redraw_rect, screen, &redraw_rect);
              SDL_UpdateRect(screen,
                             redraw_rect.x,
                             redraw_rect.y,
                             redraw_rect.w,
                             redraw_rect.h);
            }

            /* Convert text to UTF-8: */
            //Unicode_to_UTF8((const wchar_t*)buf, player_name);
            wcstombs((char*) player_name, buf, HIGH_SCORE_NAME_LENGTH * 3);

            s3 = black_outline(player_name, default_font, &yellow);
            if (s3)
            {
              loc.x = 320 - (s3->w/2);
              loc.y = 300;
              SDL_BlitSurface( s3, NULL, screen, &loc);

              /* for some reason we need to update a little beyond s3 to get clean image */
              redraw_rect.x = loc.x - 20;
              redraw_rect.y = loc.y - 10;
              redraw_rect.h = s3->h + 20;
              redraw_rect.w = s3->w + 40;
              first_draw = 0;

              SDL_UpdateRect(screen,
                             redraw_rect.x,
                             redraw_rect.y,
                             redraw_rect.w,
                             redraw_rect.h);
              SDL_FreeSurface(s3);
              s3 = NULL;
            }

          }
        }
      }
    }
 
    /* --- make tux blink --- */
    switch (frame % TUX6)
    {
      case 0:    tux_frame = 1; break;
      case TUX1: tux_frame = 2; break;
      case TUX2: tux_frame = 3; break;
      case TUX3: tux_frame = 4; break;			
      case TUX4: tux_frame = 3; break;
      case TUX5: tux_frame = 2; break;
      default: tux_frame = 0;
    }

    if (Tux && tux_frame)
    {
      SDL_BlitSurface(Tux->frame[tux_frame - 1], NULL, screen, &Tuxdest);
      SDL_UpdateRect(screen, Tuxdest.x+37, Tuxdest.y+40, 70, 45);
    }




    /* Wait so we keep frame rate constant: */
    while ((SDL_GetTicks() - start) < 33)
    {
      SDL_Delay(20);
    }
    frame++;
  }  // End of while (!finished) loop

  SDL_FreeSurface(s1);
  SDL_FreeSurface(s2);
  SDL_FreeSurface(s3);
  SDL_FreeSurface(s4);

  /* Turn off SDL Unicode lookup (because has some overhead): */
  SDL_EnableUNICODE(SDL_DISABLE);
}



/* FIXME add some background shading to improve legibility */
void ShowMessage(char* str1, char* str2, char* str3, char* str4)
{
  SDL_Surface *s1, *s2, *s3, *s4;
  SDL_Rect loc;
  int finished = 0;
  int tux_frame = 0;
  Uint32 frame = 0;
  Uint32 start = 0;

  s1 = s2 = s3 = s4 = NULL;

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "ShowMessage() - creating text\n" );
#endif

  if (str1)
    s1 = black_outline(str1, default_font, &white);
  if (str2)
    s2 = black_outline(str2, default_font, &white);
  if (str3)
    s3 = black_outline(str3, default_font, &white);
  /* When we get going with i18n may need to modify following - see below: */
  if (str4)
    s4 = black_outline(str4, default_font, &white);

//   /* we always want the URL in english */
//   if (!useEnglish)
//   {
//     TTF_Font *english_font;
//     useEnglish = 1;
//     english_font = LoadFont( menu_font, menu_font_size );
//     s4 = black_outline( "tuxmath-devel@lists.sourceforge.net", english_font, &white);
//     TTF_CloseFont(english_font);
//     useEnglish = 0;
//   }
//   else 
//   {
//     s4 = black_outline( "tuxmath-devel@lists.sourceforge.net", default_font, &white);
//   }

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "NotImplemented() - drawing screen\n" );
#endif

  /* Redraw background: */
  if (images[IMG_MENU_BKG])
    SDL_BlitSurface( images[IMG_MENU_BKG], NULL, screen, NULL );

  /* Red "Stop" circle in upper right corner to go back to main menu: */
  if (images[IMG_STOP])
  {
    stopRect.w = images[IMG_STOP]->w;
    stopRect.h = images[IMG_STOP]->h;
    stopRect.x = screen->w - images[IMG_STOP]->w;
    stopRect.y = 0;
    SDL_BlitSurface(images[IMG_STOP], NULL, screen, &stopRect);
  }

  if (Tux && Tux->num_frames) /* make sure sprite has at least one frame */
  {
    SDL_BlitSurface(Tux->frame[0], NULL, screen, &Tuxdest);
  }

  /* Draw lines of text (do after drawing Tux so text is in front): */
  if (s1)
  {
    loc.x = 320 - (s1->w/2); loc.y = 10;
    SDL_BlitSurface( s1, NULL, screen, &loc);
  }
  if (s2)
  {
    loc.x = 320 - (s2->w/2); loc.y = 60;
    SDL_BlitSurface( s2, NULL, screen, &loc);
  }
  if (s3)
  {
    loc.x = 320 - (s3->w/2); loc.y = 300;
    SDL_BlitSurface( s3, NULL, screen, &loc);
  }
  if (s4)
  {
    loc.x = 320 - (s4->w/2); loc.y = 340;
    SDL_BlitSurface( s4, NULL, screen, &loc);
  }

  /* and update: */
  SDL_UpdateRect(screen, 0, 0, 0, 0);

  while (!finished)
  {
    start = SDL_GetTicks();

    while (SDL_PollEvent(&event)) 
    {
      switch (event.type)
      {
        case SDL_QUIT:
        {
          cleanup();
        }

        case SDL_MOUSEBUTTONDOWN:
        /* "Stop" button - go to main menu: */
        { 
          if (inRect(stopRect, event.button.x, event.button.y ))
          {
            finished = 1;
            tuxtype_playsound(sounds[SND_TOCK]);
            break;
          }
        }
        case SDL_KEYDOWN:
        {
          finished = 1;
          tuxtype_playsound(sounds[SND_TOCK]);
        }
      }
    }

    /* --- make tux blink --- */
    switch (frame % TUX6)
    {
      case 0:    tux_frame = 1; break;
      case TUX1: tux_frame = 2; break;
      case TUX2: tux_frame = 3; break;
      case TUX3: tux_frame = 4; break;			
      case TUX4: tux_frame = 3; break;
      case TUX5: tux_frame = 2; break;
      default: tux_frame = 0;
    }

    if (Tux && tux_frame)
    {
      SDL_BlitSurface(Tux->frame[tux_frame - 1], NULL, screen, &Tuxdest);
//      SDL_UpdateRect(screen, Tuxdest.x+37, Tuxdest.y+40, 70, 45);
      SDL_UpdateRect(screen, 0, 0, 0, 0);

    }
    /* Wait so we keep frame rate constant: */
    while ((SDL_GetTicks() - start) < 33)
    {
      SDL_Delay(20);
    }
    frame++;
  }  // End of while (!finished) loop

  SDL_FreeSurface(s1);
  SDL_FreeSurface(s2);
  SDL_FreeSurface(s3);
  SDL_FreeSurface(s4);
}






/* choose_config_file() - adapted from chooseWordlist() from tuxtype. */
/* Display a list of tuxmath config files in the missions directory   */
/* and allow the player to pick one (AKA "Lessons").                  */

/* returns 0 if user pressed escape (or if dir not found)
 *         1 if config was set correctly
 */
int choose_config_file(void)
{
  SDL_Surface **titles = NULL;
  SDL_Surface **select = NULL;

  SDL_Rect leftRect, rightRect;
  SDL_Rect titleRects[8];               //8 lessons displayed per page 
  SDL_Rect lesson_menu_button[8];      // Translucent mouse "buttons"

  Uint32 frame = 0;                             //For keeping frame rate constant 
  Uint32 frame_start = 0;
  int stop = 0;
  int loc = 0;                                  //The currently selected lesson file
  int old_loc = 1;
  int redraw = 0;
  int i = 0;
  int tux_frame = 0;
  int click_flag = 1;


#ifdef TUXMATH_DEBUG
  fprintf(stderr, "Entering choose_config_file():\n");
#endif

  /* Display the list of lessons for the player to select: */
  titles = (SDL_Surface**)malloc(num_lessons * sizeof(SDL_Surface*));
  select = (SDL_Surface**)malloc(num_lessons * sizeof(SDL_Surface*));

  if (titles == NULL || select == NULL) {
    free(titles);
    free(select);
    return 0;
  }
  for (i = 0; i < num_lessons; i++)
  {
    titles[i] = black_outline( _(lesson_list[i].display_name), default_font, &white );
    select[i] = black_outline( _(lesson_list[i].display_name), default_font, &yellow);
  }

//   if (images[IMG_MENU_BKG])
//   {
//     SDL_FreeSurface(images[IMG_MENU_BKG]);
//   }
//   bkg = LoadImage("title/main_bkg.jpg", IMG_REGULAR);

  /* Put arrow buttons in right lower corner, inset by 20 pixels */
  /* with a 10 pixel space between:                              */
  if (images[IMG_RIGHT])
  {
    rightRect.w = images[IMG_RIGHT]->w;
    rightRect.h = images[IMG_RIGHT]->h;
    rightRect.x = screen->w - images[IMG_RIGHT]->w - 20;
    rightRect.y = screen->h - images[IMG_RIGHT]->h - 20;
  }

  if (images[IMG_LEFT])
  {
    leftRect.w = images[IMG_LEFT]->w;
    leftRect.h = images[IMG_LEFT]->h;
    leftRect.x = rightRect.x - 10 - images[IMG_LEFT]->w;
    leftRect.y = screen->h - images[IMG_LEFT]->h - 20;
  }
  /* Red "Stop" circle in upper right corner to go back to main menu: */
  if (images[IMG_STOP])
  {
    stopRect.w = images[IMG_STOP]->w;
    stopRect.h = images[IMG_STOP]->h;
    stopRect.x = screen->w - images[IMG_STOP]->w;
    stopRect.y = 0;
  }

  /* set initial title rect sizes */
  titleRects[0].y = 20;
  titleRects[0].w = titleRects[0].h = titleRects[0].x = 0;

  for (i = 1; i < 8; i++)
  { 
    titleRects[i].y = titleRects[i - 1].y + 55;
    titleRects[i].w = titleRects[i].h = titleRects[i].x = 0;
  }

  /* Set up background, title, and Tux: */
  if (images[IMG_MENU_BKG])
    SDL_BlitSurface(images[IMG_MENU_BKG], NULL, screen, NULL);
  if (images[IMG_MENU_TITLE])
    SDL_BlitSurface(images[IMG_MENU_TITLE], NULL, screen, &Titledest);
  if (Tux && Tux->frame[0])
    SDL_BlitSurface(Tux->frame[0], NULL, screen, &Tuxdest);
  SDL_UpdateRect(screen, 0, 0, 0 ,0);

//   /* Move mouse to top button: */
//   cursor.x = screen->w/2; //titleRects[1].x + (menu_button[1].w / 2);
//   cursor.y = titleRects[0].y + 20; //+ (3 * menu_button[1].h / 4);
//   SDL_WarpMouse(cursor.x, cursor.y);
//   SDL_WM_GrabInput(SDL_GRAB_OFF);

  while (!stop)
  {
    frame_start = SDL_GetTicks();         /* For keeping frame rate constant. */

    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_QUIT:
        {
          cleanup();
          //exit(0);
          break;
        }
 
        case SDL_MOUSEMOTION:
        {
          for (i = 0; (i < 8) && (loc -(loc % 8) + i < num_lessons); i++)
          {
            if (inRect(lesson_menu_button[i], event.motion.x, event.motion.y))
            {
              // Play sound if loc is being changed:
              if (Opts_MenuSound() && (loc != loc - (loc % 8) + i)) 
              {
                tuxtype_playsound(sounds[SND_TOCK]);
              }
              loc = loc - (loc % 8) + i;
              break;
            }
          }

          /* "Left" button - make click if button active: */
          if (inRect(leftRect, event.motion.x, event.motion.y))
          {
            if (loc - (loc % 8) - 8 >= 0)
            {
              if (Opts_MenuSound() && click_flag)
              {
                tuxtype_playsound(sounds[SND_TOCK]);
                click_flag = 0;
              }
              break;
            }
          }

          /* "Right" button - go to next page: */
          else if (inRect( rightRect, event.motion.x, event.motion.y ))
          {
            if (loc - (loc % 8) + 8 < num_lessons)
            {
              if (Opts_MenuSound() && click_flag)
              {
                tuxtype_playsound(sounds[SND_TOCK]);
                click_flag = 0;
              }
              break;
            }
          }
          else  // Mouse outside of arrow rects - re-enable click sound:
          {
            click_flag = 1;
            break;
          }
        }

        case SDL_MOUSEBUTTONDOWN:
        {
          /* Lesson buttons - play game with corresponding lesson file: */
          for (i = 0; (i < 8) && (loc - (loc % 8) + i < num_lessons); i++)
          {
            if (inRect(lesson_menu_button[i], event.button.x, event.button.y))
            {
              if (Opts_MenuSound())
              {
                  tuxtype_playsound(sounds[SND_POP]);
              }

              loc = loc - (loc % 8) + i;

              /* Re-read global settings first in case any settings were */
              /* clobbered by other lesson or arcade games this session: */
              read_global_config_file();

              /* Now read the selected file and play the "mission": */ 
              if (read_named_config_file(lesson_list[loc].filename))
              {
                if (Opts_MenuMusic())  //Turn menu music off for game
                {
                  audioMusicUnload();
                }

                game();

                if (Opts_MenuMusic()) //Turn menu music back on
                {
                  audioMusicLoad( "tuxi.ogg", -1 );
                }
                redraw = 1;
              }
              else  // Something went wrong - could not read config file:
              {
                fprintf(stderr, "\nCould not find file: %s\n", lesson_list[loc].filename);
                stop = 1;
              }
              break;
            }
          }
        
          /* "Left" button - go to previous page: */
          if (inRect(leftRect, event.button.x, event.button.y))
          {
            if (loc - (loc % 8) - 8 >= 0)
            {
              loc = loc - (loc % 8) - 8;
              if (Opts_MenuSound())
              {
                tuxtype_playsound(sounds[SND_TOCK]);
              }
              break;
            }
          }

          /* "Right" button - go to next page: */
          if (inRect( rightRect, event.button.x, event.button.y ))
          {
            if (loc - (loc % 8) + 8 < num_lessons)
            {
              loc = loc - (loc % 8) + 8;
              if (Opts_MenuSound())
              {
                tuxtype_playsound(sounds[SND_TOCK]);
              }
              break;
            }
          }

          /* "Stop" button - go to main menu: */
          if (inRect(stopRect, event.button.x, event.button.y ))
          {
            stop = 2;
            tuxtype_playsound(sounds[SND_TOCK]);
            break;
          }
        } /* End of case SDL_MOUSEDOWN */


        case SDL_KEYDOWN:
        {
          /* Proceed according to particular key pressed: */
          switch (event.key.keysym.sym)
          {
            case SDLK_ESCAPE:
            { 
              stop = 2;
              break;
            }

            case SDLK_RETURN:
            case SDLK_SPACE:
            case SDLK_KP_ENTER:
            {
              if (Opts_MenuSound())
                {tuxtype_playsound(sounds[SND_POP]);}

              /* Re-read global settings first in case any settings were */
              /* clobbered by other lesson or arcade games this session: */
              read_global_config_file();

              /* Now read the selected file and play the "mission": */ 
              if (read_named_config_file(lesson_list[loc].filename))
              {
                if (Opts_MenuMusic())  //Turn menu music off for game
                  {audioMusicUnload();}

                game();

                if (Opts_MenuMusic()) //Turn menu music back on
                  {audioMusicLoad( "tuxi.ogg", -1 );}
                redraw = 1;
              }
              else  // Something went wrong - could not read config file:
              {
                fprintf(stderr, "\nCould not find file: %s\n", lesson_list[loc].filename);
                stop = 1;
              }
              break;
            }


            /* Go to previous page, if present: */
            case SDLK_LEFT:
            case SDLK_PAGEUP:
            {
              if (Opts_MenuSound())
                {tuxtype_playsound(sounds[SND_TOCK]);}
              if (loc - (loc % 8) - 8 >= 0)
                {loc = loc - (loc % 8) - 8;}
              break;
            }


            /* Go to next page, if present: */
            case SDLK_RIGHT:
            case SDLK_PAGEDOWN:
            {
              if (Opts_MenuSound())
                {tuxtype_playsound(sounds[SND_TOCK]);}
              if (loc - (loc % 8) + 8 < num_lessons)
                {loc = (loc - (loc % 8) + 8);}
              break; 
            }

            /* Go up one entry, if present: */
            case SDLK_UP:
            {
              if (Opts_MenuSound())
                {tuxtype_playsound(sounds[SND_TOCK]);}
              if (loc > 0)
                {loc--;}
              break;
            }


            /* Go down one entry, if present: */
            case SDLK_DOWN:
            {
              if (Opts_MenuSound())
                {tuxtype_playsound(sounds[SND_TOCK]);}
              if (loc + 1 < num_lessons)
                {loc++;}
              break; 
           }


            /* Toggle screen mode: */
            case SDLK_F10: 
            {
              switch_screen_mode();
              redraw = 1;
              break;
            }

            /* Toggle menu music: */
            case SDLK_F11:
            {
              if (Opts_MenuMusic())
              {
                audioMusicUnload( );
                Opts_SetMenuMusic(0);
              }
              else
              {
                Opts_SetMenuMusic(1);
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
    }  // End SDL_PollEvent while loop

    /* Redraw screen: */
    if (old_loc != loc) 
      redraw = 1;

    if (redraw)
    {
      int start;
      start = loc - (loc % 8);
      /* FIXME could use some segfault prevention if()s here: */
      /* Redraw background, title, stop button, and Tux: */
      SDL_BlitSurface(images[IMG_MENU_BKG], NULL, screen, NULL);
      SDL_BlitSurface(images[IMG_MENU_TITLE], NULL, screen, &Titledest);
      SDL_BlitSurface(images[IMG_STOP], NULL, screen, &stopRect);
      SDL_BlitSurface(Tux->frame[0], NULL, screen, &Tuxdest);

      /* FIXME get rid of "evil" macro ;)       */
      for (i = start; i < MIN(start+8,num_lessons); i++)
      {
        titleRects[i % 8].x = 240;     //Like main menu
        titleRects[i % 8].w = titles[i]->w;
        titleRects[i % 8].h = titles[i]->h;

        /* Now set up mouse button rects: */
        lesson_menu_button[i % 8].x = titleRects[i % 8].x - 15;
        lesson_menu_button[i % 8].y = titleRects[i % 8].y - 15;
        lesson_menu_button[i % 8].h = titleRects[i % 8].h + 30;
        lesson_menu_button[i % 8].w = titleRects[i % 8].w + 30;

        if (i == loc)  //Draw text in yellow
        {
          draw_button(&lesson_menu_button[i % 8], lesson_menu_button[i % 8].w, sel);
          SDL_BlitSurface(select[loc], NULL, screen, &titleRects[i % 8]);
        }
        else           //Draw text in white
        {
          draw_button(&lesson_menu_button[i % 8], lesson_menu_button[i % 8].w, reg);
          SDL_BlitSurface(titles[i], NULL, screen, &titleRects[i % 8]);
        }
      }

      /* --- draw 'left' and 'right' buttons --- */
      if (start > 0)        // i.e. not on first page
      {
        SDL_BlitSurface(images[IMG_LEFT], NULL, screen, &leftRect);
      }
      else  /* Draw grayed-out left button: */
      {
        SDL_BlitSurface(images[IMG_LEFT_GRAY], NULL, screen, &leftRect);
      }

      if (start + 8 < num_lessons)  // not on last page
      {
        SDL_BlitSurface(images[IMG_RIGHT], NULL, screen, &rightRect);
      }
      else  /* Draw grayed-out right button: */
      {
        SDL_BlitSurface(images[IMG_RIGHT_GRAY], NULL, screen, &rightRect);
      }


      SDL_UpdateRect(screen, 0, 0, 0 ,0);
    }
    redraw = 0;
    old_loc = loc;


    /* --- make Tux blink --- */
    switch (frame % TUX6)
    {
      case 0:    tux_frame = 1; break;
      case TUX1: tux_frame = 2; break;
      case TUX2: tux_frame = 3; break;
      case TUX3: tux_frame = 4; break;			
      case TUX4: tux_frame = 3; break;
      case TUX5: tux_frame = 2; break;
      default: tux_frame = 0;
    }

    if (Tux && tux_frame)
    {
      SDL_BlitSurface(Tux->frame[tux_frame - 1], NULL, screen, &Tuxdest);
 //     SDL_UpdateRect(screen, Tuxdest.x+37, Tuxdest.y+40, 70, 45);
      SDL_UpdateRect(screen, 0, 0, 0, 0);

    }

    /* Wait so we keep frame rate constant: */
    while ((SDL_GetTicks() - frame_start) < 33)
    {
      SDL_Delay(20);
    }
    frame++;
  }  // End !stop while loop

  /* --- clear graphics before leaving function --- */ 
  for (i = 0; i < num_lessons; i++)
  {
    SDL_FreeSurface(titles[i]);
    SDL_FreeSurface(select[i]);
  }
  free(titles);
  free(select);


#ifdef TUXMATH_DEBUG
  fprintf( stderr, "Leaving choose_config_file();\n" );
#endif

  if (stop == 2)  // Means player pressed ESC
    return 0;

  return 1;
}





// Was in playgame.c in tuxtype:

/*************************************************/
/* TransWipe: Performs various wipes to new bkgs */
/*************************************************/
/*
 * Given a wipe request type, and any variables
 * that wipe requires, will perform a wipe from
 * the current screen image to a new one.
 */
void TransWipe(SDL_Surface* newbkg, int type, int var1, int var2)
{
    int i, j, x1, x2, y1, y2;
    int step1, step2, step3, step4;
    int frame;
    SDL_Rect src;
    SDL_Rect dst;

    if (!screen)
    {
      LOG("TransWipe(): screen not valid!\n");
      return;
    }

    if (!newbkg)
    {
      LOG("TransWipe(): newbkg not valid!\n");
      return;
    }

    numupdates = 0;
    frame = 0;

    if(newbkg->w == screen->w && newbkg->h == screen->h) {
        if( type == RANDOM_WIPE )
            type = (RANDOM_WIPE * ((float) rand()) / (RAND_MAX+1.0));

        switch( type ) {
            case WIPE_BLINDS_VERT: {

                LOG("--+ Doing 'WIPE_BLINDS_VERT'\n");

                /* var1 is num of divisions
                   var2 is how many frames animation should take */
                if( var1 < 1 ) var1 = 1;
                if( var2 < 1 ) var2 = 1;
                step1 = screen->w / var1; 
                step2 = step1 / var2;

                for(i = 0; i <= var2; i++) 
                {
                    for(j = 0; j <= var1; j++)
                    {
                        x1 = step1 * (j - 0.5) - i * step2 + 1;
                        x2 = step1 * (j - 0.5) + i * step2 + 1;
                        src.x = x1;
                        src.y = 0;
                        src.w = step2;
                        src.h = screen->h;
                        dst.x = x2;
                        dst.y = 0;
                        dst.w = step2;
                        dst.h = screen->h;

                        SDL_BlitSurface(newbkg, &src, screen, &src);
                        SDL_BlitSurface(newbkg, &dst, screen, &dst);

                        AddRect(&src, &src);
                        AddRect(&dst, &dst);
                    }
                    UpdateScreen(&frame);
                }

                src.x = 0;
                src.y = 0;
                src.w = screen->w;
                src.h = screen->h;
                SDL_BlitSurface(newbkg, NULL, screen, &src);
                SDL_Flip(screen);

                break;
            } case WIPE_BLINDS_HORIZ: {
                LOG("--+ Doing 'WIPE_BLINDS_HORIZ'\n");
                /* var1 is num of divisions
                   var2 is how many frames animation should take */
                if( var1 < 1 ) var1 = 1;
                if( var2 < 1 ) var2 = 1;
                step1 = screen->h / var1;
                step2 = step1 / var2;

                for(i = 0; i <= var2; i++) {
                    for(j = 0; j <= var1; j++) {
                        y1 = step1 * (j - 0.5) - i * step2 + 1;
                        y2 = step1 * (j - 0.5) + i * step2 + 1;
                        src.x = 0;
                        src.y = y1;
                        src.w = screen->w;
                        src.h = step2;
                        dst.x = 0;
                        dst.y = y2;
                        dst.w = screen->w;
                        dst.h = step2;

                        SDL_BlitSurface(newbkg, &src, screen, &src);
                        SDL_BlitSurface(newbkg, &dst, screen, &dst);

                        AddRect(&src, &src);
                        AddRect(&dst, &dst);
                    }
                    UpdateScreen(&frame);
                }

                src.x = 0;
                src.y = 0;
                src.w = screen->w;
                src.h = screen->h;
                SDL_BlitSurface(newbkg, NULL, screen, &src);
                SDL_Flip(screen);

                break;
            } case WIPE_BLINDS_BOX: {
                LOG("--+ Doing 'WIPE_BLINDS_BOX'\n");
                /* var1 is num of divisions
                   var2 is how many frames animation should take */
                if( var1 < 1 ) var1 = 1;
                if( var2 < 1 ) var2 = 1;
                step1 = screen->w / var1;
                step2 = step1 / var2;
                step3 = screen->h / var1;
                step4 = step1 / var2;

                for(i = 0; i <= var2; i++) {
                    for(j = 0; j <= var1; j++) {
                        x1 = step1 * (j - 0.5) - i * step2 + 1;
                        x2 = step1 * (j - 0.5) + i * step2 + 1;
                        src.x = x1;
                        src.y = 0;
                        src.w = step2;
                        src.h = screen->h;
                        dst.x = x2;
                        dst.y = 0;
                        dst.w = step2;
                        dst.h = screen->h;

                        SDL_BlitSurface(newbkg, &src, screen, &src);
                        SDL_BlitSurface(newbkg, &dst, screen, &dst);

                        AddRect(&src, &src);
                        AddRect(&dst, &dst);
                        y1 = step3 * (j - 0.5) - i * step4 + 1;
                        y2 = step3 * (j - 0.5) + i * step4 + 1;
                        src.x = 0;
                        src.y = y1;
                        src.w = screen->w;
                        src.h = step4;
                        dst.x = 0;
                        dst.y = y2;
                        dst.w = screen->w;
                        dst.h = step4;
                        SDL_BlitSurface(newbkg, &src, screen, &src);
                        SDL_BlitSurface(newbkg, &dst, screen, &dst);
                        AddRect(&src, &src);
                        AddRect(&dst, &dst);
                    }
                    UpdateScreen(&frame);
                }

                src.x = 0;
                src.y = 0;
                src.w = screen->w;
                src.h = screen->h;
                SDL_BlitSurface(newbkg, NULL, screen, &src);
                SDL_Flip(screen);

                break;
            } default:
                break;
        }
    }
    LOG("->TransWipe(): FINISH\n");
}

/************************
UpdateScreen : Update the screen and increment the frame num
***************************/
void UpdateScreen(int *frame) {
	int i;

	/* -- First erase everything we need to -- */
	for (i = 0; i < numupdates; i++)
		if (blits[i].type == 'E') 
			SDL_LowerBlit(blits[i].src, blits[i].srcrect, screen, blits[i].dstrect);
//	SNOW_erase();

	/* -- then draw -- */ 
	for (i = 0; i < numupdates; i++)
		if (blits[i].type == 'D') 
			SDL_BlitSurface(blits[i].src, blits[i].srcrect, screen, blits[i].dstrect);
//	SNOW_draw();

	/* -- update the screen only where we need to! -- */
//	if (SNOW_on) 
//		SDL_UpdateRects(screen, SNOW_add( (SDL_Rect*)&dstupdate, numupdates ), SNOW_rects);
//	else 
		SDL_UpdateRects(screen, numupdates, dstupdate);

	numupdates = 0;
	*frame = *frame + 1;
}


/******************************
AddRect: Don't actually blit a surface,
    but add a rect to be updated next
    update
*******************************/
void AddRect(SDL_Rect * src, SDL_Rect * dst) {
    /*borrowed from SL's alien (and modified)*/

    struct blit    *update;

    if (!src || !dst)
    {
      LOG("AddRect(): src or dst invalid!\n");
      return;
    }

    update = &blits[numupdates++];

    update->srcrect->x = src->x;
    update->srcrect->y = src->y;
    update->srcrect->w = src->w;
    update->srcrect->h = src->h;
    update->dstrect->x = dst->x;
    update->dstrect->y = dst->y;
    update->dstrect->w = dst->w;
    update->dstrect->h = dst->h;
    update->type = 'I';
}

/***********************
 InitEngine
 ***********************/
void InitEngine(void) {
    int i;

    /* --- Set up the update rectangle pointers --- */
	
    for (i = 0; i < MAX_UPDATES; ++i) {
        blits[i].srcrect = &srcupdate[i];
        blits[i].dstrect = &dstupdate[i];
    }
}


