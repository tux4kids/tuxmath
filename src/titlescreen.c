/***************************************************************************
 -  file: titlescreen.c
 -  description: splash, title and menu screen functionality 
                            ------------------
    begin                : Thur May 4 2000
    copyright            : (C) 2000 by Sam Hart
                         : (C) 2003 by Jesse Andrews
    email                : tuxtype-dev@tux4kids.net

    Modified for use in tuxmath by David Bruce - 2006.
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
#include "options.h"
#include "fileops.h"
#include "game.h"
#include "mathcards.h"
#include "setup.h"     //for cleanup()

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

// globals from tuxtype's globals.h defined outside of titlescreen.c (in tuxtype):
int show_tux4kids;
int debugOn; //FIXME switch to TUXMATH_DEBUG


TTF_Font*  font;
SDL_Event   event;
SDL_Surface*  bkg;

/* --- media for menus --- */

/* FIXME Should all these arrays be defined as size [TITLE_MENU_ITEMS + 1][TITLE_MENU_DEPTH + 1]?  */
/* FIXME Instead of six parallel arrays, make struct with six fields and create a single array */
/* of the struct. */

/* --- define menu structure --- */
/* (these values are all in the Game_Type enum in titlescreen.h) */
/* They are the "commands" associated with the menu items.   */
const int menu_item[][6]= {{0, 0,         0,         0,        0           },
			   {0, LESSONS,      ARCADE_CADET,   LEVEL1,   NOT_CODED   },
			   {0, ARCADE,       ARCADE_SCOUT,   LEVEL2,   FREETYPE    },
			   {0, OPTIONS,      ARCADE_RANGER,  LEVEL3,   PROJECT_INFO},
			   {0, MORE_OPTIONS, ARCADE_ACE,     LEVEL4,   SET_LANGUAGE},
			   {0, QUIT_GAME,    MAIN,           MAIN,     MAIN        }};

/* --- menu text --- */
const unsigned char* menu_text[][6]= 
/*    Main Menu                                       'Arcade' Games                    Math Options                     Game Options            */
{{"", "",                                             "",                             "",                              ""                       },
 {"", gettext_noop("Math Command Training Academy"),  gettext_noop("Space Cadet"), gettext_noop("Addition"),       gettext_noop("Speed")    },
 {"", gettext_noop("Play Arcade Game"),               gettext_noop("Scout"),       gettext_noop("Subtraction"),    gettext_noop("Sound")    },
 {"", gettext_noop("Play Custom Game"),               gettext_noop("Ranger"),      gettext_noop("Multiplication"), gettext_noop("Graphics") },
 {"", gettext_noop("More Options"),                   gettext_noop("Ace"),         gettext_noop("Division"),       gettext_noop("Advanced Options")},
 {"", gettext_noop("Quit"),                           gettext_noop("Main Menu"),   gettext_noop("Main Menu"),      gettext_noop("Main Menu") }};


/* These are the filenames of the images used in the animated menu icons: */
/* --- menu icons --- */
const unsigned char* menu_icon[][6]= 
{{"", "",        "",       "",        ""        },
 {"", "lesson",   "easy",   "grade1_", "list"    },
 {"", "comet",    "medium", "grade2_", "practice"},
 {"", "keyboard", "hard",   "grade3_", "keyboard"},
 {"", "keyboard", "tutor",  "grade4_", "lang"    },
 {"", "quit",     "main",   "main",    "main"   }};


/* this will contain pointers to all of the menu 'icons' */
sprite* menu_gfx[TITLE_MENU_ITEMS + 1][TITLE_MENU_DEPTH + 1];

/* images of regular and selected text of menu items: */
SDL_Surface* reg_text[TITLE_MENU_ITEMS + 1][TITLE_MENU_DEPTH + 1];
SDL_Surface* sel_text[TITLE_MENU_ITEMS + 1][TITLE_MENU_DEPTH + 1];
/* reg and sel are used to create the translucent button backgrounds. */
sprite* reg;
sprite* sel;


/* keep track of the width of each menu: */
int menu_width[TITLE_MENU_DEPTH + 1];

/* NOTE for 'depth', think pages like a restaurant menu, */
/* not heirarchical depth - choice of term is misleading */
int menu_depth; // how deep we are in the menu
settings localsettings;

/* --- other media --- */
SDL_Surface* titlepic;
SDL_Surface* speaker;
SDL_Surface* speakeroff;
sprite* Tux;
Mix_Chunk* snd_move;
Mix_Chunk* snd_select;

/* --- locations we need --- */
SDL_Rect text_dst[TITLE_MENU_ITEMS + 1];     // location of text for menu
SDL_Rect menu_gfxdest[TITLE_MENU_ITEMS + 1]; // location of animated icon
/* These are the rectangular mouse event "buttons" for each menu item */
SDL_Rect menu_button[TITLE_MENU_ITEMS + 1];  // size of "button"

SDL_Rect dest,
	 Tuxdest,
	 Titledest,
	 spkrdest,
	 cursor;

