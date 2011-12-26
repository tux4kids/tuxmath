#ifndef DRAW_UTILS_H
#define DRAW_UTILS_H

#define SCALE_EXPONENT 0.7

#include <SDL_video.h>


float get_scale();

void putpixel(SDL_Surface* surface, int x, int y, Uint32 pixel);

void draw_line(SDL_Surface* surface, int x1, int y1, int x2, int y2, int r, int g, int b);

void draw_numbers(SDL_Surface* surface, const char* str, int x, int y);

void draw_nums(float zoom, const char* str, int x, int y, SDL_Color* col);

void draw_console_image(int i);


#endif
