/* menu_lan.c

   Graphical menus for managing LAN games and connections.
   Modified from code previously in menu.c and highscore.c.

   Copyright 2009, 2010, 2011.
Authors: David Bruce, Akash Gangil, Brendan Luchen.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

menu_lan.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.  */




#include "tuxmath.h"
#include "highscore.h"
#include "menu.h"
#include "titlescreen.h"
#include "fileops.h"
#include "setup.h"
#include "network.h"
#include "menu_lan.h"


/* lan_player_type now defined in network.h */
lan_player_type lan_player_info[MAX_CLIENTS];

/* Local function prototypes: ------------------- */
void draw_player_table(void);


int ConnectToServer(void)
{
#ifndef HAVE_LIBSDL_NET
    return 0;
#else
    SDL_Rect loc;
    SDL_Rect stopRect;
    SDL_Event event;

    int finished = 0;
    Uint32 timer = 0;
    int servers_found = 0;  

    DEBUGMSG(debug_lan, "\n Enter ConnectToServer()\n");

    /* Draw background: */
    if (current_bkg())
        SDL_BlitSurface(current_bkg(), NULL, screen, NULL);

    /* Red "Stop" circle in upper right corner to go back to main menu: */
    if (images[IMG_STOP])
    {
        stopRect.w = images[IMG_STOP]->w;
        stopRect.h = images[IMG_STOP]->h;
        stopRect.x = screen->w - images[IMG_STOP]->w;
        stopRect.y = 0;
        SDL_BlitSurface(images[IMG_STOP], NULL, screen, &stopRect);
    }


    /* Draw heading: */
    {
        SDL_Surface* s = T4K_BlackOutline(_("Detecting servers"), DEFAULT_MENU_FONT_SIZE, &white);
        if (s)
        {
            loc.x = (screen->w/2) - (s->w/2);
            loc.y = 110;
            SDL_BlitSurface(s, NULL, screen, &loc);
            SDL_FreeSurface(s);
        }

        s = T4K_BlackOutline(_("Please wait"),
                DEFAULT_MENU_FONT_SIZE, &white);
        if (s)
        {
            loc.x = (screen->w/2) - (s->w/2);
            loc.y = 140;
            SDL_BlitSurface(s, NULL, screen, &loc);
            SDL_FreeSurface(s);
        }
        s = NULL;
    }

    /* Draw Tux (use "reset" flavor so Tux gets drawn immediately): */
    HandleTitleScreenAnimations_Reset(true);
    /* and update: */
    SDL_UpdateRect(screen, 0, 0, 0, 0);

    while (!finished)
    {

        //Scan local network to find running server:
        servers_found = LAN_DetectServers();

        if(servers_found < 1)
        {
            DEBUGMSG(debug_lan, "No server could be found - returning.\n");
            return 0;
        }
        else if(servers_found  == 1)  //One server - connect without player intervention
        {
            DEBUGMSG(debug_lan, "Single server found - connecting automatically...");

            if(!LAN_AutoSetup(0))  //i.e.first (and only) entry in list
            {
                DEBUGMSG(debug_lan, "LAN_AutoSetup() failed - returning.\n");
                return 0;
            }


            finished = 1;
            DEBUGMSG(debug_lan, "connected\n");
            break;  //So we quit scanning as soon as we connect
        } else if (servers_found  > 1) //Multiple servers - ask player for choice
        {
            char buf[256];
            int server_choice;
            char** servernames;
            int i;

            snprintf(buf, 256, _("TuxMath detected %d running servers.\nClick to continue..."), servers_found);

            ShowMessageWrap(DEFAULT_MENU_FONT_SIZE,buf); 
            servernames = malloc(servers_found * sizeof(char*));

            for(i = 0; i < servers_found; i++)
            {
                servernames[i] = LAN_ServerName(i);
            }

            T4K_CreateOneLevelMenu(MENU_SERVERSELECT, servers_found, servernames,
                    NULL, "Server Selection", NULL);
            T4K_PrerenderMenu(MENU_SERVERSELECT);
            server_choice = T4K_RunMenu(MENU_SERVERSELECT, true, &DrawTitleScreen,
                    &HandleTitleScreenEvents, &HandleTitleScreenAnimations, NULL);

            if(!LAN_AutoSetup(server_choice))
            {
                return 0;
            }

            finished = 1;
            DEBUGMSG(debug_lan, "connected\n");
            break;
        }


        while (SDL_PollEvent(&event)) 
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    {
                        cleanup();
                    }

                case SDL_MOUSEBUTTONDOWN:
                    /* "Stop" button - go to main menu: */
                    { 
                        if (T4K_inRect(stopRect, event.button.x, event.button.y ))
                        {
                            finished = 1;
                            playsound(SND_TOCK);
                            break;
                        }
                    }
            }
        }

        /* Draw Tux: */
        HandleTitleScreenAnimations();
        /* and update: */
        SDL_UpdateRect(screen, 0, 0, 0, 0);
        /* Wait so we keep frame rate constant: */
        T4K_Throttle(20, &timer);
    }  // End of while (!finished) loop

    //Now connected - get player nickname:
    {
        char buf[256];
        char buf2[256];
        char player_name[HIGH_SCORE_NAME_LENGTH * 3];

        /* Display server name and current lesson, ask player for LAN nickname: */
        snprintf(buf, 256, _("Connected to server: %s"), LAN_ConnectedServerName());
        snprintf(buf2, 256, _("%s"), LAN_ConnectedServerLesson());
        NameEntry(player_name, buf, buf2, _("Enter your name:")); //get nickname from user
        /* If sock lost during name entry, handle it correctly: */
        if(!LAN_SetName(player_name)) //tell server nickname
        {
            ShowMessageWrap(DEFAULT_MENU_FONT_SIZE, _("Connection with server lost"));
            return 0;
        }
    }

    return 1;

