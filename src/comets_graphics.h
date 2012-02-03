#ifndef COMETS_GRAPHICS_H
#define COMETS_GRAPHICS_H

#define IMG_CITY_NONE 0

#include <stdbool.h>

#include "comets.h"


void comets_draw_background(SDL_Surface *bkgd, int wave);

void comets_draw_comets(const comet_type *comets);

void comets_draw_comet_nums(const comet_type *comet, bool answered, SDL_Color *col);

void comets_draw_cities(int igloo_vertical_offset, const cloud_type *cloud, const city_type *cities,
        const penguin_type *penguins, const steam_type *steam);

void comets_draw_misc(MC_MathGame *curr_game, int wave, int extra_life_earned, int bonus_comet_counter,
        int score, int total_questions_left, help_controls_type *help_controls);

void comets_draw_question_counter(MC_MathGame *curr_game, int total_questions_left);

void comets_draw_led_console(MC_MathGame *curr_game, int neg_answer_picked, int *digits);

void comets_draw_powerup(powerup_comet_type* powerup_comet);

void comets_draw_smartbomb(int smartbomb_alive);


#endif
