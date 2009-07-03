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
#include "options.h"
#include "fileops.h"

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
  N_OF_MENUS
} MenuType;

MenuNode* menus[N_OF_MENUS];

/* buffer size used when reading attributes or names */
const int buf_size = 128;

/* local functions */
MenuNode* create_empty_node();
char* get_attribute_name(const char* token);
char* get_attribute_value(const char* token);
void read_attributes(FILE* xml_file, MenuNode* node);
MenuNode* load_menu_from_file(FILE* xml_file);
void free_menu(MenuNode* menu);

SDL_Surface** render_buttons(MenuNode* menu, bool selected);



/* creates new MenuNode struct with all fields set to NULL (or 0) */
MenuNode* create_empty_node()
{
  MenuNode* new_node = malloc(sizeof(MenuNode));
  new_node->title = NULL;
  new_node->icon_name = NULL;
  new_node->icon = NULL;
  new_node->submenu_size = 0;
  new_node->submenu = NULL;
  new_node->activity = 0;
  new_node->begin = 0;

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
MenuNode* load_menu_from_file(FILE* xml_file)
{
  MenuNode* new_node = create_empty_node();
  char buffer[buf_size];
  int i;

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
        new_node->submenu[i] = load_menu_from_file(xml_file);
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
    DEBUGMSG(debug_menu_parser, "load_menu_from_file(): unknown tag: %s\n, exiting", buffer);
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



/* Display the menu and run the event loop.
   if return_choice = true then return chosen value instead of
   running handle_choice() */
int run_menu(MenuNode* menu, bool return_choice)
{
  SDL_Surface **menu_item_unselected = NULL;
  SDL_Surface **menu_item_selected = NULL;
  SDL_Event event;
  int redraw, i;
  int stop = 0;
  int items;

  for(;;) /* one execution for one menu level */
  {
    /* render buttons for current menu page */
    menu_item_unselected = render_buttons(menu, false);
    menu_item_selected = render_buttons(menu, true);
    items = min(menu->entries_per_page, menu->submenu_size - menu->begin);


    redraw = 1;  // force a full redraw on first pass
    while (SDL_PollEvent(&event));  // clear pending events
    while (!stop)
    {
      while (SDL_PollEvent(&event))
      {
        switch (event.type)
        {
          case SDL_MOUSEMOTION:
          {
            for (i = menu->begin; i < menu->begin + items; i++)
            {
              if (inRect(menu->submenu[i]->button_rect, event.motion.x, event.motion.y))
              {
                // Play sound if loc is being changed:
                if (Opts_GetGlobalOpt(MENU_SOUND))
                {
                  playsound(SND_TOCK);
                }
                break;   /* from for loop */
              }
            }

            break;
          }
        }
      }

      if(redraw)
      {
        for(i = 0; i < menu->submenu_size; i++)
          SDL_BlitSurface(menu_item_unselected[i], NULL, screen, &menu->submenu[i]->button_rect);
        SDL_UpdateRect(screen, 0, 0, 0, 0);
        redraw = 0;
      }
    }
    break;

    /* free button surfaces */
    for(i = 0; i < items; i++)
    {
      SDL_FreeSurface(menu_item_unselected[i]);
      SDL_FreeSurface(menu_item_selected[i]);
    }
    free(menu_item_unselected);
    free(menu_item_selected);
  }

  return -1;
}

SDL_Surface** render_buttons(MenuNode* menu, bool selected)
{
  SDL_Surface** menu_items = NULL;
  SDL_Rect curr_rect, tmp_rect;
  SDL_Surface* tmp_surf = NULL;
  int i;
  int items = min(menu->entries_per_page, menu->submenu_size - menu->begin);

  menu_items = (SDL_Surface**) malloc(items * sizeof(SDL_Surface*));
  if(NULL == menu_items)
  {
    DEBUGMSG(debug_menu, "render_buttons(): failed to allocate memory for buttons!");
    return NULL;  // error
  }

  for (i = 0; i < items; i++)
  {
    curr_rect = menu->submenu[menu->begin + i]->button_rect;
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
    tmp_surf = BlackOutline(_(menu->submenu[menu->begin + i]->title),
                            DEFAULT_MENU_FONT_SIZE, selected ? &yellow : &white);
    tmp_rect = tmp_surf->clip_rect;
    tmp_rect.x = curr_rect.h * 2;
    tmp_rect.y = (curr_rect.h - tmp_surf->h) / 2;
    SDL_BlitSurface(tmp_surf, NULL, menu_items[i], &tmp_rect);
    SDL_FreeSurface(tmp_surf);

    /* icon */
    if(menu->submenu[menu->begin + i]->icon)
    {
      tmp_rect = menu->submenu[menu->begin + i]->icon->default_img->clip_rect;
      tmp_rect.x = 0;
      tmp_rect.y = 0;
      SDL_BlitSurface(menu->submenu[menu->begin + i]->icon->default_img, NULL, menu_items[i], &tmp_rect);
    }
  }

  return menu_items;
}

/* recursively load sprites and calculate button rects
   to fit into current screen */
void render_menu(MenuNode* menu)
{
  SDL_Rect menu_rect;
  SDL_Surface* temp_surf;
  MenuNode* curr_node;
  int i, max_text_h = 0, max_text_w = 0;
  int button_h, button_w;
  float gap;
  char filename[buf_size];

  if(NULL == menu)
  {
    DEBUGMSG(debug_menu, "render_menu(): NULL pointer, exiting !");
    return;
  }

  if(0 == menu->submenu_size)
  {
    DEBUGMSG(debug_menu, "render_menu(): no submenu, exiting.");
    return;
  }

  menu_rect.x = 0.3 * screen->w;
  menu_rect.y = 0.25 * screen->h;
  menu_rect.w = 0.65 * screen->w;
  menu_rect.h = 0.75 * screen->h;

  for(i = 0; i < menu->submenu_size; i++)
  {
    temp_surf = NULL;
    temp_surf = SimpleText(menu->submenu[i]->title, DEFAULT_MENU_FONT_SIZE, &black);
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
  menu->entries_per_page = (int) ( (menu_rect.h - gap * button_h) / (1.0 + gap) );

  for(i = 0; i < menu->submenu_size; i++)
  {
    curr_node = menu->submenu[i];
    curr_node->button_rect.x = menu_rect.x;
    curr_node->button_rect.y = menu_rect.y + i * button_h + (i + 1) * gap * button_h;
    curr_node->button_rect.w = button_w;
    curr_node->button_rect.h = button_h;
    curr_node->font_size = DEFAULT_MENU_FONT_SIZE;

    if(curr_node->icon)
      FreeSprite(curr_node->icon);

    if(curr_node->icon_name)
    {
      sprintf(filename, "sprites/%s", curr_node->icon_name);
      DEBUGMSG(debug_menu, "render_menu(): loading sprite %s for item #%d.\n", filename, i);
      curr_node->icon = LoadSpriteOfBoundingBox(filename, IMG_ALPHA, button_h, button_h);
    }
    else
      DEBUGMSG(debug_menu, "render_menu(): no sprite for item #%d.\n", i);
  }

}

/* load menu trees from disk */
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
    menus[MENU_MAIN] = load_menu_from_file(menu_file);
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
    menus[MENU_DIFFICULTY] = load_menu_from_file(menu_file);
    fclose(menu_file);
  }
}

/* create login menu tree, run it and set the user home directory
   -1 indicates that the user wants to quit without logging in,
    0 indicates that a choice has been made. */
int RunLoginMenu(void)
{
  return 0;
}

/* run main menu. If this function ends it means that tuxmath is going to quit */
void RunMainMenu(void)
{
  DEBUGMSG(debug_menu, "Entering RunMainMenu()\n");
  render_menu(menus[MENU_MAIN]);
  run_menu(menus[MENU_MAIN], false);
  DEBUGMSG(debug_menu, "Leaving RunMainMenu()\n");
}

/* free all loaded menu trees */
void UnloadMenus(void)
{
  int i;

  DEBUGMSG(debug_menu, "entering UnloadMenus()\n");
  for(i = 0; i < N_OF_MENUS; i++)
    free_menu(menus[i]);
}

