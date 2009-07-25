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
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 

//#include "SDL_net.h"
#include "../src/transtruct.h"
#include "mathcards.h"
#include "testclient.h"
#include "../src/throttle.h"
#include "../src/network.h"

/* Local (to testclient.c) "globals": */

int quit = 0;
int game_status = GAME_NOT_STARTED;
MC_FlashCard flash;    //current question

MC_FlashCard comets[TEST_COMETS];    //current questions
int remaining_quests = 0;


/* Local function prototypes: */
int playgame(void);
int erase_flashcard(MC_FlashCard* fc);
MC_FlashCard* find_comet_by_id(int id);
MC_FlashCard* check_answer(int ans);

int read_stdin_nonblock(char* buf, size_t max_length);

/* Functions to handle messages from server: */
int game_check_msgs(void);
int add_quest_recvd(char* buf);
int remove_quest_recvd(char* buf);
int player_msg_recvd(char* buf);
int total_quests_recvd(char* buf);
int mission_accompl_recvd(char* buf);

/* Display to player: */
void print_current_quests(void);

/* Main function: ------------------------------------- */

int main(int argc, char **argv)
{
  char buf[NET_BUF_LEN];     // for network messages from server
  char buffer[NET_BUF_LEN];  // for command-line input
  int servers_found = 0;
  Uint32 server_ip = 0;
  Uint16 server_port = DEFAULT_PORT;

  //Scan local network to find running server:
  servers_found = LAN_DetectServers();

  if(servers_found < 1)
  {
    printf("No server could be found - exiting.\n");
    exit(EXIT_FAILURE);
  }
  else if(servers_found  == 1)  //One server - connect without player intervention
  {
    if(!LAN_AutoSetup(0))  //i.e.first (and only) entry in list
    {
      printf("setup_client() failed - exiting.\n");
      exit(EXIT_FAILURE);
    }
  } 
  else  // More than one server - will have to get player selection 
  {
    //Display list so player can choose
    //   TO BE IMPLEMENTED


    /* Connect to server, create socket set, get player nickname, etc: */
    if(!LAN_Setup(server_ip, server_port))
    {
      printf("setup_client() failed - exiting.\n");
      exit(EXIT_FAILURE);
    }
  }
  

  /* Now we are connected - get nickname from player: */
  {
    char name[NAME_SIZE];
    char* p;

    printf("Please enter your name:\n>\n");
    fgets(buffer, NAME_SIZE, stdin);
    p = strchr(buffer, '\n');  //get rid of newline character
    if(p)
      *p = '\0';
    strncpy(name, buffer, NAME_SIZE);
    /* If no nickname received, use default: */
    if(strlen(name) == 1)
      strcpy(name, "Anonymous Coward");

    snprintf(buffer, NET_BUF_LEN, "%s", name);
    LAN_SetName(name);
  }

  printf("Welcome to the Tux Math Test Client!\n");
  printf("Type:\n"
         "'game' to start math game;\n"
         "'exit' to end client leaving server running;\n"
         "'quit' to end both client and server\n>\n"); 

  
  /* Set stdin to be non-blocking: */
  fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) | O_NONBLOCK);


  /* FIXME we're not listening for server messages in this loop */
  quit = 0;
  while(!quit)
  { 
    //Get user input from command line and send it to server: 
    /*now display the options*/
    if(read_stdin_nonblock(buffer, NET_BUF_LEN))
    { 
      //Figure out if we are trying to quit:
      if( (strncmp(buffer, "exit", 4) == 0)
        ||(strncmp(buffer, "quit", 4) == 0))
      //FIXME need a "LAN_Logout() function" so testclient doesn't need to know about SDL_Net
      {
        quit = 1;
/*        if (SDLNet_TCP_Send(sd, (void *)buffer, NET_BUF_LEN) < NET_BUF_LEN)
        {
          fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
          exit(EXIT_FAILURE);
        }*/
      }
      else if (strncmp(buffer, "game", 4) == 0)
      {
        playgame();
        printf("Math game finished.\n\n");
        printf("Type:\n"
               "'game' to start math game;\n"
               "'exit' to end client leaving server running;\n"
               "'quit' to end both client and server\n>\n"); 
      }
      else
      {
        printf("Command not recognized. Type:\n"
               "'game' to start math game;\n"
               "'exit' to end client leaving server running;\n"
               "'quit' to end both client and server\n\n>\n");
      }
    }
    //Limit loop to once per 10 msec so we don't eat all CPU
    Throttle(10);
  }
 
  LAN_Cleanup();
 
  return EXIT_SUCCESS;
}





