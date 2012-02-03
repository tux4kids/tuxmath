#ifndef FACTOROIDS_GRAPHICS_H
#define FACTOROIDS_GRAPHICS_H

#include <SDL_video.h>

#include "factoroids.h"


/* Enumerations for Button Types in Cockpit */
enum BUTTON_TYPE
{
    ACTIVE,
    SELECTED,
    PRESSED,
    DISABLED
};


SDL_Surface* current_bkgd(void);

int factoroids_init_graphics(void);

void factoroids_cleanup_graphics(void);

void factoroids_intro(void);

void factoroids_draw(asteroid_type *asteroid, tuxship_type *tuxship, FF_laser_type *laser,
        int bonus, int bonus_time, int *digits, int wave, int score, int num,
        int tux_img, int button_pressed);

void factoroids_draw_bkgr(void);

void factoroids_draw_led_console(int wave, int num, int button_pressed);

void factoroids_init_buttons(void);

void factoroids_draw_button(int img_id, enum BUTTON_TYPE type, int x, int y);

void factoroids_show_message(char *str);

void factoroids_level_message(int wave);

void factoroids_level_objs_hints(char *label, char *contents, int x, int y );

int tuxship_img_h(int deg);
int tuxship_img_w(int deg);

SDL_Surface* get_asteroid_image(int size,int angle);

#endif
