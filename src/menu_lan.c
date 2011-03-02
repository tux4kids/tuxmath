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
#include "options.h"
#include "transtruct.h"
#include "network.h"
#include "menu_lan.h"

#include <string.h>


int ClickWhenReady(const char* heading)
{
  SDL_Rect loc;
  SDL_Rect okRect;
  int finished = 0;
  Uint32 frame = 0;
  Uint32 timer = 0;
  const int BG_Y = 100;
  const int BG_WIDTH = 400;
  const int BG_HEIGHT = 200;

  DEBUGMSG(debug_highscore, "Enter ClickWhenReady()\n" );

  DrawTitleScreen();

  /* Draw translucent background for text: */
  {
    SDL_Rect bg_rect;
    bg_rect.x = (screen->w)/2 - BG_WIDTH/2;
    bg_rect.y = BG_Y;
    bg_rect.w = BG_WIDTH;
    bg_rect.h = BG_HEIGHT;
    T4K_DrawButton(&bg_rect, 15, REG_RGBA);

    bg_rect.x += 10;
    bg_rect.y += 10;
    bg_rect.w -= 20;
    bg_rect.h = 60;
    T4K_DrawButton(&bg_rect, 10, SEL_RGBA);
  }

  /* Draw heading: */
  {
    SDL_Surface* s = T4K_BlackOutline(_(heading),
                                  DEFAULT_MENU_FONT_SIZE, &white);
    if (s)
    {
      loc.x = (screen->w/2) - (s->w/2);
      loc.y = 110;
      SDL_BlitSurface(s, NULL, screen, &loc);
      SDL_FreeSurface(s);
    }
  }

  /* Red "Stop" circle in upper right corner to go back to main menu: */
  if (stop_button)
  {
    SDL_BlitSurface(stop_button, NULL, screen, &stop_rect);
  }

  /* "Next_arrow" to indicate ready to proceed: */
  if (next_arrow)
  {
    okRect.x = (screen->w)/2;
    okRect.y = 240;
    SDL_BlitSurface(next_arrow, NULL, screen, &okRect);
  }

  /* and update: */
  SDL_UpdateRect(screen, 0, 0, 0, 0);


  while (!finished)
  {
    /* Handle user events: */
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
            finished = -1;
            playsound(SND_TOCK);
            break;
          } 
          else if (T4K_inRect(okRect, event.button.x, event.button.y ))
          {
            finished = 1;
            playsound(SND_TOCK);
            break;
          }

        }
        case SDL_KEYDOWN:
        {
          switch (event.key.keysym.sym)
          {
            case SDLK_ESCAPE:
            case SDLK_BACKSPACE:
            {
              finished = -2;
              playsound(SND_TOCK);
              break;
            }
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
            case SDLK_SPACE:
            {
              finished = 1;
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
    }

    HandleTitleScreenAnimations();
    T4K_Throttle(20, &timer);
    frame++;
  }  // End of while (!finished) loop

  DEBUGMSG(debug_highscore, "Leave ClickWhenReady()\n" );

  /* 1 means we start game, -1 means we go back to menu */
  return finished;
}


