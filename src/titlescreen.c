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
    Also significantly enhanced by Tim Holy - 2007
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
#include "SDL_extras.h"

/* --- Data Structure for Dirty Blitting --- */
SDL_Rect srcupdate[MAX_UPDATES];
SDL_Rect dstupdate[MAX_UPDATES];
int numupdates = 0; // tracks how many blits to be done

// Colors we use:
SDL_Color black;
SDL_Color gray;
SDL_Color dark_blue;
SDL_Color red;
SDL_Color white;
SDL_Color yellow;

// Type needed for TransWipe():
struct blit {
    SDL_Surface *src;
    SDL_Rect *srcrect;
    SDL_Rect *dstrect;
    unsigned char type;
} blits[MAX_UPDATES];

// Lessons available for play
unsigned char **lesson_list_titles = NULL;
unsigned char **lesson_list_filenames = NULL;
int num_lessons = 0;


/* --- media for menus --- */

enum {
  SPRITE_TRAINING,
  SPRITE_ARCADE,
  SPRITE_HELP,
  SPRITE_CUSTOM,
  SPRITE_OPTIONS,
  SPRITE_CADET,
  SPRITE_SCOUT,
  SPRITE_RANGER,
  SPRITE_ACE,
  SPRITE_QUIT,
  SPRITE_MAIN,
  SPRITE_GOLDSTAR,
  SPRITE_NO_GOLDSTAR,
  SPRITE_TROPHY,
  SPRITE_CREDITS,
  N_SPRITES};

const unsigned char* menu_sprite_files[N_SPRITES] =
{
  "lesson",
  "comet",
  "help",
  "tux_config",
  "tux_config_brown",
  "tux_helmet_yellow",
  "tux_helmet_green",
  "tux_helmet_blue",
  "tux_helmet_red",
  "quit",
  "main",
  "goldstar",
  "no_goldstar",
  "trophy",
  "credits"
};
   
sprite **sprite_list = NULL;

/* reg and sel are used to create the translucent button backgrounds. */
sprite* Tux = NULL;


SDL_Event event;

/* --- locations we need --- */

SDL_Rect dest,
	 Tuxdest,
	 Titledest,
         stopRect,
	 cursor;

/* Local function prototypes: */
void TitleScreen_load_menu(void);
void TitleScreen_unload_menu(void);
int TitleScreen_load_media(void);
void TitleScreen_unload_media(void);
void NotImplemented(void);
void TransWipe(SDL_Surface* newbkg, int type, int var1, int var2);
void UpdateScreen(int* frame);
void AddRect(SDL_Rect* src, SDL_Rect* dst);
void InitEngine(void);
void ShowMessage(char* str1, char* str2, char* str3, char* str4);
void set_buttons_max_width(SDL_Rect *,int);
int run_main_menu(void);
int run_arcade_menu(void);
int run_custom_menu(void);
int run_options_menu(void);
int run_lessons_menu(void);




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

  Uint32 start = 0;

  int i; 
  int n_subdirs;
  char **subdir_names;


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
  if (TitleScreen_load_media() == 0) {
    fprintf(stderr,"Media was not properly loaded, exiting");
    return;
  }

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

  /* If necessary, have the user log in */
  if (run_login_menu() != -1) {
    /* Finish parsing user options */
    initialize_options_user();
    /* Start the main menu */
    run_main_menu();
  }

  /* User has selected quit, clean up */

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "->>Freeing title screen images\n");
#endif

  TitleScreen_unload_media();

#ifdef TUXMATH_DEBUG
  fprintf(stderr,"->TitleScreen():END \n");
#endif

}




/***********************************************************/
/*                                                         */
/*    "Private functions" (callable only from this file)   */
/*                                                         */
/***********************************************************/


// 1 = success, 0 = failure
int TitleScreen_load_media(void)
{
  char fn[PATH_MAX];
  int i;


#ifdef TUXMATH_DEBUG
  fprintf(stderr, "Entering TitleScreen_load_media():\n");
#endif

  Tux = LoadSprite("tux/bigtux", IMG_ALPHA);

  SDL_ShowCursor(1);

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "loading sprites\n");
#endif

  sprite_list = (sprite**) malloc(N_SPRITES*sizeof(sprite*));
  if (sprite_list == NULL)
    return 0;
    
  for (i = 0; i < N_SPRITES; i++) {
    /* --- load animated icon for menu item --- */
    sprintf(fn, "sprites/%s", menu_sprite_files[i]);
    sprite_list[i] = LoadSprite(fn, IMG_ALPHA);
  }
  return 1;
}




void TitleScreen_unload_menu(void)
{
  int i;

  for (i = 0; i < N_SPRITES; i++)
    FreeSprite(sprite_list[i]);
  free(sprite_list);
  sprite_list = NULL;
}



