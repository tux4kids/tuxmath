/* exercise_mathcards.c

   A simple standalone program to test the creation of question lists by
   the mathcards code.

   Copyright 2009, 2010, 2011.
Author: David Bruce.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org


exercise_mathcards.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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



#include <stdio.h>
#include "mathcards.h"

int main()
{
    int i, iter, op;
    MC_FlashCard c;

    MC_Initialize();

    for (i = 0; i < 100; i++)
    {

        fprintf(stderr, "\n\nGame: i = %d\n", i); 
        op = rand() % 2;
        MC_SetAddAllowed(op);
        op = rand() % 2;
        MC_SetSubAllowed(op);
        op = rand() % 2;
        MC_SetMultAllowed(op);
        op = rand() % 2;
        MC_SetDivAllowed(op);

        if (!MC_StartGame())
            continue;

        iter = 0;

        while(!MC_MissionAccomplished())
        {
            MC_NextQuestion(&c);
            op = rand() % 2;
            if (op)
                MC_AnsweredCorrectly(&c);
            else
                MC_NotAnsweredCorrectly(&c);
            iter++;
        }
        //    MC_EndGame();
    }
    return 1;
}
