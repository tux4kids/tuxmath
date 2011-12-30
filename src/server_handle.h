#ifndef SERVER_HANDLE_H
#define SERVER_HANDLE_H
#include"global.h"
#include"thread_handle.h"
extern int global_port;
extern int max_slave_server;
int common_connect_server(IPaddress *host_ipaddress,TCPsocket *server_socket,Uint16 port,const char *host);
void * slave_server(void * data);
void * lobby_server(void * data);
pthread_t * SDLserver_init(int _max_slave_server,int _global_port); // funtion for starting server
void SDLserver_quit(); //function for exiting server
#endif
