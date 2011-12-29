#include <SDL.h>

#include "game.h"
#include "tuxmath.h"
#include "options.h"
#include "fileops.h"



/* Pause loop: */
int pause_game(void)
{
    /* NOTE - done and quit changed to pause_done and pause_quit */
    /* due to potentially confusing name collision */
    int pause_done, pause_quit;
    SDL_Event event;
    SDL_Rect dest;

    /* Only pause if pause allowed: */
    if (!Opts_AllowPause())
    {
        fprintf(stderr, "Pause requested but not allowed by Opts!\n");
        return 0;
    }

    pause_done = 0;
    pause_quit = 0;

    dest.x = (screen->w - images[IMG_PAUSED]->w) / 2;
    dest.y = (screen->h - images[IMG_PAUSED]->h) / 2;
    dest.w = images[IMG_PAUSED]->w;
    dest.h = images[IMG_PAUSED]->h;

    T4K_DarkenScreen(1);  // cut all channels by half
    SDL_BlitSurface(images[IMG_PAUSED], NULL, screen, &dest);
    SDL_UpdateRect(screen, 0, 0, 0, 0);

#ifndef NOSOUND
    if(Opts_GetGlobalOpt(USE_SOUND))
        Mix_PauseMusic();
#endif

    do
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN)
                pause_done = 1;
            else if (event.type == SDL_QUIT)
            {
                user_quit_received = GAME_OVER_WINDOW_CLOSE;
                pause_quit = 1;
            }
        }

        SDL_Delay(100);
    }
    while (!pause_done && !pause_quit);

#ifndef NOSOUND
    if(Opts_GetGlobalOpt(USE_SOUND))
        Mix_ResumeMusic();
#endif

    return (pause_quit);
}
