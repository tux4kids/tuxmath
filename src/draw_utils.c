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

// Function to display a list of strings for review and ask if player wants to replay with them
int T4K_DisplayReviewList_WithOption(const char* title, char** items, int num_items, SDL_Surface* background_surface, int show_play_again_option)
{
    SDL_Surface *screen_surface = T4K_GetScreen();
    SDL_Event event;
    int waiting_for_input = 1;
    int i;
    SDL_Color text_color = {255, 255, 255, 255}; // White
    SDL_Color prompt_color = {255, 255, 150, 255}; // Yellowish for prompt
    SDL_Surface* title_surf = NULL;
    SDL_Surface* item_surf = NULL;
    SDL_Surface* prompt_surf = NULL;
    SDL_Rect title_pos = {0,0,0,0}; // Initialize to suppress potential warnings
    SDL_Rect item_pos;
    SDL_Rect prompt_pos;
    int font_size = DEFAULT_MENU_FONT_SIZE;
    int line_spacing = 5;
    int player_choice = 0; // 0 for No/Continue, 1 for Yes

    if (!screen_surface) {
        fprintf(stderr, "T4K_DisplayReviewList_WithOption: ERROR - T4K_GetScreen() returned NULL.\n");
        return 0;
    }

    if (screen_surface->h > 600) font_size = 24;
    else if (screen_surface->h > 480) font_size = 20;

    if (background_surface) {
        T4K_BlitSurfaceToScreen(background_surface, NULL, NULL);
    } else {
        SDL_FillRect(screen_surface, NULL, SDL_MapRGB(screen_surface->format, 20, 20, 50));
    }

    if (title) {
        title_surf = T4K_BlackOutline(title, font_size + 4, &text_color);
        if (title_surf) {
            title_pos.x = (screen_surface->w - title_surf->w) / 2;
            title_pos.y = 20;
            T4K_BlitSurface(title_surf, NULL, screen_surface, &title_pos);
        }
    }

    item_pos.y = title_pos.y + (title_surf ? title_surf->h : 0) + 20 + line_spacing;
    if(title_surf) SDL_FreeSurface(title_surf); // Free title_surf after use

    int max_items_on_screen = (screen_surface->h - item_pos.y - 80) / (font_size + line_spacing); // Reserve space for prompt
    if (num_items > max_items_on_screen) num_items = max_items_on_screen;


    for (i = 0; i < num_items; ++i) {
        if (items[i]) {
            item_surf = T4K_BlackOutline((char*)items[i], font_size, &text_color);
            if (item_surf) {
                item_pos.x = 50;
                if (item_pos.y + item_surf->h > screen_surface->h - (show_play_again_option && num_items > 0 ? 60 : 20) ) { // Check space
                    SDL_FreeSurface(item_surf);
                    break;
                }
                T4K_BlitSurface(item_surf, NULL, screen_surface, &item_pos);
                item_pos.y += item_surf->h + line_spacing;
                SDL_FreeSurface(item_surf);
            }
        }
    }

    int prompt_y_pos = screen_surface->h - 40; // Default y for "continue"
    if (item_pos.y > screen_surface->h - 80) prompt_y_pos = screen_surface->h - 40; // Adjust if list is long
    else prompt_y_pos = item_pos.y + 20;


    if (show_play_again_option && num_items > 0) {
        prompt_surf = T4K_BlackOutline(_("Play again with these questions? (Y/N)"), font_size, &prompt_color);
        if (prompt_surf) {
            prompt_pos.x = (screen_surface->w - prompt_surf->w) / 2;
            prompt_pos.y = prompt_y_pos;
            T4K_BlitSurface(prompt_surf, NULL, screen_surface, &prompt_pos);
            SDL_FreeSurface(prompt_surf);
        }
    } else {
        prompt_surf = T4K_BlackOutline(_("Press any key or click to continue..."), font_size - 2, &text_color);
        if (prompt_surf) {
            prompt_pos.x = (screen_surface->w - prompt_surf->w) / 2;
            prompt_pos.y = prompt_y_pos;
            T4K_BlitSurface(prompt_surf, NULL, screen_surface, &prompt_pos);
            SDL_FreeSurface(prompt_surf);
        }
    }

    SDL_Flip(screen_surface);

    while (waiting_for_input) {
        FC_frame_begin();
        while (SDL_PollEvent(&event)) {
            T4K_HandleStdEvents(&event);
            switch (event.type) {
                case SDL_QUIT:
                    waiting_for_input = 0;
                    player_choice = 0; // Treat as "No" or "Continue"
                    break;
                case SDL_KEYDOWN:
                    if (show_play_again_option && num_items > 0) {
                        if (event.key.keysym.sym == SDLK_y) {
                            player_choice = 1; // Yes
                            waiting_for_input = 0;
                        } else if (event.key.keysym.sym == SDLK_n) {
                            player_choice = 0; // No
                            waiting_for_input = 0;
                        } else { // Any other key dismisses if Y/N not strictly required
                           // player_choice = 0;
                           // waiting_for_input = 0;
                        }
                    } else { // Not showing Y/N option, any key continues
                        player_choice = 0;
                        waiting_for_input = 0;
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN: // Any mouse click continues
                    player_choice = (show_play_again_option && num_items > 0) ? 0 : 0; // Default to No if Y/N shown
                    waiting_for_input = 0;
                    break;
            }
        }
        FC_frame_end_Synch();
    }
    return player_choice;
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

// SDL_BlitSurface wrappers
void T4K_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
    if (!src || !dst) {
        fprintf(stderr, "T4K_BlitSurface: ERROR - src or dst surface is NULL.\n");
        return;
    }

    // It's common for srcrect to be NULL, meaning blit entire source.
    // dstrect can also be NULL for blitting to (0,0) of dst, though less common in this codebase.

    if (SDL_BlitSurface(src, srcrect, dst, dstrect) < 0) {
        fprintf(stderr, "T4K_BlitSurface: SDL_BlitSurface error: %s\n", SDL_GetError());
    }
}

void T4K_BlitSurfaceToScreen(SDL_Surface *src, SDL_Rect *srcrect, SDL_Rect *dstrect)
{
    SDL_Surface *screen_surface = T4K_GetScreen();
    if (!screen_surface) {
        fprintf(stderr, "T4K_BlitSurfaceToScreen: ERROR - T4K_GetScreen() returned NULL.\n");
        return;
    }
    T4K_BlitSurface(src, srcrect, screen_surface, dstrect);
}

void T4K_BlitEntireSurfaceToScreen(SDL_Surface *src, int x, int y)
{
    if (!src) {
        fprintf(stderr, "T4K_BlitEntireSurfaceToScreen: ERROR - src surface is NULL.\n");
        return;
    }
    SDL_Rect dstrect_val = {x, y, 0, 0}; // Width and height are set by SDL_BlitSurface from src if srcrect is NULL

    // If src surface has dimensions, use them for dstrect_val for clarity, though SDL_BlitSurface ignores w/h in dstrect if srcrect is NULL.
    // However, if srcrect is NOT NULL, then dstrect w/h are used for scaling if different from srcrect w/h.
    // For this specific function (EntireSurface), srcrect is always NULL.
    dstrect_val.w = src->w;
    dstrect_val.h = src->h;

    T4K_BlitSurfaceToScreen(src, NULL, &dstrect_val);
}

// Function to display a list of strings for review
void T4K_DisplayReviewList(const char* title, char** items, int num_items, SDL_Surface* background_surface)
{
    SDL_Surface *screen_surface = T4K_GetScreen();
    SDL_Event event;
    int waiting_for_input = 1;
    int i;
    SDL_Color text_color = {255, 255, 255, 255}; // White
    SDL_Surface* title_surf = NULL;
    SDL_Surface* item_surf = NULL;
    SDL_Rect title_pos;
    SDL_Rect item_pos;
    int font_size = DEFAULT_MENU_FONT_SIZE; // Use a default font size
    int line_spacing = 5; // Additional spacing between lines

    if (!screen_surface) {
        fprintf(stderr, "T4K_DisplayReviewList: ERROR - T4K_GetScreen() returned NULL.\n");
        return;
    }

    // Adjust font size based on screen height, similar to get_scale() but simpler for now
    if (screen_surface->h > 600) {
        font_size = 24;
    } else if (screen_surface->h > 480) {
        font_size = 20;
    }


    // 1. Clear screen or draw background
    if (background_surface) {
        T4K_BlitSurfaceToScreen(background_surface, NULL, NULL); // Blit to (0,0)
    } else {
        SDL_FillRect(screen_surface, NULL, SDL_MapRGB(screen_surface->format, 20, 20, 50)); // Dark blue background
    }

    // 2. Draw title
    if (title) {
        title_surf = T4K_BlackOutline(title, font_size + 4, &text_color); // Slightly larger title
        if (title_surf) {
            title_pos.x = (screen_surface->w - title_surf->w) / 2;
            title_pos.y = 20;
            T4K_BlitSurface(title_surf, NULL, screen_surface, &title_pos);
            SDL_FreeSurface(title_surf);
        }
    }

    // 3. Draw items
    item_pos.y = title_pos.y + (title_surf ? title_surf->h : 0) + 20 + line_spacing;
    for (i = 0; i < num_items; ++i) {
        if (items[i]) {
            item_surf = T4K_BlackOutline((char*)items[i], font_size, &text_color);
            if (item_surf) {
                item_pos.x = 50; // Left margin
                if (item_pos.y + item_surf->h > screen_surface->h - 20) { // Stop if overflowing screen
                    SDL_FreeSurface(item_surf);
                    break;
                }
                T4K_BlitSurface(item_surf, NULL, screen_surface, &item_pos);
                item_pos.y += item_surf->h + line_spacing;
                SDL_FreeSurface(item_surf);
            }
        }
    }

    // Add a prompt to continue
    SDL_Surface* continue_surf = T4K_BlackOutline(_("Press any key or click to continue..."), font_size - 2, &text_color);
    if (continue_surf) {
        SDL_Rect continue_pos;
        continue_pos.x = (screen_surface->w - continue_surf->w) / 2;
        continue_pos.y = screen_surface->h - continue_surf->h - 20;
        if (continue_pos.y < item_pos.y + line_spacing) { // Ensure it doesn't overlap last item too much
             continue_pos.y = item_pos.y + line_spacing;
        }
        if (continue_pos.y + continue_surf->h > screen_surface->h - 5) { // Keep it on screen
            continue_pos.y = screen_surface->h - continue_surf->h - 5;
        }
        T4K_BlitSurface(continue_surf, NULL, screen_surface, &continue_pos);
        SDL_FreeSurface(continue_surf);
    }


    // 4. Flip screen
    SDL_Flip(screen_surface);

    // 5. Wait for input
    while (waiting_for_input) {
        FC_frame_begin(); // For consistent timing if we add animations later
        while (SDL_PollEvent(&event)) {
            T4K_HandleStdEvents(&event); // Handle quit, fullscreen toggle
            switch (event.type) {
                case SDL_QUIT:
                    // Propagate quit signal if possible, or handle directly
                    // For now, just exit the review screen
                    waiting_for_input = 0;
                    // Potentially set a global quit flag if one exists
                    // extern int user_quit_received; user_quit_received = GAME_OVER_WINDOW_CLOSE; (example)
                    break;
                case SDL_KEYDOWN:
                case SDL_MOUSEBUTTONDOWN:
                    waiting_for_input = 0;
                    break;
            }
        }
        FC_frame_end_Synch(); // Synchronize frame rate
    }
}
