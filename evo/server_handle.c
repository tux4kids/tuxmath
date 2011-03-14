#include"server_handle.h"
#define PAUSE 3
#include<time.h>
int quit=1; //for quiting lobby_server
int common_connect_server(IPaddress *host_ipaddress,TCPsocket *server_socket,Uint16 port,const char *host)
{
	if(SDLNet_ResolveHost(host_ipaddress,host,port)!=0)
	{
		printf("\nUnable to initialize ipaddress for server");
		return 0;
	}
	printf("\n I am listening on port %d",port);
	if(!(*server_socket=SDLNet_TCP_Open(host_ipaddress)))
	{	
		printf("\nUnable to initialize server socket:%s",SDLNet_GetError());
		return 0;
	}
	return 1;
}
/*///////////////////////////////////////////////////////////

	Function for operation on threads 
/////////////////////////////////////////////////////////////*/
void * slave_server(void * data)
{
	struct threadID * temp_thread=(struct threadID *) data;
	int quit1=1;
	int *temp=(int *) data;
	char buffer[512];
	TCPsocket client_socket;
	while(1)   // to make sure thread doesn't terminate
	{
		if(temp_thread->status==0)
		{
			sem_wait(&(temp_thread->binary_sem));
		}
		printf("\n Activating thread");
		// here thread start receiving data from client	
		while(quit1)
		{
			if(client_socket=SDLNet_TCP_Accept(temp_thread->client.server_socket))
			{	
				printf("\nGetting request from client");
				quit1=1;
				while(quit1)
				{
					if(temp_thread->client.client_ipaddress=SDLNet_TCP_GetPeerAddress(client_socket))
						printf("\n Listening from client:%x",SDLNet_Read32(&temp_thread->client.client_ipaddress->host));
					if(SDLNet_TCP_Recv(client_socket,buffer,512))
					{
						printf("\nClient send:%s",buffer);
						if(!strcmp(buffer,"exit"))
						{
							printf("\n Client requesting for closing connection");
							quit1=0;
						}
						if(!strcmp(buffer,"quit"))
						{
							quit1=0;
							quit=0;
						}
					}
				}
				printf("\nClosing client socket");
				SDLNet_TCP_Close(client_socket);
			}
		}
		quit1=1; //so that this thread can be used again
		temp_thread->status=0;
		push_thread(temp_thread);
	}// here it ends 
	printf("\n Terminating thread"); //TODO 
}

void * lobby_server(void * data)
{
	int flag=1,choice,n;
	int temp;
	int i;
	struct timespec t;
	t.tv_sec=PAUSE/1000;
	t.tv_nsec=(PAUSE%1000) * (1000*1000);
	TCPsocket client_socket,server_socket;
	for(i=1;i<=max_slave_server;i++)
	{
		start_new_thread(2000+i);
	}
	printf("\n Value of active ports is:%d",get_active_threads());

	// first server receive request from client to connect and open master server socket. To be created only once.-permanent 
	if(common_connect_server(&host_ipaddress,&server_socket,global_port,(const char *)NULL)==0)
		return (void *)1;
	while(quit) 
	{
		nanosleep(&t,NULL);
		// Open client socket on server side for sending dynamic port address-temprary
		if((client_socket=SDLNet_TCP_Accept(server_socket)))
		{
 			// send port address to client
			pthread_mutex_lock(&mutex_variable);
			printf("\n Value of active ports:%d",get_active_threads());
			if(get_active_threads()==max_slave_server)
			{
				int temp=0;
				SDLNet_TCP_Send(client_socket,&(temp),2);
				SDLNet_TCP_Close(client_socket);
				pthread_mutex_unlock(&mutex_variable);
			}
			else
				if(SDLNet_TCP_Send(client_socket,&(thread_header->client.port),2)==2)
				{
					printf("\nNew Port is send to client"); 
					// close temprary client socket 
					SDLNet_TCP_Close(client_socket);
					// opening port so that server can accept content from client in different thread;
					pthread_mutex_unlock(&mutex_variable);
					printf("\n Activating thread");
					activate_thread();
				}
		}
	}
		printf("\nEverything is OK now exiting");	
		SDLNet_TCP_Close(server_socket);
		cleanup_thread();
		return NULL;
}
pthread_t * SDLserver_init(int _max_slave_server,int _global_port)   
{
	static pthread_t thread;
	if(SDLNet_Init()!=0)
	{
		printf("\nUnable to initialize sdl");
	}
	atexit(SDLNet_Quit);
	max_slave_server=_max_slave_server;
	global_port=_global_port;
	pthread_mutex_init(&mutex_variable,NULL);
	pthread_create(&thread,NULL,lobby_server,NULL);
	return (&thread);
}
void SDLserver_quit()
{
pthread_mutex_destroy(&mutex_variable);
}




