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


// NOTE: here are some pseudo-code thoughts about structuring the server program - DSB:
// 
// int main()
// {
//   //All the stuff before starting the main loop:
//   if (!setup_server())
//   {
//     //print error and exit...
//   }
// 
//   while (!done)
//   {
//     //See if any clients trying to connect:
//     //If we aren't playing a game yet, we take their name and add them to the client set,
//     //if we have room.
//     //If a game is in progress, we just tell them "sorry, call back later"
//     check_for_new_clients();
//     
//     //Now check all the connected clients to see if we have any messages from them:
//     check_client_messages();
// 
//   }
// 
//   //Free resources, call MC_EndGame(), and so forth:
//   server_cleanup();
// }
// 
// 
// //This function will use SDLNet_CheckSockets() and SDLNet_SocketReady()
// //Some of the messages will only be meaningful between games, and others will
// //only be meaningful during games;
// void check_client_messages(void)
// {
//   for (go through client set and handle active ones)
//   {
//     get_buffer_from_client();
//     get_title_out_of_buffer();
// 
//     //Handle all the 'non-game'messages:
//     if (strcmp(title, NON_GAME_FOO) == 0)
//     {
//       //do something (e.g. disconnect)
//     }
//     else if (strcmp(title, NON_GAME_BAR) == 0)
//     {
//       //do something else (e.g. start a new game)
//     }
// 
//     //Now handle gameplay-related messages:
//     if (strcmp(title, GAME_FOO) == 0)
//     {
//       //do something (e.g. client answered correctly)
//     }
//     else if (strcmp(title, GAME_BAR) == 0)
//     {
//       //do something else (e.g. client quits game)
//     }
// 
//   }
// }







#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include "server.h" 
#include "transtruct.h"
#include "mathcards.h"


#define NUM_CLIENTS 16

TCPsocket server_sock; /* Socket descriptor for server            */
TCPsocket temp_sock;        /* Just used when client can't be accepted */

SDLNet_SocketSet client_set = NULL;


static client_type client[NUM_CLIENTS];

/* Local function prototypes: */
int setup_server(void);
int SendQuestion(MC_FlashCard flash,TCPsocket client_sock);
int SendMessage(int message, int z,TCPsocket client_sock);