void TitleScreen_unload_media(void)
{
  FreeSprite(Tux);
  Tux = NULL;
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
    s1 = BlackOutline(str1, default_font, &white);
  if (str2)
    s2 = BlackOutline(str2, default_font, &white);
  if (str3)
    s3 = BlackOutline(str3, default_font, &white);
  /* When we get going with i18n may need to modify following - see below: */
  if (str4)
    s4 = BlackOutline(str4, default_font, &white);

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
    //loc.x = 320 - (s3->w/2); loc.y = 300;
    loc.x = 320 - (s3->w/2); loc.y = 110;
    SDL_BlitSurface( s3, NULL, screen, &loc);
  }
  if (s4)
  {
    //loc.x = 320 - (s4->w/2); loc.y = 340;
    loc.x = 320 - (s4->w/2); loc.y = 200;
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
            playsound(SND_TOCK);
            break;
          }
        }
        case SDL_KEYDOWN:
        {
          finished = 1;
          playsound(SND_TOCK);
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
      SDL_UpdateRect(screen, Tuxdest.x, Tuxdest.y, Tuxdest.w, Tuxdest.h);

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


int run_main_menu(void)
{
  const unsigned char* menu_text[6] =
    {(const unsigned char*)N_("Math Command Training Academy"),
     (const unsigned char*)N_("Play Arcade Game"),
     (const unsigned char*)N_("Play Custom Game"),
     (const unsigned char*)N_("Help"),
     (const unsigned char*)N_("More Options"),
     (const unsigned char*)N_("Quit")};
  sprite* sprites[6] =
    {NULL, NULL, NULL, NULL, NULL, NULL};
  menu_options menu_opts;
  int choice,ret;

  // Set up the sprites
  sprites[0] = sprite_list[SPRITE_TRAINING];
  sprites[1] = sprite_list[SPRITE_ARCADE];
  sprites[2] = sprite_list[SPRITE_CUSTOM];
  sprites[3] = sprite_list[SPRITE_HELP];
  sprites[4] = sprite_list[SPRITE_OPTIONS];
  sprites[5] = sprite_list[SPRITE_QUIT];

  set_default_menu_options(&menu_opts);
  menu_opts.ytop = 100;
  menu_opts.ygap = 15;

  choice = choose_menu_item(menu_text,sprites,6,menu_opts);
  
  while (choice >= 0) {
    switch (choice) {
      case 0: {
	// Training academy lessons
	ret = run_lessons_menu();
	break;
      }
      case 1: {
	// Arcade games
	ret = run_arcade_menu();
	break;
      }
      case 2: {
	// Custom game
	ret = run_custom_menu();
	break;
      }
      case 3: {
	// Help
	Opts_SetHelpMode(1);
	Opts_SetDemoMode(0);
	if (Opts_MenuMusic())  //Turn menu music off for game
	  {audioMusicUnload();}
	game();
	if (Opts_MenuMusic()) //Turn menu music back on
	  {audioMusicLoad( "tuxi.ogg", -1 );}
	Opts_SetHelpMode(0);
	break;
      }
      case 4: {
	// More options
	ret = run_options_menu();
        break;
      }
      case 5: {
	// Quit
        return 0;
      }
    }
    menu_opts.starting_entry = choice;
    choice = choose_menu_item(menu_text,sprites,6,menu_opts);
  }
  return 0;
}

int run_arcade_menu(void)
{
  const unsigned char* menu_text[6] =
    {(const unsigned char*)N_("Space Cadet"),
     (const unsigned char*)N_("Scout"),
     (const unsigned char*)N_("Ranger"),
     (const unsigned char*)N_("Ace"),
     (const unsigned char*)N_("Hall Of Fame"),
     (const unsigned char*)N_("Main menu")};
  const char* arcade_config_files[4] =
    {"arcade/space_cadet", "arcade/scout", "arcade/ranger", "arcade/ace"};
  const int arcade_high_score_tables[4] =
    {CADET_HIGH_SCORE,SCOUT_HIGH_SCORE,RANGER_HIGH_SCORE,ACE_HIGH_SCORE};
  sprite* sprites[6] =
    {NULL, NULL, NULL, NULL, NULL, NULL};
  menu_options menu_opts;
  int choice,hs_table;

  // Set up the sprites
  sprites[0] = sprite_list[SPRITE_CADET];
  sprites[1] = sprite_list[SPRITE_SCOUT];
  sprites[2] = sprite_list[SPRITE_RANGER];
  sprites[3] = sprite_list[SPRITE_ACE];
  sprites[4] = sprite_list[SPRITE_TROPHY];
  sprites[5] = sprite_list[SPRITE_MAIN];

  set_default_menu_options(&menu_opts);
  menu_opts.ytop = 100;

  choice = choose_menu_item(menu_text,sprites,6,menu_opts);

  while (choice >= 0) {
    if (choice < 4) {
      // Play arcade game
      if (read_named_config_file(arcade_config_files[choice]))
      {
	audioMusicUnload();
	game();
	if (Opts_MenuMusic()) {
	  audioMusicLoad( "tuxi.ogg", -1 );
	}
	/* See if player made high score list!                        */
	hs_table = arcade_high_score_tables[choice];
	if (check_score_place(hs_table, Opts_LastScore()) < HIGH_SCORES_SAVED){
          
	  unsigned char player_name[HIGH_SCORE_NAME_LENGTH * 3];
	  
	  /* Get name from player: */
	  HighScoreNameEntry(&player_name[0]);
	  insert_score(player_name, hs_table, Opts_LastScore());
	  DisplayHighScores(hs_table);
	  /* save to disk: */
	  write_high_scores();
	  
#ifdef TUXMATH_DEBUG
	  print_high_scores(stderr);
#endif 
	}
      } else {
	fprintf(stderr, "\nCould not find %s config file\n",arcade_config_files[choice]);
      }

    } else if (choice == 4) {
      // Display the Hall of Fame
      DisplayHighScores(CADET_HIGH_SCORE);
    }
    else {
      // Return to main menu
      return 0;
    }

    menu_opts.starting_entry = choice;
    choice = choose_menu_item(menu_text,sprites,6,menu_opts);
  }

  return 0;
}


int run_custom_menu(void)
{
  char *s1, *s2, *s3, *s4;
  s1 = _("Edit 'options' file in your home directory");
  s2 = _("to create customized game!");
  s3 = _("Press a key or click your mouse to start game.");
  s4 = N_("See README.txt for more information");
  ShowMessage(s1, s2, s3, s4);

  if (read_user_config_file()) {
    if (Opts_MenuMusic())
      audioMusicUnload();

    game();
    write_user_config_file();

    if (Opts_MenuMusic())
      audioMusicLoad( "tuxi.ogg", -1 );
  }

  return 0;
}

int run_options_menu(void)
{
  /*
    // Use the following version if we get "Settings" implemented
  const unsigned char* menu_text[5] =
    {(const unsigned char*)N_("Settings"),
     (const unsigned char*)N_("Demo"),
     (const unsigned char*)N_("Credits"),
     (const unsigned char*)N_("Project Info"),
     (const unsigned char*)N_("Main Menu")};
  sprite* sprites[5] =
    {NULL, NULL, NULL, NULL, NULL};
  */
  const unsigned char* menu_text[4] =
    {(const unsigned char*)N_("Demo"),
     (const unsigned char*)N_("Project Info"),
     (const unsigned char*)N_("Credits"),
     (const unsigned char*)N_("Main Menu")};
  sprite* sprites[4] =
    {NULL, NULL, NULL, NULL};
  int n_menu_entries = 4;
  menu_options menu_opts;
  int choice;

  // Set up the sprites
  sprites[0] = sprite_list[SPRITE_ARCADE];
  sprites[1] = sprite_list[SPRITE_HELP];
  sprites[2] = sprite_list[SPRITE_CREDITS];
  sprites[3] = sprite_list[SPRITE_MAIN];

  set_default_menu_options(&menu_opts);
  menu_opts.ytop = 100;

  choice = choose_menu_item(menu_text,sprites,n_menu_entries,menu_opts);

  while (choice >= 0) {
    switch (choice) {
      /*
    case 0: {
      // Settings
      NotImplemented();
      break;
      }*/
    case 0: {
      // Demo
      if (read_named_config_file("demo"))
      {
	audioMusicUnload();
	game();
	if (Opts_MenuMusic()) {
	  audioMusicLoad( "tuxi.ogg", -1 );
	}
      } else {
	fprintf(stderr, "\nCould not find demo config file\n");
      }

      break;
    }
    case 1: {
      // Project Info
      //NotImplemented();
      ShowMessage(_("TuxMath is free and open-source!"),
		  _("You can help make it better by reporting problems,"),
		  _("suggesting improvements, or adding code."),
		  _("Discuss the future at tuxmath-devel@lists.sourceforge.net"));
      break;
    }
    case 2: {
      // Credits
      //TitleScreen_unload_media();
      credits();
      //TitleScreen_load_media();
      break;
    }
    case 3: {
      // Main menu
      return 0;
    }
    }

    menu_opts.starting_entry = choice;
    choice = choose_menu_item(menu_text,sprites,n_menu_entries,menu_opts);
  }

  return 0;
}


/* Display a list of tuxmath config files in the missions directory   */
/* and allow the player to pick one (AKA "Lessons").                  */

/* returns 0 if user pressed escape
 *         1 if config was set correctly
 */
int run_lessons_menu(void)
{
  int chosen_lesson = -1;
  menu_options menu_opts;
  sprite** star_sprites = NULL;

  /* Set up sprites (as long as gold star list is valid) */
  if (lesson_list_goldstars != NULL)
  {
    int i;
    star_sprites = (sprite**)malloc(num_lessons * sizeof(sprite*));
    for (i = 0; i < num_lessons; i++)
    {
      if (lesson_list_goldstars[i])
        star_sprites[i] = sprite_list[SPRITE_GOLDSTAR];
      else
        star_sprites[i] = sprite_list[SPRITE_NO_GOLDSTAR];
    }
  }
  set_default_menu_options(&menu_opts);

  chosen_lesson = choose_menu_item((const unsigned char**)lesson_list_titles, star_sprites, num_lessons, menu_opts);

  while (chosen_lesson >= 0) 
  {
    if (Opts_MenuSound())
      playsound(SND_POP);
    
    /* Re-read global settings first in case any settings were */
    /* clobbered by other lesson or arcade games this session: */
    read_global_config_file();
    
    /* Now read the selected file and play the "mission": */ 
    if (read_named_config_file(lesson_list_filenames[chosen_lesson]))
    {
      if (Opts_MenuMusic())  //Turn menu music off for game
        {audioMusicUnload();}

      game();

      /* If successful, display Gold Star for this lesson! */
      if (MC_MissionAccomplished())
      {
        lesson_list_goldstars[chosen_lesson] = 1;
        star_sprites[chosen_lesson] = sprite_list[SPRITE_GOLDSTAR];
       /* and save to disk: */
        write_goldstars();
      }

      if (Opts_MenuMusic()) //Turn menu music back on
        {audioMusicLoad("tuxi.ogg", -1);}
    }
    else  // Something went wrong - could not read lesson config file:
    {
      fprintf(stderr, "\nCould not find file: %s\n", lesson_list_filenames[chosen_lesson]);
      chosen_lesson = -1;
    }
    // Let the user choose another lesson; start with the screen and
    // selection that we ended with
    menu_opts.starting_entry = chosen_lesson;
    chosen_lesson = choose_menu_item((const unsigned char**)lesson_list_titles, star_sprites, num_lessons, menu_opts);
  }
  if (star_sprites)
  {
    free(star_sprites);
    star_sprites = NULL;
  }

  if (chosen_lesson < 0)
    return 0;
  else
    return 1;
}


/* Sets the user home directory in a tree of possible users     */
/* -1 indicates that the user wants to quit without logging in, */
/* 0 indicates that a choice has been made.                     */
int run_login_menu(void)
{
  int n_login_questions = 0;
  char **user_login_questions = NULL;
  int n_users = 0;
  char **user_names = NULL;
  
  menu_options menu_opts;
  int chosen_login = -1;
  char *user_home;
  int level;
  int i;
  char *trailer_quit = "Quit";
  char *trailer_back = "Back";

  // Check for & read user_login_questions file
  n_login_questions = read_user_login_questions(&user_login_questions);

  // Check for & read user_menu_entries file
  n_users = read_user_menu_entries(&user_names);
  
  if (n_users == 0)
    return 0;   // a quick exit, there's only one user

  level = 0;
  user_home = get_user_data_dir();
  set_default_menu_options(&menu_opts);
  if (n_login_questions > 0)
    menu_opts.title = user_login_questions[0];
  menu_opts.trailer = trailer_quit;

  while (n_users) {
    // Get the user choice
    chosen_login = choose_menu_item(user_names, NULL, n_users, menu_opts);
    if (chosen_login == -1 || chosen_login == n_users) {
      // User pressed escape or selected Quit/Back, handle by quitting
      // or going up a level
      if (level == 0) {
	// We are going to quit without logging in. So, we don't have
	// to worry about cleaning up memory.
	return -1;
      }
      else {
	// Go back up one level of the directory tree
	dirname_up(user_home);
	level--;
	menu_opts.starting_entry = -1;
      }
    }
    else {
      // User chose an entry, set it up
      strcat(user_home,user_names[chosen_login]);
      strcat(user_home,"/");
      level++;
      menu_opts.starting_entry = 0;
    }
    // Free the entries from the previous menu
    for (i = 0; i < n_users; i++)
      free(user_names[i]);
    free(user_names);
    user_names = NULL;
    // Set the title appropriately for the next menu
    if (level < n_login_questions)
      menu_opts.title = user_login_questions[level];
    else
      menu_opts.title = NULL;
    if (level == 0)
      menu_opts.trailer = trailer_quit;
    else
      menu_opts.trailer = trailer_back;
    // Check to see if there are more choices to be made
    n_users = read_user_menu_entries(&user_names);
  }

  // The user home directory is set, signal success
  return 0;
}


/****************************************************************/
/* choose_menu_item: menu navigation utility function           */
/* (the function returns the index for the selected menu item)  */
/* -1 indicates that the user pressed escape                    */
/****************************************************************/
int choose_menu_item(const unsigned char **menu_text, sprite **menu_sprites, int n_menu_entries, menu_options menu_opts)
{
  // Pixel renderings of menu text choices
  SDL_Surface **menu_item_unselected = NULL;
  SDL_Surface **menu_item_selected = NULL;
  // Display region for menu choices
  SDL_Rect *menu_text_rect = NULL;
  // Translucent mouse "buttons" around menu text
  SDL_Rect *menu_button_rect = NULL;
  // Menu sprite locations
  SDL_Rect *menu_sprite_rect = NULL;

  SDL_Rect left_arrow_rect, right_arrow_rect;

  Uint32 frame_counter = 0;
  Uint32 frame_start = 0;       //For keeping frame rate constant 
  Uint32 frame_now = 0;
  int stop = 0;
  int loc = 0;                  //The currently selected menu item
  int old_loc = 1;
  int loc_screen_start = 0;     //The number of the top entry on current screen
  int old_loc_screen_start = 0;
  int redraw = 0;
  int n_entries_per_screen = 0;
  int buttonheight = 0;
  int i = 0;
  int imod = 0;                 // i % n_entries_per_screen
  int tux_frame = 0;
  int click_flag = 1;
  int use_sprite = 0;
  int warp_mouse = 0;
  int title_offset = 0;
  int have_trailer = 0;

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "Entering choose_menu_item():\n");
#endif

#ifdef TUXMATH_DEBUG
  fprintf(stderr,"%d menu entries:\n",n_menu_entries);
  for (i = 0; i < n_menu_entries; i++)
    fprintf(stderr,"%s\n",menu_text[i]);
#endif

  /**** Memory allocation for menu text  ****/
  title_offset = 0;
  if (menu_opts.title != NULL)
    title_offset = 1;
  if (menu_opts.trailer != NULL)
    have_trailer = 1;
  menu_item_unselected = (SDL_Surface**)malloc((n_menu_entries+title_offset+have_trailer) * sizeof(SDL_Surface*));
  menu_item_selected = (SDL_Surface**)malloc((n_menu_entries+title_offset+have_trailer) * sizeof(SDL_Surface*));
  if (menu_item_unselected == NULL || menu_item_selected == NULL) {
    free(menu_item_unselected);
    free(menu_item_selected);
    return -2;  // error
  }

  /**** Render the menu choices                               ****/
  if (title_offset)
  {
    menu_item_unselected[0] = BlackOutline( _(menu_opts.title),default_font,&red);
    // It will never be selected, so we don't have to do anything for selected.
    menu_item_selected[0] = NULL;
  }
  for (i = 0; i < n_menu_entries; i++)
  {
    menu_item_unselected[i+title_offset] = BlackOutline( _(menu_text[i]), default_font, &white );
    menu_item_selected[i+title_offset] = BlackOutline( _(menu_text[i]), default_font, &yellow);
  }
  if (have_trailer) {
    menu_item_unselected[n_menu_entries+title_offset] = BlackOutline( _(menu_opts.trailer), default_font, &white );
    menu_item_selected[n_menu_entries+title_offset] = BlackOutline( _(menu_opts.trailer), default_font, &yellow);
  }
  // We won't need the menu_text again, so now we can keep track of
  // the total entries including the title & trailer
  n_menu_entries += title_offset+have_trailer;

  /**** Calculate the menu item heights and the number of     ****/
  /**** entries per screen                                    ****/
  if (menu_opts.buttonheight <= 0) {
    buttonheight = 0;
    for (i = 0; i < n_menu_entries; i++)
      if (buttonheight < menu_item_unselected[i]->h)
	buttonheight = menu_item_unselected[i]->h;
    buttonheight += 10;
  } else
    buttonheight = menu_opts.buttonheight;

  // First try using the whole screen; if we need more than one
  // screen, then we have to save space for the arrows by respecting
  // ybottom
  n_entries_per_screen = (int) (screen->h - menu_opts.ytop+menu_opts.ygap)/(buttonheight + menu_opts.ygap);
  if (n_entries_per_screen < n_menu_entries)
    n_entries_per_screen = (int) (menu_opts.ybottom - menu_opts.ytop+menu_opts.ygap)/(buttonheight + menu_opts.ygap);

  if (n_entries_per_screen > n_menu_entries)
    n_entries_per_screen = n_menu_entries;

  /**** Memory allocation for current screen rects  ****/
  menu_text_rect = (SDL_Rect*) malloc(n_entries_per_screen * sizeof(SDL_Rect));
  menu_button_rect = (SDL_Rect*) malloc(n_entries_per_screen * sizeof(SDL_Rect));
  if (menu_text_rect == NULL || menu_button_rect == NULL) {
    free(menu_text_rect);
    free(menu_button_rect);
    return -2;
  }
  if (menu_sprites != NULL) {
    menu_sprite_rect = (SDL_Rect*) malloc(n_entries_per_screen * sizeof(SDL_Rect));
    if (menu_sprite_rect == NULL) {
      free(menu_sprite_rect);
      return -2;
    }
  }

  /**** Define the locations of graphical elements on the screen ****/
  /* Arrow buttons in right lower corner, inset by 20 pixels     */
  /* with a 10 pixel space between:                              */
  if (images[IMG_RIGHT])
  {
    right_arrow_rect.w = images[IMG_RIGHT]->w;
    right_arrow_rect.h = images[IMG_RIGHT]->h;
    right_arrow_rect.x = screen->w - images[IMG_RIGHT]->w - 20;
    right_arrow_rect.y = screen->h - images[IMG_RIGHT]->h - 20;
  }

  if (images[IMG_LEFT])
  {
    left_arrow_rect.w = images[IMG_LEFT]->w;
    left_arrow_rect.h = images[IMG_LEFT]->h;
    left_arrow_rect.x = right_arrow_rect.x - 10 - images[IMG_LEFT]->w;
    left_arrow_rect.y = screen->h - images[IMG_LEFT]->h - 20;
  }
  /* Red "Stop" circle in upper right corner to go back to main menu: */
  if (images[IMG_STOP])
  {
    stopRect.w = images[IMG_STOP]->w;
    stopRect.h = images[IMG_STOP]->h;
    stopRect.x = screen->w - images[IMG_STOP]->w;
    stopRect.y = 0;
  }

  /* Set initial menu rect sizes. The widths will change depending      */
  /* on the size of the text displayed in each rect.  Set the widths    */
  /* for the current screen of menu items.                              */
  loc = menu_opts.starting_entry + title_offset;  // Initially selected item
  loc_screen_start = loc - (loc % n_entries_per_screen);
  if (loc_screen_start < 0 || loc_screen_start*n_entries_per_screen > n_menu_entries)
    loc_screen_start = 0;  // in case starting_entry was -1 (or wasn't set)
  imod = loc-loc_screen_start;
  for (i = 0; i < n_entries_per_screen; i++)
  { 
    menu_button_rect[i].x = menu_opts.xleft;
    menu_text_rect[i].x = menu_opts.xleft + 15;  // 15 is left gap
    if (menu_sprites != NULL)
      menu_text_rect[i].x += 60;  // 40 is sprite width, 20 is gap
    if (i > 0)
      menu_text_rect[i].y = menu_text_rect[i - 1].y + buttonheight + menu_opts.ygap;
    else
      menu_text_rect[i].y = menu_opts.ytop;
    menu_button_rect[i].y = menu_text_rect[i].y-5;
    menu_text_rect[i].h = buttonheight-10;
    menu_button_rect[i].h = buttonheight;
    menu_button_rect[i].w = menu_text_rect[i].w = 0;
    if (i + loc_screen_start < n_menu_entries) {
      menu_text_rect[i].w = menu_item_unselected[i+loc_screen_start]->w;
      menu_button_rect[i].w = menu_text_rect[i].w + 30;
    }
    if (menu_sprite_rect != NULL) {
      menu_sprite_rect[i].x = menu_button_rect[i].x+3;
      menu_sprite_rect[i].y = menu_button_rect[i].y+3;
      menu_sprite_rect[i].w = 40;
      menu_sprite_rect[i].h = 50;
    }
  }
  if (menu_opts.button_same_width)
    set_buttons_max_width(menu_button_rect,n_entries_per_screen);

  /**** Draw background, title, and Tux:                            ****/
  if (images[IMG_MENU_BKG])
    SDL_BlitSurface(images[IMG_MENU_BKG], NULL, screen, NULL);
  if (images[IMG_MENU_TITLE])
    SDL_BlitSurface(images[IMG_MENU_TITLE], NULL, screen, &Titledest);
  if (Tux && Tux->frame[0])
    SDL_BlitSurface(Tux->frame[0], NULL, screen, &Tuxdest);
  SDL_UpdateRect(screen, 0, 0, 0 ,0);

  /* Move mouse to current button: */
  cursor.x = menu_button_rect[imod].x + menu_button_rect[imod].w/2;
  cursor.y = menu_button_rect[imod].y + menu_button_rect[imod].h/2;
//  SDL_WarpMouse(cursor.x, cursor.y);
  SDL_WM_GrabInput(SDL_GRAB_OFF);


  /******** Main loop:                                *********/
  redraw = 1;  // force a full redraw on first pass
  old_loc_screen_start = loc_screen_start;
  while (SDL_PollEvent(&event));  // clear pending events
  while (!stop)
  {
    frame_start = SDL_GetTicks();         /* For keeping frame rate constant.*/

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
	  loc = -1;  // By default, don't be in any entry
          for (i = 0; (i < n_entries_per_screen) && (loc_screen_start + i < n_menu_entries); i++)
          {
            if (inRect(menu_button_rect[i], event.motion.x, event.motion.y))
            {
              // Play sound if loc is being changed:
              if (Opts_MenuSound() && (old_loc != loc_screen_start + i)) 
              {
                playsound(SND_TOCK);
              }
              loc = loc_screen_start + i;
              break;   /* from for loop */
            }
          }

          /* "Left" button - make click if button active: */
          if (inRect(left_arrow_rect, event.motion.x, event.motion.y))
          {
            if (loc_screen_start - n_entries_per_screen >= 0)
            {
              if (Opts_MenuSound() && click_flag)
              {
                playsound(SND_TOCK);
                click_flag = 0;
              }
            }
            break;  /* from case switch */
          }

          /* "Right" button - go to next page: */
          else if (inRect(right_arrow_rect, event.motion.x, event.motion.y ))
          {
            if (loc_screen_start + n_entries_per_screen < n_menu_entries)
            {
              if (Opts_MenuSound() && click_flag)
              {
                playsound(SND_TOCK);
                click_flag = 0;
              }
            }
            break;  /* from case switch */
          }

          else  // Mouse outside of arrow rects - re-enable click sound:
          {
            click_flag = 1;
            break;  /* from case switch */
          }
        }

        case SDL_MOUSEBUTTONDOWN:
        {
          /* Choose a menu entry by mouse click */
          for (i = 0; (i < n_entries_per_screen) && (loc_screen_start + i < n_menu_entries); i++)
          {
            if (inRect(menu_button_rect[i], event.button.x, event.button.y))
            {
              if (Opts_MenuSound())
              {
                playsound(SND_POP);
              }

              loc = loc_screen_start + i;
	      stop = 1;
	      break;
            }
          }
        
          /* "Left" button - go to previous page: */
          if (inRect(left_arrow_rect, event.button.x, event.button.y))
          {
            if (loc_screen_start - n_entries_per_screen >= 0)
            {
              //loc = loc_screen_start - n_entries_per_screen;
	      loc_screen_start -= n_entries_per_screen;
	      loc = -1;  // nothing selected
              if (Opts_MenuSound())
              {
                playsound(SND_TOCK);
              }
              break;
            }
          }

          /* "Right" button - go to next page: */
          if (inRect( right_arrow_rect, event.button.x, event.button.y ))
          {
            if (loc_screen_start + n_entries_per_screen < n_menu_entries)
            {
              //loc = loc_screen_start + n_entries_per_screen;
	      loc_screen_start += n_entries_per_screen;
	      loc = -1;  // nothing selected
              if (Opts_MenuSound())
              {
                playsound(SND_TOCK);
              }
              break;
            }
          }

          /* "Stop" button - go to main menu: */
          if (inRect(stopRect, event.button.x, event.button.y ))
          {
            stop = 2;
            playsound(SND_TOCK);
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
                playsound(SND_POP);
	      stop = 1;
              break;
            }


            /* Go to previous page, if present: */
            case SDLK_LEFT:
            case SDLK_PAGEUP:
            {
              if (Opts_MenuSound())
                playsound(SND_TOCK);
              if (loc_screen_start - n_entries_per_screen >= 0) {
		loc_screen_start -= n_entries_per_screen;
		loc = -1;
	      }
              //  {loc = loc_screen_start - n_entries_per_screen;}
              break;
            }


            /* Go to next page, if present: */
            case SDLK_RIGHT:
            case SDLK_PAGEDOWN:
            {
              if (Opts_MenuSound())
                playsound(SND_TOCK);
              if (loc_screen_start + n_entries_per_screen < n_menu_entries) {
		loc_screen_start += n_entries_per_screen;
		loc = -1;
	      }
              //  {loc = (loc_screen_start + n_entries_per_screen);}
              break; 
            }

            /* Go up one entry, if present: */
            case SDLK_UP:
            {
              if (Opts_MenuSound())
                playsound(SND_TOCK);
              if (loc > title_offset)
                {loc--;}
	      else if (n_menu_entries <= n_entries_per_screen) {
		loc = n_menu_entries-1;  // wrap around if only 1 screen
	      }
	      else if (loc == -1 && loc_screen_start > 0) {
		loc = loc_screen_start-1;
		loc_screen_start -= n_entries_per_screen;
	      }
	      if (loc != old_loc)
		warp_mouse = 1;
              break;
            }


            /* Go down one entry, if present: */
            case SDLK_DOWN:
            {
              if (Opts_MenuSound())
                playsound(SND_TOCK);
              if (loc >= 0 && loc + 1 < n_menu_entries)
                {loc++;}
	      else if (n_menu_entries <= n_entries_per_screen)
		loc = title_offset;       // wrap around if only 1 screen
	      else if (loc == -1)
		loc = loc_screen_start;
	      if (loc != old_loc)
		warp_mouse = 1;
              break; 
           }


            /* Toggle screen mode: */
            case SDLK_F10: 
            {
              SwitchScreenMode();
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


    // Make sure the menu title is not selected
    if (loc == 0 && title_offset)
      loc = title_offset;

    /* Redraw screen: */
    if (loc >= 0)
      loc_screen_start = loc - (loc % n_entries_per_screen);
    if (old_loc_screen_start != loc_screen_start) 
      redraw = 1;
    if (redraw)
    {
      /* This is a full-screen redraw */
      /* Redraw background, title, stop button, and Tux: */
      if (images[IMG_MENU_BKG])
        SDL_BlitSurface(images[IMG_MENU_BKG], NULL, screen, NULL); 
      if (images[IMG_MENU_TITLE])
        SDL_BlitSurface(images[IMG_MENU_TITLE], NULL, screen, &Titledest);
      if (images[IMG_STOP])
        SDL_BlitSurface(images[IMG_STOP], NULL, screen, &stopRect);
      if (Tux->frame[0])
	SDL_BlitSurface(Tux->frame[0], NULL, screen, &Tuxdest);

      /* Redraw the menu entries */
      for (imod = 0; imod < n_entries_per_screen; imod++)
	menu_button_rect[imod].w = 0;  // so undrawn buttons don't affect width
      for (i = loc_screen_start, imod = 0; i < loc_screen_start+n_entries_per_screen && i < n_menu_entries; i++, imod++) {
	menu_text_rect[imod].w = menu_item_unselected[i]->w;
	if (i >= title_offset) {
	  menu_button_rect[imod].w = menu_text_rect[imod].w + 30;
	  if (menu_sprites != NULL)
	    menu_button_rect[imod].w += 60;
	}
      }

      if (menu_opts.button_same_width)
	set_buttons_max_width(menu_button_rect,n_entries_per_screen);
      // Make sure the menu title mouse button didn't get turned on
      if (loc_screen_start == 0 && title_offset)
	menu_button_rect[0].w = 0;

      for (i = loc_screen_start, imod = 0; i < loc_screen_start+n_entries_per_screen && i < n_menu_entries; i++, imod++) {
	if (i == loc) {  //Draw text in yellow
	  DrawButton(&menu_button_rect[imod], 10, SEL_RGBA);
	  SDL_BlitSurface(menu_item_selected[loc], NULL, screen, &menu_text_rect[imod]);
	}
	else {          //Draw text in white
	  if (menu_button_rect[imod].w > 0)
	    DrawButton(&menu_button_rect[imod], 10, REG_RGBA);
	  SDL_BlitSurface(menu_item_unselected[i], NULL, screen, &menu_text_rect[imod]);
	}
	if (menu_sprites != NULL && (i >= title_offset) && menu_sprites[i-title_offset] != NULL)
	  SDL_BlitSurface(menu_sprites[i-title_offset]->default_img, NULL, screen, &menu_sprite_rect[imod]);
      }

      /* --- draw 'left' and 'right' buttons --- */
      if (n_menu_entries > n_entries_per_screen) {
	if (loc_screen_start > 0)        // i.e. not on first page
	{
	    SDL_BlitSurface(images[IMG_LEFT], NULL, screen, &left_arrow_rect);
	}
	else  /* Draw grayed-out left button: */
        {
	  SDL_BlitSurface(images[IMG_LEFT_GRAY], NULL, screen, &left_arrow_rect);
	}

	if (loc_screen_start + n_entries_per_screen < n_menu_entries)  // not on last page
        {
	  SDL_BlitSurface(images[IMG_RIGHT], NULL, screen, &right_arrow_rect);
	}
	else  /* Draw grayed-out right button: */
	{
	  SDL_BlitSurface(images[IMG_RIGHT_GRAY], NULL, screen, &right_arrow_rect);
	}
      }
      SDL_UpdateRect(screen, 0, 0, 0 ,0);
    } else if (old_loc != loc) {
      // This is not a full redraw, but the selected entry did change.
      // By just redrawing the old and new selections, we avoid flickering.
      if (old_loc >= 0) {
	imod = old_loc-loc_screen_start;
	use_sprite = (menu_sprites != NULL && old_loc >= title_offset && menu_sprites[old_loc-title_offset] != NULL);
	SDL_BlitSurface(images[IMG_MENU_BKG], &menu_button_rect[imod], screen, &menu_button_rect[imod]);   // redraw background
	if (use_sprite) {
	  // Some of the sprites extend beyond the menu button, so we
	  // have to make sure we redraw in the sprite rects, too
	  SDL_BlitSurface(images[IMG_MENU_BKG], &menu_sprite_rect[imod], screen, &menu_sprite_rect[imod]);
	}
	DrawButton(&menu_button_rect[imod], 10, REG_RGBA);  // draw button
	SDL_BlitSurface(menu_item_unselected[old_loc], NULL, screen, &menu_text_rect[imod]);  // draw text
	if (use_sprite) {
	  SDL_BlitSurface(menu_sprites[old_loc-title_offset]->default_img, NULL, screen, &menu_sprite_rect[imod]);
	  // Also update the sprite rect (in some cases the sprite
	  // extends beyond the menu button)
	  SDL_UpdateRect(screen, menu_sprite_rect[imod].x, menu_sprite_rect[imod].y, menu_sprite_rect[imod].w, menu_sprite_rect[imod].h);
	}
	SDL_UpdateRect(screen, menu_button_rect[imod].x, menu_button_rect[imod].y, menu_button_rect[imod].w, menu_button_rect[imod].h);
      }

      if (loc >= 0) {
	imod = loc-loc_screen_start;
	use_sprite = (menu_sprites != NULL && loc >= title_offset && menu_sprites[loc] != NULL);
	SDL_BlitSurface(images[IMG_MENU_BKG], &menu_button_rect[imod], screen, &menu_button_rect[imod]);
	if (use_sprite)
	  SDL_BlitSurface(images[IMG_MENU_BKG], &menu_sprite_rect[imod], screen, &menu_sprite_rect[imod]);
	DrawButton(&menu_button_rect[imod], 10, SEL_RGBA);
	SDL_BlitSurface(menu_item_selected[loc], NULL, screen, &menu_text_rect[imod]);
	if (use_sprite) {
	  menu_sprites[loc-title_offset]->cur = 0;  // start at beginning of animation sequence
	  SDL_BlitSurface(menu_sprites[loc-title_offset]->frame[menu_sprites[loc-title_offset]->cur], NULL, screen, &menu_sprite_rect[imod]);
	  SDL_UpdateRect(screen, menu_sprite_rect[imod].x, menu_sprite_rect[imod].y, menu_sprite_rect[imod].w, menu_sprite_rect[imod].h);
	  next_frame(menu_sprites[loc-title_offset]);
	}
	SDL_UpdateRect(screen, menu_button_rect[imod].x, menu_button_rect[imod].y, menu_button_rect[imod].w, menu_button_rect[imod].h);
      }
    } else if (frame_counter % 5 == 0 && loc >= 0) {
      // No user input changed anything, but check to see if we need to
      // animate the sprite
      if (menu_sprites != NULL && loc >= title_offset && menu_sprites[loc-title_offset] != NULL) {
	imod = loc-loc_screen_start;
	//SDL_BlitSurface(images[IMG_MENU_BKG], &menu_button_rect[imod], screen, &menu_button_rect[imod]);
	SDL_BlitSurface(images[IMG_MENU_BKG], &menu_sprite_rect[imod], screen, &menu_sprite_rect[imod]);
	DrawButton(&menu_button_rect[imod], 10, SEL_RGBA);
	//SDL_BlitSurface(menu_item_selected[loc], NULL, screen, &menu_text_rect[imod]);
	// Note: even though the whole button was redrawn, we don't
	// have to redraw the text & background as long as we don't
	// update that rect. If something else changes and we go to
	// full-screen updates, then remove the "commenting-out" on
	// the two lines above
	SDL_BlitSurface(menu_sprites[loc-title_offset]->frame[menu_sprites[loc-title_offset]->cur], NULL, screen, &menu_sprite_rect[imod]);
	SDL_UpdateRect(screen, menu_sprite_rect[imod].x, menu_sprite_rect[imod].y, menu_sprite_rect[imod].w, menu_sprite_rect[imod].h);
	next_frame(menu_sprites[loc-title_offset]);
      }
    }

    redraw = 0;

    /* Move the mouse pointer if there is only a single screen */
    if (warp_mouse && n_menu_entries <= n_entries_per_screen) {
      imod = loc - loc_screen_start;
      cursor.x = menu_button_rect[imod].x + (menu_button_rect[imod].w / 2);
      cursor.y = menu_button_rect[imod].y + (3 * menu_button_rect[imod].h / 4);
//      SDL_WarpMouse(cursor.x, cursor.y);
      warp_mouse = 0;
    }

    old_loc = loc;
    old_loc_screen_start = loc_screen_start;

    /* --- make Tux blink --- */
    switch (frame_counter % TUX6)
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
      /* Redraw background to keep edges anti-aliased properly: */
      SDL_BlitSurface(images[IMG_MENU_BKG],&Tuxdest, screen, &Tuxdest);
      SDL_BlitSurface(Tux->frame[tux_frame - 1], NULL, screen, &Tuxdest);
      SDL_UpdateRect(screen, Tuxdest.x, Tuxdest.y, Tuxdest.w, Tuxdest.h);
      //SDL_UpdateRect(screen, 0, 0, 0, 0);
    }

    /* Wait so we keep frame rate constant: */
    frame_now = SDL_GetTicks();
    if (frame_now < frame_start)
      frame_start = frame_now;  // in case the timer wraps around
    if (frame_now - frame_start < 33)
      SDL_Delay(33-(frame_now-frame_start));

    frame_counter++;
  }  // End !stop while loop


  /***** User made a choice, clean up and return the choice.   ******/

  /* --- clear graphics before leaving function --- */ 
  for (i = 0; i < n_menu_entries; i++)
  {
    SDL_FreeSurface(menu_item_unselected[i]);
    SDL_FreeSurface(menu_item_selected[i]);
  }
  free(menu_item_unselected);
  free(menu_item_selected);
  free(menu_text_rect);
  free(menu_button_rect);
  free(menu_sprite_rect);

  /* Return the value of the chosen item (-1 indicates escape) */
  if (stop == 2)
    return -1;
  else
    return loc - title_offset;
}



