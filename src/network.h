#ifndef NETWORK_H
#define NETWORK_H

/*

network.h     - Provides routines for various networking functions to be used
                in the LAN multiplayer game.


*/

#include "mathcards.h"

int lan_server_connect(char *port);
int lan_client_connect(char *host,char *port);
int SendQuestion(MC_FlashCard* fc);
int ReceiveQuestion(MC_FlashCard* fc);
int ReceiveInt(int x);
int SendInt(int x); 
int SendQuestionList(MC_MathQuestion* ql,int list_length);
int ReceiveQuestionList(MC_MathQuestion* ql,int list_length);
#endif // NETWORK_H
