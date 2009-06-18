/*
*  C Implementation: server.c
*
*       Description: Server program for LAN-based play in Tux,of Math Command.
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
#include "server.h" 

TCPsocket sd; /* Socket descriptor, Client socket descriptor */
SDLNet_SocketSet client_set;
int SendMessage(int ,int );
 
int main(int argc, char **argv)
{ 
  IPaddress ip, *remoteIP;
  int quit, quit2;
  char buffer[NET_BUF_LEN];
  int command_type = -1;
  int sockets_used;
  //     size_t length;
  MC_FlashCard flash;
  static int initialize = 0;
  int id;

  printf("Started tuxmathserver, waiting for client to connect:\n>\n");

  if (!MC_Initialize())
  {
    fprintf(stderr, "Could not initialize MathCards\n");
    exit(EXIT_FAILURE);
  }

      
  if (SDLNet_Init() < 0)
  {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
  /* Resolving the host using NULL make network interface to listen */
  if (SDLNet_ResolveHost(&ip, NULL, DEFAULT_PORT) < 0)
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

  client_set = SDLNet_AllocSocketSet(16);
  if(!client_set) {
    printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }

 
  /* Wait for a connection, send data and term */
  quit = 0;
  while (!quit)
  {
   for(i=0;i<16;i++)
   {
    if(client[i].flag==0)
    break;
   } 
  
    /* This check the sd if there is a pending connection.
     * If there is one, accept that, and open a new socket for communicating */
    if ((client[i].csd = SDLNet_TCP_Accept(sd)))
    {
      sockets_used = SDLNet_TCP_AddSocket(client_set,client[i].csd);
      if(sockets_used == -1) {
      printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
      // perhaps you need to restart the set and make it bigger...
      }
     client[i].flag=1;



      /* Now we can communicate with the client using csd socket
       * sd will remain opened waiting other connections */
 
      /* Get the remote address */
      if ((remoteIP = SDLNet_TCP_GetPeerAddress(client[i].csd)))
        /* Print the address, converting in the host format */
      {
        printf("Client connected\n>\n");
#ifdef LAN_DEBUG
        printf("Client: IP = %x, Port = %d\n",
	       SDLNet_Read32(&remoteIP->host),
	       SDLNet_Read16(&remoteIP->port));
#endif
      }
      else
        fprintf(stderr, "SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError());
                        
      quit2 = 0;
      while (!quit2)
      {
        char command[NET_BUF_LEN];
        // basically we cant wait here anymore we need to check if the socket is ready if not then give a chance to other client sockets
        if (SDLNet_TCP_Recv(client[i].csd, buffer, NET_BUF_LEN) > 0)
        {
          command_type = -1;
#ifdef LAN_DEBUG  
          printf("Buffer received from client: %s\n", buffer);
#endif
          sscanf (buffer,"%s %d\n",
                         command,
                         &id);  
     
          if(strcmp(command, "CORRECT_ANSWER") == 0)
          {
            command_type = CORRECT_ANSWER;              
          }                             
          
          //'a' for the setting up the question list                                           
          if(strcmp(command, "a") == 0)
          {
            initialize = 1; 
            command_type = NEW_GAME;              
          } 
                                       
          //'b' for asking for a question(flashcard)
          if(strcmp(command, "b") == 0)
          {
#ifdef LAN_DEBUG
            printf("received request to send question\n");
#endif
            if(!initialize)
            {
              command_type = LIST_NOT_SETUP;                    
            }
            else
              command_type = SEND_A_QUESTION;              
          } 

          if(strcmp(command, "exit") == 0) /* Terminate this connection */
          {
            quit2 = 1;
            printf("Terminating client connection\n");
          }

          if(strcmp(command, "quit") == 0) /* Quit the program */
          {
            quit2 = 1;
            quit = 1;
            printf("Quit program\n");
          }
          
          switch(command_type)
          {
            case NEW_GAME:  //mainly to setup the question list
            {
              if (!MC_StartGame())
              {
                fprintf(stderr, "\nMC_StartGame() failed!");
              }
              if(!SendMessage(LIST_SET_UP,0))
              {
                printf("Unable to communicate to the client\n");
              }
              break;                                           
            } 

            case CORRECT_ANSWER:
            {
              if(!SendMessage(ANSWER_CORRECT,id))
              {
                printf("Unable to communicate to the client\n");
              }
              break;
            }

            case LIST_NOT_SETUP:                    //to send any message to the client 
            {              
              if(!SendMessage(NO_QUESTION_LIST,id))
              {
                printf("Unable to communicate to the client\n");
              }
              break;
            }

            case SEND_A_QUESTION:
            {
              if (!MC_NextQuestion(&flash))
              { 
                /* no more questions available */
                printf("MC_NextQuestion() returned NULL - no questions available\n");
              }
              else
              {                                     
#ifdef LAN_DEBUG
                printf("WILL SEND >>\n");  
                printf("QUESTION_ID       :      %d\n", flash.question_id);
                printf("FORMULA_STRING    :      %s\n", flash.formula_string);
                printf("ANSWER STRING     :      %s\n", flash.answer_string);
                printf("ANSWER            :      %d\n",flash.answer);
                printf("DIFFICULTY        :      %d\n",flash.difficulty);
#endif
                if(!SendQuestion(flash))
                {
                  printf("Unable to send Question\n");
                }
              } 

              break;
            } 
        
            default:
              break;
          }

        }
      }

      /* Close the client socket */
      SDLNet_TCP_Close(client[i].csd);            //  int SDLNet TCP DelSocket(SDLNet_SocketSet set, TCPsocket sock )
      
    }
  }
  /* Clean up mathcards heap memory */
  MC_EndGame();
  SDL_NetFreeSocketSet(client_set);
  SDLNet_TCP_Close(sd);
  SDLNet_Quit();
 
  return EXIT_SUCCESS;
}


//function to send a flashcard(question) from the server to the client
int SendQuestion(MC_FlashCard flash)
{
  int x;

  char buf[NET_BUF_LEN];
  snprintf(buf, NET_BUF_LEN, 
                "%s\t%d\t%d\t%d\t%s\t%s\n",
                "SEND_QUESTION",
                flash.question_id,
                flash.difficulty,
                flash.answer,
                flash.answer_string,
                flash.formula_string);
  x = SDLNet_TCP_Send(client[i].csd, buf, sizeof(buf));

#ifdef LAN_DEBUG
  printf("SendQuestion() - buf sent:::: %d bytes\n", x);
  printf("buf is: %s\n", buf);
#endif

  if (x == 0)
    return 0;
  return 1;
}


/*Function to send any messages to the client be it any warnings
  or anything the client is made to be informed*/
int SendMessage(int message, int z)         
{
 int x, len;
 char buf[NET_BUF_LEN];
 char msg[100];  

 /* Create appropriate message: */
  switch(message)
  {
    case NO_QUESTION_LIST:
      sprintf(msg,"%s", "Please! first setup the question list by typing <a>\n");
      break;
    case ANSWER_CORRECT:
      sprintf(msg,"%s %d %s", "Question ID:",
              z, "was answered correctly by the client\n");
      break;
   case LIST_SET_UP:
      sprintf(msg,"%s", "Question list was successfully setup\n");
      break;
   default :
     fprintf(stderr, "SendMessage() - unrecognized message type\n");
     return 0;
  }
  //transmit:
  snprintf(buf, NET_BUF_LEN, "%s\t%s\n", "SEND_MESSAGE", msg);
  x = SDLNet_TCP_Send(client[i].csd, buf, NET_BUF_LEN);

//#ifdef LAN_DEBUG
  printf("buf is: %s\n", buf);
  printf("SendMessage() - buf sent:::: %d bytes\n", x);
//#endif

  return 1;
}


