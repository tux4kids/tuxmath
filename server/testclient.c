/*
*  C Implementation: server.c
*
*       Description: Test client program for LAN-based play in Tux,of Math Command.
*
*
* Author: Akash Gangil, David Bruce, and the TuxMath team, (C) 2009
* Developers list: <tuxmath-devel@lists.sourceforge.net>
*
* Copyright: See COPYING file that comes with this distribution.  (Briefly, GNU GPL).
*
* NOTE: This file was initially based on example code from The Game Programming Wiki
* (http://gpwiki.org), in a tutorial covered by the GNU Free Documentation License 1.2.
* No invariant sections were indicated, and no separate license for the example code
* was listed. The author was also not listed. AFAICT,this scenario allows incorporation of
* derivative works into a GPLv2+ project like TuxMath - David Bruce 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include "SDL_net.h"
 
int main(int argc, char **argv)
{
        IPaddress ip;           /* Server address */
        TCPsocket sd;           /* Socket descriptor */
        int quit, len;
        char buffer[512];
 
        /* Simple parameter checking */
        if (argc < 3)
        {
                fprintf(stderr, "Usage: %s host port\n", argv[0]);
                exit(EXIT_FAILURE);
        }
 
        if (SDLNet_Init() < 0)
        {
                fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
                exit(EXIT_FAILURE);
        }
 
        /* Resolve the host we are connecting to */
        if (SDLNet_ResolveHost(&ip, argv[1], atoi(argv[2])) < 0)
        {
                fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
                exit(EXIT_FAILURE);
        }
 
        /* Open a connection with the IP provided (listen on the host's port) */
        if (!(sd = SDLNet_TCP_Open(&ip)))
        {
                fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
                exit(EXIT_FAILURE);
        }
 
        /* Send messages */
        quit = 0;
        while (!quit)
        {
                printf("Write something:\n>");
                scanf("%s", buffer);
 
                len = strlen(buffer) + 1;
                if (SDLNet_TCP_Send(sd, (void *)buffer, len) < len)
                {
                        fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
                        exit(EXIT_FAILURE);
                }
 
                if(strcmp(buffer, "exit") == 0)
                        quit = 1;
                if(strcmp(buffer, "quit") == 0)
                        quit = 1;
        }
 
        SDLNet_TCP_Close(sd);
        SDLNet_Quit();
 
        return EXIT_SUCCESS;
}