/* Local function prototypes: */
//int chooseWordlist(void);
void draw_button(SDL_Rect* target_rect, int width, sprite* s);
void TitleScreen_load_menu(void);
void TitleScreen_unload_menu(void);
void TitleScreen_load_media(void);
void TitleScreen_unload_media(void);
void NotImplemented(void);
void TransWipe(SDL_Surface* newbkg, int type, int var1, int var2);
void UpdateScreen(int* frame);
void AddRect(SDL_Rect* src, SDL_Rect* dst);
void InitEngine(void);

/***********************************************************/
/*                                                         */
/*       "Public functions" (callable throughout program)  */
/*                                                         */
/***********************************************************/



/****************************************
* TitleScreen: Display the title screen *
*****************************************
* display title screen, get input
*/
void TitleScreen(void)
{

  Uint32 frame = 0;
  Uint32 start = 0;

  int i, j; 
  int tux_frame = 0;
  int done = 0;
  int firstloop = 1;
  int menu_opt = NONE;
  int sub_menu = NONE;
  int update_locs = 1;
  int redraw = 0;
  int key_menu = 1;
  int old_key_menu = 5;
  char phrase[128];

  debugOn = 1; //for now
  show_tux4kids = 1; //for now

  if (Opts_UseSound())
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


  /* FIXME phrase(s) should come from file */
  strncpy( phrase, "Now is the time for all good men to come to the aid of their country.", 128);
  start = SDL_GetTicks();


  /*
  * StandbyScreen: Display the Standby screen.... 
  */

  if (show_tux4kids)
  {
    SDL_Surface *standby;
    standby = LoadImage("status/standby.png", IMG_REGULAR|IMG_NO_THEME);

    dest.x = ((screen->w) / 2) - (standby->w) / 2;  // Center horizontally
    dest.y = ((screen->h) / 2) - (standby->h) / 2;  // Center vertically
    dest.w = standby->w;
    dest.h = standby->h;

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_BlitSurface(standby, NULL, screen, &dest);
    SDL_UpdateRect(screen, 0, 0, 0, 0);
    SDL_FreeSurface(standby);  // Unload image
  }


  /* Load media and menu data: */
  TitleScreen_load_media();
  SDL_WM_GrabInput(SDL_GRAB_ON); // User input goes to TuxType, not window manager


  /***************************
  * Tux and Title animations *
  ***************************/

  LOG( "->Now Animating Tux and Title onto the screen\n" );

  Tuxdest.x = 0;
  Tuxdest.y = screen->h;
  Tuxdest.w = Tux->frame[0]->w;
  Tuxdest.h = Tux->frame[0]->h;

  Titledest.x = screen->w;
  Titledest.y = 10;
  Titledest.w = titlepic->w;
  Titledest.h = titlepic->h;

  spkrdest.x = 520;
  spkrdest.y = 420;
  spkrdest.w = speaker->w;
  spkrdest.h = speaker->h;

  /* --- wait if the first time in the game --- */

  if (show_tux4kids)
  {
    while ((SDL_GetTicks() - start) < 2000)
    {
      SDL_Delay(50);
    }
    show_tux4kids = 0;
  }

  SDL_ShowCursor(1);    
  /* FIXME not sure the next line works in Windows: */
  TransWipe(bkg, RANDOM_WIPE, 10, 20);


  /* --- Pull tux & logo onscreen --- */
  /* NOTE we wind up with Tuxdest.y == (screen->h)  - (Tux->frame[0]->h), */
  /* and Titledest.x == 0.                                                */
  for (i = 0; i < (PRE_ANIM_FRAMES * PRE_FRAME_MULT); i++)
  {
    start = SDL_GetTicks();
    SDL_BlitSurface(bkg, &Tuxdest, screen, &Tuxdest);
    SDL_BlitSurface(bkg, &Titledest, screen, &Titledest);

    Tuxdest.y -= Tux->frame[0]->h / (PRE_ANIM_FRAMES * PRE_FRAME_MULT);
    Titledest.x -= (screen->w) / (PRE_ANIM_FRAMES * PRE_FRAME_MULT);

    SDL_BlitSurface(Tux->frame[0], NULL, screen, &Tuxdest);
    SDL_BlitSurface(titlepic, NULL, screen, &Titledest);

    SDL_UpdateRect(screen, Tuxdest.x, Tuxdest.y, Tuxdest.w, Tuxdest.h);
    SDL_UpdateRect(screen, Titledest.x, Titledest.y, Titledest.w+40, Titledest.h);

    while ((SDL_GetTicks() - start) < 33) 
    {
      SDL_Delay(2);
    }
  }

  SDL_BlitSurface(titlepic, NULL, screen, &Titledest);

  LOG( "Tux and Title are in place now\n" );


  /* Pick speaker graphic according to whether music is on: */
  if (Opts_MenuMusic())
  {
    SDL_BlitSurface(speaker, NULL, screen, &spkrdest);
  }
  else
  {
    SDL_BlitSurface(speakeroff, NULL, screen, &spkrdest);
  }

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
  Tuxdest.y = screen->h - Tux->frame[0]->h;


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
            if ((cursor.x >= menu_button[j].x && cursor.x <= (menu_button[j].x + menu_button[j].w)) && 
                (cursor.y >= menu_button[j].y && cursor.y <= (menu_button[j].y + menu_button[j].h)))
            {
              menu_opt = menu_item[j][menu_depth];
              if (Opts_MenuSound())
              {
                tuxtype_playsound(snd_select);
              }
              DEBUGCODE
              {
                fprintf(stderr, "->>BUTTON CLICK menu_opt = %d\n", menu_opt);
                fprintf(stderr, "->J = %d menu_depth=%d\n", j, menu_depth);
              }
            }
          }

          /* If mouse over speaker, toggle menu music off or on: */
          if ((cursor.x >= spkrdest.x && cursor.x <= (spkrdest.x + spkrdest.w)) && 
              (cursor.y >= spkrdest.y && cursor.y <= (spkrdest.y + spkrdest.h)))
          {
            if (Opts_MenuMusic())
            {
              audioMusicUnload();
              Opts_SetMenuMusic(0);
            }
            else
            {
              Opts_SetMenuMusic(1);
              audioMusicLoad("tuxi.ogg", -1);
            }
            redraw = 1;
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
                tuxtype_playsound(snd_select);
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
              redraw = 1;
              break;
            }


            /* --- reload translation/graphics/media: for themers/translaters --- */
            case SDLK_F12:
            {
              TitleScreen_unload_media();
              LoadLang();
              TitleScreen_load_media();
              redraw = 1;
              break;
            }


            case SDLK_UP:
            {
              if (Opts_MenuSound())
                tuxtype_playsound(snd_move);
              key_menu--;
              if (key_menu < 1)
                key_menu = 5;
              break;
            }


            case SDLK_DOWN:
            {
              key_menu++;
              if (Opts_MenuSound())
                tuxtype_playsound(snd_move);
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
                DOUT(key_menu);
		DOUT(menu_depth);
                menu_opt = menu_item[key_menu][menu_depth];
                DOUT(menu_opt);

                if (Opts_MenuSound())
                  tuxtype_playsound(snd_select);
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

    /* First page:-----------------------------------------------*/

    if (menu_opt == LESSONS) /* Go to 'lessons' menu: */
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
    }


    if (menu_opt == ARCADE)   /* Go to Arcade submenu */
    {
       menu_depth = ARCADE_SUBMENU; /* i.e. 2 */
       sub_menu = ARCADE;           /* i.e. 1 */
       update_locs = 1;
       redraw=1;
    }


    if (menu_opt == HELP)
    {
      NotImplemented();
      redraw = 1;
    }


    if (menu_opt == OPTIONS)  /* Still using old options() for now */
    {
      options();

      if (Opts_MenuMusic())
      {
        audioMusicUnload();
      }

      game();

      if (Opts_MenuMusic())
      {
        audioMusicLoad( "tuxi.ogg", -1 );
      }
      redraw = 1;
    }


    if (menu_opt == MORE_OPTIONS)
    {
      NotImplemented();
      redraw = 1;
    }


    if (menu_opt == QUIT_GAME)
    {
      done = 1;
    }

    /* Second (Arcade) page:-----------------------------------------------*/

    /* Play game of selected difficulty:                                   */
    /* TODO save high scores for each difficulty level.                    */
    /* TODO display brief description of type of questions at each level.  */
    if (menu_opt == ARCADE_CADET)
    {
      LOG("menu_opt == ARCADE_CADET");
      if (read_named_config_file("arcade/space_cadet"))
      {
        if (Opts_MenuMusic())
        {
          audioMusicUnload();
        }
        game();
      }
      else
      {
        fprintf(stderr, "\nCould not find arcade space_cadet config file\n");
      }

      TitleScreen_load_media();

      if (Opts_MenuMusic())
      {
        audioMusicLoad( "tuxi.ogg", -1 );
      }
      redraw = 1;
    }


    if (menu_opt == ARCADE_SCOUT)
    {
      LOG("menu_opt == ARCADE_SCOUT");
      if (read_named_config_file("arcade/scout"))
      {
        if (Opts_MenuMusic())
        {
          audioMusicUnload();
        }
        game();
      }
      else
      {
        fprintf(stderr, "\nCould not find arcade scout config file\n");
      }

      TitleScreen_load_media();

      if (Opts_MenuMusic())
      {
        audioMusicLoad( "tuxi.ogg", -1 );
      }
      redraw = 1;
    }


    if (menu_opt == ARCADE_RANGER)
    {
      LOG("menu_opt == ARCADE_RANGER");
      if (read_named_config_file("arcade/ranger"))
      {
        if (Opts_MenuMusic())
        {
          audioMusicUnload();
        }
        game();
      }
      else
      {
        fprintf(stderr, "\nCould not find arcade ranger config file\n");
      }

      TitleScreen_load_media();

      if (Opts_MenuMusic())
      {
        audioMusicLoad( "tuxi.ogg", -1 );
      }
      redraw = 1;
    }



    if (menu_opt == ARCADE_ACE)
    {
      LOG("menu_opt == ARCADE_ACE");
      if (read_named_config_file("arcade/ace"))
      {
        if (Opts_MenuMusic())
        {
          audioMusicUnload();
        }

	/* The 'Ace' list is _very_ long, so only use a random 10%  */
        /* of questions meeting the criteria (and reset afterward): */
        MC_SetFractionToKeep(0.1);
        game();
        MC_SetFractionToKeep(1);
      }
      else
      {
        fprintf(stderr, "\nCould not find arcade ace config file\n");
      }

      TitleScreen_load_media();

      if (Opts_MenuMusic())
      {
        audioMusicLoad( "tuxi.ogg", -1 );
      }
      redraw = 1;
    }

    /* Go back to main menu: */
    if (menu_opt == MAIN)
    {
      menu_depth = ROOTMENU;
      update_locs = 1;
      redraw = 1;
    }

    /* Rest of menu_opts are not currently used: */