int game_check_msgs(void)
{
  char buf[NET_BUF_LEN];
  int status = 1;
  while(1)
  {
    buf[0] = '\0';
    status = LAN_NextMsg(buf);
    if (status == -1)  //Fatal error
    {
      printf("Error - LAN_NextMsg() returned -1\n");
      return -1;
    }

    if (status == 0)   //No more messages
    {
      break;
    }
#ifdef LAN_DEBUG
    printf("Buffer from server is: %s\n", buf);
#endif

    /* Now we process the buffer according to the command: */
    if(strncmp(buf, "SEND_QUESTION", strlen("SEND_QUESTION")) == 0)
    {
      if(!add_quest_recvd(buf))
        printf("SEND_QUESTION received but could not add question\n");
      else
        // If we successfully added question, show new questions to user:
        print_current_quests();
    }
    else if(strncmp(buf, "ADD_QUESTION", strlen("ADD_QUESTION")) == 0)
    {
      if(!add_quest_recvd(buf))
        printf("ADD_QUESTION received but could not add question\n");
      else  
        print_current_quests();
    }
    else if(strncmp(buf, "REMOVE_QUESTION", strlen("REMOVE_QUESTION")) == 0)
    {
      if(!remove_quest_recvd(buf)) //remove the question with id in buf
        printf("REMOVE_QUESTION received but could not remove question\n");
      else 
        print_current_quests();
    }
    else if(strncmp(buf, "SEND_MESSAGE", strlen("SEND_MESSAGE")) == 0)
    {
      printf("%s\n", buf);
    }
    else if(strncmp(buf, "PLAYER_MSG", strlen("PLAYER_MSG")) == 0)
    {
      player_msg_recvd(buf);
    }
    else if(strncmp(buf, "TOTAL_QUESTIONS", strlen("TOTAL_QUESTIONS")) == 0)
    {
      //update the "questions remaining" counter
      total_quests_recvd(buf);
    }
    else if(strncmp(buf, "MISSION_ACCOMPLISHED", strlen("MISSION_ACCOMPLISHED")) == 0)
    {
      game_status = GAME_OVER_WON; 
    }
    else 
    {
      printf("game_check_msgs() - unrecognized message: %s\n", buf);
    }
  }

  return 1;
}



int add_quest_recvd(char* buf)
{
  MC_FlashCard* fc = find_comet_by_id(-1);

  if(!fc || !buf)
  {
    printf("NULL fc or buf\n");
    return 0;
  }
  /* function call to parse buffer and receive question */
  if(!Make_Flashcard(buf, fc))
  {
    printf("Unable to parse buffer into FlashCard\n");
    return 0;
  }

  return 1;
}



int remove_quest_recvd(char* buf)
{
  int id = 0;
  char* p = NULL;
  MC_FlashCard* fc = NULL;

  if(!buf)
    return 0;

  p = strchr(buf, '\t');
  if(!p)
    return 0;

  id = atoi(p);
  fc = find_comet_by_id(id);
  if(!fc)
    return 0;

  erase_flashcard(fc);
  return 1;
}



/* This function prints the 'msg' part of the buffer (i.e. everything */
/* after the first '\t') to stdout.                                   */
int player_msg_recvd(char* buf)
{
  char* p;
  if(buf == NULL)
    return 0;
  p = strchr(buf, '\t');
  if(p)
  { 
    p++;
    printf("%s\n", p);
    return 1;
  }
  else
    return 0;
}


int total_quests_recvd(char* buf)
{
  char* p;
  if(buf == NULL)
    return 0;
  p = strchr(buf, '\t');
  if(p)
  { 
    p++;
    remaining_quests = atoi(p); 
    return 1;
  }
  else
    return 0;
}






