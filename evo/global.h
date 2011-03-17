#ifndef _GLOBAL_H
#define _GLOBAL_H
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<SDL/SDL_net.h>
#include<semaphore.h>
#include<unistd.h>
#include<pthread.h>
IPaddress host_ipaddress; // will hold server ip address

void * slave_server(void* data);

// structure which contain information for a particular thread 
struct socket 
{
	IPaddress *client_ipaddress;
	TCPsocket server_socket;
	Uint16 port;
};
struct server_info   // information of server. Used to transmit available/active  server information to client  
{
char info[100];
Uint16 port;
}; 
struct threadID
{
	int reference;   // to be used as a reference in thread pool 
	pthread_t ID;
	int status; // 1 if thread is in use or otherwise zero.	
	sem_t binary_sem; // used as binary semphore
	struct socket client;
	struct threadID *next;
	//struct server_info info;   // store information of a server contained in server pool
}; 
extern struct threadID * thread_header; // header use for pointing to standby/inactive threads
extern pthread_mutex_t mutex_variable;

#endif 
