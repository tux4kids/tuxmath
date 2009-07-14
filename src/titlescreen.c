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
#include "campaign.h"
#include "factoroids.h"
#include "multiplayer.h"
#include "mathcards.h"
#include "setup.h"     //for cleanup()
#include "loaders.h"
#include "credits.h"
#include "highscore.h"
#include "convert_utf.h" // for wide char to UTF-8 conversion
#include "SDL_extras.h"
#include "menu.h"

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

// Type needed for trans_wipe():
struct blit {
    SDL_Surface *src;
    SDL_Rect *srcrect;
    SDL_Rect *dstrect;
    unsigned char type;
} blits[MAX_UPDATES];

// Lessons available for play
char **lesson_list_titles = NULL;
char **lesson_list_filenames = NULL;
int num_lessons = 0;

/*TODO: move these constants into a config file
  (together with menu.c constants ? ) */
const float title_pos[4] = {0.0, 0.0, 0.3, 0.25};
const float tux_pos[4] = {0.0, 0.6, 0.3, 0.4};
const char* bkg_path = "title/menu_bkg.jpg";
const char* standby_path = "status/standby.png";
const char* title_path = "title/title1.png";
const char* egg_path = "title/egg.svg";
const char* tux_path = "tux/bigtux";
/* beak coordinates relative to tux rect */
const float beak_pos[4] = {0.36, 0.21, 0.27, 0.14};


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
  SPRITE_COMMANDO,
  SPRITE_QUIT,
  SPRITE_MAIN,
  SPRITE_GOLDSTAR,
  SPRITE_NO_GOLDSTAR,
  SPRITE_TROPHY,
  SPRITE_CREDITS,
  SPRITE_ALONE,
  SPRITE_FRIENDS,
  SPRITE_FACTOROIDS,
  SPRITE_FACTORS,
  SPRITE_FRACTIONS,
  SPRITE_CAMPAIGN,
  SPRITE_SSWEEP,
  SPRITE_ELIMINATION,
  SPRITE_SERVER,
  SPRITE_CLIENT,
  N_SPRITES};

const char* menu_sprite_files[N_SPRITES] =
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
  "tux_helmet_black",
  "quit",
  "main",
  "goldstar",
  "no_goldstar",
  "trophy",
  "credits",
  "alone", 
  "friends", 
  "factoroids",
  "factors",
  "fractions",
  "fleet",
  "nums",
  "exclamation"
};

sprite **sprite_list = NULL;


SDL_Event event;

/* screen dimensions to which titlescreen graphics are currently rendered */
int curr_res_x = -1;
int curr_res_y = -1;

/* titlescreen items */
SDL_Surface* win_bkg = NULL;
SDL_Surface* fs_bkg = NULL;

SDL_Surface* logo = NULL;
sprite* Tux = NULL;
SDL_Surface* title = NULL;

/* "Easter Egg" cursor */
SDL_Surface* egg = NULL;
int egg_active = 0; //are we currently using the egg cursor?

/* locations we need */
SDL_Rect bkg_rect,
         logo_rect,
         tux_rect,
         title_rect,
         cursor,
         beak;

SDL_Rect stopRect; /* part of the menu */

SDL_Surface* current_bkg()
  /* This syntax makes my brain start to explode! */
  { return screen->flags & SDL_FULLSCREEN ? fs_bkg : win_bkg; }

void set_current_bkg(SDL_Surface* new_bkg)
{
  if(screen->flags & SDL_FULLSCREEN)
  {
    if(fs_bkg != NULL)
      SDL_FreeSurface(fs_bkg);
    fs_bkg = new_bkg;
  }
  else
  {
    if(win_bkg != NULL)
      SDL_FreeSurface(win_bkg);
    win_bkg = new_bkg;
  }
}

/* Local function prototypes: */
int TitleScreen_load_menu(void);
void TitleScreen_unload_menu(void);
void NotImplemented(void);

void free_titlescreen(void);

void trans_wipe(SDL_Surface* newbkg, int type, int var1, int var2);
void init_blits(void);
void update_screen(int* frame);
void add_rect(SDL_Rect* src, SDL_Rect* dst);

void RecalcMenuPositions(int*, int, menu_options*, void (*)(menu_options*),
                         SDL_Rect**, SDL_Rect**, SDL_Rect**,
                         SDL_Rect**, SDL_Rect**, SDL_Rect**,
                         SDL_Rect*, SDL_Rect*);
void set_buttons_max_width(SDL_Rect *, SDL_Rect *, int);

int run_login_menu(void);
int run_main_menu(void);
int run_game_menu(void);
int run_multiplay_menu(void);
int run_lessons_menu(void);
int run_arcade_menu(void);
int run_campaign_menu(void);
int run_custom_menu(void);
int run_activities_menu(void);
int run_options_menu(void);
int run_lan_menu(void);
int run_server_menu(void);
int handle_easter_egg(const SDL_Event* evt);



/***********************************************************/
/*                                                         */
/*       "Public functions" (callable throughout program)  */
/*                                                         */
/***********************************************************/


/* Display Tux4Kids logo, then animate title screen
   items onto the screen and run main menu */
void TitleScreen(void)
{
  Uint32 start_time = 0;
  SDL_Rect tux_anim, title_anim;
  int i, tux_pix_skip, title_pix_skip, curr_time;

  if (Opts_UsingSound())
  {
    Opts_SetGlobalOpt(MENU_SOUND, 1);
    Opts_SetGlobalOpt(MENU_MUSIC, 1);
  }

  start_time = SDL_GetTicks();
  logo = LoadImage(standby_path, IMG_REGULAR);

  /* display the Standby screen */
  if(logo)
  {
    /* Center horizontally and vertically */
    logo_rect.x = (screen->w - logo->w) / 2;
    logo_rect.y = (screen->h - logo->h) / 2;

    logo_rect.w = logo->w;
    logo_rect.h = logo->h;

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_BlitSurface(logo, NULL, screen, &logo_rect);
    SDL_UpdateRect(screen, 0, 0, 0, 0);
    /* Play "harp" greeting sound lifted from Tux Paint */
    playsound(SND_HARP);
    SDL_FreeSurface(logo);
  }

  /* load backgrounds */
  LoadBothBkgds(bkg_path, &fs_bkg, &win_bkg);
  if(fs_bkg == NULL || win_bkg == NULL)
  {
    fprintf(stderr, "Backgrounds were not properly loaded, exiting");
    if(fs_bkg)
      SDL_FreeSurface(fs_bkg);
    if(win_bkg)
      SDL_FreeSurface(win_bkg);
    return;
  }

  //TitleScreen_load_menu();

  /* load titlescreen images */
  if(RenderTitleScreen() == 0)
  {
    fprintf(stderr, "Media was not properly loaded, exiting");
    return;
  }

  /* --- wait  --- */
  while ((SDL_GetTicks() - start_time) < 2000)
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

  /* NOTE: do we need this ? */
  DEBUGCODE(debug_titlescreen)
    SDL_WM_GrabInput(SDL_GRAB_OFF); /* in case of a freeze, this traps the cursor */
  else
    SDL_WM_GrabInput(SDL_GRAB_ON);  /* User input goes to TuxMath, not window manager */
  SDL_ShowCursor(1);


  /* Tux and Title animations */
  DEBUGMSG(debug_titlescreen, "TitleScreen(): Now Animating Tux and Title onto the screen\n" );

  /* Draw background (center it if it's smaller than screen) */
  if(current_bkg())
  {
    /* FIXME not sure trans_wipe() works in Windows: */
    trans_wipe(current_bkg(), RANDOM_WIPE, 10, 20);
    /* Make sure background gets drawn (since trans_wipe() doesn't */
    /* seem to work reliably as of yet):                          */
    SDL_BlitSurface(current_bkg(), NULL, screen, &bkg_rect);
  }

  /* --- Pull tux & logo onscreen --- */
  if(title && Tux && Tux->frame[0])
  {
    /* final tux & title positioins are already calculated,
       start outside the screen */
    tux_anim = tux_rect;
    tux_anim.y = screen->h;

    title_anim = title_rect;
    title_anim.x = screen->w;

    for(i = 0; i < ANIM_FRAMES; i++)
    {
      start_time = SDL_GetTicks();

      /* Draw the entire background, over a black screen if necessary */
      if(current_bkg()->w != screen->w || current_bkg()->h != screen->h)
        SDL_FillRect(screen, &screen->clip_rect, 0);

      SDL_BlitSurface(current_bkg(), NULL, screen, &bkg_rect);

      /* calculate shifts */
      tux_pix_skip = (tux_anim.y - tux_rect.y) / (ANIM_FRAMES - i);
      tux_anim.y -= tux_pix_skip;
      title_pix_skip = (title_anim.x - title_rect.x) / (ANIM_FRAMES - i);
      title_anim.x -= title_pix_skip;

      /* update screen */
      SDL_BlitSurface(Tux->frame[0], NULL, screen, &tux_anim);
      SDL_BlitSurface(title, NULL, screen, &title_anim);

      SDL_UpdateRect(screen, tux_anim.x, tux_anim.y, tux_anim.w,
          min(tux_anim.h + tux_pix_skip, screen->h - tux_anim.y));
      SDL_UpdateRect(screen, title_anim.x, title_anim.y,
          min(title_anim.w + title_pix_skip, screen->w - title_anim.x), title_anim.h);

      curr_time = SDL_GetTicks();
      if((curr_time - start_time) < 1000 / ANIM_FPS)
        SDL_Delay(1000 / ANIM_FPS - (curr_time - start_time));
    }
  }

  DEBUGMSG(debug_titlescreen, "TitleScreen(): Tux and Title are in place now\n");


  /* Red "Stop" circle in upper right corner to go back to main menu: */
  /* this is going to be part of the menu */
  /*if (images[IMG_STOP])
  {
    stopRect.w = images[IMG_STOP]->w;
    stopRect.h = images[IMG_STOP]->h;
    stopRect.x = screen->w - images[IMG_STOP]->w;
    stopRect.y = 0;
    SDL_BlitSurface(images[IMG_STOP], NULL, screen, &stopRect);
  }
  SDL_UpdateRect(screen, 0, 0, 0, 0);*/

  /* Start playing menu music if desired: */
  if (Opts_GetGlobalOpt(MENU_MUSIC))
  {
    audioMusicLoad("tuxi.ogg", -1);
  }

  /* If necessary, have the user log in */
  //if (run_login_menu() != -1) {
  if (RunLoginMenu() != -1) {
    /* Finish parsing user options */
    initialize_options_user();
    /* Start the main menu */
    //run_main_menu();
    RunMainMenu();
  }

  /* User has selected quit, clean up */
  DEBUGMSG(debug_titlescreen, "TitleScreen(): Freeing title screen images\n");

  free_titlescreen();
  //TitleScreen_unload_menu();

  DEBUGMSG(debug_titlescreen, "leaving TitleScreen()\n");
}

