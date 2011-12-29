#include <t4k_common.h>

#include "draw_utils.h"
#include "tuxmath.h"
#include "fileops.h"


float get_scale(void)
{
    /* Adjust font size for resolution - note that it doesn't have to be as
     * proportionately large on larger screens, hence the pow() step.
     * The degree to which the font enlarges with larger screen size can be
     * tweaked by adjusting SCALE_EXPONENT.
     */
    int win_w, win_h, full_w, full_h;

    T4K_GetResolutions(&win_w, &win_h, &full_w, &full_h);
    if(T4K_GetScreen()->h == full_h)
        return  pow(((float)full_h/(float)win_h), SCALE_EXPONENT);
    else
        return  1;
}


/* Draw a line: */
void draw_line(SDL_Surface* surface, int x1, int y1, int x2, int y2, int red, int grn, int blu)
{
    if(!surface)
        surface = T4K_GetScreen();

    int dx, dy, tmp;
    float m, b;
    Uint32 pixel;
    SDL_Rect dest;

    pixel = SDL_MapRGB(surface->format, red, grn, blu);

    dx = x2 - x1;
    dy = y2 - y1;

    putpixel(surface, x1, y1, pixel);

    if (dx != 0)
    {
        m = ((float) dy) / ((float) dx);
        b = y1 - m * x1;

        if (x2 > x1)
            dx = 1;
        else
            dx = -1;

        while (x1 != x2)
        {
            x1 = x1 + dx;
            y1 = m * x1 + b;

            putpixel(surface, x1, y1, pixel);
        }
    }
    else
    {
        if (y1 > y2)
        {
            tmp = y1;
            y1 = y2;
            y2 = tmp;
        }

        dest.x = x1;
        dest.y = y1;
        dest.w = 3;
        dest.h = y2 - y1;

        SDL_FillRect(surface, &dest, pixel);
    }
}


/* Draw a single pixel into the surface: */

void putpixel(SDL_Surface* surface, int x, int y, Uint32 pixel)
{
#ifdef PUTPIXEL_RAW
    int bpp;
    Uint8* p;

    /* Determine bytes-per-pixel for the surface in question: */

    bpp = surface->format->BytesPerPixel;


    /* Set a pointer to the exact location in memory of the pixel
       in question: */

    p = (Uint8 *) (surface->pixels +       /* Start at beginning of RAM */
            (y * surface->pitch) +  /* Go down Y lines */
            (x * bpp));             /* Go in X pixels */


    /* Assuming the X/Y values are within the bounds of this surface... */

    if (x >= 0 && y >= 0 && x < surface->w && y < surface->h)
    {
        /* Set the (correctly-sized) piece of data in the surface's RAM
           to the pixel value sent in: */

        if (bpp == 1)
            *p = pixel;
        else if (bpp == 2)
            *(Uint16 *)p = pixel;
        else if (bpp == 3)
        {
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            }
            else
            {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
        }
        else if (bpp == 4)
        {
            *(Uint32 *)p = pixel;
        }
    }
#else
    SDL_Rect dest;

    dest.x = x;
    dest.y = y;
    dest.w = 3;
    dest.h = 4;

    SDL_FillRect(surface, &dest, pixel);
#endif
}


void draw_numbers(SDL_Surface* surface, const char* str, int x, int y)
{
    int i, cur_x, c;
    SDL_Rect src, dest;

    cur_x = x;

    /* Draw each character: */

    for (i = 0; i < strlen(str); i++)
    {
        c = -1;

        /* Determine which character to display: */
        if (str[i] >= '0' && str[i] <= '9')
            c = str[i] - '0';

        /* Display this character! */
        if (c != -1)
        {
            src.x = c * (images[IMG_NUMBERS]->w / 10);
            src.y = 0;
            src.w = (images[IMG_NUMBERS]->w / 10);
            src.h = images[IMG_NUMBERS]->h;

            dest.x = cur_x;
            dest.y = y;
            dest.w = src.w;
            dest.h = src.h;

            SDL_BlitSurface(images[IMG_NUMBERS], &src,
                    surface, &dest);

            /* Move the 'cursor' one character width: */
            cur_x = cur_x + (images[IMG_NUMBERS]->w / 10);
        }
    }
}


/* Draw numbers/symbols over the attacker: */
void draw_nums(float zoom, const char* str, int x, int y, SDL_Color* col)
{
    if(!str || !col)
        return;

    SDL_Surface* surf = NULL;
    surf = T4K_BlackOutline(str, 48 * zoom, col);
    if(surf)
    {
        int w = T4K_GetScreen()->w;
        x -= surf->w/2;
        // Keep formula at least 8 pixels inside screen:
        if(surf->w + x > (w - 8))
            x -= (surf->w + x - (w - 8));
        if(x < 8)
            x = 8;

        SDL_Rect pos = {x, y};
        SDL_BlitSurface(surf, NULL, T4K_GetScreen(), &pos);
        SDL_FreeSurface(surf);
    }
}


/* Draw image at lower center of screen: */
void draw_console_image(int i)
{
    SDL_Rect dest;

    dest.x = (screen->w - images[i]->w) / 2;
    dest.y = (screen->h - images[i]->h);
    dest.w = images[i]->w;
    dest.h = images[i]->h;

    SDL_BlitSurface(images[i], NULL, screen, &dest);
}
