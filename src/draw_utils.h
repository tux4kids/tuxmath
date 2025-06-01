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

// SDL_BlitSurface wrappers
void T4K_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
void T4K_BlitSurfaceToScreen(SDL_Surface *src, SDL_Rect *srcrect, SDL_Rect *dstrect);
void T4K_BlitEntireSurfaceToScreen(SDL_Surface *src, int x, int y);

// Function to display a list of strings for review
void T4K_DisplayReviewList(const char* title, char** items, int num_items, SDL_Surface* background_surface);

// Function to display a list of strings for review and ask if player wants to replay with them
int T4K_DisplayReviewList_WithOption(const char* title, char** items, int num_items, SDL_Surface* background_surface, int show_play_again_option);


#endif