int playgame(void)
{
  int numready;
  int command_type;
  int ans = 0;
  MC_FlashCard* fc = NULL;
  int x=0, i = 0;
  char buf[NET_BUF_LEN];
  char buffer[NET_BUF_LEN];
  char ch;

  printf("\nStarting Tux, of the Math Command Line ;-)\n");
  printf("Waiting for other players to be ready...\n\n");

  //Tell server we're ready to start:
  LAN_StartGame(); 
  game_status = GAME_IN_PROGRESS;

  /* Start out with our "comets" empty: */
  {
    int i;
    for(i = 0; i < TEST_COMETS; i ++)
      erase_flashcard(&comets[i]);
  }

  //Begin game loop:
  while (game_status == GAME_IN_PROGRESS)
  {

    //Check our network messages, bailing out for fatal errors:
    if (game_check_msgs() == -1)
      return -1;


    //Now we check for any user responses

    //This function returns 1 and updates buf with input from
    //stdin if input is present.
    //If no input, it returns 0 without blocking or waiting
    if(read_stdin_nonblock(buf, NET_BUF_LEN))
    {
      //While in game, these just quit the current math game:
      if ((strncmp(buf, "quit", 4) == 0)
        ||(strncmp(buf, "exit", 4) == 0)
        ||(strncmp(buf, "q", 1) == 0))
      {
        game_status = GAME_OVER_ESCAPE;
//        end = 1;   //Exit our loop in playgame()
        //Tell server we are quitting current game:
        LAN_LeaveGame();
      }
      else
      {
        /*NOTE atoi() will return zero for any string that is not
        a valid int, not just '0' - should not be a big deal for
        our test program - DSB */
        ans = atoi(buf);
        fc = check_answer(ans);
        if((fc != NULL))
        {  
          printf("%s is correct!\nAwait next question...\n>\n", buf);
          //Tell server we answered it right:
          LAN_AnsweredCorrectly(fc);
          erase_flashcard(fc);  
        }
        else  //we got input, but not the correct answer:
        {
          printf("Sorry, %s is incorrect. Try again!\n", buf); 
          print_current_quests();
        }
      }  //input wasn't any of our keywords
    } // Input was received 

    Throttle(10);  //so don't eat all CPU
  } //End of game loop 

  switch(game_status)
  {
    case GAME_OVER_ESCAPE:
      printf("You quit :(\n");
      break;
    case GAME_OVER_WON:
      printf("You won! :-)\n");
  }

#ifdef LAN_DEBUG
  printf("Leaving playgame()\n");
#endif
}




//Here we read up to max_length bytes from stdin into the buffer.
//The first '\n' in the buffer, if present, is replaced with a
//null terminator.
//returns 0 if no data ready, 1 if at least one byte read.

int read_stdin_nonblock(char* buf, size_t max_length)
{
  int bytes_read = 0;
  char* term = NULL;
  buf[0] = '\0';

  bytes_read = fread (buf, 1, max_length, stdin);
  term = strchr(buf, '\n');
  if (term)
    *term = '\0';
     
  if(bytes_read > 0)
    bytes_read = 1;
  else
    bytes_read = 0;
      
  return bytes_read;
}

/* Display the current questions and the number of remaining questions: */
void print_current_quests(void)
{
  int i;
  printf("\n------------  Current Questions:  -----------\n");
  for(i = 0; i < TEST_COMETS; i ++)
  { 
    if(comets[i].question_id != -1)
      printf("Comet %d - question %d:\t%s\n", i, comets[i].question_id, comets[i].formula_string);
    else
      printf("Comet %d:\tEmpty\n", i);
  }
  printf("-----------------------------------------------\n");
}


int erase_flashcard(MC_FlashCard* fc)
{
  if(!fc)
    return 0;
  fc->formula_string[0] = '\0';
  fc->answer_string[0] = '\0';
  fc->question_id = -1;
  fc->answer = 0;
  fc->difficulty = 0;
  return 1;
}


/* Return a pointer to an empty comet slot, */
/* returning NULL if no vacancy found:      */

MC_FlashCard* find_comet_by_id(int id)
{
  int i = 0;
  for(i = 0; i < TEST_COMETS; i++)
  {
    if(comets[i].question_id == id)
      return &comets[i];
  }
  //if we don't find a match:
  return NULL;
}

/* Check the "comets" in order to see if the given */
/* value matches the answer for any of the comets: */
/* Returns a pointer to the matching comet, or     */
/* NULL if the answer doesn't match:               */

MC_FlashCard* check_answer(int ans)
{
  int i = 0;
  for(i = 0; i < TEST_COMETS; i++)
  {
    /* Make sure we don't "match" an empty question with a zero answer: */
    if( (comets[i].question_id != -1)
     && (comets[i].answer == ans))
   
      return &comets[i];
  }
  //if we don't find a matching question:
  return NULL;
}