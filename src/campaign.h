#ifndef CAMPAIGN_H
#define CAMPAIGN_H

/*
 * campaign.h - prose and function declarations for TuxMath's 'Mission mode' 
 * 
 * Author: B. Luchen
 */
 
#include "SDL_extras.h"
#include "tuxmath.h"


//#define TESTING_CAMPAIGN //allow ESC to skip missions instead of exiting

#define NUM_STAGES 5 
#define NUM_ROUNDS 3

/* NOTE this has to be static to be in a header file or it will cause */
/* multiple definition errors if included in more than one file.      */

/* NOTE: the convention has changed. Use " " for a blank line (note
   the space), and use "" (rather than NULL) for the termination
   string. This is a consequence of the linewrapping code.  TEH Feb
   2009. */
static const char* briefings[NUM_STAGES][20] = {
  //cadet
  {
    N_("-[Esc] to skip"),
    N_("Mission One: Careful Cadet"),
    "--------------------------",
    N_("I'm so glad you've come!"),
    " ",
    N_("The penguins need your help! Comets are falling from the sky, and are melting the penguins' igloos. To save their homes, we need you to find the secret code that will zap each comet."),
    " ",
    N_("Do your best!"),
    ""
  },
  //scout
  {
    "-[Esc] to skip",
    N_("Mission Two: Smart Scout"),
    "------------------------",
    N_("Great job! Since you saved the penguins' homes, we are promoting you to Scout. Scouts are good for keeping an eye out for trouble..."),
    " ",
    N_("...like what's happening right now! The TakeAways have come, and they're sending new, trickier comets against the penguins!"),
    N_("But you can save them!"),
    ""
  },
  //ranger
  {
    "-[Esc] to skip",
    N_("Mission Three: Royal Ranger"),
    "---------------------------",
    N_("You've done it again! The Penguin Emperor has chosen you to join his team of Rangers that help protect the city.  We're sending you there now..."),
    " ",
    N_("...oh no! Now the Emperor himself is under attack, from new types of comets: these problems are multiplying! To fight these, you need great skill. We think you can do it. Join the Rangers and help save the city!"),
    ""
  },
  //ace
  {
    "-[Esc] to skip",
    N_("Mission Four: Imperial Ace"),
    "--------------------------",
    N_("You did it! The Emperor wants to thank you in person. We are taking you to his ice palace for a great honor: you will become the Imperial Ace!"),
    " ",
    N_("But right in the middle of the ceremony, a new attack from the land of Division starts!"),
    N_("Now is no time for resting; the city needs your help!"),
    ""
  },
  //commando
  {
    "-[Esc] to skip",
    N_("Final Mission: Computing Commando"),
    "---------------------------------",
    N_("Penguin scientists have learned that all these attacks are coming from a secret base, and they need you to go fight the final battle. They also give you this clue: first do multiplication and division, and then do addition and subtraction."),
    N_("I hope that hint helps!"),
    " ",
    N_("This is it! You can stop these attacks forever, Commando!"),
    ""
  },
};

int start_campaign();

#endif // CAMPAIGN_H
