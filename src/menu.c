/*
  menu.c

  Functions responsible for loading, parsing and displaying game menu.

  Part of "Tux4Kids" Project
  http://www.tux4kids.com/

  Author: Boleslaw Kulbabinski <bkulbabinski@gmail.com>, (C) 2009

  Copyright: See COPYING file that comes with this distribution.
*/

#include "menu.h"
#include "SDL_extras.h"
#include "titlescreen.h"
#include "mathcards.h"
#include "campaign.h"
#include "game.h"
#include "options.h"
#include "fileops.h"
#include "setup.h"

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
  N_OF_MENUS
} MenuType;

/* actions available while viewing the menu */
enum { NONE, CLICK, PAGEUP, PAGEDOWN, STOP, RESIZED };

MenuNode* menus[N_OF_MENUS];

/* buffer size used when reading attributes or names */
const int buf_size = 128;



/* local functions */
MenuNode* create_empty_node();
char* get_attribute_name(const char* token);
char* get_attribute_value(const char* token);
void read_attributes(FILE* xml_file, MenuNode* node);
MenuNode* load_menu_from_file(FILE* xml_file, MenuNode* parent);
void free_menu(MenuNode* menu);

void handle_activity(int act);
int run_academy(void);

int run_menu(MenuNode* menu, bool return_choice);
void prerender_menu(MenuNode* menu);
SDL_Surface** render_buttons(MenuNode* menu, bool selected);
MenuNode* create_one_level_menu(int items, char** item_names, char* title, char* trailer);


/*
  functions responsible for parsing menu files
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
  new_node->first_entry = 0;

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


/*
  handlers for specific game activities
*/

void handle_activity(int act)
{
  DEBUGMSG(debug_menu, "entering handle_activity()\n");
  switch(act)
  {
    case RUN_CAMPAIGN:
      start_campaign();
      break;

    case RUN_ACADEMY:
      run_academy();
      break;
  }
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
      //RenderTitleScreen();

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
  if (chosen_lesson < 0)
    return 0;
  else
    return 1;
}

/* Display the menu and run the event loop.
   if return_choice = true then return chosen value instead of
   running handle_activity()
   this function is a modified copy of choose_menu_item() */
