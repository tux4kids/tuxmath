#include<stdio.h>
#include<stdlib.h>
#include<SDL/SDL_net.h>
#include<unistd.h>
int main(int argc, char **argv)
{
	IPaddress server_ipaddress;
	TCPsocket client_socket;
	int quit=1,len;
	char buffer[512];
	
	Uint16 *temp_port=malloc(sizeof(Uint16));
	if(argc<3)
	{
		printf("\n Wrong usage");
		return 1;	
	}
	if(SDLNet_Init()!=0)
	{
		printf("\n Unable to initialize SDL_net");
		return 1;
	}
	atexit(SDLNet_Quit);
	//connecting to server 
	if( SDLNet_ResolveHost(&server_ipaddress,argv[1],atoi(argv[2]))!=0)
	{
		printf("\nUnable to find host");
		return 0;
	}
	else
	{
		printf("\nAble to find host");
	}
	if(!(client_socket=SDLNet_TCP_Open(&server_ipaddress)))
	{
		printf("\nUnable to connect to server");
		return 0;
	}
	else
	{
		printf("\nConnected to host");
	}
	//receving from the server
	while(!SDLNet_TCP_Recv(client_socket,temp_port,2))
		printf("\nERROR");
	printf("\nNew port number is:%d",*temp_port);
	//closing connection
	SDLNet_TCP_Close(client_socket);
	// Inorder to avoid race condition 
	if((*temp_port)!=0)
	{
		sleep(1);
		//switching port for connecting to server
		if( SDLNet_ResolveHost(&server_ipaddress,argv[1],*temp_port)!=0)
		{
			printf("\nUnable to find host");
			return 0;
		}
		else
		{
			printf("\nAble to find host");
		}
		if(!(client_socket=SDLNet_TCP_Open(&server_ipaddress)))
		{
			printf("\nUnable to connect to server");
			return 0;
		}
		else
		{
			printf("\nConnected to host");
		}
		while(quit)
		{	
			printf("\nWrite here to send:");
			scanf("%s",buffer);
			len=strlen(buffer)+1;

			if(SDLNet_TCP_Send(client_socket,(void *)buffer,512)<len)
			{
				printf("\n There is some error while sending data");
			}
			if((strcmp(buffer,"quit")==0) ||(strcmp(buffer,"exit")==0))
				quit=0;
		}
	}
	else
		printf("\n Server refuses request");
	free (temp_port);
	SDLNet_TCP_Close(client_socket);
}
		
				
