/*

   factoroids.h: Contains headers for the factor and
   fraction activities.

   Copyright 2008, 2010, 2011
Author: Jesus M. Mager H. (fongog@gmail.com)
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

factoroids.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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




#ifndef FACTOROIDS_H
#define FACTOROIDS_H

#include <stdbool.h>

#include "fileops.h"


typedef struct asteroid_type {
    int alive, size;
    int angle, angle_speed;
    int xspeed, yspeed;
    int x, y;
    int rx, ry;
    int centerx, centery;
    int radius;
    int fact_number;
    int isprime;
    int a, b; /*  a / b */
    int count;
    int xdead, ydead, isdead, countdead;
} asteroid_type;


typedef struct tuxship_type {
    int lives, size;
    int xspeed, yspeed;
    int x, y;
    int rx, ry;
    int x1,y1,x2,y2,x3,y3;
    int radius;
    int centerx, centery;
    int angle;
    int hurt, hurt_count;
    int count;
    bool thrust;
} tuxship_type;


typedef struct FF_laser_type{
    int alive;
    int x, y;
    int destx,desty;
    int r, g, b;
    float count;
    int angle;
    int m;
    int n;
} FF_laser_type;


/********* Enums ******************/

typedef enum _TuxBonus {
    TB_CLOAKING, TB_FORCEFIELD, TB_POWERBOMB, TB_SIZE
} TuxBonus;

enum {
    FACTOROIDS_GAME,
    FRACTIONS_GAME
};


// Used in titleecreen.c

void factors(void);
void fractions(void);

void wait_for_input(void);

#endif
