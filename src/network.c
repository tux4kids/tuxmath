
//** this file would contain all the network related functions**



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "SDL.h"
#include "SDL_net.h"

//*** ipaddress of the server and the port would be taken by the user @ the time he selects "LAN multiplayer" from the options menu..

//***also should I fix the port beforehand or ask it from the user...

int lan_client_connect(char *host,int port)            //here "host" is either the hostname or the ipaddress(of the server) in string
{
IPaddress ip;
TCPsocket sock;
Uint16 portnum;

portnum=(Uint16) strtol(port,NULL,0);

// initialize SDL
if(SDL_Init(0)==-1)
{
printf("SDL_Init: %s\n",SDL_GetError());
exit(1);
}

// initialize SDL_net
if(SDLNet_Init()==-1)
{
printf("SDLNet_Init: %s\n",SDLNet_GetError());
exit(2);
}


// Resolve the argument into an IPaddress type
if(SDLNet_ResolveHost(&ip,*host,portnum)==-1)
{
printf("SDLNet_ResolveHost: %s\n",SDLNet_GetError());
exit(3);
}

//connect to the "host" @ port "portnum"
sock=SDLNet_TCP_Open(&ip);
if(!sock)
{
printf("SDLNet_TCP_Open: %s\n",SDLNet_GetError());
exit(4);
}



return(0);
}


/***      server connection function    ****/

int lan_server_connect(int port)
{
IPaddress ip; //int *remoteip;
TCPsocket server,client;
//Uint32 ipaddr;
Uint16 portnum;


// initialize SDL
if(SDL_Init(0)==-1)
{
printf("SDL_Init: %s\n",SDL_GetError());
exit(1);
}

// initialize SDL_net
if(SDLNet_Init()==-1)
{
printf("SDLNet_Init: %s\n",SDLNet_GetError());
exit(2);
}


portnum=(Uint16)strtol(port,NULL,0);

// Resolve the argument into an IPaddress type
if(SDLNet_ResolveHost(&ip,NULL,portum)==-1)
{
printf("SDLNet_ResolveHost: %s\n",SDLNet_GetError());
exit(3);
}

// open the server socket
server=SDLNet_TCP_Open(&ip);
if(!server)
{
printf("SDLNet_TCP_Open: %s\n",SDLNet_GetError());
exit(4);
}

while(1)
{
// try to accept a connection
client=SDLNet_TCP_Accept(server);
if(!client)
{ // no connection accepted
//printf("SDLNet_TCP_Accept: %s\n",SDLNet_GetError());
SDL_Delay(100); //sleep 1/10th of a second
continue;
}

/// get the clients IP and port number
//remoteip=SDLNet_TCP_GetPeerAddress(client);
//if(!remoteip)
//{
//printf("SDLNet_TCP_GetPeerAddress: %s\n",SDLNet_GetError());
//continue;
//}

/*for testing purpose to check if it is connected to the desired client
// print out the clients IP and port number
ipaddr=SDL_SwapBE32(remoteip->host);
printf("Accepted a connection from %d.%d.%d.%d port %hu\n",
ipaddr>>24,
(ipaddr>>16)&0xff,
(ipaddr>>8)&0xff,
ipaddr&0xff,
remoteip->port);

*/

return(0);
}


