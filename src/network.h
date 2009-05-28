#ifndef NETWORK_H
#define NETWORK_H

/*

network.h     - Provides routines for various networking functions to be used
                in the LAN multiplayer game.


*/

int lan_server_connect(int port);
int lan_client_connect(char *host,int port);
 
#endif // NETWORK_H
