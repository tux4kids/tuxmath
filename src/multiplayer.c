/*

multiplayer.h - Provides routines for organizing and running a turn-based
                multiplayer that can accommodate up to four players (more with
                a recompilation)

Author: B. Luchen

*/

#include "SDL.h"
#include "multiplayer.h"
#include "game.h"
#include "options.h"
#include "fileops.h"
#include "highscore.h"
#include "credits.h"

int params[NUM_PARAMS];

int inprogress = 0;
int pscores[MAX_PLAYERS];
char* pnames[MAX_PLAYERS];

//local function decs
static void playerWon(int player); //show a sequence recognizing this player as winner
static int initMP();
static void cleanupMP();

void mp_set_parameter(unsigned int param, int value)
{
  if (inprogress)
  {
    tmdprintf("Oops, set param %d in the middle of a game\n", param);
    return;
  }
  params[param] = value;
}

void mp_run_multiplayer()
{
  int round = 1;
  int currentplayer = 0;
  int result = 0;
  int done = 0;
  int activeplayers = params[PLAYERS];
  
  if (initMP() )
  {
    printf("Initialization failed, bailing out\n");
    return;
  }
  
  read_global_config_file();
  
  if (params[MODE] == ELIMINATION)
  {
    while(!done)
    {
              
      game_set_start_message(pnames[currentplayer], "Go!", "", "");
      result = game();
      
      if (result == GAME_OVER_LOST || result == GAME_OVER_ESCAPE) 
      {
        //eliminate player
        pnames[currentplayer] = NULL;
        --activeplayers;
      }
      
      while (pnames[++currentplayer] == NULL) //skip over eliminated players
      {
        currentplayer %= params[PLAYERS];
        if (currentplayer == 0)
          ++round;
      }
      if (activeplayers <= 1) //last man standing!
      {
        playerWon(currentplayer);
      }
    }
  }
  else if (params[MODE] == SCORE_SWEEP)
  {
    int hiscore = 0;
    int winner = -1;
    for (round = 1; round < params[ROUNDS]; ++round)
    {
      for (currentplayer = 0; currentplayer < params[PLAYERS]; ++currentplayer)
      {
        game_set_start_message(pnames[currentplayer], "Go!", NULL, NULL);
        result = game();
        pscores[currentplayer] += Opts_LastScore(); //add this player's score
        if (result == GAME_OVER_WON)
          pscores[currentplayer] += 500; //plus a possible bonus
      }
    }
    for (currentplayer = 0; currentplayer < params[PLAYERS]; ++currentplayer)
    {
      if (pscores[currentplayer] > hiscore)
      {
        hiscore = pscores[currentplayer];
        winner = currentplayer;
      }
    }
    playerWon(winner);
  }
}


void playerWon(int player)
{
  char* text[2] = {
    "-------------------------------- wins!",
    NULL
  };
  snprintf(text[0], strlen(text[0])-1, "%s wins!", pnames[player]);
  scroll_text(text, screen->clip_rect, 4 );
}

int initMP()
{
  int i;
  int nplayers = params[PLAYERS];
  
  pscores[0] = pscores[1] = pscores[2] = pscores[3] = 0;
  pnames[0] = pnames[1] = pnames[2] = pnames[3] = NULL;
  
  //allocate and enter player names
  for (i = 0; i < nplayers; ++i)
    pnames[i] = malloc(HIGH_SCORE_NAME_LENGTH * sizeof(char) );
  for (i = 0; i < nplayers; ++i)
    if (pnames[i])
      NameEntry(pnames[i], "Who is playing?", "Enter your name:");
    else
      {
        printf("Can't allocate name %d!\n", i);
        return 1;
      }
      
  inprogress = 1; //now we can start the game
  return 0;
}

void cleanupMP()
{
  int i;
  
  for (i = 0; i < params[PLAYERS]; ++i)
    if (pnames[i])
      free(pnames[i]);
  inprogress = 0;
}