int run_menu(MenuNode* root, bool return_choice)
{
  SDL_Surface** menu_item_unselected = NULL;
  SDL_Surface** menu_item_selected = NULL;
  //SDL_Surface* title = NULL;
  SDL_Event event;
  MenuNode* menu = root;
  MenuNode* tmp_node;

  SDL_Rect left_arrow_rect, right_arrow_rect, stopRect, tmp_rect;
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
    SDL_UpdateRect(screen, 0, 0, 0, 0);

    /* Move mouse to current button: */
    //cursor.x = menu_button_rect[imod].x + menu_button_rect[imod].w/2;
    //cursor.y = menu_button_rect[imod].y + menu_button_rect[imod].h/2;
    SDL_WM_GrabInput(SDL_GRAB_OFF);

    /******** Main loop:                                *********/
    while (SDL_PollEvent(&event));  // clear pending events

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
            cleanup();
            return -1;
          }

          case SDL_MOUSEMOTION:
          {
            loc = -1;  // By default, don't be in any entry

            for (i = 0; i < items; i++)
            {
              if (inRect(menu->submenu[menu->first_entry + i]->button_rect, event.motion.x, event.motion.y))
              {
                // Play sound if loc is being changed:
                if (Opts_GetGlobalOpt(MENU_SOUND) && old_loc != i)
                  playsound(SND_TOCK);
                loc = i;
                break;   /* from for loop */
              }
            }

            /* "Left" button - make click if button active: */
            if (inRect(left_arrow_rect, event.motion.x, event.motion.y))
            {
              if (menu->first_entry > 0)
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
              if (menu->first_entry + items < menu->submenu_size)
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
            if (inRect(left_arrow_rect, event.motion.x, event.motion.y))
            {
              if (menu->first_entry > 0)
              {
                if (Opts_GetGlobalOpt(MENU_SOUND))
                  playsound(SND_POP);
                action = PAGEDOWN;
              }
              break;  /* from case switch */
            }

            /* "Right" button - go to next page: */
            else if (inRect(right_arrow_rect, event.motion.x, event.motion.y ))
            {
              if (menu->first_entry + items < menu->submenu_size)
              {
                if (Opts_GetGlobalOpt(MENU_SOUND) && click_flag)
                  playsound(SND_POP);
                action = PAGEUP;
              }
              break;  /* from case switch */
            }

          /* "Stop" button - go to main menu: */
            else if (inRect(stopRect, event.button.x, event.button.y ))
            {
              playsound(SND_TOCK);
              action = STOP;
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
                action = STOP;
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
                  action = PAGEDOWN;
                break;
              }

              /* Go to next page, if present: */
              case SDLK_RIGHT:
              case SDLK_PAGEDOWN:
              {
                if (Opts_GetGlobalOpt(MENU_SOUND))
                  playsound(SND_TOCK);
                if (menu->first_entry + items < menu->submenu_size)
                  action = PAGEUP;
                break;
              }

              /* Go up one entry, if present: */
              /*case SDLK_UP:
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
              }*/


              /* Go down one entry, if present: */
              /*case SDLK_DOWN:
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
              }*/

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
                  prerender_menu(menu);
                  action = RESIZED;
                }
                break;
              }

              /* Toggle screen mode: */
              case SDLK_F10:
              {
                SwitchScreenMode();
                RenderTitleScreen();
                prerender_menu(menu);
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
              SDL_BlitSurface(menu->submenu[menu->first_entry + loc]->icon->default_img, NULL, screen, &menu->submenu[menu->first_entry + loc]->icon_rect);
            SDL_UpdateRect(screen, tmp_rect.x, tmp_rect.y, tmp_rect.w, tmp_rect.h);
          }
          old_loc = loc;
        }

        if(HandleTitleScreenEvents(&event))
          stop = true;

        switch(action)
        {
          case RESIZED:
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
                    handle_activity(tmp_node->activity);
                }
              }
              else
                menu = tmp_node;
              stop = true;
            }
            break;
        }

      }  // End of SDL_PollEvent while loop

      if(!stop && frame_counter % 5 == 0 && loc >= 0 && loc < items)
      {
        tmp_sprite = menu->submenu[menu->first_entry + loc]->icon;
        if(tmp_sprite)
        {
          SDL_BlitSurface(menu_item_selected[loc], NULL, screen, &menu->submenu[menu->first_entry + loc]->icon_rect);
          SDL_BlitSurface(tmp_sprite->frame[tmp_sprite->cur], NULL, screen, &menu->submenu[menu->first_entry + loc]->icon_rect);
          UpdateRect(screen, &menu->submenu[menu->first_entry + loc]->icon_rect);
          NextFrame(tmp_sprite);
        }
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

  return -1;
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
      tmp_surf = CreateButton(curr_rect.w, curr_rect.h, 10, SEL_RGBA);
    else
      tmp_surf = CreateButton(curr_rect.w, curr_rect.h, 10, REG_RGBA);

    SDL_BlitSurface(tmp_surf, NULL, menu_items[i], NULL);
    SDL_FreeSurface(tmp_surf);

    /* text */
    tmp_surf = BlackOutline(_(menu->submenu[menu->first_entry + i]->title),
                            DEFAULT_MENU_FONT_SIZE, selected ? &yellow : &white);
    SDL_BlitSurface(tmp_surf, NULL, menu_items[i], &menu->submenu[menu->first_entry + i]->text_rect);
    SDL_FreeSurface(tmp_surf);
  }

  return menu_items;
}

/* recursively load sprites and calculate button rects
   to fit into current screen */
