/*
  menu.c

  Functions responsible for loading, parsing and displaying game menu.

  Part of "Tux4Kids" Project
  http://www.tux4kids.com/

  Author: Boleslaw Kulbabinski <bkulbabinski@gmail.com>, (C) 2009

  Copyright: See COPYING file that comes with this distribution.
*/

#include "globals.h"
#include "menu.h"

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
  MENU_GAME_DIFFICULTY,
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


/* creates new MenuNode struct with all field set to NULL (or 0) */
MenuNode* create_empty_node()
{
  MenuNode* new_node = malloc(sizeof(MenuNode));
  new_node->title = NULL;
  new_node->sprite = NULL;
  new_node->choice = 0;
  new_node->submenu_size = 0;
  new_node->submenu = NULL;

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
      node->sprite = strdup(attr_val);
    else if(strcmp(attr_name, "run") == 0)
    {
      for(i = 0; i < N_OF_ACTIVITIES; i++)
        if(strcmp(attr_val, activities[i]) == 0)
          node->choice = i;
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
    if(menu->sprite != NULL)
      free(menu->sprite);

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

/* display the menu
   if return_choice = true then return chosen value instead of
   running handle_choice() */
int run_menu(MenuNode* menu, bool return_choice)
{
  
}

/* load menu trees from disk */
void LoadMenus(void)
{
  FILE* main_menu_file = fopen(DATA_PREFIX "/menus/main_menu.xml", "r");
  if(main_menu_file == NULL)
  {
    DEBUGMSG(debug_menu, "LoadMenus(): Could not load main menu file !\n");
  }
  else
  {
    menus[MENU_MAIN] = load_menu_from_file(main_menu_file);
    fclose(main_menu_file);
  }
}

/* create login menu tree, run it and set the user home directory
   -1 indicates that the user wants to quit without logging in,
    0 indicates that a choice has been made. */
int RunLoginMenu(void)
{

}

/* run main menu. If this function ends it means that tuxmath is going to quit */ 
void RunMainMenu(void)
{
  run_menu(menus[MENU_MAIN], false);
}

/* calculate proper sizes and positions of menu elements,
   load sprites */
void RenderMenus(void)
{

}

/* free all loaded menu trees */
void UnloadMenus(void)
{
  int i;

  DEBUGMSG(debug_menu, "entering UnloadMenus()\n");
  for(i = 0; i < N_OF_MENUS; i++)
    free_menu(menus[i]);
}

