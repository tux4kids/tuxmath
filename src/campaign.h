#ifndef CAMPAIGN_H
#define CAMPAIGN_H

/*
 * campaign.h - prose and function declarations for TuxMath's 'Mission mode' 
 * 
 * Author: B. Luchen
 */
 
#include "SDL_extras.h"
#include "tuxmath.h"
#include "linewrap.h"


//#define TESTING_CAMPAIGN //allow ESC to skip missions instead of exiting

#define NUM_STAGES 5 
#define NUM_ROUNDS 3

/* NOTE - moved 'briefings' into campaign.c' as data local to briefPlayer() */
/* because that was the only place it was used - just occupying memory.     */

int start_campaign();

#endif // CAMPAIGN_H