void DrawTitleScreen(void)
{
  SDL_BlitSurface(current_bkg(), NULL, screen, &bkg_rect);
  SDL_BlitSurface(Tux->frame[0], NULL, screen, &tux_rect);
  SDL_BlitSurface(title, NULL, screen, &title_rect);
  //SDL_UpdateRect(screen, 0, 0, 0, 0);
}

/* Render and position all titlescreen items to match current
   screen size. Rendering is done only if needed.
   This function must be called after every resolution change
   returns 1 on success, 0 on failure */
int RenderTitleScreen(void)
{
  SDL_Surface* new_bkg = NULL;

  if(curr_res_x != screen->w || curr_res_y != screen->h)
  {
    /* we need to rerender titlescreen items */
    DEBUGMSG(debug_titlescreen, "Re-rendering titlescreen items.\n");

    /* we keep two backgrounds to make screen mode switch faster */
    if(current_bkg()->w != screen->w || current_bkg()->h != screen->h)
    {
      new_bkg = LoadBkgd(bkg_path, screen->w, screen->h);
      if(new_bkg == NULL)
      {
        DEBUGMSG(debug_titlescreen, "RenderTitleScreen(): Failed to load new background.\n");
        return 0;
      }
      else
      {
        DEBUGMSG(debug_titlescreen, "RenderTitleScreen(): New background loaded.\n");
        set_current_bkg(new_bkg);
      }
    }

    bkg_rect = current_bkg()->clip_rect;
    bkg_rect.x = (screen->w - bkg_rect.w) / 2;
    bkg_rect.y = (screen->h - bkg_rect.h) / 2;

    /* Tux in lower left corner of the screen */
    SetRect(&tux_rect, tux_pos);
    Tux = LoadSpriteOfBoundingBox(tux_path, IMG_ALPHA, tux_rect.w, tux_rect.h);
    if(Tux && Tux->frame[0])
    {
      tux_rect.w = Tux->frame[0]->clip_rect.w;
      tux_rect.h = Tux->frame[0]->clip_rect.h;
    }
    else
    {
      DEBUGMSG(debug_titlescreen, "RenderTitleScreen(): Failed to load Tux image.\n");
      return 0;
    }

    /* "Tux, of math command" title in upper right corner */
    SetRect(&title_rect, title_pos);
    title = LoadImageOfBoundingBox(title_path, IMG_ALPHA, title_rect.w, title_rect.h);
    if(title)
    {
      title_rect.w = title->clip_rect.w;
      title_rect.h = title->clip_rect.h;
    }
    else
    {
      DEBUGMSG(debug_titlescreen, "RenderTitleScreen(): Failed to load title image.\n");
      return 0;
    }

    /* easter egg */
#ifdef HAVE_RSVG
    egg = LoadImage(egg_path, IMG_ALPHA | IMG_NOT_REQUIRED);
#else
    egg = LoadImage(egg_path, IMG_COLORKEY | IMG_NOT_REQUIRED);
#endif

    beak.x = tux_rect.x + beak_pos[0] * tux_rect.w;
    beak.y = tux_rect.y + beak_pos[1] * tux_rect.h;
    beak.w = beak_pos[2] * tux_rect.w;
    beak.h = beak_pos[3] * tux_rect.h;


    /* stop button - going to be part of the menu */
    stopRect.x = screen->w - stopRect.w;
    stopRect.y = 0;



    curr_res_x = screen->w;
    curr_res_y = screen->h;

    DEBUGMSG(debug_titlescreen, "Leaving RenderTitleScreen().\n");
  }
  return 1;
}


/* handle titlescreen events (easter egg)
   this function should be called from event loops
   return 1 if events require full redraw */
int HandleTitleScreenEvents(const SDL_Event* evt)
{
  return handle_easter_egg(evt);
}

/* handle all titlescreen blitting
   this function should be called after every animation frame */
void HandleTitleScreenAnimations()
{
  static int frame_counter = 0;
  int tux_frame;

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
    SDL_BlitSurface(current_bkg(),&tux_rect, screen, &tux_rect);
    SDL_BlitSurface(Tux->frame[tux_frame - 1], NULL, screen, &tux_rect);
    UpdateRect(screen, &tux_rect);
  }

  if (egg_active) { //if we need to, draw the egg cursor
    //who knows why GetMouseState() doesn't take Sint16's...
    SDL_GetMouseState((int*)(&cursor.x), (int*)(&cursor.y));
    cursor.x -= egg->w / 2; //center vertically
    SDL_BlitSurface(egg, NULL, screen, &cursor);
    UpdateRect(screen, &cursor);
  }

  frame_counter++;
}


/***********************************************************/
/*                                                         */
/*    "Private functions" (callable only from this file)   */
/*                                                         */
/***********************************************************/


/* this is going to be moved to menu.c */
int TitleScreen_load_menu(void)
{
  char fn[PATH_MAX];
  int i;

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
  {
    DEBUGMSG(debug_menu, "Freeing image %d: ", i);
    FreeSprite(sprite_list[i]);
    sprite_list[i] = NULL;
  }
  free(sprite_list);
  DEBUGMSG(debug_menu, "Images freed\n");
  sprite_list = NULL;
}



void free_titlescreen(void)
{
  DEBUGMSG(debug_titlescreen, "Entering free_titlescreen()\n");

  FreeSprite(Tux);
  Tux = NULL;

  if(egg)
  {
    SDL_FreeSurface(egg);
    egg = NULL;
  }

  if(title)
  {
    SDL_FreeSurface(title);
    title = NULL;
  }

  if(fs_bkg)
  {
    SDL_FreeSurface(fs_bkg);
    fs_bkg = NULL;
  }

  if(win_bkg)
  {
    SDL_FreeSurface(win_bkg);
    win_bkg = NULL;
  }
}



