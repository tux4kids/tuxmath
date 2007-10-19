/*
*  C Implementation: exercise_mathcards
*
* Description: 
*
*
* Author: David Bruce <dbruce@tampabay.rr.com>, (C) 2007
*
* Copyright: See COPYING file that comes with this distribution
*
*/

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
