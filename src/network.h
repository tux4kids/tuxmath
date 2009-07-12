/*

        network.h

        Description: Provides routines for various networking functions to be used
                     in the LAN multiplayer game.
        Author: David Bruce ,Akash Gangil and the TuxMath team, (C) 2009

        Copyright: See COPYING file that comes with this distribution (briefly, GNU GPL version 2 or later)

*/


#include "transtruct.h"


#ifndef NETWORK_H
#define NETWORK_H

int setup_net(char *host, int port);
int say_to_server(char *statement);
int evaluate(char *statement);
int LAN_AnsweredCorrectly(MC_FlashCard* fc);
void cleanup_client(void);
int check_messages(char *);
int player_msg_recvd(char* buf);
int Make_Flashcard(char* buf, MC_FlashCard* fc);
void server_pinged(void);        //The ping system is not yet used and so is this function.
#endif // NETWORK_H
