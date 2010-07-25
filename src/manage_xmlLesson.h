/***************************************************************
 *  manage_xmlLesson.h                                          *
 *                                                              *
 *  Description:  Headers for managing XML files generated      *                                       *
 *  Author:       Vikas Singh 					*
 *                 vikassingh008@gmail.com ,2010 		*
 *  Copyright:    GPL v3 or later                               *
 *  							    	*
 *  						          	*
 *                                                          	*
 *                                                          	*
 *  TuxMath                                                 	*
 *  Part of "Tux4Kids" Project                              	*
 *  http://tux4kids.alioth.debian.org/                      	*
 ***************************************************************/

#include "tuxmath.h"

#define MAX_MENU_ITEMS 21
#define MENU_ITEM_LENGTH 15
#define MAX_WAVES 20   

//char * menu_names[MAX_MENU_ITEMS];
extern char menu_names[MAX_MENU_ITEMS][MENU_ITEM_LENGTH];
extern int total_no_menus;  // total no of menus

//extern int waves_parsed[MAX_WAVES+1];

//main function for parsing and writing
int manage_xmlLesson(char *);

//The following implementation uses the same input/result variables for
//fractions and factors as only one game can be played at a time.


//input data
struct input_per_wave
{
int wave_no;
}; 

struct input_factoroids_game
{
struct input_per_wave *wave_input;
int lives;
};

struct input_factoroids_game input_factoroids;





//for result 
struct result_per_wave
{
int wave_completed;  //bool to store the status of wave
int wave_no;
Uint32 wave_time;
}; 


struct result_factoroids_game
{
struct result_per_wave *wave_result;
int score;
int lives_remaining;
};

struct result_factoroids_game result_factoroids;


//used to store whether game was completed or not 
int *game_completed,current_game_index;


