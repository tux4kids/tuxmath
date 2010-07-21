/*
 * campaign.c - handle TuxMath's 'Mission mode' 
 * 
 * Author: B. Luchen
 */
 
#include "campaign.h"
#include "tuxmath.h"
#include "credits.h"
#include "titlescreen.h"
#include "game.h"
#include "fileops.h"
#include "mathcards.h"
#include "options.h"


void briefPlayer(int stage); //show text introducing the given stage
void readStageSettings(int stage);
void readRoundSettings(int stage, int round);
void showGameOver();
void showGameWon();

char* stagenames[NUM_STAGES] = {"cadet", "scout", "ranger", "ace", "commando"};

int start_campaign()
{
  int i, j;
  int gameresult = 0, endcampaign = 0;
  char roundmessage[10];
  char endtext[2][MAX_LINEWIDTH] = {N_("Congratulations! You win!"), " "};
  printf("Entering start_campaign()\n");
  
  
  for (i = 0; i < NUM_STAGES; ++i)
  {
    printf("Stage %s\n", stagenames[i]);
    briefPlayer(i);
    for (j = 1; j <= NUM_ROUNDS; ++j)
    {
      printf("Round %d\n", j);
     
      //read in settings 
      read_named_config_file("campaign/campaign");    
      readStageSettings(i);
      readRoundSettings(i, j);
      Opts_SetKeepScore(0);
          
      snprintf(roundmessage, 10, "%s %d", N_("Round"), j);
      game_set_start_message(roundmessage, "", "", "");

      DEBUGCODE(debug_setup)
      {
        MC_PrintMathOptions(stdout, 0);
      }

      //play!
      printf("Starting game...\n");
      gameresult = game();
      
      //move on if we've won, game over if not
      if (gameresult == GAME_OVER_WON)
        ;
      else if (gameresult == GAME_OVER_LOST)
      {
        showGameOver();
        endcampaign = 1;
      }
      else if (gameresult == GAME_OVER_ERROR)
      {
        DEBUGMSG(debug_game, "Error!\n");
        endcampaign = 1;
      }
#ifndef TESTING_CAMPAIGN
      else if (gameresult == GAME_OVER_ESCAPE)
      {
        DEBUGMSG(debug_game, "hit escape\n");
        endcampaign = 1;
      }
#endif      
      else
      {
        printf("gameresult = %d\n", gameresult);
        endcampaign = 0;
      }
      
      if (endcampaign)
        return 0;
    }
      
    //if we've beaten the last stage, there is no bonus, skip to win sequence
    if (i == NUM_STAGES - 1)
    {
      showGameWon();
      break;
    }
/*    //bonus round
    readStageSettings(i);
    readRoundSettings(i, -1);
    game_set_start_message("Bonus", "", "", "");
    game();
*/
  }
  scroll_text(endtext, screen->clip_rect, 3);
  return 0;
}

