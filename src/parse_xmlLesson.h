
// 1 extra for storing '\0'
#define MAX_MENU_ITEMS 21
#define MENU_ITEM_LENGTH 15
#define MAX_WAVES 20
//char * menu_names[MAX_MENU_ITEMS];
extern char menu_names[MAX_MENU_ITEMS][MENU_ITEM_LENGTH];
extern int total_no_menus;  // total no of menus

extern int waves_parsed[MAX_WAVES+1];


//Functions used in parse_xmlLesson.c
//void parse_xmlLesson(void);