#endif
}


/* Pregame() displays the currently connected players and whether they
 * have indicated that they are ready to start, waiting until all are ready.
 * Returns 1 when all connected players are ready, -1 on errors or if player
 * decides not to play.
 */

int Pregame(void)
{
    int widest = 0;
    int status = PREGAME_WAITING;
    Uint32 timer = 0;
    const int loop_msec = 20;
    SDL_Event event;
    SDL_Rect title_rect, ready_rect;  //NOTE stop_rect is a global from t4k_common.h (good idea???)
    SDL_Surface* play_surf = NULL;
    SDL_Surface* pause_surf = NULL;
    SDL_Surface* ready_title = NULL;
    SDL_Surface* notready_title = NULL;
    SDL_Surface* ready_subtitle = NULL;
    SDL_Surface* notready_subtitle = NULL;
    SDL_Surface* s = NULL;
    int more_msgs;
    bool ready = false;
    char buf[NET_BUF_LEN];

    //set up locations:
    ready_rect.x = screen->w * 0.45;            ready_rect.y = screen->h * 0.15;
    ready_rect.w = ready_rect.h = screen->w * 0.1;  
    //set up surfaces for remaining buttons:
    play_surf = T4K_LoadImageOfBoundingBox("status/player_play.svg", IMG_ALPHA,ready_rect.w, ready_rect.h);
    pause_surf = T4K_LoadImageOfBoundingBox("status/player_pause.svg", IMG_ALPHA, ready_rect.w, ready_rect.h);
    //set up surfaces for titles:
    ready_title = T4K_BlackOutline(_("Waiting for other players"), DEFAULT_MENU_FONT_SIZE, &white);
    notready_title = T4K_BlackOutline(_("Waiting until you are ready"), DEFAULT_MENU_FONT_SIZE, &white);
    ready_subtitle = T4K_BlackOutline(_("Click \"Pause\" if not ready"), DEFAULT_MENU_FONT_SIZE, &white);
    notready_subtitle = T4K_BlackOutline(_("Click \"Play\" when ready"), DEFAULT_MENU_FONT_SIZE, &white);

    //Make sure we have needed surfaces:
    if(!stop_button || !play_surf || !pause_surf || !ready_title
            || !notready_title || !ready_subtitle || !notready_subtitle)
        return PREGAME_OVER_ERROR;

    //Figure out which heading is widest (for shaded box)
    if(widest < ready_title->w) widest = ready_title->w;
    if(widest < notready_title->w) widest = notready_title->w;
    if(widest < ready_subtitle->w) widest = ready_subtitle->w;
    if(widest < notready_subtitle->w) widest = notready_subtitle->w;
    widest += 10; //add margin

    while(status == PREGAME_WAITING)
    {
        //Draw -------------------------------
        DrawTitleScreen();
        HandleTitleScreenAnimations();
        SDL_BlitSurface(stop_button, NULL, screen, &stop_rect);
        //Draw "play" or "pause" button:
        if(ready)
            SDL_BlitSurface(pause_surf, NULL, screen, &ready_rect);
        else
            SDL_BlitSurface(play_surf, NULL, screen, &ready_rect);
        //Draw shaded background for headings:
        title_rect.x = screen->w/2 - widest/2;
        title_rect.y = 0;
        title_rect.w = widest;
        title_rect.h = screen->h * 0.4;
        T4K_DrawButton(&title_rect, 20, REG_RGBA);
        //Draw headings:
        if(ready)
            s = ready_title;
        else
            s = notready_title;
        title_rect.x = screen->w/2 - s->w/2;
        title_rect.y = screen->h * 0.05;
        SDL_BlitSurface(s, NULL, screen, &title_rect);

        if(ready)
            s = ready_subtitle;
        else
            s = notready_subtitle;
        title_rect.x = screen->w/2 - s->w/2;
        title_rect.y = screen->h * 0.3;
        SDL_BlitSurface(s, NULL, screen, &title_rect);
        //Draw status of other players:
        draw_player_table();

        SDL_UpdateRect(screen, 0, 0, 0, 0);

        //Check SDL events:
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    {
                        cleanup();
                    }

                case SDL_MOUSEBUTTONDOWN:
                    /* "Stop" button - go to main menu: */
                    {
                        if (T4K_inRect(stop_rect, event.button.x, event.button.y ))
                        {
                            status = PREGAME_OVER_ESCAPE;
                            playsound(SND_TOCK);
                            break;
                        } 
                        else if (T4K_inRect(ready_rect, event.button.x, event.button.y ))
                        {
                            //Player clicked play/pause, toggle "ready" flag:       
                            ready = !ready;
                            LAN_SetReady(ready);  //tell server we are ready to start
                            playsound(SND_TOCK);
                            break;
                        }

                    }
                case SDL_KEYDOWN:
                    {
                        switch (event.key.keysym.sym)
                        {
                            case SDLK_ESCAPE:
                                {
                                    status =  PREGAME_OVER_ESCAPE;
                                    playsound(SND_TOCK);
                                    break;
                                }
                            case SDLK_RETURN:
                            case SDLK_KP_ENTER:
                            case SDLK_SPACE:
                                {
                                    ready = true;
                                    LAN_SetReady(true);  //tell server we are ready to start
                                    playsound(SND_TOCK);
                                    break;
                                }
                            case SDLK_BACKSPACE:
                                {
                                    ready = false;
                                    LAN_SetReady(false);  //tell server we are NOT ready to start
                                    playsound(SND_TOCK);
                                    break;
                                }
                            default:
                                {
                                    //Do nothing - event. add support for toggle fullscreen, etc.
                                }
                        } 
                    }
            }
        }  // End while(SDL_PollEvent(&event))


        //Check network events:
        for(more_msgs = 1; more_msgs > 0; more_msgs = LAN_NextMsg(buf))
        {
            if(strncmp(buf,"GO_TO_GAME", strlen("GO_TO_GAME")) == 0)
            {
                //TODO display "countdown" before game starts
                status = PREGAME_OVER_START_GAME;
                break;
            }
            else if(strncmp(buf, "GAME_IN_PROGRESS", strlen("GAME_IN_PROGRESS")) == 0)
            {
                status = PREGAME_GAME_IN_PROGRESS;
                break;
            }
            else if(strncmp(buf, "NETWORK_ERROR", strlen("NETWORK_ERROR")) == 0)
            {
                fprintf(stderr, "NETWORK_ERROR msg received!\n");
                status = PREGAME_OVER_LAN_DISCONNECT;
                break;
            }
            else
            {
                DEBUGMSG(debug_lan, "Unrecognized message from server: %s\n", buf);
                continue;
            }
        }  // End checking network messages
        //Don't eat CPU:
        T4K_Throttle(loop_msec, &timer);
    }  // End while(status = PREGAME_WAITING)

    SDL_FreeSurface(play_surf);    //we know these can't be NULL from check above
    SDL_FreeSurface(pause_surf);
    SDL_FreeSurface(ready_title);
    SDL_FreeSurface(notready_title);
    SDL_FreeSurface(ready_subtitle);
    SDL_FreeSurface(notready_subtitle);

    return status;
}