void briefPlayer(int stage)
{
/* NOTE: the convention has changed. Use " " for a blank line (note
   the space), and use "" (rather than NULL) for the termination
   string. This is a consequence of the linewrapping code.  TEH Feb
   2009. */

  const char briefings[NUM_STAGES][MAX_LINES][MAX_LINEWIDTH] = 
  {
    //cadet
    {
      {N_("-[Esc] to skip")},
      {N_("Mission One: Careful Cadet")},
      {"--------------------------"},
      {N_("I'm so glad you've come!")},
      {" "},
      {N_("The penguins need your help! Comets are falling from the sky, and are melting the penguins' igloos. To save their homes, we need you to find the secret code that will zap each comet.")},
      {" "},
      {N_("Do your best!")},
      {""}
    },
    //scout
    {
      {N_("-[Esc] to skip")},
      {N_("Mission Two: Smart Scout")},
      {"------------------------"},
      {N_("Great job! Since you saved the penguins' homes, we are promoting you to Scout. Scouts are good for keeping an eye out for trouble...")},
      {" "},
      {N_("...like what's happening right now! The TakeAways have come, and they're sending new, trickier comets against the penguins!")},
      {N_("But you can save them!")},
      {""}
    },
    //ranger
    {
      {"-[Esc] to skip"},
      {N_("Mission Three: Royal Ranger")},
      {"---------------------------"},
      {N_("You've done it again! The Penguin Emperor has chosen you to join his team of Rangers that help protect the city.  We're sending you there now...")},
      {" "},
      {N_("...oh no! Now the Emperor himself is under attack, from new types of comets: these problems are multiplying! To fight these, you need great skill. We think you can do it. Join the Rangers and help save the city!")},
      {""}
    },
    //ace
    {
      {N_("-[Esc] to skip")},
      {N_("Mission Four: Imperial Ace")},
      {"--------------------------"},
      {N_("You did it! The Emperor wants to thank you in person. We are taking you to his ice palace for a great honor: you will become the Imperial Ace!")},
      {" "},
      {N_("But right in the middle of the ceremony, a new attack from the land of Division starts!")},
      {N_("Now is no time for resting; the city needs your help!")},
      {""}
    },
    //commando
    {
      {N_("-[Esc] to skip")},
      {N_("Final Mission: Computing Commando")},
      {"---------------------------------"},
      {N_("Penguin scientists have learned that all these attacks are coming from a secret base, and they need you to go fight the final battle. They also give you this clue: first do multiplication and division, and then do addition and subtraction.")},
      {N_("I hope that hint helps!")},
      {" "},
      {N_("This is it! You can stop these attacks forever, Commando!")},
      {""}
    },
  };


  char* sprites[] = {
    "sprites/tux_helmet_yellow.svg",
    "sprites/tux_helmet_green.svg",
    "sprites/tux_helmet_blue.svg",
    "sprites/tux_helmet_red.svg",
    "sprites/tux_helmet_black.svg"
  };

  SDL_Surface* icon = NULL;
  SDL_Rect textarea = screen->clip_rect;
  SDL_Surface* loadedsprite = LoadScaledImage(
    sprites[stage], IMG_REGULAR|IMG_NOT_REQUIRED, 
    screen->h / 4, screen->h / 4
  );



  if (loadedsprite) //if using an image, make sure the text doesn't hit it
  {
    icon = loadedsprite;
    textarea.x = icon->w;
    textarea.y = icon->h;
    textarea.w = screen->w - icon->w;
    textarea.h = screen->h - icon->h;
  }

  //background is dark blue with a black text area
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 32));
  SDL_FillRect(screen, &textarea, 0);
  
  //show this stage's text
  DEBUGMSG(debug_game, "Briefing\n");

  SDL_BlitSurface(icon, NULL, screen, NULL);

  linewrap_list(briefings[stage], wrapped_lines, 40, MAX_LINES, MAX_LINEWIDTH);
  scroll_text(wrapped_lines, textarea, 1);

  DEBUGMSG(debug_game, "Finished briefing\n");

  SDL_FreeSurface(loadedsprite);
  if (icon != loadedsprite)
    SDL_FreeSurface(icon);
}

void readStageSettings(int stage)
{
  char fn[PATH_MAX];
  snprintf(fn,PATH_MAX, "campaign/%s/%s", stagenames[stage], stagenames[stage]);
  read_named_config_file(fn);
}

void readRoundSettings(int stage, int round)
{
  char fn[PATH_MAX];
  if (round == -1)
    snprintf(fn, PATH_MAX, "campaign/%s/bonus", stagenames[stage]);
  else
    snprintf(fn,PATH_MAX, "campaign/%s/round%d", stagenames[stage], round);
  read_named_config_file(fn);
}

void showGameOver()
{
  const char text[2][MAX_LINEWIDTH] = {N_("Sorry, try again!"), ""};
  linewrap_list(text, wrapped_lines, 40, MAX_LINES, MAX_LINEWIDTH);
  scroll_text(wrapped_lines, screen->clip_rect, 3);
}

void showGameWon()
{
  const char text[2][MAX_LINEWIDTH] = {N_("Mission accomplished. The galaxy is safe!"), ""};
  linewrap_list(text, wrapped_lines, 40, MAX_LINES, MAX_LINEWIDTH);
  scroll_text(wrapped_lines, screen->clip_rect, 3);
}
