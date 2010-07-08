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

extern int waves_parsed[MAX_WAVES+1];

//main function for parsing and writing
int manage_xmlLesson(char *);

//input data
struct input_per_wave
{
int wave_no;
}; 

//for result 
struct result_per_wave
{
int wave_completed;  //bool to store the status of wave
int wave_no;
Uint32 wave_time;
}; 