/*
    if (menu_opt == LASER)
    {
      menu_depth = LASER_SUBMENU;
      sub_menu = LASER;
      update_locs = 1;
      redraw = 1;
    }


    if (menu_opt == NOT_CODED)
    {
      NotImplemented();
      redraw = 1;
    }


    if (menu_opt == PROJECT_INFO)
    {
//      projectInfo();
      redraw = 1;
    }


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


    if (menu_opt == INSTRUCT)
    {
      TitleScreen_unload_media();

      switch (sub_menu)
      {
//        case CASCADE: InstructCascade(); break;
//        case LASER:   InstructLaser();   break;
      }

      TitleScreen_load_media();

      if (Opts_MenuMusic())
      { 
        audioMusicLoad( "tuxi.ogg", -1 );
      }
      redraw = 1;
    }


    if (menu_opt == FREETYPE)
    {
      TitleScreen_unload_media();
//      Phrases( phrase );
      //Practice();
      TitleScreen_load_media();
      redraw = 1;
    }
*/
    /* ------ End menu_opt processing ----------- */



    if (redraw)
    {
      SDL_BlitSurface(bkg, NULL, screen, NULL); 
      SDL_BlitSurface(titlepic, NULL, screen, &Titledest);

      if (Opts_MenuMusic())
      {
        SDL_BlitSurface(speaker, NULL, screen, &spkrdest);
      }
      else
      {
        SDL_BlitSurface(speakeroff, NULL, screen, &spkrdest);
      }

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
        SDL_BlitSurface(bkg, &menu_button[i], screen, &menu_button[i]);
        menu_button[i].w = menu_width[menu_depth] + (2 * reg->frame[2]->w);
      }

      update_locs = 0;

      /* --- draw the full menu --- */

      for (j = 1; j <= TITLE_MENU_ITEMS; j++)
      {
        draw_button(&menu_button[j], menu_width[menu_depth], reg);
        SDL_BlitSurface(reg_text[j][menu_depth], NULL, screen, &text_dst[j]);
        SDL_BlitSurface(menu_gfx[j][menu_depth]->default_img, NULL, screen, &menu_gfxdest[j]);
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

    if (tux_frame)
    {
      SDL_BlitSurface(bkg, &Tuxdest, screen, &Tuxdest);
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
      SDL_BlitSurface(bkg, &menu_button[old_key_menu], screen, &menu_button[old_key_menu]);
      draw_button(&menu_button[old_key_menu], menu_width[menu_depth], reg );
      SDL_BlitSurface(reg_text[old_key_menu][menu_depth], NULL, screen, &text_dst[old_key_menu]);
      SDL_BlitSurface(menu_gfx[old_key_menu][menu_depth]->default_img, NULL, screen, &menu_gfxdest[old_key_menu]);
    }


    /* --- draw current selection --- */

    if ((key_menu != 0) &&
       ((old_key_menu != key_menu) || (frame % 5 == 0))) // Redraw every fifth frame?
    {
      if (key_menu != old_key_menu)
      {
        rewind(menu_gfx[key_menu][menu_depth]);
        tuxtype_playsound(snd_move);
      }

      SDL_BlitSurface(bkg, &menu_button[key_menu], screen, &menu_button[key_menu]);
      draw_button(&menu_button[key_menu], menu_width[menu_depth], sel );
      SDL_BlitSurface(sel_text[key_menu][menu_depth], NULL, screen, &text_dst[key_menu]);
      SDL_BlitSurface(menu_gfx[key_menu][menu_depth]->frame[menu_gfx[key_menu][menu_depth]->cur], NULL, screen, &menu_gfxdest[key_menu]);

      next_frame(menu_gfx[key_menu][menu_depth]);
    }


    // HACK This is still more than we need to update every frame but
    // it cuts cpu on my machine %60 so it seems better...
    if (Opts_MenuMusic())
    {
      SDL_BlitSurface(speaker, NULL, screen, &spkrdest);
    }
    else
    {
      SDL_BlitSurface(speakeroff, NULL, screen, &spkrdest);
    }

    SDL_UpdateRect(screen, spkrdest.x, spkrdest.y, spkrdest.w, spkrdest.h);

    for ( i=1; i<6; i++ )
    {
      SDL_UpdateRect(screen, menu_button[i].x, menu_button[i].y, menu_button[i].w, menu_button[i].h);
    }

    if (tux_frame)
    {
      SDL_UpdateRect(screen, Tuxdest.x+37, Tuxdest.y+40, 70, 45);
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



  LOG( "->>Freeing title screen images\n" );

  localsettings.menu_music = Opts_MenuMusic();
  TitleScreen_unload_media();

  LOG( "->TitleScreen():END \n" );
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
  DEBUGCODE
  {
    fprintf(stderr, "Entering TitleScreen_load_media():\n");
    fprintf(stderr, "realPath[0] = %s\n", realPath[0]);
    fprintf(stderr, "realPath[1] = %s\n", realPath[1]);
  }

  if (Opts_MenuSound())
  {
    snd_move = LoadSound("tock.wav");
    snd_select = LoadSound("pop.wav");
  }
 
  /* --- load graphics --- */

  titlepic = LoadImage("title/title1.png", IMG_ALPHA);
  LOG("About to try to load speaker image\n");

  speaker = LoadImage( "title/sound.png", IMG_ALPHA );
  speakeroff = LoadImage( "title/nosound.png", IMG_ALPHA );
  bkg = LoadImage( "title/main_bkg.jpg", IMG_REGULAR );

  sel = LoadSprite("sprites/sel", IMG_ALPHA);
  reg = LoadSprite("sprites/reg", IMG_ALPHA);
  Tux = LoadSprite("tux/bigtux", IMG_ALPHA);

  font = LoadFont(menu_font, menu_font_size);
  /* Should probably call this directly from TitleScreen() */
  TitleScreen_load_menu();
}



void TitleScreen_load_menu(void)
{
  unsigned char fn[FNLEN];
  int max, i, j;

  SDL_ShowCursor(1);

  LOG("loading & parsing menu\n");

  for (j = 1; j <= TITLE_MENU_DEPTH; j++)  /* Each 'depth' is a different menu */
  {
    max = 0;  // max will be width of widest text entry of this menu

    for (i = 1; i <= TITLE_MENU_ITEMS; i++)
    {
      /* --- create text surfaces --- */
      reg_text[i][j] = black_outline( _((unsigned char*)menu_text[i][j]), font, &white);
      sel_text[i][j] = black_outline( _((unsigned char*)menu_text[i][j]), font, &yellow);

      if (sel_text[i][j]->w > max)
      {
        max = sel_text[i][j]->w;
      }

      /* --- load animated icon for menu item --- */
      sprintf(fn, "sprites/%s", menu_icon[i][j]);
      menu_gfx[i][j] = LoadSprite(fn, IMG_ALPHA);
    }
    menu_width[j] = max + 20 + 40; // 40 is width of sprite, 20 is gap
  }

  LOG("done creating graphics, now setting positions\n");


  /* --- setup menu item destinations --- */

  menu_button[1].x = 240;    // center of top button hardcoded to (240, 100)
  menu_button[1].y = 100;
  menu_button[1].w = menu_width[1];  //calc from width of widest menu item
  menu_button[1].h = sel->frame[1]->h; //height of sprite image

  menu_gfxdest[1].x = menu_button[1].x + 6; // inset graphic by (6, 4) */
  menu_gfxdest[1].y = menu_button[1].y + 4;
  menu_gfxdest[1].w = 40;      // maybe should be MENU_SPRITE_WIDTH?
  menu_gfxdest[1].h = 50;      //   "      "    " MENU_SPRITE_HEIGHT?

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
    menu_gfxdest[j].x = menu_gfxdest[j - 1].x;
    menu_gfxdest[j].y = menu_gfxdest[j - 1].y + 60;
    menu_gfxdest[j].w = menu_gfxdest[j - 1].w;
    menu_gfxdest[j].h = menu_gfxdest[j - 1].h;
  }
}

