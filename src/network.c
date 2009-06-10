
//** this file would contain all the network related functions**



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "SDL.h"
#include "SDL_net.h"
#include "network.h"
#include "SDL.h"


//*** ipaddress of the server and the port would be taken by the user @ the time he selects "LAN multiplayer" from the options menu..

//***also should I fix the port beforehand or ask it from the user...


extern char host[1024];
extern char port[1024];

int SendQuestion(MC_FlashCard* fc)                                                            //function to send a flashcard from the server to the client
{
char *ch;
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
		SDLNet_TCP_Send(client,fc->formula_string,4);
		SDLNet_TCP_Recv(client,ch,1);                                     //will send in the next item only when the first one is received
                if(*ch=='1')
                 {
                  SDLNet_TCP_Send(client,fc->answer_string,4);
		  SDLNet_TCP_Recv(client,ch,1);
                  if(*ch=='1')
                   { 
                    SDLNet_TCP_Send(client,&(fc->answer),4);
		    SDLNet_TCP_Recv(client,ch,1);
                    if(*ch=='1')
                    {
                     SDLNet_TCP_Send(client,&(fc->difficulty),4);
                     SDLNet_TCP_Recv(client,ch,1);
                     if(*ch=='1')
                      {		
			break;
                      }
                    }
                  }
                 }
                 
       

}

//	SDLNet_TCP_Close(client);	
//	SDLNet_TCP_Close(server);
        // shutdown SDL_net
//	SDLNet_Quit();


return 0;
}


int ReceiveQuestion(MC_FlashCard* fc)                                             //function for the client to receive the flashcard "from" the server
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
if(SDLNet_ResolveHost(&ip,host,portnum)==-1)
{
printf("SDLNet_ResolveHost: %s\n",SDLNet_GetError());
exit(3);
}

//connect to the "host" @ port "portnum"
sock=SDLNet_TCP_Open(&ip);

while(!sock)
{
 if(!sock)
 {
  printf("SDLNet_TCP_Open: %s\n",SDLNet_GetError());
  sock=SDLNet_TCP_Open(&ip);

  continue;
 }
}	

        SDLNet_TCP_Recv(sock,fc->formula_string,4);
	SDLNet_TCP_Send(sock,"1",1); 		                  // send a conformation that the 1st item has been received				
	SDLNet_TCP_Recv(sock,fc->answer_string,4);
        SDLNet_TCP_Send(sock,"1",1);
        SDLNet_TCP_Recv(sock,&(fc->answer),4);
        SDLNet_TCP_Send(sock,"1",1);
        SDLNet_TCP_Recv(sock,&(fc->difficulty),4);
 


 //       SDLNet_TCP_Close(sock);
	
	// shutdown SDL_net
//	SDLNet_Quit();



	return(0);
}



int SendInt(int x)
{

IPaddress ip; 
TCPsocket server,client;
Uint16 portnum;
char *ch;
ch=(char*)x;

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
		SDLNet_TCP_Send(client,ch,1);
		 
       

}

//	SDLNet_TCP_Close(client);	
//	SDLNet_TCP_Close(server);
        // shutdown SDL_net
//	SDLNet_Quit();


return 0;
}

int ReceiveInt(int x)
{

IPaddress ip;
TCPsocket server,client;
Uint16 portnum;
char *ch;

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
		SDLNet_TCP_Recv(client,ch,1);
		x=(int)(*ch);                                                              //typecasting so as to convert the char to int
       

}

//	SDLNet_TCP_Close(client);	
//	SDLNet_TCP_Close(server);
        // shutdown SDL_net
//	SDLNet_Quit();


return 0;
}


int SendQuestionList(MC_MathQuestion* ql,int list_length)               //   function to send in the MC_MathQuestion data structure
{                                                                       //   this helps in sending the question list from the server to the client
 int randomizer_value;
 randomizer_value=ql->randomizer;
 SendInt(randomizer_value);
 SendInt(list_length);
 while(ql!=NULL)
 {
  SendQuestion(&(ql->card));
  ql=ql->next;
 }
 return 0;
}


int ReceiveQuestionList(MC_MathQuestion* ql,int list_length)               //   function to receive in the MC_MathQuestion data structure
{                                                                          //   this helps in receiving the question list 
 int m;
 ReceiveInt(ql->randomizer);
 ReceiveInt(list_length);
 for(m=list_length;m>0;m--)
 {
  ReceiveQuestion(&(ql->card));
  ql->next=NULL;
  ql=ql->next;
 }
 return 0;
}
   
 


int lan_client_connect(char *host,char *port)            //here "host" is either the hostname or the ipaddress(of the server) in string
{
char message[]="Client got connected";
int len;
IPaddress ip;
TCPsocket sock;
Uint16 portnum;
SDL_Event event;

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
while(!sock)
{

 while(SDL_PollEvent(&event)) 
  {
    if(event.type==SDL_KEYDOWN)
     {
       if(event.key.keysym.sym==SDLK_ESCAPE)
       return 7;
     }
  }
 if(!sock)
 {
 printf("SDLNet_TCP_Open: %s\n",SDLNet_GetError());
 sock=SDLNet_TCP_Open(&ip);

 continue;
 }
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
SDL_Event event;

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


printf("%s\n",waiting);
while(1)
{
  while( SDL_PollEvent( &event ) ) 
  {
    if(event.type==SDL_KEYDOWN)
     {
       if(event.key.keysym.sym==SDLK_ESCAPE)
       return 7;
     }
  }
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
                break;

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


