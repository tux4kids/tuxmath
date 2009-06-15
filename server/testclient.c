/*
*  C Implementation: server.c
*
*       Description: Test client program for LAN-based play in Tux,of Math Command.
*
*
* Author: Akash Gangil, David Bruce, and the TuxMath team, (C) 2009
* Developers list: <tuxmath-devel@lists.sourceforge.net>
*
* Copyright: See COPYING file that comes with this distribution.  (Briefly, GNU GPL).
*
* NOTE: This file was initially based on example code from The Game Programming Wiki
* (http://gpwiki.org), in a tutorial covered by the GNU Free Documentation License 1.2.
* No invariant sections were indicated, and no separate license for the example code
* was listed. The author was also not listed. AFAICT,this scenario allows incorporation of
* derivative works into a GPLv2+ project like TuxMath - David Bruce 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include "SDL_net.h"
#include "transtruct.h"
#include "mathcards.h"
#include "mathcards.c"

TCPsocket sd;           /* Socket descriptor */

int main(int argc, char **argv)
{
        IPaddress ip;           /* Server address */
        int quit, len;
        char buffer[512];
        MC_FlashCard* fc;
      
        /* Simple parameter checking */
        if (argc < 3)
        {
                fprintf(stderr, "Usage: %s host port\n", argv[0]);
                exit(EXIT_FAILURE);
        }
 
        if (SDLNet_Init() < 0)
        {
                fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
                exit(EXIT_FAILURE);
        }
 
        /* Resolve the host we are connecting to */
        if (SDLNet_ResolveHost(&ip, argv[1], atoi(argv[2])) < 0)
        {
                fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
                exit(EXIT_FAILURE);
        }
 
        /* Open a connection with the IP provided (listen on the host's port) */
        if (!(sd = SDLNet_TCP_Open(&ip)))
        {
                fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
                exit(EXIT_FAILURE);
        }
 
        /* Send messages */
        quit = 0;
        while (!quit)
        {
                printf("Write something:\n>");
                scanf("%s", buffer);
 
                len = strlen(buffer) + 1;
                if (SDLNet_TCP_Send(sd, (void *)buffer, len) < len)
                {
                        fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
                        exit(EXIT_FAILURE);
                }
                if(strcmp(buffer,"b")==0)
		{ 
                  fc = (MC_FlashCard *)malloc(sizeof(MC_FlashCard));
        
                                                    if (fc == NULL) 
            						{
						             printf("Allocation of comets failed");
						             return 0;
					                }
         
					           else 
					                {
    						            *fc = MC_AllocateFlashcard();
						            if (!MC_FlashCardGood(fc) ) 
						             {
						              //something's wrong
						              printf("Allocation of flashcard failed\n");
						              MC_FreeFlashcard(fc);
						              return 0;
						             }
					                }
                 if(!RecvQuestion(fc))
                 printf("unable to recv question\n"); 
                }  

                if(strcmp(buffer, "exit") == 0)
                        quit = 1;
                if(strcmp(buffer, "quit") == 0)
                        quit = 1;
        }
 
        SDLNet_TCP_Close(sd);
        SDLNet_Quit();
 
        return EXIT_SUCCESS;
}

int RecvQuestion(MC_FlashCard* fc)                           //function to receive a flashcard(question) by the client
{
       char ch[5];
       int x,i=0;

       x=SDLNet_TCP_Recv(sd,&(fc->question_id),sizeof(fc->question_id));
       printf("no:(1):::QUESTION_ID::::Received %d bytes\n",x);
 
       x=SDLNet_TCP_Recv(sd,&(fc->difficulty),sizeof(fc->difficulty));
       printf("no:(2):::DIFFICULTY::::Received %d bytes\n",x);
 
       x=SDLNet_TCP_Recv(sd,&(fc->answer),sizeof(fc->answer));
       printf("no:(3):::ANSWER::::Received %d bytes\n",x);

       do{
       x=SDLNet_TCP_Recv(sd,&ch[i],1);      
       printf("<<<SUB-PACKET%d>>>no:(4):::ANSWER_STRING::::Received %d bytes\n",i,x);
       i++;
       }while(ch[i-1]!='\0');
       strncpy(fc->answer_string,ch,i+1);

       x=SDLNet_TCP_Recv(sd,fc->formula_string,13);
       printf("no:(5):::FORMULA_STRING::::Received %d bytes\n",x);
       
       printf("RECEIVED >>\n");
       printf("QUESTION_ID    >>          %d\n",fc->question_id);  
       printf("FORMULA_STRING >>          %s\n",fc->formula_string);  
       printf("ANSWER_STRING  >>          %s\n",fc->answer_string);  
       printf("ANSWER         >>          %d\n",fc->answer);  
       printf("DIFFICULTY     >>          %d\n",fc->difficulty);  
       
       return 1;
}



 
