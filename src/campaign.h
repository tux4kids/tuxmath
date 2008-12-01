#ifndef CAMPAIGN_H
#define CAMPAIGN_H

/*
 * campaign.h - prose and function declarations for TuxMath's 'Mission mode' 
 * 
 * Author: B. Luchen
 */
 
#include "SDL_extras.h"
#include "tuxmath.h"


#define TESTING_CAMPAIGN //allow ESC to skip missions instead of exiting

#define NUM_STAGES 5 
#define NUM_ROUNDS 3


static const char* briefings[NUM_STAGES][20] = {
  //cadet
  {
    N_("-[Esc] to skip"),
    N_("Mission One: Careful Cadet"),
    "--------------------------",
    N_("I'm so glad you've come!"),
    "",
    N_("The penguins need your help! Comets"),
    N_("are falling from the sky, and are melting"),
    N_("the penguins' igloos. To save their homes,"),
    N_("we need you to find the secret code that"),
    N_(" will zap each comet."),
    "",
    N_("Do your best!"),
    NULL
  },
  //scout
  {
    "-[Esc] to skip",
    N_("Mission Two: Smart Scout"),
    "------------------------",
    N_("Great job! Since you saved the penguins' homes,"),
    N_("we are promoting you to Scout. Scouts are good"),
    N_("for keeping an eye out for trouble..."),
    "",
    N_("...like what's happening right now!"),
    N_("The TakeAways have come, and they're sending"),
    N_("new, trickier comets against the penguins!"),
    N_("But you can save them!"),
    NULL
  },
  //ranger
  {
    "-[Esc] to skip",
    N_("Mission Three: Royal Ranger"),
    "---------------------------",
    N_("You've done it again! The Penguin Emperor has"),
    N_("chosen you to join his team of Rangers that"),
    N_("help protect the city.  We're sending you"),
    N_("there now..."),
    "",
    N_("...oh no! Now the Emperor himself is under attack,"),
    N_("from new types of comets: these problems are"),
    N_("multiplying! To fight these, you need great"),
    N_("skill. We think you can do it. Join the"),
    N_("Rangers and help save the city!"),
    NULL
  },
  //ace
  {
    "-[Esc] to skip",
    N_("Mission Four: Imperial Ace"),
    "--------------------------",
    N_("You did it! The Emperor wants to thank you"),
    N_("in person. We are taking you to his ice palace"),
    N_("for a great honor: you will become"),
    N_("the Imperial Ace!"),
    "",
    N_("But right in the middle of the ceremony,"),
    N_("a new attack from the land of Division starts!"),
    N_("Now is no time for resting; the city"),
    N_("needs your help!"),
    NULL
  },
  //commando
  {
    "-[Esc] to skip",
    N_("Final Mission: Computing Commando"),
    "---------------------------------",
    N_("Penguin scientists have learned that all"),
    N_("these attacks are coming from a secret"),
    N_("base, and they need you to go fight"),
    N_("the final battle. They also give you"),
    N_("this clue: first do multiplication and"),
    N_("division, and then do addition and subtraction."),
    N_("I hope that hint helps!"),
    "",
    N_("This is it! You can stop these attacks"),
    N_("forever, Commando!"),
    NULL
  },
};

int start_campaign();

#endif // CAMPAIGN_H