void draw_player_table(void)
{
    int i = 0;
    char* txt;
    char buf[256];
    SDL_Surface* surf = NULL;
    SDL_Rect loc;
    SDL_Color* col;
    const int name_x = screen->w * 0.25;
    const int ready_x = screen->w * 0.7;

    //Draw shaded rectangle for player table
    {
        int nr_width = T4K_SimpleText(_("Not Ready"), DEFAULT_MENU_FONT_SIZE, &white)->w;
        int text_h = T4K_SimpleText(_("Not Ready"), DEFAULT_MENU_FONT_SIZE, &white)->h;
        SDL_Rect shaded_loc;
        shaded_loc.x = name_x - 10;
        shaded_loc.y = screen->h * 0.45;
        shaded_loc.w = (ready_x + nr_width) - name_x + 20;
        shaded_loc.h = ((2 + LAN_NumPlayers()) * text_h) + 20;
        T4K_DrawButton(&shaded_loc, 20, REG_RGBA);
    }

    //Draw server name and lesson:
    txt = LAN_ConnectedServerName();
    if(txt)
    {
        snprintf(buf, 256, _("Server Name: %s"), txt);
        surf = T4K_BlackOutline(buf, DEFAULT_MENU_FONT_SIZE, &white);
    }
    if(surf)
    {
        loc.x = name_x;
        loc.y = screen->h * 0.45;
        SDL_BlitSurface(surf, NULL, screen, &loc);
        SDL_FreeSurface(surf);
        surf = NULL;
    }

    txt = LAN_ConnectedServerLesson();
    if(txt)
    {
        snprintf(buf, 256, _("Lesson: %s"), txt);
        surf = T4K_BlackOutline(buf, DEFAULT_MENU_FONT_SIZE, &white);
    }
    if(surf)
    {
        loc.x = name_x;
        loc.y += surf->h;
        SDL_BlitSurface(surf, NULL, screen, &loc);
        SDL_FreeSurface(surf);
        surf = NULL;
    }

    //Now draw connected players and ready status:
    for(i = 0; i < MAX_CLIENTS; i++)
    {
        if(i == LAN_MyIndex())
            col = &yellow;
        else
            col = &white;

        if(LAN_PlayerConnected(i))
        {
            DEBUGMSG(debug_lan, "Socket %d is connected\n", i);

            surf = T4K_BlackOutline(LAN_PlayerName(i), DEFAULT_MENU_FONT_SIZE, col);
            if(surf)
            {
                loc.x = name_x;
                loc.y += surf->h;
                SDL_BlitSurface(surf, NULL, screen, &loc);
                SDL_FreeSurface(surf);
                surf = NULL;
            }
            if(LAN_PlayerReady(i))
            {
                col = &bright_green;
                txt = _("Ready");
            }
            else
            {
                col = &red;
                txt = _("Not Ready");
            }
            surf = T4K_BlackOutline(txt, DEFAULT_MENU_FONT_SIZE, col);
            if(surf)
            {
                loc.x = ready_x;
                SDL_BlitSurface(surf, NULL, screen, &loc);
                SDL_FreeSurface(surf);
                surf = NULL;
            }
        }
        else
            DEBUGMSG(debug_lan, "Socket %d is not connected\n", i);
    }
}