int WaitForOthers(const char* heading, const char* sub)
{
#ifndef HAVE_LIBSDL_NET
  return 0;
#else
  SDL_Rect loc;
  int finished = 0;
  int more_msgs = 1;
  Uint32 frame = 0;
  Uint32 timer = 0;
  const int BG_Y = 100;
  const int BG_WIDTH = 400;
  const int BG_HEIGHT = 200;

  char buf[NET_BUF_LEN];

  DEBUGMSG(debug_lan, "Enter WaitForOthers()\n" );

  DrawTitleScreen();

  /* Draw translucent background for text: */
  {
    SDL_Rect bg_rect;
    bg_rect.x = (screen->w)/2 - BG_WIDTH/2;
    bg_rect.y = BG_Y;
    bg_rect.w = BG_WIDTH;
    bg_rect.h = BG_HEIGHT;
    T4K_DrawButton(&bg_rect, 15, REG_RGBA);

    bg_rect.x += 10;
    bg_rect.y += 10;
    bg_rect.w -= 20;
    bg_rect.h = 60;
    T4K_DrawButton(&bg_rect, 10, SEL_RGBA);
  }

  /* Draw heading: */
  {
    SDL_Surface* s = T4K_BlackOutline(_(heading),
                                  DEFAULT_MENU_FONT_SIZE, &white);
    if (s)
    {
      loc.x = (screen->w/2) - (s->w/2);
      loc.y = 110;
      SDL_BlitSurface(s, NULL, screen, &loc);
      SDL_FreeSurface(s);
    }

    s = T4K_BlackOutline(_(sub),
                     DEFAULT_MENU_FONT_SIZE, &white);
    if (s)
    {
      loc.x = (screen->w/2) - (s->w/2);
      loc.y = 140;
      SDL_BlitSurface(s, NULL, screen, &loc);
      SDL_FreeSurface(s);
    }
  }

  /* Red "Stop" circle in upper right corner to go back to main menu: */
  if (stop_button)
  {
    SDL_BlitSurface(stop_button, NULL, screen, &stop_rect);
  }

  /* and update: */
  SDL_UpdateRect(screen, 0, 0, 0, 0);


  while (!finished)
  {
    /* Handle user events: */
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
            finished = 1;
            playsound(SND_TOCK);
            break;
          }
        }
        case SDL_KEYDOWN:
        {
          switch (event.key.keysym.sym)
          {
            case SDLK_ESCAPE:
            case SDLK_BACKSPACE:
            {
              finished = -2;
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
    }

    /* Handle server messages (need to make sure we enter loop even
     * if first message is error):
     */
    for(more_msgs = 1; more_msgs > 0; more_msgs = LAN_NextMsg(buf))
    {
      if(strncmp(buf,"GO_TO_GAME", strlen("GO_TO_GAME")) == 0)
      {
        finished = 1;
        playsound(SND_TOCK);
        break;
      }
      else if(strncmp(buf, "GAME_IN_PROGRESS", strlen("GAME_IN_PROGRESS")) == 0)
      {
        finished = -1;
        playsound(SND_TOCK);
        break;
      }
      else if(strncmp(buf, "NETWORK_ERROR", strlen("NETWORK_ERROR")) == 0)
      {
	printf("NETWORK_ERROR msg received!\n");
        finished = -1;
        playsound(SND_TOCK);
	ShowMessageWrap(DEFAULT_MENU_FONT_SIZE, _("Connection with server was lost"));
        break;
      }
      else
      {
        DEBUGMSG(debug_lan, "Unrecognized message from server: %s\n", buf);
        continue;
      }
    }
    DEBUGMSG(debug_lan, "In WaitForOthers(), after LAN_NextMsg():"
		        " finished = %d\tmore_msgs = %d\tbuf = %s\tstrlen(buf) = %d\n",
		       	finished, more_msgs, buf, strlen(buf));

    HandleTitleScreenAnimations();
    T4K_Throttle(20, &timer);
    frame++;
  }  // End of while (!finished) loop

  DEBUGMSG(debug_lan, "Leave WaitForOthers()\n" );

  /* 1 means we start game, -1 means we go back to menu */
  return finished;
#endif
}



int ConnectToServer(const char* heading, const char* sub)
{
#ifndef HAVE_LIBSDL_NET
  return 0;
#else
  SDL_Rect loc;
  SDL_Rect stopRect;

  int finished = 0;
  Uint32 timer = 0;
  int servers_found = 0;  

  DEBUGMSG(debug_lan, "\nEnter detecting_servers()\n");


  /* We need to get Unicode vals from SDL keysyms */
  SDL_EnableUNICODE(SDL_ENABLE);

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
    SDL_Surface* s = T4K_BlackOutline(_(heading),
                                  DEFAULT_MENU_FONT_SIZE, &white);
    if (s)
    {
      loc.x = (screen->w/2) - (s->w/2);
      loc.y = 110;
      SDL_BlitSurface(s, NULL, screen, &loc);
      SDL_FreeSurface(s);
    }

    s = T4K_BlackOutline(_(sub),
                     DEFAULT_MENU_FONT_SIZE, &white);
    if (s)
    {
      loc.x = (screen->w/2) - (s->w/2);
      loc.y = 140;
      SDL_BlitSurface(s, NULL, screen, &loc);
      SDL_FreeSurface(s);
    }
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
      /* Turn off SDL Unicode lookup (because has some overhead): */
      SDL_EnableUNICODE(SDL_DISABLE);
      return 0;
    }
    else if(servers_found  == 1)  //One server - connect without player intervention
    {
      DEBUGMSG(debug_lan, "Single server found - connecting automatically...");

      if(!LAN_AutoSetup(0))  //i.e.first (and only) entry in list
      {
        DEBUGMSG(debug_lan, "LAN_AutoSetup() failed - returning.\n");
        /* Turn off SDL Unicode lookup (because has some overhead): */
        SDL_EnableUNICODE(SDL_DISABLE);
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
        DEBUGMSG(debug_lan, "LAN_AutoSetup() failed - returning.\n");
        /* Turn off SDL Unicode lookup (because has some overhead): */
        SDL_EnableUNICODE(SDL_DISABLE);
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


  /* Turn off SDL Unicode lookup (because has some overhead): */
  SDL_EnableUNICODE(SDL_DISABLE);

  return 1;

#endif
}

