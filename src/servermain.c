/*
   servermain.c

   main() function to allow standalone use of server program for 
   LAN-based play in Tux,of Math Command.

   Copyright 2009, 2010, 2011.
Author: David Bruce.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

servermain.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "server.h"
#include "mathcards.h"

/* This function has to be in its own file that is not linked into tuxmath */
/* itself because there can only be one main() in a program.  All of the   */
/* server functionality is contained in server.h and server.c              */
/* We do have to initialize and cleanup SDL and SDL_net here rather than
 * in RunServer(), so we don't crash tuxmath by cleaning up SDL if the
 * server is running in a thread. Similar considerations apply to MC_EndGame().
 */
MC_MathGame* lan_game_settings = NULL;

int main(int argc, char** argv)
{
    int ret;
#ifdef HAVE_LIBSDL_NET
    //Initialize a copy of mathcards to hold settings:
    lan_game_settings = (MC_MathGame*) malloc(sizeof(MC_MathGame));
    if (lan_game_settings == NULL)
    {
        fprintf(stderr, "\nUnable to allocate MC_MathGame\n");
        exit(1);
    }
    lan_game_settings->math_opts = NULL;
    if (!MC_Initialize(lan_game_settings))
    {
        fprintf(stderr, "\nUnable to initialize MathCards\n");
        exit(1);
    }
    //Initialize SDL and SDL_net:
    if(SDL_Init(0) == -1)
    {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 0;;
    }
    if (SDLNet_Init() < 0)
    {
        fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
        return 0;
    }

    /* Run actual program: */
    ret = RunServer(argc, argv);
    /* cleanup */
    SDLNet_Quit();
    SDL_Quit();
    if (lan_game_settings)
    {
        MC_EndGame(lan_game_settings);
        free(lan_game_settings);
        lan_game_settings = NULL;
    }
    return ret;
#else
    return 0;
#endif
}