/* draw_button() creates and draws a translucent button with */
/* rounded ends.  The location and size are taken from the */
/* SDL_Rect* and width arguments.  The sprite is used to   */
/* fill in the rect with the desired translucent color and */
/* give it nice, rounded ends.                             */
void draw_button(SDL_Rect* target_rect, int width, sprite* s )
{
  SDL_Rect button;

  if (! target_rect || !s)
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
      FreeSprite(menu_gfx[i][j]);
    }
  }
}







void TitleScreen_unload_media( void ) {

	/* --- unload sounds --- */

	if (Opts_MenuSound()){
	    Mix_FreeChunk(snd_move);
	    Mix_FreeChunk(snd_select);
	}

	/* --- unload graphics --- */

	SDL_FreeSurface(titlepic);
	SDL_FreeSurface(speaker);
	SDL_FreeSurface(speakeroff);
	SDL_FreeSurface(bkg);

	FreeSprite(sel);
	FreeSprite(reg);

	FreeSprite(Tux);

	TTF_CloseFont(font);
	TitleScreen_unload_menu();
}

void NotImplemented(void)
{
  SDL_Surface *s1, *s2, *s3, *s4;
  sprite *tux;
  SDL_Rect loc;
  int finished = 0;
  int tux_frame = 0;
  Uint32 frame = 0;
  Uint32 start = 0;


  LOG( "NotImplemented() - creating text\n" );

  s1 = black_outline( _("Work In Progress!"), font, &white);
  s2 = black_outline( _("This feature is not ready yet"), font, &white);
  s3 = black_outline( _("Discuss the future of TuxMath at"), font, &white);

  /* we always want the URL in english */
  if (!useEnglish)
  {
    TTF_Font *english_font;
    useEnglish = 1;
    english_font = LoadFont( menu_font, menu_font_size );
    s4 = black_outline( "tuxmath-devel@lists.sourceforge.net", english_font, &white);
    TTF_CloseFont(english_font);
    useEnglish = 0;
  }
  else 
  {
    s4 = black_outline( "tuxmath-devel@lists.sourceforge.net", font, &white);
  }

  LOG( "NotImplemented() - drawing screen\n" );

  /* Draw lines of text: */
  SDL_BlitSurface( bkg, NULL, screen, NULL );
  loc.x = 320-(s1->w/2); loc.y = 10;
  SDL_BlitSurface( s1, NULL, screen, &loc);
  loc.x = 320-(s2->w/2); loc.y = 60;
  SDL_BlitSurface( s2, NULL, screen, &loc);
  loc.x = 320-(s3->w/2); loc.y = 400;
  SDL_BlitSurface( s3, NULL, screen, &loc);
  loc.x = 320-(s4->w/2); loc.y = 440;
  SDL_BlitSurface( s4, NULL, screen, &loc);

  //  FIXME make Tux blink correctly.
  tux = LoadSprite("tux/bigtux", IMG_ALPHA);
  if (tux && tux->num_frames) /* make sure sprite has at least one frame */
  {
    SDL_BlitSurface(tux->frame[0], NULL, screen, &Tuxdest);
  }
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
          //exit(0);
        }

        case SDL_MOUSEBUTTONDOWN:
        case SDL_KEYDOWN:
        {
          finished = 1;
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

    if (tux_frame)
    {
      SDL_BlitSurface(tux->frame[tux_frame - 1], NULL, screen, &Tuxdest);
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
  FreeSprite(tux);
}




/* choose_config_file() - adapted from chooseWordlist() from tuxtype. */
/* Display a list of tuxmath config files in the missions directory   */
/* and allow the player to pick one.                                  */
/* FIXME the directory search and list generation belongs in fileops.c */
/* FIXME set up frame counter so Tux blinks.                           */

/* returns 0 if user pressed escape (or if dir not found)
 *         1 if config was set correctly
 */
int choose_config_file(void)
{
  SDL_Surface *titles[MAX_LESSONS];
  SDL_Surface *select[MAX_LESSONS];
  SDL_Surface* lesson_title;
  SDL_Surface *left, *right, *left_gray, *right_gray;
  SDL_Rect leftRect, rightRect;
  SDL_Rect titleRects[8];               //8 lessons displayed per page 
  SDL_Rect lesson_menu_button[8];      // Translucent mouse "buttons"

  Uint32 frame = 0;                             //For keeping frame rate constant 
  Uint32 frame_start = 0;
  int stop = 0;
  int loc = 0;                                  //The currently selected lesson file
  int old_loc = 1;
  int lessons = 0;                              //Number of lesson files found in dir
  int i;
  int length;
  sprite* tux;
  int tux_frame;
  int click_flag = 1;

  unsigned char lesson_path[FNLEN];             //Path to lesson directory
  unsigned char lesson_list[MAX_LESSONS][200];  //List of lesson file names
  unsigned char lesson_names[MAX_LESSONS][200]; //List of lesson names for display
  unsigned char name_buf[200];

  DIR* lesson_dir;
  struct dirent* lesson_file;
  struct stat fileStats;
  FILE* tempFile;

  LOG("Entering choose_config_file():\n");

  /* find the directory containing the lesson files:  */

  for (i = useEnglish; i < 2; i++)
  {
    fileStats.st_mode = 0; // clear last use!
    sprintf( lesson_path, "%s/missions/lessons", realPath[i] );

#ifdef TUXMATH_DEBUG
    fprintf(stderr, "Checking path: %s\n", lesson_path);
#endif

    stat(lesson_path, &fileStats);

    if (fileStats.st_mode & S_IFDIR)
    {
#ifdef TUXMATH_DEBUG
      fprintf(stderr, "Path found\n");
#endif
      break;
    }
    else
    {
#ifdef TUXMATH_DEBUG
      fprintf(stderr, "Path NOT found\n");
#endif
    }
  }

  if (i==2)
  {
    fprintf(stderr, "ERROR: Unable to find lesson directory\n");
    return 0;
//		exit(1);  // FIXME if exiting, need to restore screen resolution, cleanup heap, etc.
  }

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "lesson_path is: %s\n", lesson_path);
#endif

  /* create a list of all the lesson files */
  lesson_dir = opendir(lesson_path);	

  do
  {
    /* readdir() returns ptr to next file in dir AND resets ptr to following file: */
    lesson_file = readdir(lesson_dir);
    /* Get out when no more files: */
    if (!lesson_file)
    {
      break;
    }

    /* file names must begin with 'lesson' (case-insensitive) */
    if (0 != strncasecmp(&lesson_file->d_name, "lesson", 6))
    { 
      continue;
    }

    /* FIXME Should somehow test each file to see if it is a tuxmath config file */
    /* Put file name into array of names found in lesson directory */
    sprintf(lesson_list[lessons], "%s/%s", lesson_path, lesson_file->d_name);

#ifdef TUXMATH_DEBUG
    fprintf(stderr, "Found lesson file %d:\t%s\n", lessons, lesson_list[lessons]);
#endif

    /* load the name for the lesson from the file ... (1st line) */
    tempFile = fopen(lesson_list[lessons], "r");

    if (tempFile==NULL)
    {
      /* By leaving the loop without incrementing 'lessons', */
      /* the bad file name will get clobbered next time through: */
      continue;
    }

    /* FIXME I think the following could overflow: */
    fscanf(tempFile, "%[^\n]\n", name_buf);

    /* check to see if it has a \r at the end of it (dos format!) */
    length = strlen(name_buf);
    if (name_buf[length - 1] == '\r')
    {
      name_buf[length - 1] = '\0';
    }

    /* Go past leading '#', ';', or whitespace: */
    /* NOTE the value of i on exit is the main goal of the loop */
    for (  i = 0;
           ((name_buf[i] == '#') ||
           (name_buf[i] == ';') ||
           isspace(name_buf[i])) &&
           (i < 200);
           i++  )
    {
      length--;
    }
    /* Now copy the rest of the first line into the list: */
    memmove(&lesson_names[lessons], &name_buf[i], length); 
    lessons++;
    fclose(tempFile);
  } while (1);        // Loop will end when 'break' encountered

  closedir(lesson_dir);	


  /* Display the list of lessons for the player to select: */

  /* FIXME black_outline() segfaults if passed "" as arg */
  for (i = 0; i < lessons; i++)
  {
    titles[i] = black_outline( lesson_names[i], font, &white );
    select[i] = black_outline( lesson_names[i], font, &yellow);
  }

  SDL_FreeSurface(bkg);
  bkg = LoadImage("title/main_bkg.jpg", IMG_REGULAR);

  /* Put arrow buttons in right lower corner, inset by 20 pixels */
  /* with a 10 pixel space between:                              */
  right = LoadImage("title/right.png", IMG_ALPHA);
  right_gray = LoadImage("title/right_gray.png", IMG_ALPHA);
  rightRect.w = right->w;
  rightRect.h = right->h;
  rightRect.x = screen->w - right->w - 20;
  rightRect.y = screen->h - right->h - 20;

  left = LoadImage("title/left.png", IMG_ALPHA);       
  left_gray = LoadImage("title/left_gray.png", IMG_ALPHA);       
  leftRect.w = left->w;
  leftRect.h = left->h;
  leftRect.x = rightRect.x - 10 - left->w;
  leftRect.y = screen->h - left->h - 20;

  tux = LoadSprite("tux/bigtux", IMG_ALPHA);
  lesson_title = LoadImage("title/title1.png", IMG_ALPHA);

  /* set initial title rect sizes */
  titleRects[0].y = 20;
  titleRects[0].w = titleRects[0].h = titleRects[0].x = 0;

  for (i = 1; i < 8; i++)
  { 
    titleRects[i].y = titleRects[i - 1].y + 55;
    titleRects[i].w = titleRects[i].h = titleRects[i].x = 0;
  }

  /* Set up background, title, and Tux: */
  SDL_BlitSurface(bkg, NULL, screen, NULL);
  SDL_BlitSurface(lesson_title, NULL, screen, &Titledest);
  SDL_BlitSurface(tux->frame[0], NULL, screen, &Tuxdest);
  SDL_UpdateRect(screen, 0, 0, 0 ,0);

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
          for (i = 0; (i < 8) && (loc -(loc % 8) + i < lessons); i++)
          {
            if (inRect(lesson_menu_button[i], event.motion.x, event.motion.y))
            {
              // Play sound if loc is being changed:
              if (Opts_MenuSound() && (loc != loc - (loc % 8) + i)) 
              {
                tuxtype_playsound(snd_move);
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
                tuxtype_playsound(snd_move);
                click_flag = 0;
              }
              break;
            }
          }

          /* "Right" button - go to next page: */
          else if (inRect( rightRect, event.motion.x, event.motion.y ))
          {
            if (loc - (loc % 8) + 8 < lessons)
            {
              if (Opts_MenuSound() && click_flag)
              {
                tuxtype_playsound(snd_move);
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
          for (i = 0; (i < 8) && (loc - (loc % 8) + i < lessons); i++)
          {
            if (inRect(lesson_menu_button[i], event.button.x, event.button.y))
            {
              if (Opts_MenuSound())
              {
                  tuxtype_playsound(snd_select);
              }

              loc = loc - (loc % 8) + i;

              if (read_named_config_file(lesson_list[loc]))
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
              }
              else  // Something went wrong - could not read config file:
              {
                fprintf(stderr, "\nCould not find file: %s\n", lesson_list[loc]);
              }

              stop = 1;
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
                tuxtype_playsound(snd_move);
              }
              break;
            }
          }

          /* "Right" button - go to next page: */
          if (inRect( rightRect, event.button.x, event.button.y ))
          {
            if (loc - (loc % 8) + 8 < lessons)
            {
              loc = loc - (loc % 8) + 8;
              if (Opts_MenuSound())
              {
                tuxtype_playsound(snd_move);
              }
              break;
            }
          }
        }


        case SDL_KEYDOWN:
        {
          // TODO could make these a switch/case statement
          if (event.key.keysym.sym == SDLK_ESCAPE)
          { 
            stop = 2;
            break;
          }

          /* Selecting lesson game! */
          if ((event.key.keysym.sym == SDLK_RETURN)
           || (event.key.keysym.sym == SDLK_SPACE)
           || (event.key.keysym.sym == SDLK_KP_ENTER))
          {
            if (Opts_MenuSound())
            {
              tuxtype_playsound(snd_select);
            }

            if (read_named_config_file(lesson_list[loc]))
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
            }
            else  // Something went wrong - could not read config file:
            {
              fprintf(stderr, "\nCould not find file: %s\n", lesson_list[loc]);
            }

            stop = 1;
            break;
          }


          /* Go to previous page, if present: */
          if ((event.key.keysym.sym == SDLK_LEFT) || (event.key.keysym.sym == SDLK_PAGEUP))
          {
            if (Opts_MenuSound())
            {
              tuxtype_playsound(snd_move);
            }

            if (loc - (loc % 8) - 8 >= 0)
            {
              loc = loc - (loc % 8) - 8;
            }
          }

          /* Go to next page, if present: */
          if ((event.key.keysym.sym == SDLK_RIGHT) || (event.key.keysym.sym == SDLK_PAGEDOWN))
          {
            if (Opts_MenuSound())
            {
              tuxtype_playsound(snd_move);
            }

            if (loc - (loc % 8) + 8 < lessons)
            {
              loc = (loc - (loc % 8) + 8);
            }
          }

          /* Go up one entry, if present: */
          if (event.key.keysym.sym == SDLK_UP)
          {
            if (Opts_MenuSound())
            {
              tuxtype_playsound(snd_move);
            }

            if (loc > 0)
            {
              loc--;
            }
          }

          /* Go down one entry, if present: */
          if (event.key.keysym.sym == SDLK_DOWN)
          {
            if (Opts_MenuSound())
            {
              tuxtype_playsound(snd_move);
            }

            if (loc + 1 < lessons)
            {
              loc++;
            }
          }
        }  // End of key handling
      }  // End switch statement
    }  // End SDL_PollEvent while loop

    /* Redraw screen: */
    if (old_loc != loc) 
    {
      int start;
      start = loc - (loc % 8);

      /* Redraw background, title, and Tux: */
      SDL_BlitSurface(bkg, NULL, screen, NULL);
      SDL_BlitSurface(lesson_title, NULL, screen, &Titledest);
      SDL_BlitSurface(tux->frame[0], NULL, screen, &Tuxdest);

      /* FIXME get rid of "evil" macro ;)       */
      for (i = start; i < MIN(start+8,lessons); i++)
      {
        titleRects[i % 8].x = 320 - (titles[i]->w/2); //Center in screen
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
        SDL_BlitSurface( left, NULL, screen, &leftRect );
      }
      else  /* Draw grayed-out left button: */
      {
        SDL_BlitSurface( left_gray, NULL, screen, &leftRect );
      }

      if (start + 8 < lessons)  // not on last page
      {
        SDL_BlitSurface( right, NULL, screen, &rightRect );
      }
      else  /* Draw grayed-out right button: */
      {
        SDL_BlitSurface( right_gray, NULL, screen, &rightRect );
      }

      SDL_UpdateRect(screen, 0, 0, 0 ,0);
    }

    old_loc = loc;


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
      SDL_BlitSurface(tux->frame[tux_frame - 1], NULL, screen, &Tuxdest);
      SDL_UpdateRect(screen, Tuxdest.x+37, Tuxdest.y+40, 70, 45);
    }

    /* Wait so we keep frame rate constant: */
    while ((SDL_GetTicks() - frame_start) < 33)
    {
      SDL_Delay(20);
    }
    frame++;
  }  // End !stop while loop

  /* --- clear graphics before leaving function --- */ 
  for (i = 0; i < lessons; i++)
  {
    SDL_FreeSurface(titles[i]);
    SDL_FreeSurface(select[i]);
  }

  SDL_FreeSurface(left);
  SDL_FreeSurface(right);
  SDL_FreeSurface(left_gray);
  SDL_FreeSurface(right_gray);
  SDL_FreeSurface(lesson_title);
  FreeSprite(tux);

  DEBUGCODE { fprintf( stderr, "Leaving choose_config_file();\n" ); }

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