void prerender_menu(MenuNode* menu)
{
  SDL_Rect menu_rect;
  SDL_Surface* temp_surf;
  MenuNode* curr_node;
  int i, imod, max_text_h = 0, max_text_w = 0;
  int button_h, button_w;
  float gap;
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

  menu_rect.x = 0.4 * screen->w;
  menu_rect.y = 0.25 * screen->h;
  menu_rect.w = 0.55 * screen->w;
  menu_rect.h = 0.7 * screen->h;

  for(i = 0; i < menu->submenu_size; i++)
  {
    temp_surf = NULL;
    temp_surf = SimpleText(_(menu->submenu[i]->title), DEFAULT_MENU_FONT_SIZE, &black);
    if(temp_surf)
    {
      max_text_h = max(max_text_h, temp_surf->h);
      max_text_w = max(max_text_w, temp_surf->w);
      SDL_FreeSurface(temp_surf);
    }
  }

  button_h = 2 * max_text_h;
  button_w = max_text_w + 3 * max_text_h;

  gap = 0.2;
  menu->entries_per_screen = (int) ( (menu_rect.h - gap * button_h) / ( (1.0 + gap) * button_h ));

  for(i = 0; i < menu->submenu_size; i++)
  {
    curr_node = menu->submenu[i];
    curr_node->button_rect.x = menu_rect.x;
    imod = i % menu->entries_per_screen;
    curr_node->button_rect.y = menu_rect.y + imod * button_h + (imod + 1) * gap * button_h;
    curr_node->button_rect.w = button_w;
    curr_node->button_rect.h = button_h;

    curr_node->icon_rect = curr_node->button_rect;
    curr_node->icon_rect.w = curr_node->icon_rect.h;

    curr_node->text_rect = curr_node->button_rect;
    curr_node->text_rect.y = 0.25 * button_h;
    curr_node->text_rect.x = (1.0 + gap) * button_h;
    curr_node->text_rect.h -= 0.25 * button_h;
    curr_node->text_rect.w -= (1.0 + gap) * button_h;

    curr_node->font_size = DEFAULT_MENU_FONT_SIZE;

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

/* load menu trees from disk and prerender them */
void LoadMenus(void)
{
  /* main menu */
  FILE* menu_file = fopen(DATA_PREFIX "/menus/main_menu.xml", "r");
  if(menu_file == NULL)
  {
    DEBUGMSG(debug_menu, "LoadMenus(): Could not load main menu file !\n");
  }
  else
  {
    menus[MENU_MAIN] = load_menu_from_file(menu_file, NULL);
    prerender_menu(menus[MENU_MAIN]);
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
    prerender_menu(menus[MENU_DIFFICULTY]);
    fclose(menu_file);
  }
}

/* create a simple one-level menu without sprites.
   all given strings are copied */
MenuNode* create_one_level_menu(int items, char** item_names, char* title, char* trailer)
{
  MenuNode* menu = create_empty_node();
  int i;

  if(title)
    menu->title = strdup(title);
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
  MenuNode* menu;
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

  menu = create_one_level_menu(n_users, user_names, title, trailer_quit);

  while (n_users) {
    // Get the user choice
    prerender_menu(menu);
    chosen_login = run_menu(menu, true);
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
        free_menu(menu);
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
    DEBUGMSG(debug_menu, "aa");
    free_menu(menu);
    menu = create_one_level_menu(n_users, user_names, title, trailer);
  }

  // The user home directory is set, clean up remaining memory
  for (i = 0; i < n_login_questions; i++)
    free(user_login_questions[i]);
  free(user_login_questions);
  free_menu(menu);

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
    tmp_node->submenu[i]->icon_name = lesson_list_goldstars[i] ? "goldstar" : "no_goldstar";
    tmp_node->submenu[i]->title = (char*) malloc( (strlen(lesson_list_titles[i]) + 1) * sizeof(char) );
    strcpy(tmp_node->submenu[i]->title, lesson_list_titles[i]);
    tmp_node->submenu[i]->activity = i;
  }
  menus[MENU_LESSONS] = tmp_node;
  prerender_menu(menus[MENU_LESSONS]);

  run_menu(menus[MENU_MAIN], false);
  DEBUGMSG(debug_menu, "Leaving RunMainMenu()\n");
}

/* free all loaded menu trees */
void UnloadMenus(void)
{
  int i;

  DEBUGMSG(debug_menu, "entering UnloadMenus()\n");
  for(i = 0; i < N_OF_MENUS; i++)
    if(menus[i] != NULL)
      free_menu(menus[i]);
}

