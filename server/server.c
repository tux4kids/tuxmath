/*
*  C Implementation: server.c
*
*       Description: Server program for LAN-based play in Tux,of Math Command.
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
        TCPsocket sd, csd; /* Socket descriptor, Client socket descriptor */
        IPaddress ip, *remoteIP;
        int quit, quit2;
        char buffer[512];
 
        if (SDLNet_Init() < 0)
        {
                fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
                exit(EXIT_FAILURE);
        }
 
        /* Resolving the host using NULL make network interface to listen */
        if (SDLNet_ResolveHost(&ip, NULL, 2000) < 0)
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
 
        /* Wait for a connection, send data and term */
        quit = 0;
        while (!quit)
        {
                /* This check the sd if there is a pending connection.
                * If there is one, accept that, and open a new socket for communicating */
                if ((csd = SDLNet_TCP_Accept(sd)))
                {
                        /* Now we can communicate with the client using csd socket
                        * sd will remain opened waiting other connections */
 
                        /* Get the remote address */
                        if ((remoteIP = SDLNet_TCP_GetPeerAddress(csd)))
                                /* Print the address, converting in the host format */
                                printf("Host connected: %x %d\n", SDLNet_Read32(&remoteIP->host), SDLNet_Read16(&remoteIP->port));
                        else
                                fprintf(stderr, "SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError());
                        
                        quit2 = 0;
                        while (!quit2)
                        {
                                if (SDLNet_TCP_Recv(csd, buffer, 512) > 0)
                                {
                                        printf("Client say: %s\n", buffer);
 
                                        if(strcmp(buffer, "exit") == 0) /* Terminate this connection */
                                        {
                                                quit2 = 1;
                                                printf("Terminate connection\n");
                                        }
                                        if(strcmp(buffer, "quit") == 0) /* Quit the program */
                                        {
                                                quit2 = 1;
                                                quit = 1;
                                                printf("Quit program\n");
                                        }
                                }
                        }
 
                        /* Close the client socket */
                        SDLNet_TCP_Close(csd);
                }
        }
 
        SDLNet_TCP_Close(sd);
        SDLNet_Quit();
 
        return EXIT_SUCCESS;
}