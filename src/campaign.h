/* campaign.h - handle TuxMath's 'Mission mode'

   Copyright (C) 2008, 2009, 2010, 2011.
Authors: Brendan Luchen, David Bruce.
email: <tuxmath-devel@lists.sourceforge.net>

campaign.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.  */


#ifndef CAMPAIGN_H
#define CAMPAIGN_H


#include "tuxmath.h"

//#define TESTING_CAMPAIGN //allow ESC to skip missions instead of exiting

#define NUM_STAGES 5 
#define NUM_ROUNDS 3

/* NOTE - moved 'briefings' into campaign.c' as data local to briefPlayer() */
/* because that was the only place it was used - just occupying memory.     */

int start_campaign();

#endif // CAMPAIGN_H
