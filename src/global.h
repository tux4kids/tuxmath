#ifndef _GLOBAL_H
#define _GLOBAL_H
#include "globals.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<SDL/SDL_net.h>
#include<semaphore.h>
#include<unistd.h>
#include<pthread.h>

#define MAX_CLIENTS 16
#define NAME_SIZE 50


IPaddress host_ipaddress; // will hold server ip address

void * slave_server(void* data);


typedef struct srv_game_type {
  char lesson_name[NAME_SIZE];
  int wave;
  int active_quests;        //Number of questions currently "in play"
  int max_quests_on_screen;
  int quests_in_wave;
  int rem_in_wave;          //Number still to be issued in wave
}srv_game_type; 

typedef struct client_type {
    int game_ready;   //game_ready = 1 means client has said OK to start
    char name[NAME_SIZE];
    int score;
    TCPsocket sock;
}client_type; 



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

/* used for keeping record of every instance of a thread running within a server */
struct threadID
{
	int reference;   // to be used as a reference in thread pool 
	pthread_t ID;
	int status; // 1 if thread is in use or otherwise zero.	
	sem_t binary_sem; // used as binary semphore
	struct socket client_socket;
	struct threadID *next;
	//struct server_info info;   // store information of a server contained in server pool
	UDPsocket udpsock ;              // Used to listen for client's server autodetection        
	TCPsocket server_sock;    // Socket descriptor for server to accept client TCP sockets. 
	IPaddress ip;
	SDLNet_SocketSet client_set;
	struct client_type client[MAX_CLIENTS];  //TODO Deepak removed static from it as they can't be declared inside it. might result problem in future 
	int num_clients;
	struct srv_game_type srv_game;
	int game_in_progress;
}; 
extern struct threadID * thread_header; // header use for pointing to standby/inactive threads
extern pthread_mutex_t mutex_variable;

#endif 