void set_buttons_max_width(SDL_Rect *menu_button_rect,int n)
{
  int i,max;

  max = 0;
  for (i = 0; i < n; i++)
    if (max < menu_button_rect[i].w)
      max = menu_button_rect[i].w;

  for (i = 0; i < n; i++)
    menu_button_rect[i].w = max;
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
#ifdef TUXMATH_DEBUG
      fprintf(stderr, "TransWipe(): screen not valid!\n");
#endif
      return;
    }

    if (!newbkg)
    {
#ifdef TUXMATH_DEBUG
      fprintf(stderr, "TransWipe(): newbkg not valid!\n");
#endif
      return;
    }

    numupdates = 0;
    frame = 0;

    if(newbkg->w == screen->w && newbkg->h == screen->h) {
        if( type == RANDOM_WIPE )
            type = (RANDOM_WIPE * ((float) rand()) / (RAND_MAX+1.0));

        switch( type ) {
            case WIPE_BLINDS_VERT: {
 #ifdef TUXMATH_DEBUG
                fprintf(stderr, "--+ Doing 'WIPE_BLINDS_VERT'\n");
#endif
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
#ifdef TUXMATH_DEBUG
                fprintf(stderr, "--+ Doing 'WIPE_BLINDS_HORIZ'\n");
#endif
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
#ifdef TUXMATH_DEBUG
                fprintf(stderr, "--+ Doing 'WIPE_BLINDS_BOX'\n");
#endif
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
#ifdef TUXMATH_DEBUG
      fprintf(stderr, "->TransWipe(): FINISH\n");
#endif
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
void AddRect(SDL_Rect* src, SDL_Rect* dst) {
    /*borrowed from SL's alien (and modified)*/

    struct blit    *update;

    if (!src || !dst)
    {
#ifdef TUXMATH_DEBUG 
     fprintf(stderr, "AddRect(): src or dst invalid!\n");
#endif
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


void set_default_menu_options(menu_options *menu_opts)
{
  menu_opts->starting_entry = 0;
  menu_opts->xleft = 240;
  menu_opts->ytop = 30;
  // Leave room for arrows at the bottom:
  menu_opts->ybottom = screen->h - images[IMG_LEFT]->h - 20;
  menu_opts->buttonheight = -1;
  menu_opts->ygap = 10;
  menu_opts->button_same_width = 1;
  menu_opts->title = NULL;
  menu_opts->trailer = NULL;
}