void NotImplemented(void)
{
  const char *s1, *s2, *s3, *s4;

  s1 = _("Work In Progress!");
  s2 = _("This feature is not ready yet");
  s3 = _("Discuss the future of TuxMath at");
  s4 = N_("tuxmath-devel@lists.sourceforge.net");

  ShowMessage(s1, s2, s3, s4);
}





/* FIXME add some background shading to improve legibility */
void ShowMessage(const char* str1, const char* str2, const char* str3, const char* str4)
{
  SDL_Surface *s1, *s2, *s3, *s4;
  SDL_Rect loc;
  int finished = 0;
  int tux_frame = 0;
  Uint32 frame = 0;
  Uint32 start = 0;

  s1 = s2 = s3 = s4 = NULL;

  DEBUGMSG(debug_titlescreen, "ShowMessage() - creating text\n" );

  if (str1)
    s1 = BlackOutline(str1, DEFAULT_MENU_FONT_SIZE, &white);
  if (str2)
    s2 = BlackOutline(str2, DEFAULT_MENU_FONT_SIZE, &white);
  if (str3)
    s3 = BlackOutline(str3, DEFAULT_MENU_FONT_SIZE, &white);
  if (str4)
    s4 = BlackOutline(str4, DEFAULT_MENU_FONT_SIZE, &white);

  DEBUGMSG(debug_titlescreen, "ShowMessage() - drawing screen\n" );

  /* Redraw background: */
  if (current_bkg() )
    SDL_BlitSurface( current_bkg(), NULL, screen, &bkg_rect );

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
    SDL_BlitSurface(Tux->frame[0], NULL, screen, &tux_rect);
  }

  /* Draw lines of text (do after drawing Tux so text is in front): */
  if (s1)
  {
    loc.x = (screen->w / 2) - (s1->w/2); loc.y = 10;
    SDL_BlitSurface( s1, NULL, screen, &loc);
  }
  if (s2)
  {
    loc.x = (screen->w / 2) - (s2->w/2); loc.y = 60;
    SDL_BlitSurface( s2, NULL, screen, &loc);
  }
  if (s3)
  {
    //loc.x = 320 - (s3->w/2); loc.y = 300;
    loc.x = (screen->w / 2) - (s3->w/2); loc.y = 110;
    SDL_BlitSurface( s3, NULL, screen, &loc);
  }
  if (s4)
  {
    //loc.x = 320 - (s4->w/2); loc.y = 340;
    loc.x = (screen->w / 2) - (s4->w/2); loc.y = 200;
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
      SDL_BlitSurface(Tux->frame[tux_frame - 1], NULL, screen, &tux_rect);
//      SDL_UpdateRect(screen, tux_rect.x+37, tux_rect.y+40, 70, 45);
      SDL_UpdateRect(screen, tux_rect.x, tux_rect.y, tux_rect.w, tux_rect.h);

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


void main_scmo(menu_options* mo) //set custom menu opts for main
{
  mo->ygap = 15;
}

int run_main_menu(void)
{
  const char* menu_text[7] =
    {N_("Play Alone"),
     N_("LAN Game"),
     N_("Play With Friends"),
     N_("Factoroids!"),
     N_("Help"),
     N_("More Options"),
     N_("Quit")};
  sprite* sprites[7] =
    {NULL, NULL, NULL, NULL, NULL, NULL,NULL};
  menu_options menu_opts;
  int choice,ret;

  // Set up the sprites
  sprites[0] = sprite_list[SPRITE_ALONE];
  sprites[2] = sprite_list[SPRITE_FRIENDS];
  sprites[3] = sprite_list[SPRITE_FACTOROIDS];
  sprites[4] = sprite_list[SPRITE_HELP];
  sprites[5] = sprite_list[SPRITE_OPTIONS];
  sprites[6] = sprite_list[SPRITE_QUIT];
  
  //set_default_menu_options(&menu_opts);
  //menu_opts.ytop = 100;
  //menu_opts.ygap = 15;

  //This function takes care of all the drawing and receives
  //user input:
  choice = choose_menu_item(menu_text,sprites,7,NULL,main_scmo);

  while (choice >= 0) {
    switch (choice) {
      case 0: {
        // All single player modes
        ret = run_game_menu();
        break;
      }
      case 1: {
        ret = run_lan_menu();
        break;
      }
      case 2: {
        // Multiplayer games
        ret = run_multiplay_menu();
        break;
      }
      case 3: {
        // Factroids et. al.
        ret = run_activities_menu();
        break;
      }
      case 4: {
        // Help
        Opts_SetHelpMode(1);
        Opts_SetDemoMode(0);
        if (Opts_GetGlobalOpt(MENU_MUSIC))  //Turn menu music off for game
          {audioMusicUnload();}
        game();
        RenderTitleScreen();
        if (Opts_GetGlobalOpt(MENU_MUSIC)) //Turn menu music back on
          {audioMusicLoad( "tuxi.ogg", -1 );}
        Opts_SetHelpMode(0);
        break;
      }
      case 5: {
        // More options
        ret = run_options_menu();
        break;
      }
      case 6: {
        // Quit
        DEBUGMSG(debug_titlescreen, "Exiting main menu\n");
        return 0;
      }    
    }
    menu_opts.starting_entry = choice;
    choice = choose_menu_item(menu_text,sprites,7,NULL,main_scmo);
  }
  return 0;
}
                                                     
#define NUM_GAME_MENU_ITEMS 5
int run_game_menu(void)
{
  const char* menu_text[NUM_GAME_MENU_ITEMS] =
    {N_("Math Command Training Academy"),
     N_("Math Command Fleet Missions"),          
     N_("Play Arcade Game"),
     N_("Play Custom Game"),
     N_("Main menu")};

  sprite* sprites[NUM_GAME_MENU_ITEMS] = {NULL, NULL, NULL, NULL, NULL};

  int ret, choice = 0;

  sprites[0] = sprite_list[SPRITE_TRAINING];
  sprites[1] = sprite_list[SPRITE_CAMPAIGN];
  sprites[2] = sprite_list[SPRITE_ARCADE];
  sprites[3] = sprite_list[SPRITE_CUSTOM];
  sprites[4] = sprite_list[SPRITE_MAIN];

  while (choice >= 0) {
    choice = choose_menu_item(menu_text,sprites,NUM_GAME_MENU_ITEMS,NULL,NULL);
    switch (choice) {
      case 0:
        ret = run_lessons_menu();
        break;
      case 1:
        ret = start_campaign();
        break;
      case 2:
        ret = run_arcade_menu();
        break;
      case 3:
        ret = run_custom_menu();
        break;
      case 4:
        return 0;
      default:
        DEBUGMSG(debug_titlescreen, "choose_menu_item() returned %d--returning\n", choice);
        return 0;
    }
  }
  return 0;
}

/*
Set up and start a turn-based multiplayer game. Some funky heap issues so
quarantine it behind the return for the time being.
*/
int run_multiplay_menu(void)
{
  int nplayers = 0;
  int mode = -1;
  int difficulty = -1;
  char npstr[HIGH_SCORE_NAME_LENGTH * 3];

  const char* menu_text[3] =
    {N_("Score Sweep"),
     N_("Elimination"),
     N_("Main menu")};

  //just leech settings from arcade modes
  const char* diff_menu_text[NUM_MATH_COMMAND_LEVELS + 1] =
    {N_("Space Cadet"),
     N_("Scout"),
     N_("Ranger"),
     N_("Ace"),
     N_("Commando"),
     N_("Main menu")};


  sprite* modesprites[3] = {NULL, NULL, NULL};
  sprite* diffsprites[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
  // Set up the sprites
  modesprites[0] = sprite_list[SPRITE_SSWEEP];
  modesprites[1] = sprite_list[SPRITE_ELIMINATION];
  modesprites[2] = sprite_list[SPRITE_MAIN];
  
  diffsprites[0] = sprite_list[SPRITE_CADET];
  diffsprites[1] = sprite_list[SPRITE_SCOUT];
  diffsprites[2] = sprite_list[SPRITE_RANGER];
  diffsprites[3] = sprite_list[SPRITE_ACE];
  diffsprites[4] = sprite_list[SPRITE_COMMANDO];
  diffsprites[5] = sprite_list[SPRITE_MAIN];

  while (1)
  {
    //choose difficulty
    difficulty = choose_menu_item(diff_menu_text, diffsprites, 
                 NUM_MATH_COMMAND_LEVELS + 1, NULL, NULL);

    if (difficulty == -1 || difficulty >= NUM_MATH_COMMAND_LEVELS)
      break; //user chose main menu or hit escape

    //choose mode
    mode = choose_menu_item(menu_text,modesprites,3,NULL,NULL);
    if (mode == 2 || mode == -1)
      break;

    //ask how many players
    while (nplayers <= 0 || nplayers > MAX_PLAYERS)
    {
      NameEntry(npstr, _("How many kids are playing?"),
                       _("(Between 2 and 4 players)"));
      nplayers = atoi(npstr);
    }


    mp_set_parameter(PLAYERS, nplayers);
    mp_set_parameter(MODE, mode);
    mp_set_parameter(DIFFICULTY, difficulty);

    //RUN!
    mp_run_multiplayer();
  }

  return 0;
}

///////////////////////////   LAN game menu()////////////


int run_lan_menu(void)
{
  int mode = -1;
  char ipstr[HIGH_SCORE_NAME_LENGTH * 3];
  
  const char* menu_text[3] =
    {N_("Host"),
     N_("Join"),
     N_("Main menu")};

  sprite* modesprites[3] = {NULL, NULL, NULL};
  // Set up the sprites
  modesprites[0] = sprite_list[SPRITE_SERVER];
  modesprites[1] = sprite_list[SPRITE_CLIENT];
  modesprites[2] = sprite_list[SPRITE_MAIN];
   while (1)
  {
    //choose mode
    mode = choose_menu_item(menu_text,modesprites,3,NULL,NULL);
    if (mode == 2 || mode == -1)
      break;

    if(mode == 0)                     //chooses Host
    run_server_menu();
    
    if(mode == 1)
    NameEntry(ipstr, _("Enter the IP Address"),
                       _("(of the Host)"));
      

   }

  return 0;

}


int run_server_menu(void)
{

  int difficulty = -1;
  

  //just leech settings from arcade modes
  const char* diff_menu_text[NUM_MATH_COMMAND_LEVELS + 1] =
    {N_("Space Cadet"),
     N_("Scout"),
     N_("Ranger"),
     N_("Ace"),      
     N_("Commando"),
     N_("Main menu")};
 
  

   sprite* diffsprites[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
  
 
  diffsprites[0] = sprite_list[SPRITE_CADET];
  diffsprites[1] = sprite_list[SPRITE_SCOUT];
  diffsprites[2] = sprite_list[SPRITE_RANGER];
  diffsprites[3] = sprite_list[SPRITE_ACE];
  diffsprites[4] = sprite_list[SPRITE_COMMANDO];
  diffsprites[5] = sprite_list[SPRITE_MAIN];

     while (1)
  {
    //choose difficulty
    difficulty = choose_menu_item(diff_menu_text,diffsprites,6,NULL,NULL);
    break;
   }
   return 0;


}




int run_arcade_menu(void)
{
  const char* menu_text[7] =
    {N_("Space Cadet"),
     N_("Scout"),
     N_("Ranger"),
     N_("Ace"),
     N_("Commando"),
     N_("Hall Of Fame"),
     N_("Main menu")};               
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
  sprite* sprites[7] =
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
  menu_options menu_opts;
  int choice,hs_table;

  // Set up the sprites
  sprites[0] = sprite_list[SPRITE_CADET];
  sprites[1] = sprite_list[SPRITE_SCOUT];
  sprites[2] = sprite_list[SPRITE_RANGER];
  sprites[3] = sprite_list[SPRITE_ACE];
  sprites[4] = sprite_list[SPRITE_COMMANDO];
  sprites[5] = sprite_list[SPRITE_TROPHY];
  sprites[6] = sprite_list[SPRITE_MAIN];

//  set_default_menu_options(&menu_opts);
//  menu_opts.ytop = 100;

  //This function takes care of all the drawing and receives
  //user input:
  choice = choose_menu_item(menu_text,sprites,7,NULL,NULL);

  while (choice >= 0) {
    if (choice < NUM_MATH_COMMAND_LEVELS) {
      // Play arcade game
      if (read_named_config_file(arcade_config_files[choice]))
      {
        audioMusicUnload();
        game();
        RenderTitleScreen();
        if (Opts_GetGlobalOpt(MENU_MUSIC)) {
          audioMusicLoad( "tuxi.ogg", -1 );
        }
        /* See if player made high score list!                        */
        read_high_scores();  /* Update, in case other users have added to it */
        hs_table = arcade_high_score_tables[choice];
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
          append_high_score(choice,Opts_LastScore(),&player_name[0]);

          DEBUGCODE(debug_titlescreen)
            print_high_scores(stderr);
        }
      } else {
        fprintf(stderr, "\nCould not find %s config file\n",arcade_config_files[choice]);
      }

    } else if (choice == NUM_MATH_COMMAND_LEVELS) {
      // Display the Hall of Fame
      DisplayHighScores(CADET_HIGH_SCORE);
    }
    else {
      // Return to main menu
      return 0;
    }
    set_default_menu_options(&menu_opts);
    menu_opts.starting_entry = choice;
    choice = choose_menu_item(menu_text,sprites,7,NULL, NULL);
  }

  return 0;
}


int run_custom_menu(void)
{
  const char *s1, *s2, *s3, *s4;
  s1 = _("Edit 'options' file in your home directory");
  s2 = _("to create customized game!");
  s3 = _("Press a key or click your mouse to start game.");
  s4 = _("See README.txt for more information");
  ShowMessage(s1, s2, s3, s4);

  if (read_user_config_file()) {
    if (Opts_GetGlobalOpt(MENU_MUSIC))
      audioMusicUnload();

    game();
    RenderTitleScreen();
    write_user_config_file();

    if (Opts_GetGlobalOpt(MENU_MUSIC))
      audioMusicLoad( "tuxi.ogg", -1 );
  }

  return 0;
}

int run_activities_menu(void)
{ 
  const char* menu_text[3] =
    {N_("Factors"),
     N_("Fractions"),
     N_("Main menu")};
  const int factoroids_high_score_tables[2] =
    {FACTORS_HIGH_SCORE, FRACTIONS_HIGH_SCORE};
  sprite* sprites[3] =
    {NULL, NULL, NULL};
  menu_options menu_opts;
  int choice, hs_table;

  // Set up the sprites
  sprites[0] = sprite_list[SPRITE_FACTORS];
  sprites[1] = sprite_list[SPRITE_FRACTIONS];
  sprites[2] = sprite_list[SPRITE_MAIN];

  set_default_menu_options(&menu_opts);
  menu_opts.ytop = 100;

  //This function takes care of all the drawing and receives
  //user input:
  choice = choose_menu_item(menu_text, sprites, 3, NULL, NULL);

  while (choice >= 0) {
    switch(choice){
      case 0:
          audioMusicUnload();
          factors();
	  
	  if (Opts_GetGlobalOpt(MENU_MUSIC)) {
	      audioMusicLoad( "tuxi.ogg", -1 );
	  }
	  break;
      case 1:
          audioMusicUnload(); 
          fractions();
	  
	  if (Opts_GetGlobalOpt(MENU_MUSIC)) {
	     audioMusicLoad( "tuxi.ogg", -1 );
	  }
	  break;
     case 2:
          // Return to main menu
          return 0;
    }

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

    menu_opts.starting_entry = choice;
    choice = choose_menu_item(menu_text,sprites,3,NULL,NULL);


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
  const char* menu_text[4] =
    {N_("Demo"),
     N_("Project Info"),
     N_("Credits"),
     N_("Main Menu")};

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

  //set_default_menu_options(&menu_opts);
  //menu_opts.ytop = 100;

  //This function takes care of all the drawing and receives
  //user input:
  choice = choose_menu_item(menu_text, sprites, n_menu_entries, NULL, NULL);

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
        RenderTitleScreen();
        if (Opts_GetGlobalOpt(MENU_MUSIC)) {
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

    set_default_menu_options(&menu_opts);
    menu_opts.starting_entry = choice;
    choice = choose_menu_item(menu_text,sprites,n_menu_entries,NULL,NULL);
  }

  return 0;
}


void lessons_scmo(menu_options* mo)
{
mo->ytop = 30;
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
//  set_default_menu_options(&menu_opts);
//  ytop = 30;

  //This function takes care of all the drawing and receives
  //user input:
  chosen_lesson = choose_menu_item((const char**)lesson_list_titles, star_sprites, num_lessons, NULL, &lessons_scmo);

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
      RenderTitleScreen();

      /* If successful, display Gold Star for this lesson! */
      if (MC_MissionAccomplished())
      {
        lesson_list_goldstars[chosen_lesson] = 1;
        star_sprites[chosen_lesson] = sprite_list[SPRITE_GOLDSTAR];
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
    set_default_menu_options(&menu_opts);
    menu_opts.starting_entry = chosen_lesson;
    chosen_lesson = choose_menu_item((const char**)lesson_list_titles, star_sprites, num_lessons, &menu_opts, &lessons_scmo);
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
  int level;
  int i;
  char *trailer_quit = "Quit";
  char *trailer_back = "Back";
  SDLMod mod;

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
  set_default_menu_options(&menu_opts);
  if (n_login_questions > 0)
    menu_opts.title = user_login_questions[0];
  menu_opts.trailer = trailer_quit;

  while (n_users) {
    // Get the user choice
    chosen_login = choose_menu_item((const char**)user_names, NULL, n_users, &menu_opts, NULL);
    // Determine whether there were any modifier (CTRL) keys pressed
    mod = SDL_GetModState();
    if (chosen_login == -1 || chosen_login == n_users) {
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
        menu_opts.starting_entry = -1;
      }
    }
    else {
      // User chose an entry, set it up
      user_data_dirname_down(user_names[chosen_login]);
      level++;
      menu_opts.starting_entry = 0;
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

  // The user home directory is set, clean up remaining memory
  for (i = 0; i < n_login_questions; i++)
    free(user_login_questions[i]);
  free(user_login_questions);

  // Signal success
  return 0;
}


/****************************************************************/
/* choose_menu_item: menu navigation utility function           */
/* (the function returns the index for the selected menu item)  */
/* -1 indicates that the user pressed escape                    */
/****************************************************************/
int choose_menu_item(const char **menu_text, sprite **menu_sprites, int n_menu_entries, menu_options* custom_mo, void (*set_custom_menu_opts)(menu_options*) )
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

  // The section of the background that the menu rects actually cover
  SDL_Rect *back_text_rect = NULL,
           *back_button_rect = NULL,
           *back_sprite_rect = NULL;
  SDL_Rect left_arrow_rect, right_arrow_rect;
  SDL_Rect temp_rect; //temporary copy of a dest rect that may be written to by SDL_BlitSurface

  menu_options menu_opts;

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
  int click_flag = 1;
  int use_sprite = 0;
  int warp_mouse = 0;
  int title_offset = 0;
  int have_trailer = 0;

  DEBUGMSG(debug_menu, "Entering choose_menu_item():\n");

  DEBUGMSG(debug_menu, "%d menu entries:\n", n_menu_entries);
  DEBUGCODE(debug_menu)
  {
    for (i = 0; i < n_menu_entries; i++)
      DEBUGMSG(debug_menu, "%s\n", menu_text[i]);
  }

  if (custom_mo == NULL)
    set_default_menu_options(&menu_opts);
  else
    menu_opts = *custom_mo;
  if (set_custom_menu_opts != NULL)
    set_custom_menu_opts(&menu_opts);

  DEBUGMSG(debug_menu, "Allocating memory\n");
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
    menu_item_unselected[0] = BlackOutline( _(menu_opts.title), DEFAULT_MENU_FONT_SIZE, &red);
    // It will never be selected, so we don't have to do anything for selected.
    menu_item_selected[0] = NULL;
  }
  for (i = 0; i < n_menu_entries; i++)
  {
    menu_item_unselected[i+title_offset] = BlackOutline( _(menu_text[i]), DEFAULT_MENU_FONT_SIZE, &white );
    menu_item_selected[i+title_offset] = BlackOutline( _(menu_text[i]), DEFAULT_MENU_FONT_SIZE, &yellow);
  }
  if (have_trailer) {
    menu_item_unselected[n_menu_entries+title_offset] = BlackOutline( _(menu_opts.trailer), DEFAULT_MENU_FONT_SIZE, &white );
    menu_item_selected[n_menu_entries+title_offset] = BlackOutline( _(menu_opts.trailer), DEFAULT_MENU_FONT_SIZE, &yellow);
  }
  // We won't need the menu_text again, so now we can keep track of
  // the total entries including the title & trailer
  n_menu_entries += title_offset+have_trailer;

//  recalcMenuPositions();

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
  back_text_rect = (SDL_Rect*) malloc(n_entries_per_screen * sizeof(SDL_Rect));
  back_button_rect = (SDL_Rect*) malloc(n_entries_per_screen * sizeof(SDL_Rect));
  if (menu_text_rect == NULL || menu_button_rect == NULL ||
      back_text_rect == NULL || back_button_rect == NULL) {
    free(menu_text_rect);
    free(menu_button_rect);
    free(back_text_rect);
    free(back_button_rect);
    return -2;
  }
  if (menu_sprites != NULL) {
    menu_sprite_rect = (SDL_Rect*) malloc(n_entries_per_screen * sizeof(SDL_Rect));
    back_sprite_rect = (SDL_Rect*) malloc(n_entries_per_screen * sizeof(SDL_Rect));
    if (menu_sprite_rect == NULL || back_sprite_rect == NULL) {
      free(menu_sprite_rect);
      free(back_sprite_rect);
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
    set_buttons_max_width(menu_button_rect,back_button_rect,n_entries_per_screen);

  for (i = 0; i < n_entries_per_screen; ++i)
  {
    if (menu_button_rect)
    {
      back_button_rect[i] = menu_button_rect[i];
      back_button_rect[i].x -= bkg_rect.x;
      back_button_rect[i].y -= bkg_rect.y;
    }
    if (menu_text_rect)
    {
      back_text_rect[i] = menu_text_rect[i];
      back_text_rect[i].x -= bkg_rect.x;
      back_text_rect[i].y -= bkg_rect.y;
    }
    if (menu_sprite_rect)
    {
      back_sprite_rect[i] = menu_sprite_rect[i];
      back_sprite_rect[i].x -= bkg_rect.x;
      back_sprite_rect[i].y -= bkg_rect.y;
    }
  }

  /**** Draw background, title, and Tux:                            ****/
  if (current_bkg() )
    SDL_BlitSurface(current_bkg(), NULL, screen, &bkg_rect);
  if (images[IMG_MENU_TITLE])
    SDL_BlitSurface(images[IMG_MENU_TITLE], NULL, screen, &title_rect);
  if (Tux && Tux->frame[0])
    SDL_BlitSurface(Tux->frame[0], NULL, screen, &tux_rect);
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
              if (Opts_GetGlobalOpt(MENU_SOUND) && (old_loc != loc_screen_start + i))
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
              if (Opts_GetGlobalOpt(MENU_SOUND) && click_flag)
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
              if (Opts_GetGlobalOpt(MENU_SOUND) && click_flag)
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
              if (Opts_GetGlobalOpt(MENU_SOUND))
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
              if (Opts_GetGlobalOpt(MENU_SOUND))
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
              if (Opts_GetGlobalOpt(MENU_SOUND))
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
              if (Opts_GetGlobalOpt(MENU_SOUND))
                playsound(SND_POP);
              stop = 1;
              break;
            }


            /* Go to previous page, if present: */
            case SDLK_LEFT:
            case SDLK_PAGEUP:
            {
              if (Opts_GetGlobalOpt(MENU_SOUND))
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
              if (Opts_GetGlobalOpt(MENU_SOUND))
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
              if (Opts_GetGlobalOpt(MENU_SOUND))
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
              if (Opts_GetGlobalOpt(MENU_SOUND))
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

                RenderTitleScreen();
                RecalcMenuPositions(&n_entries_per_screen,
                                    n_menu_entries,
                                    &menu_opts,
                                    set_custom_menu_opts,
                                    &menu_button_rect,
                                    &menu_sprite_rect,
                                    &menu_text_rect,
                                    &back_button_rect,
                                    &back_sprite_rect,
                                    &back_text_rect,
                                    &left_arrow_rect,
                                    &right_arrow_rect);
                //we're unsure how the entries might shuffle, so return to start
                loc_screen_start = 0;
                redraw = 1;
              }
              break;
            }


            /* Toggle screen mode: */
            case SDLK_F10:
            {
              SwitchScreenMode();
              RenderTitleScreen();
              RecalcMenuPositions(&n_entries_per_screen,
                                  n_menu_entries,
                                  &menu_opts,
                                  set_custom_menu_opts,
                                  &menu_button_rect,
                                  &menu_sprite_rect,
                                  &menu_text_rect,
                                  &back_button_rect,
                                  &back_sprite_rect,
                                  &back_text_rect,
                                  &left_arrow_rect,
                                  &right_arrow_rect);
              //we're unsure how the entries might shuffle, so return to start
              loc_screen_start = 0;
              redraw = 1;
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
#ifdef TESTING_CAMPAIGN
            case SDLK_c:
            {
              start_campaign();
              RenderTitleScreen();
              RecalcMenuPositions(&n_entries_per_screen,
                                  n_menu_entries,
                                  &menu_opts,
                                  set_custom_menu_opts,
                                  &menu_button_rect,
                                  &menu_sprite_rect,
                                  &menu_text_rect,
                                  &back_button_rect,
                                  &back_sprite_rect,
                                  &back_text_rect,
                                  &left_arrow_rect,
                                  &right_arrow_rect);
              redraw = 1;
            }

#endif
            default:
            {
              /* Some other key - do nothing. */
            }

            break;  /* To get out of _outer_ switch/case statement */
          }  /* End of key switch statement */
        }  // End of case SDL_KEYDOWN in outer switch statement
      }  // End event switch statement

      if(HandleTitleScreenEvents(&event))
        redraw = 1;
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
      DEBUGMSG(debug_menu, "Updating entire screen\n");
      /* This is a full-screen redraw */
      /* Redraw background, title, stop button, and Tux: */
      if (!current_bkg() || screen->flags & SDL_FULLSCREEN )
        SDL_FillRect(screen, &screen->clip_rect, 0); //clear to black
      if (current_bkg())
        SDL_BlitSurface(current_bkg(), NULL, screen, &bkg_rect);
      if (images[IMG_MENU_TITLE])
        SDL_BlitSurface(images[IMG_MENU_TITLE], NULL, screen, &title_rect);
      if (images[IMG_STOP])
        SDL_BlitSurface(images[IMG_STOP], NULL, screen, &stopRect);
      if (Tux->frame[0])
        SDL_BlitSurface(Tux->frame[0], NULL, screen, &tux_rect);
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
        set_buttons_max_width(menu_button_rect,back_button_rect,n_entries_per_screen);
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

      SDL_Flip(screen);//SDL_UpdateRect(screen, 0, 0, 0 ,0);
    } else if (old_loc != loc) {
      // This is not a full redraw, but the selected entry did change.
      // By just redrawing the old and new selections, we avoid flickering.
      if (old_loc >= 0) {
        imod = old_loc-loc_screen_start;
        use_sprite = (menu_sprites != NULL && old_loc >= title_offset && menu_sprites[old_loc-title_offset] != NULL);
        temp_rect = menu_button_rect[imod];
        SDL_FillRect(screen, &temp_rect, 0);
        SDL_BlitSurface(current_bkg(), &back_button_rect[imod], screen, &temp_rect);   // redraw background
        if (use_sprite) {
          // Some of the sprites extend beyond the menu button, so we
          // have to make sure we redraw in the sprite rects, too
          SDL_BlitSurface(current_bkg(), &back_sprite_rect[imod], screen, &temp_rect);
        }
        DrawButton(&menu_button_rect[imod], 10, REG_RGBA);  // draw button
        //temp_rect = menu_text_rect[imod];
        SDL_BlitSurface(menu_item_unselected[old_loc], NULL, screen, &menu_text_rect[imod]);  // draw text
        if (use_sprite) {
          temp_rect = menu_sprite_rect[imod];
          DEBUGMSG(debug_menu, "Sprite %d at (%d %d)\n",  imod, temp_rect.x, temp_rect.y);
          SDL_BlitSurface(menu_sprites[old_loc-title_offset]->default_img, NULL, screen, &temp_rect);
          // Also update the sprite rect (in some cases the sprite
          // extends beyond the menu button)
          SDL_UpdateRect(screen, menu_sprite_rect[imod].x, menu_sprite_rect[imod].y, menu_sprite_rect[imod].w, menu_sprite_rect[imod].h);
        }
        SDL_UpdateRect(screen, menu_button_rect[imod].x, menu_button_rect[imod].y, menu_button_rect[imod].w, menu_button_rect[imod].h);
      }
      if (loc >= 0) {
        imod = loc-loc_screen_start;
        use_sprite = (menu_sprites != NULL && loc >= title_offset && menu_sprites[loc] != NULL);
        temp_rect = menu_button_rect[imod];
        SDL_BlitSurface(current_bkg(), &(back_button_rect[imod]), screen, &temp_rect);
        if (use_sprite)
        {
          temp_rect = menu_sprite_rect[imod];
          SDL_BlitSurface(current_bkg(), &(back_sprite_rect[imod]), screen, &temp_rect);
        }
        DrawButton(&menu_button_rect[imod], 10, SEL_RGBA);
        SDL_BlitSurface(menu_item_selected[loc], NULL, screen, &menu_text_rect[imod]);
        if (use_sprite) {
          menu_sprites[loc-title_offset]->cur = 0;  // start at beginning of animation sequence
          SDL_BlitSurface(menu_sprites[loc-title_offset]->frame[menu_sprites[loc-title_offset]->cur], NULL, screen, &menu_sprite_rect[imod]);
          SDL_UpdateRect(screen, menu_sprite_rect[imod].x, menu_sprite_rect[imod].y, menu_sprite_rect[imod].w, menu_sprite_rect[imod].h);
          NextFrame(menu_sprites[loc-title_offset]);
        }
        SDL_UpdateRect(screen, menu_button_rect[imod].x, menu_button_rect[imod].y, menu_button_rect[imod].w, menu_button_rect[imod].h);
        DEBUGMSG(debug_menu, "Updating rect: %d %d %d %d\n", menu_button_rect[imod].x, menu_button_rect[imod].y, menu_button_rect[imod].w, menu_button_rect[imod].h);
      }
    } else if (frame_counter % 5 == 0 && loc >= 0) {
      // No user input changed anything, but check to see if we need to
      // animate the sprite
      if (menu_sprites != NULL && loc >= title_offset && menu_sprites[loc-title_offset] != NULL) {
        imod = loc-loc_screen_start;
        //SDL_BlitSurface(current_bkg, &menu_button_rect[imod], screen, &menu_button_rect[imod]);
        temp_rect = menu_sprite_rect[imod];
        SDL_BlitSurface(current_bkg(), &back_sprite_rect[imod], screen, &temp_rect);
        DrawButton(&menu_button_rect[imod], 10, SEL_RGBA);
        //SDL_BlitSurface(menu_item_selected[loc], NULL, screen, &menu_text_rect[imod]);
        // Note: even though the whole button was redrawn, we don't
        // have to redraw the text & background as long as we don't
        // update that rect. If something else changes and we go to
        // full-screen updates, then remove the "commenting-out" on
        // the two lines above
        SDL_BlitSurface(menu_sprites[loc-title_offset]->frame[menu_sprites[loc-title_offset]->cur], NULL, screen, &menu_sprite_rect[imod]);
        SDL_UpdateRect(screen, menu_sprite_rect[imod].x, menu_sprite_rect[imod].y, menu_sprite_rect[imod].w, menu_sprite_rect[imod].h);
        NextFrame(menu_sprites[loc-title_offset]);
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

    HandleTitleScreenAnimations();

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
  free(back_text_rect);
  free(back_button_rect);
  free(menu_sprite_rect);
  free(back_sprite_rect);

  /* Return the value of the chosen item (-1 indicates escape) */
  if (stop == 2)
    return -1;
  else
    return loc - title_offset;
}



void set_buttons_max_width(SDL_Rect *menu_button_rect,
                           SDL_Rect *back_button_rect, int n)
{
  int i,max;

  max = 0;
  for (i = 0; i < n; i++)
    if (max < menu_button_rect[i].w)
      max = menu_button_rect[i].w;

  for (i = 0; i < n; i++)
    menu_button_rect[i].w = back_button_rect[i].w = max;

  DEBUGMSG(debug_menu, "All buttons at width %d\n", max);
}

/* Was in playgame.c in tuxtype: */

/* trans_wipe: Performs various wipes to new bkgs
   Given a wipe request type, and any variables
   that wipe requires, will perform a wipe from
   the current screen image to a new one. */
void trans_wipe(SDL_Surface* newbkg, int type, int var1, int var2)
{
  int i, j, x1, x2, y1, y2;
  int step1, step2, step3, step4;
  int frame;
  SDL_Rect src;
  SDL_Rect dst;

  if (!screen)
  {
    DEBUGMSG(debug_titlescreen, "trans_wipe(): screen not valid!\n");
    return;
  }

  if (!newbkg)
  {
    DEBUGMSG(debug_titlescreen, "trans_wipe(): newbkg not valid!\n");
    return;
  }

  init_blits();

  numupdates = 0;
  frame = 0;

  if(newbkg->w == screen->w && newbkg->h == screen->h) {
    if( type == RANDOM_WIPE )
      type = (RANDOM_WIPE * ((float) rand()) / (RAND_MAX+1.0));

    switch( type ) {
      case WIPE_BLINDS_VERT: {
        DEBUGMSG(debug_titlescreen, "trans_wipe(): Doing 'WIPE_BLINDS_VERT'\n");
        /*var1 isnum ofdivisions
          var2is howmany framesanimation shouldtake */
        if(var1 <1 )var1 =1;
        if( var2< 1) var2= 1;
        step1= screen->w/ var1;
        step2= step1/ var2;

        for(i= 0;i <=var2; i++)
        {
          for(j= 0;j <=var1; j++)
          {
            x1= step1* (j- 0.5)- i* step2+ 1;
            x2= step1* (j- 0.5)+ i* step2+ 1;
            src.x= x1;
            src.y= 0;
            src.w= step2;
            src.h= screen->h;
            dst.x= x2;
            dst.y= 0;
            dst.w= step2;
            dst.h= screen->h;

            SDL_BlitSurface(newbkg,&src, screen,&src);
            SDL_BlitSurface(newbkg, &dst,screen, &dst);

            add_rect(&src,&src);
            add_rect(&dst, &dst);
          }
          update_screen(&frame);
        }

        src.x= 0;
        src.y= 0;
        src.w= screen->w;
        src.h= screen->h;
        SDL_BlitSurface(newbkg,NULL, screen,&src);
        SDL_Flip(screen);

        break;
      }
      case WIPE_BLINDS_HORIZ:{
        DEBUGMSG(debug_titlescreen, "trans_wipe(): Doing 'WIPE_BLINDS_HORIZ'\n");
        /* var1is numof divisions
         var2 ishow manyframes animationshould take*/
        if( var1< 1) var1= 1;
        if(var2 <1 )var2 =1;
        step1 =screen->h /var1;
        step2 =step1 /var2;

        for(i =0; i<= var2;i++) {
          for(j= 0;j <=var1; j++){
            y1 =step1 *(j -0.5) -i *step2 +1;
            y2 =step1 *(j -0.5) +i *step2 +1;
            src.x =0;
            src.y =y1;
            src.w =screen->w;
            src.h =step2;
            dst.x =0;
            dst.y =y2;
            dst.w =screen->w;
            dst.h =step2;

            SDL_BlitSurface(newbkg, &src,screen, &src);
            SDL_BlitSurface(newbkg,&dst, screen,&dst);

            add_rect(&src, &src);
            add_rect(&dst,&dst);
          }
          update_screen(&frame);
        }

        src.x =0;
        src.y =0;
        src.w =screen->w;
        src.h =screen->h;
        SDL_BlitSurface(newbkg, NULL,screen, &src);
        SDL_Flip(screen);

        break;
      }
      case WIPE_BLINDS_BOX:{
        DEBUGMSG(debug_titlescreen, "trans_wipe(): Doing 'WIPE_BLINDS_BOX'\n");
        /* var1is numof divisions
         var2 ishow manyframes animationshould take*/
        if( var1< 1) var1= 1;
        if(var2 <1 )var2 =1;
        step1 =screen->w /var1;
        step2 =step1 /var2;
        step3 =screen->h /var1;
        step4 =step1 /var2;

        for(i =0; i<= var2;i++) {
          for(j= 0;j <=var1; j++){
            x1 =step1 *(j -0.5) -i *step2 +1;
            x2 =step1 *(j -0.5) +i *step2 +1;
            src.x =x1;
            src.y =0;
            src.w =step2;
            src.h =screen->h;
            dst.x =x2;
            dst.y =0;
            dst.w =step2;
            dst.h =screen->h;

            SDL_BlitSurface(newbkg, &src,screen, &src);
            SDL_BlitSurface(newbkg,&dst, screen,&dst);

            add_rect(&src, &src);
            add_rect(&dst,&dst);
            y1 =step3 *(j -0.5) -i *step4 +1;
            y2 =step3 *(j -0.5) +i *step4 +1;
            src.x =0;
            src.y =y1;
            src.w =screen->w;
            src.h =step4;
            dst.x =0;
            dst.y =y2;
            dst.w =screen->w;
            dst.h =step4;
            SDL_BlitSurface(newbkg, &src,screen, &src);
            SDL_BlitSurface(newbkg,&dst, screen,&dst);
            add_rect(&src, &src);
            add_rect(&dst,&dst);
          }
          update_screen(&frame);
        }

        src.x =0;
        src.y =0;
        src.w =screen->w;
        src.h =screen->h;
        SDL_BlitSurface(newbkg, NULL,screen, &src);
        SDL_Flip(screen);

        break;
      }
      default:
        break;
    }
  }
  DEBUGMSG(debug_titlescreen, "trans_wipe(): FINISH\n");
}

/* InitEngine - Set up the update rectangle pointers
   (user by trans_wipe() ) */
void init_blits(void) {
  int i;

  for (i = 0; i < MAX_UPDATES; ++i) {
    blits[i].srcrect = &srcupdate[i];
    blits[i].dstrect = &dstupdate[i];
  }
}


/* update_screen : Update the screen and increment the frame num
   (used by trans_wipe() ) */
void update_screen(int *frame) {
  int i;

  /* -- First erase everything we need to -- */
  for (i = 0; i < numupdates; i++)
    if (blits[i].type == 'E')
      SDL_LowerBlit(blits[i].src, blits[i].srcrect, screen, blits[i].dstrect);
//        SNOW_erase();

  /* -- then draw -- */
  for (i = 0; i < numupdates; i++)
    if (blits[i].type == 'D')
      SDL_BlitSurface(blits[i].src, blits[i].srcrect, screen, blits[i].dstrect);
//        SNOW_draw();

/* -- update the screen only where we need to! -- */
//        if (SNOW_on)
//                SDL_UpdateRects(screen, SNOW_add( (SDL_Rect*)&dstupdate, numupdates ), SNOW_rects);
//        else
    SDL_UpdateRects(screen, numupdates, dstupdate);

  numupdates = 0;
  *frame = *frame + 1;
}


/* add_rect: Don't actually blit a surface,
   but add a rect to be updated next update
   (used by trans_wipe() ) */
void add_rect(SDL_Rect* src, SDL_Rect* dst) {
  /*borrowed from SL's alien (and modified)*/

  struct blit *update;

  if (!src || !dst)
  {
    DEBUGMSG(debug_titlescreen, "add_rect(): src or dst invalid!\n");
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




void set_default_menu_options(menu_options *menu_opts)
{
  menu_opts->starting_entry = 0;
  menu_opts->xleft = screen->w / 2 - screen->w * 3 / 32;
  menu_opts->ytop = screen->h / 2 - 140;
  // Leave room for arrows at the bottom:
  menu_opts->ybottom = screen->h - images[IMG_LEFT]->h - 20;
  menu_opts->buttonheight = -1;
  menu_opts->ygap = 10;
  menu_opts->button_same_width = 1;
  menu_opts->title = NULL;
  menu_opts->trailer = NULL;
}

/* Recalculate on-screen locations for menus when screen dimensions change */
/* Perhaps consider generalizing this for use in initial menu calculations? */
void RecalcMenuPositions(int* numentries,
                         int totalentries,
                         menu_options* mo,
                         void (*set_custom_menu_opts)(menu_options*),
                         SDL_Rect** menu_button_rect,
                         SDL_Rect** menu_sprite_rect,
                         SDL_Rect** menu_text_rect,
                         SDL_Rect** back_button_rect,
                         SDL_Rect** back_sprite_rect,
                         SDL_Rect** back_text_rect,
                         SDL_Rect* left_arrow_rect,
                         SDL_Rect* right_arrow_rect)
{
  int i;
  SDL_Rect* old_mbr = *menu_button_rect;
  SDL_Rect* old_msr = *menu_sprite_rect;
  SDL_Rect* old_mtr = *menu_text_rect;
  SDL_Rect* old_bbr = *back_button_rect;
  SDL_Rect* old_bsr = *back_sprite_rect;
  SDL_Rect* old_btr = *back_text_rect;

  int old_ne = *numentries;
  int buttonheight = old_mbr->h; //height shouldn't change
  int textwidth = old_mtr->w; //neither should width =P

  right_arrow_rect->x = screen->w - images[IMG_RIGHT]->w - 20;
  right_arrow_rect->y = screen->h - images[IMG_RIGHT]->h - 20;
  left_arrow_rect->x = right_arrow_rect->x - 10 - images[IMG_LEFT]->w;
  left_arrow_rect->y = screen->h - images[IMG_LEFT]->h - 20;

  set_default_menu_options(mo);
  if (set_custom_menu_opts != NULL)
    set_custom_menu_opts(mo);

  *numentries     = (int)(screen->h - mo->ytop+mo->ygap)/(buttonheight + mo->ygap);
  if (*numentries < totalentries)
    *numentries = (int)(mo->ybottom - mo->ytop+mo->ygap)/(buttonheight + mo->ygap);
  if (*numentries > totalentries)
    *numentries = totalentries;




  /**** Memory allocation for new screen rects  ****/
  *menu_text_rect = (SDL_Rect*) malloc(*numentries * sizeof(SDL_Rect));
  *menu_button_rect = (SDL_Rect*) malloc(*numentries * sizeof(SDL_Rect));
  *menu_sprite_rect = (SDL_Rect*) malloc(*numentries * sizeof(SDL_Rect));
  *back_text_rect = (SDL_Rect*) malloc(*numentries * sizeof(SDL_Rect));
  *back_button_rect = (SDL_Rect*) malloc(*numentries * sizeof(SDL_Rect));
  *back_sprite_rect = (SDL_Rect*) malloc(*numentries * sizeof(SDL_Rect));
  if (*menu_text_rect == NULL ||
      *back_text_rect == NULL ||
      *menu_button_rect == NULL ||
      *back_button_rect == NULL ||
      *menu_sprite_rect == NULL ||
      *back_sprite_rect == NULL) {
    free(*menu_text_rect);
    free(*menu_button_rect);
    free(*menu_sprite_rect);
    free(*back_text_rect);
    free(*back_button_rect);
    free(*back_sprite_rect);
    *menu_text_rect = old_mtr;
    *menu_button_rect = old_mbr;
    *menu_sprite_rect = old_msr;
    *numentries = old_ne;
    return;
  }
  else {
    free(old_mtr);
    free(old_mbr);
    free(old_msr);
    free(old_btr);
    free(old_bbr);
    free(old_bsr);
  }


  //note: the [0] notation is merely to avoid typing out (*menu_xxx_rect)[i]
  for (i = 0; i < *numentries; i++)
  {
    menu_button_rect[0][i].x = mo->xleft;
    menu_text_rect[0][i].x = mo->xleft + 15;  // 15 is left gap
    menu_text_rect[0][i].x += 60;  // for now, assume we have a sprite
    if (i > 0)
      menu_text_rect[0][i].y = menu_text_rect[0][i - 1].y + buttonheight + mo->ygap;
    else
      menu_text_rect[0][i].y = mo->ytop;

    menu_button_rect[0][i].y = menu_text_rect[0][i].y - 5;
    menu_text_rect[0][i].h = buttonheight - 10;
    menu_button_rect[0][i].h = buttonheight;

    menu_text_rect[0][i].w = textwidth;
    menu_button_rect[0][i].w = menu_text_rect[0][i].w + 30;

    if (menu_sprite_rect != NULL) {
      menu_sprite_rect[0][i].x = menu_button_rect[0][i].x + 3;
      menu_sprite_rect[0][i].y = menu_button_rect[0][i].y + 3;
      menu_sprite_rect[0][i].w = 40;
      menu_sprite_rect[0][i].h = 50;
    }

    DEBUGMSG(debug_menu, "***Rects[%d]****\n", i);
    DEBUGMSG(debug_menu, "%3d %3d %3d %3d\n", menu_button_rect[0][i].x, menu_button_rect[0][i].y, menu_button_rect[0][i].w, menu_button_rect[0][i].h);
    DEBUGMSG(debug_menu, "%3d %3d %3d %3d\n", menu_text_rect[0][i].x, menu_text_rect[0][i].y, menu_text_rect[0][i].w, menu_text_rect[0][i].h);
    DEBUGMSG(debug_menu, "%3d %3d %3d %3d\n", menu_sprite_rect[0][i].x, menu_sprite_rect[0][i].y, menu_sprite_rect[0][i].w, menu_sprite_rect[0][i].h);
    DEBUGMSG(debug_menu, "***************\n");
  }
  for (i = 0; i < *numentries; ++i)
  {
    back_button_rect[0][i] = menu_button_rect[0][i];
    back_button_rect[0][i].x -= bkg_rect.x;
    back_button_rect[0][i].y -= bkg_rect.y;

    back_text_rect[0][i] = menu_text_rect[0][i];
    back_text_rect[0][i].x -= bkg_rect.x;
    back_text_rect[0][i].y -= bkg_rect.y;

    back_sprite_rect[0][i] = menu_sprite_rect[0][i];
    back_sprite_rect[0][i].x -= bkg_rect.x;
    back_sprite_rect[0][i].y -= bkg_rect.y;
  }

}

int handle_easter_egg(const SDL_Event* evt)
  {
  static int eggtimer = 0;
  int tuxframe;

  // Avoid segfaults if needed images not available:
  if (!Tux || !egg)
  {
    fprintf(stderr,
      "handle_easter_egg() - needed images not avail, bailing out\n");
    egg_active = 0;
    return 1;
  }

  tuxframe = Tux->num_frames;


  if (egg_active) //are we using the egg cursor?
    {

    if (eggtimer < SDL_GetTicks() ) //time's up
      {
      SDL_ShowCursor(SDL_ENABLE);
      //SDL_FillRect(screen, &cursor, 0);
      SDL_BlitSurface(current_bkg(), NULL, screen, &bkg_rect); //cover egg up once more
      SDL_WarpMouse(cursor.x, cursor.y);
      SDL_UpdateRect(screen, cursor.x, cursor.y, cursor.w, cursor.h); //egg->x, egg->y, egg->w, egg->h);
      egg_active = 0;
      }
    return 1;
    }
  else //if not, see if the user clicked Tux's beak
    {
    eggtimer = 0;
    if (evt->type == SDL_MOUSEBUTTONDOWN &&
          inRect(beak, evt->button.x, evt->button.y) )
      {
      SDL_ShowCursor(SDL_DISABLE);

      //animate
      while (tuxframe != 0)
        {
        SDL_BlitSurface(current_bkg(), &tux_rect, screen, &tux_rect);
        SDL_BlitSurface(Tux->frame[--tuxframe], NULL, screen, &tux_rect);
        SDL_UpdateRect(screen, tux_rect.x, tux_rect.y, tux_rect.w, tux_rect.h);
        SDL_Delay(GOBBLE_ANIM_MS / Tux->num_frames);
        }

      eggtimer = SDL_GetTicks() + EASTER_EGG_MS;
      egg_active = 1;
      SDL_WarpMouse(tux_rect.x + tux_rect.w / 2, tux_rect.y + tux_rect.h - egg->h);

      }

    return 0;
    }
  }
