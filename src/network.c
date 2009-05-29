
//** this file would contain all the network related functions**



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "SDL.h"
#include "SDL_net.h"



//*** ipaddress of the server and the port would be taken by the user @ the time he selects "LAN multiplayer" from the options menu..

//***also should I fix the port beforehand or ask it from the user...


int lan_client_connect(char *host,char *port)            //here "host" is either the hostname or the ipaddress(of the server) in string
{
char message[]="Client got connected";
int len;
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
if(SDLNet_ResolveHost(&ip,host,portnum)==-1)
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
	len=strlen(message);

	// strip the newline
	message[len]='\0';
	
	if(len)
	{
		int result;
		
		// print out the message
		printf("Sending: %.*s\n",len,message);

		result=SDLNet_TCP_Send(sock,message,len); // add 1 for the NULL
		if(result<len)
			printf("SDLNet_TCP_Send: %s\n",SDLNet_GetError());
	}


	SDLNet_TCP_Close(sock);
	
	// shutdown SDL_net
	SDLNet_Quit();



	return(0);
}



/***      server connection function    ****/


int lan_server_connect(char *port)
{
IPaddress ip; //int *remoteip;
TCPsocket server,client;
//Uint32 ipaddr;
Uint16 portnum;
int len;
char message[1024];
char waiting[]="WAITING FOR OTHER PLAYER(minimum 2 players required)";

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
if(SDLNet_ResolveHost(&ip,NULL,portnum)==-1)
{
printf("SDLNet_ResolveHost: %s\n",SDLNet_GetError());
exit(3);
}

// open the server socket
server=SDLNet_TCP_Open(&ip);
if(!server)
{
printf("SDLNet_TCP_Open: %s\n",SDLNet_GetError());
return 1;
}

game_set_start_message(waiting, "", "", "");
printf("%s\n",waiting);
while(1)
{

// try to accept a connection
 client=SDLNet_TCP_Accept(server);
 if(!client)
  {    // no connection accepted
      //printf("SDLNet_TCP_Accept: %s\n",SDLNet_GetError());
    SDL_Delay(100); //sleep 1/10th of a second
    continue;
  }
  

              // read the buffer from client
		len=SDLNet_TCP_Recv(client,message,1024);
		if(!len)
		{
			printf("SDLNet_TCP_Recv: %s\n",SDLNet_GetError());
			continue;
		}

		// print out the message
		printf("Received: %.*s\n",len,message);

		if(message[0]=='Q')
		{
			printf("Quitting on a Q received\n");
			break;
		}
                break;
}

	SDLNet_TCP_Close(client);	
	SDLNet_TCP_Close(server);
        // shutdown SDL_net
	SDLNet_Quit();


return 0;
}


