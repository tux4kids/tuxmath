#ifndef CAMPAIGN_H
#define CAMPAIGN_H

/*
 * campaign.h - prose and function declarations for TuxMath's 'Mission mode' 
 * 
 * Author: B. Luchen
 */
 
#include "SDL_extras.h"


#define TESTING_CAMPAIGN //allow ESC to skip missions instead of exiting

#define NUM_STAGES 5 
#define NUM_ROUNDS 3


static char* briefings[NUM_STAGES][100] = {
  //cadet
  {
    "-[Esc] to skip",
    "Mission One: Careful Cadet",
    "--------------------------",
    "I'm so glad you've come!",
    "",
    "The penguins need your help! Comets",
    "are falling from the sky, and are melting",
    "the penguins' igloos. To save their homes,",
    "we need you to find the secret code that",
    " will zap each comet.",
    "",
    "Do your best!",
    NULL
  },
  //scout
  {
    "-[Esc] to skip",
    "Mission Two: Smart Scout",
    "------------------------",
    "Great job! Since you saved the penguins' homes,",
    "we are promoting you to Scout. Scouts are good",
    "for keeping an eye out for trouble...",
    "",
    "...like what's happening right now!",
    "The TakeAways have come, and they're sending",
    "new, trickier comets against the penguins!",
    "But you can save them!",
    NULL
  },
  //ranger
  {
    "-[Esc] to skip",
    "Mission Three: Royal Ranger",
    "---------------------------",
    "You've done it again! The Penguin Emperor has",
    "chosen you to join his team of Rangers that",
    "help protect the city.  We're sending you",
    "there now...",
    "",
    "...oh no! Now the Emperor himself is under attack,",
    "from new types of comets: these problems are",
    "multiplying! To fight these, you need great",
    "skill. We think you can do it. Join the",
    "Rangers and help save the city!",
    NULL
  },
  //ace
  {
    "-[Esc] to skip",
    "Mission Four: Imperial Ace",
    "--------------------------",
    "You did it! The Emperor wants to thank you",
    "in person. We are taking you to his ice palace",
    "for a great honor: you will become",
    "the Imperial Ace!",
    "",
    "But right in the middle of the ceremony,",
    "a new attack from the land of Division starts!",
    "Now is no time for resting; the city",
    "needs your help!",
    NULL
  },
  //commando
  {
    "-[Esc] to skip",
    "Final Mission: Computing Commando",
    "---------------------------------",
    "Penguin scientists have learned that all",
    "these attacks are coming from a secret",
    "base, and they need you to go fight",
    "the final battle. They also give you",
    "this clue: first do multiplication and",
    "division, and then do addition and subtraction.",
    "I hope that hint helps!",
    "",
    "This is it! You can stop these attacks",
    "forever, Commando!",
    NULL
  },
};

int start_campaign();

#endif // CAMPAIGN_H