int main(int argc, char **argv)
{ 
  int h;
  IPaddress ip, *remoteIP;
  int quit, quit2;
  char buffer[NET_BUF_LEN];
  char buf[NET_BUF_LEN];
  int command_type = -1,numready,j;
  static int sockets_used=0;
  static int i=0;
  static int num_clients=0;
  MC_FlashCard flash;
  static int initialize = 0;
  int id,x;

  for(h=0;h<NUM_CLIENTS;h++)
  {
       client[h].connected=0;                          /*doing all connecteds = 0 meaning no clients are connected */
  }

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
  if (!(server_sock = SDLNet_TCP_Open(&ip)))
  {
    fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }

  client_set = SDLNet_AllocSocketSet(16);
  if(!client_set)
  {
    printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }

 
  /* Wait for a client connections*/
  quit = 0;                                                    /*****can say this loop to be the connection manager, which after accepting starts the game*****/
  while (!quit)
  {

      numready=SDLNet_CheckSockets(client_set,0);
      if(numready==-1)
      {
        printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
        //most of the time this is a system error, where perror might help you.
        perror("SDLNet_CheckSockets");
      }

      else if(numready) 
      {
        printf("There are %d sockets with activity!\n",numready);
        // check all sockets with SDLNet_SocketReady and handle the active ones.
        for(j=0;j<sockets_used;j++)
        {
         if(SDLNet_SocketReady(client[j].sock)) 
         {
          if (SDLNet_TCP_Recv(client[j].sock, buffer, NET_BUF_LEN) > 0)
          {
           if(strncmp(buffer,"start",5)==0)
           {
           quit=1;                                        //if any one player is ready the game stops accepting the connections , this can be modified later to suit the needs
           snprintf(buf, NET_BUF_LEN, 
                "%s\n",
                "Success");
           x = SDLNet_TCP_Send(client[j].sock, buf, sizeof(buf));
           client[j].connected=1;
           }
          }
         }
        }
      }

    /* This check the sd if there is a pending connection.
     * If there is one, accept that, and open a new socket for communicating */
    client[i].sock = SDLNet_TCP_Accept(server_sock);
    if (client[i].sock !=NULL)
    {
    
     if( SDLNet_TCP_Recv(client[i].sock, buffer, NET_BUF_LEN) > 0)
     {
      strcpy(client[i].name, buffer);
      printf(" JOINED  :::   %s",client[i].name);
     }
      printf("this is the value of i = %d\n",i);
      num_clients++;
      /* Now we can communicate with the client using client[i].sock socket
      /* sd will remain opened waiting other connections */
      /* Get the remote address */
      if ((remoteIP = SDLNet_TCP_GetPeerAddress(client[i].sock)))
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
      sockets_used = SDLNet_TCP_AddSocket(client_set,client[i].sock);
      if(sockets_used == -1) 
      {
        printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
        // perhaps you need to restart the set and make it bigger...
      }
      i++;
    
    }//end of *if(client[i].sock = SDLNet_TCP_Accept(sd))*  
    
  }

num_clients=sockets_used;


/*This loop sees that the game starts only when all the players are ready */
 for(j=0;j<num_clients;j++)
 {
  if(client[j].connected!=1)
  {
   if (SDLNet_TCP_Recv(client[j].sock, buffer, NET_BUF_LEN) > 0)
   {
    if(strncmp(buffer,"start",5)==0)
    {
     client[j].connected=1;
     snprintf(buf, NET_BUF_LEN, 
                "%s\n",
                "Success");
     x = SDLNet_TCP_Send(client[j].sock, buf, sizeof(buf));
    }
   }
  }
 }

/* If no players join the game */
if(num_clients==0)
{
 printf("There were no players........=(\n");
 SDLNet_FreeSocketSet(client_set);              //releasing the memory of the client socket set
 client_set=NULL; //this helps us remember that this set is not allocated
 SDLNet_TCP_Close(server_sock);
 SDLNet_Quit();
 exit(1);
}

#ifdef LAN_DEBUG
printf("We have %d players.......\n",sockets_used);
#endif



 if (!MC_StartGame())                   //setting up the list itself
 {
  fprintf(stderr, "\nMC_StartGame() failed!");
 }


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
 }
 for(j=0;j<num_clients;j++)
 {
  if(!SendQuestion(flash,client[j].sock))
  {
    printf("Unable to send Question\n");
  }
 } 

 quit2 = 0;
 while (!quit2)
 {
   while(1)       //keep on checking for all the clients in a round robin manner
   {
    for(j=0;j<num_clients;j++)                  // keep on looping across the num_clients in a round-robin manner
    {


      /* this is mainly to avoid joining of clients after the game has started*/
      temp_sock = SDLNet_TCP_Accept(server_sock);
      if (temp_sock !=NULL)
      {

  snprintf(buf, NET_BUF_LEN, 
                "%s\n",
                "Sorry the game has started...... =(\n");
     x = SDLNet_TCP_Send(temp_sock, buf, sizeof(buf));
#ifdef LAN_DEBUG
  printf("buf sent:::: %d bytes\n", x);
  printf("buf is: %s\n", buf);
#endif
    
      }        
      /*This loop mainly checks if all the clients have disconnected*/
      int c;
      for(c=0;c<num_clients;c++)
      {
       if(client[c].connected==1)
       break;     
       if(c==(num_clients-1))
        {
         printf("All the clients have disconnected..=( \n");
         exit(2);
        }
      }

      /*Implies that this particular client has already quit itself , so move on to other clients*/         
      if(client[j].connected==0)
      continue;                                           
      numready=SDLNet_CheckSockets(client_set,0);
      if(numready==-1) 
      {
        printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
        //most of the time this is a system error, where perror might help you.
        perror("SDLNet_CheckSockets");
      }
      else if(numready) 
      {
        printf("There are %d sockets with activity!\n",numready);
        // check all sockets with SDLNet_SocketReady and handle the active ones.
        if(SDLNet_SocketReady(client[j].sock)) 
        {
         if (SDLNet_TCP_Recv(client[j].sock, buffer, NET_BUF_LEN) > 0  )
         {
           char command[NET_BUF_LEN];
           command_type = -1;
#ifdef LAN_DEBUG  
           printf("Buffer received from client: %s\n", buffer);
#endif
           sscanf (buffer,"%s %d\n",
                         command,
                         &id);  
     
           if(strncmp(command, "CORRECT_ANSWER",14) == 0)
           {
             command_type = CORRECT_ANSWER; 
             printf("question id %d was answered correctly by %s",id,client[j].name);             
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
               }
               int n;
               for(n=0;n<num_clients&&client[n].sock;n++)
               {

                 if(!SendQuestion(flash,client[n].sock))
                 {
                   printf("Unable to send Question\n");
                 }
               } 
           }                             
                                        
           //'b' for asking for a question(flashcard)                    //Now it has no use , since the clients dont ask question.
           if(strncmp(command, "next_question",13) == 0)
           {
#ifdef LAN_DEBUG
             printf("received request to send question\n");
#endif
               command_type = SEND_A_QUESTION;              
           } 

           if(strncmp(command, "exit",4) == 0) /* Terminate this connection */
           {
             printf("LEFT the GAME : %s",client[j].name);
             client[j].connected=0;
             SDLNet_TCP_DelSocket(client_set,client[j].sock);
             SDLNet_TCP_Close(client[j].sock);
             printf("Terminating client connection\n");
           }

           if(strncmp(command, "quit",4) == 0) /* Quit the program */
           {
             printf("Server has been shut down by %s",client[j].name); 
             client[j].connected=0;
             SDLNet_TCP_DelSocket(client_set,client[j].sock);
             SDLNet_TCP_Close(client[j].sock);
             quit2 = 1;
             printf("Quit program....Server is shutting down...\n");
           }
          
           switch(command_type)
           {
//             case NEW_GAME:  //mainly to setup the question list
//             {
//               if (!MC_StartGame())
//               {
//                 fprintf(stderr, "\nMC_StartGame() failed!");
//               }
//               if(!SendMessage(LIST_SET_UP,0,client[j].sock))
//              {
//                 printf("Unable to communicate to the client\n");
//               }
//               break;                                           
//             } 

             case CORRECT_ANSWER:
             {
 //              if(!SendMessage(ANSWER_CORRECT,id,client[j].sock))
 //              {
 //                printf("Unable to communicate to the client\n");
 //              }
               break;
             }

             case LIST_NOT_SETUP:                    //to send any message to the client 
             {              
               if(!SendMessage(NO_QUESTION_LIST,id,client[j].sock))
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
                 if(!SendQuestion(flash,client[j].sock))
                 {
                   printf("Unable to send Question\n");
                 }
               } 

               break;
             } 
        
             default:
               break;                             //this *break* comes out of the switch statement
           }

         }//if loop
       }
    }

         if(quit2==1)
         break;   

        }//end of for loop
        if(quit2==1)
        break;   
       }//  end of while(1) loop
      }//while loop

      /* Close the client socket */
  
    for(j=0;j<num_clients;j++)
    {
     if(client[j].connected==1)                           //close only those clients that are still connected 
     SDLNet_TCP_Close(client[j].sock);                //close all the client sockets one by one
    }          
    SDLNet_FreeSocketSet(client_set);              //releasing the memory of the client socket set
    client_set=NULL; //this helps us remember that this set is not allocated
      
  
 /* Clean up mathcards heap memory */
  MC_EndGame();
  SDLNet_TCP_Close(server_sock);
  SDLNet_Quit();
 
  return EXIT_SUCCESS;
}


/*********************************************************************/
/*  "Private" (to server.c) functions                                */
/*********************************************************************/

// Initialize mathcards with hard-coded defaults and set up server
// socket:
int setup_server(void)
{
  return 1;
}




//function to send a flashcard(question) from the server to the client
int SendQuestion(MC_FlashCard flash,TCPsocket client_sock)
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
  x = SDLNet_TCP_Send(client_sock, buf, sizeof(buf));

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
int SendMessage(int message, int z,TCPsocket client_sock)         
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
  x = SDLNet_TCP_Send(client_sock, buf, NET_BUF_LEN);

//#ifdef LAN_DEBUG
  printf("buf is: %s\n", buf);
  printf("SendMessage() - buf sent:::: %d bytes\n", x);
//#endif

  return 1;
}


