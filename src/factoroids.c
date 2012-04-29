/* factoroids.c

   The main game loop for a factoring game resembling the
   arcade classic "Asteroids".

   Some code adapted from the GPL-licensed game "vectoroids"
   by Bill Kendrick (http://www.newbreedsoftware.com).

   Copyright 2001, 2002, 2008, 2009, 2010, 2011.
Authors: Bill Kendrick, Jesus M. Mager H., Tim Holy, David Bruce
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org


factoroids.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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



#include "tuxmath.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"
#ifndef NOSOUND
#include "SDL_mixer.h"
#endif
#include "SDL_image.h"
#include "SDL_rotozoom.h"

#include "factoroids.h"
#include "factoroids_graphics.h"
#include "credits.h"
#include "game.h"
#include "fileops.h"
#include "setup.h"
#include "mathcards.h"
#include "titlescreen.h"
#include "options.h"
#include "frame_counter.h"
#include "draw_utils.h"

#define BASE_RES_X 1280

#define MAX_LASER 5
#define MAX_ASTEROIDS 50
#define NUM_TUXSHIPS 2
#define NUM_SPRITES 11
#define TUXSHIP_LIVES 3
#define DEG_PER_ROTATION 2
#define NUM_OF_ROTO_IMGS 360/DEG_PER_ROTATION
/* TUXSHIP_DECEL controls "friction" - 1 means ship glides infinitely, 0 stops it instantly */
#define TUXSHIP_DECEL 0.95
#define DEG_TO_RAD 0.0174532925
#define MAX(a,b)           (((a) > (b)) ? (a) : (b))
#define ROUND(X)           ((X) > 0 ? (int)((X)+0.5f) : (int)((X)-0.5f))
//the prime set keeps increasing till its size reaches this value
#define PRIME_MAX_LIMIT 6

#define CTRL_NEXT SDLK_f
#define CTRL_PREV SDLK_d


//a value (float) indicating the sensitivity of the mouse
//0 = disable mouse; 0.1 high ... 1 low, 2 lower, so on
//FIXME - this is a very quick/dirty response to my observation
//that the mouse sensitivity is too high in my initial tests
//of the win32 build. We ought to figure out if we can get
//some info from the OS under SDL to set this better, and
//also provide a way for users to adjust this setting.
#if defined BUILD_MINGW32 || defined __APPLE__
#define MOUSE_SENSITIVITY 2
#else
#define MOUSE_SENSITIVITY 0.5
#endif

//a special value indicating that a bonus hasn't been used yet
#define BONUS_NOTUSED -1

/********* Enumerations ***********/

enum FF_STATUS
{
    FF_OVER_SDL_QUIT,
    FF_OVER_ESCAPE,
    FF_OVER_LOST,
    FF_OVER_WON,
    FF_OVER_ERROR,
    FF_OVER_OTHER,
    FF_IN_PROGRESS
};

/********* Structures *********/

typedef struct colorRGBA_type {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
} ColorRGBA_type;


/********* Global vars ************/

/* Trig junk:  (thanks to Atari BASIC for this) */

static int trig[12] = {
    1024,
    1014,
    984,
    935,
    868,
    784,
    685,
    572,
    448,
    316,
    117,
    0
};

static int bonus = -1;
static int bonus_time = BONUS_NOTUSED;

static const int prime_numbers[] = {2, 3, 5, 7, 11, 13};
static const int prime_power_limit[] = {7, 4, 3, 3, 2, 2}; //custom calibrated power limits for extra "goodness"

static const int prime_next[] = {2, 2, 3, 5, 5, 7, 7, 11, 11, 11, 11, 13, 13, 2};
static const int prime_prev[] = {13, 13, 13, 2, 3, 3, 3, 5, 5, 7, 7, 7, 7, 11, 11};

static char* game_music_filenames[NUM_MUSICS] = {
    "01_rush.ogg",
    "02_on_the_edge_of_the_universe.ogg",
    "03_gravity.ogg",
    "game.mod",
    "game2.mod",
    "game3.mod",
};

// ControlKeys
static int mouseroto;
static int left_pressed;
static int right_pressed;
static int up_pressed;
static int shift_pressed;
static int shoot_pressed;
static int button_pressed;

// GameControl
static int game_status;
//static int gameover_counter;
static int escape_received;

// Game type
int FF_game;

// Game vars
static int score;
static int wave;
static int paused;
static int escape_received;
static int game_status;
static int SDL_quit_received;
static int quit;
static int digits[3];
static int num;
static int mouse_reset;

static int neg_answer_picked;
static int tux_pressing;
static int doing_answer;
static int tux_img;
//static int FF_level;

static asteroid_type* asteroid = NULL;
static tuxship_type tuxship;
static FF_laser_type laser[MAX_LASER];

static int NUM_ASTEROIDS;
static int roto_speed;

/*************** The Factor and Fraction Activity Game Functions ***************/

/* Local function prototypes: */

static int FF_init(void);

static void FF_handle_ship(void);
static void FF_handle_asteroids(void);
static void FF_handle_answer(void);
static int check_exit_conditions(void);
static int cockpit_tux_image(int prime);

static void FF_add_level(void);
static int FF_over(int game_status);
static void FF_exit_free(void);

static int FF_add_laser(void);
static int FF_add_asteroid(int x, int y, int xspeed, int yspeed, int size, int angle, int angle_speed, int fact_num, int a, int b, int new_wave);
static int FF_destroy_asteroid(int i, float xspeed, float yspeed);

static int AsteroidColl(int astW,int astH,int astX,int astY,
        int x, int y);
static int is_prime(int num);
static int fast_cos(int angle);
static int fast_sin(int angle);
static int generatenumber(int wave);
static int validate_number(int num, int wave);
static void game_handle_user_events(void);
static int game_mouse_event(SDL_Event event);
static int game_mouseroto(SDL_Event event) {return event.motion.xrel;}
static void _tb_PowerBomb(int n);

/************** factors(): The factor main function ********************/
void factors(void)
{
    static int previous_fps = -1;
    quit = 0;

    DEBUGMSG(debug_factoroids, "Entering factors():\n");

    //Workaround: there are some problems on higher frame rates
    if(Opts_FPSLimit() > 30 || Opts_FPSLimit() == 0)
    {
        previous_fps = Opts_FPSLimit();
        Opts_SetFPSLimit(30);
    }

    FF_game = FACTOROIDS_GAME;

    if (!FF_init())
    {
        fprintf(stderr, "FF_init() failed!\n");
        FF_exit_free();
        return;
    }

    factoroids_draw(asteroid, &tuxship, laser, bonus, bonus_time, digits, wave, score, num, tux_img, button_pressed);
    factoroids_level_message(wave);
    /* Reset frame counter */
    FC_init();
    /* Escape key disable in overview screen */
    escape_received = 0;

    while (game_status == FF_IN_PROGRESS)
    {
        FC_frame_begin();

        game_handle_user_events();
        if(SDL_GetTicks() > bonus_time && bonus_time != -1) {bonus_time = 0;}

        FF_handle_ship();
        FF_handle_asteroids();
        FF_handle_answer();

        tux_img = cockpit_tux_image(num);

        factoroids_draw(asteroid, &tuxship, laser, bonus, bonus_time, digits, wave, score, num, tux_img, button_pressed);
        SDL_Flip(screen);

        game_status = check_exit_conditions();

        if (paused)
        {
            pause_game();
            paused = 0;
        }


#ifndef NOSOUND
        if (Opts_UsingSound())
        {
            //...when the music's over, turn out the lights!
            //...oops, wrong song! Actually, we just pick next music at random:
            if (!Mix_PlayingMusic())
            {
                T4K_AudioMusicLoad(game_music_filenames[(rand() % NUM_MUSICS)], T4K_AUDIO_PLAY_ONCE);
            }
        }
#endif

        FC_frame_end();
    }
    FF_over(game_status);

    if(previous_fps != -1)
    {
        Opts_SetFPSLimit(previous_fps);
    }
}


/************** fractions(): The fractions main function ********************/
void fractions(void)
{
    quit = 0;
    tux_img = IMG_TUX_CONSOLE1;

    DEBUGMSG(debug_factoroids, "Entering factors():\n");
    /*****Initalizing the Factor activiy *****/
    FF_game = FRACTIONS_GAME;

    if (!FF_init())
    {
        fprintf(stderr, "FF_init() failed!\n");
        FF_exit_free();
        return;
    }

    /************ Main Loop **************/
    while (game_status == FF_IN_PROGRESS)
    {
        static float timer = 0;

        FC_frame_begin();

        if((timer+=FC_time_elapsed) >= 1)
        {
            if(tux_img < IMG_TUX_CONSOLE4)
                tux_img++;
            else
                tux_img = IMG_TUX_CONSOLE1;
        }

        game_handle_user_events();

        FF_handle_ship();
        FF_handle_asteroids();
        FF_handle_answer();
        factoroids_draw(asteroid, &tuxship, laser, bonus, bonus_time, digits, wave, score, num, tux_img, button_pressed);
        SDL_Flip(screen);

        game_status = check_exit_conditions();

        if (paused)
        {
            pause_game();
            paused = 0;
        }


#ifndef NOSOUND
        if (Opts_UsingSound())
        {
            if (!Mix_PlayingMusic())
            {
                T4K_AudioMusicLoad(game_music_filenames[(rand() % 3)], T4K_AUDIO_PLAY_ONCE);
            }
        }
#endif

        FC_frame_end();
    }
    FF_over(game_status);
}


/************ Initialize all vars... ****************/
static int FF_init(void)
{
    int i;
    mouse_reset = 0;

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_Flip(screen);
    SDL_ShowCursor(0);

    /* Settings to let us track mouse movement even beyond edge of screen
     * for control of ship rotation.  Note that SDL reportedly supports
     * this only on "Windows and Unix-alikes", i.e. maybe not OS-X
     */
    SDL_WM_GrabInput(SDL_GRAB_ON);

    //Initialize frame counter
    FC_init();

    if(!factoroids_init_graphics())
        return 0;

    factoroids_intro();

    // Allocate memory
    asteroid = NULL;  // set in case allocation fails partway through
    asteroid = (asteroid_type *) malloc(MAX_ASTEROIDS * sizeof(asteroid_type));

    if (asteroid == NULL)
    {
        fprintf(stderr, "Allocation of asteroids failed");
        return 0;
    }

    memset(asteroid, 0, MAX_ASTEROIDS * sizeof(asteroid_type));

    NUM_ASTEROIDS = 4;

    /**************Setting up the ship values! **************/
    tuxship.x = ((screen->w)/2) - 20;
    tuxship.y = ((screen->h)/2) - 20;
    tuxship.lives = TUXSHIP_LIVES;
    tuxship.hurt = 0;
    tuxship.hurt_count = 0;
    tuxship.angle = 90;
    tuxship.xspeed = 0;
    tuxship.yspeed = 0;
    tuxship.radius = (images[IMG_SHIP01]->h)/2;
    tuxship.thrust = 0;

    tuxship.x1 = images[IMG_SHIP01]->w-(images[IMG_SHIP01]->w/8);
    tuxship.y1 = images[IMG_SHIP01]->h/2;
    tuxship.x2 = images[IMG_SHIP01]->w/8;
    tuxship.y2 = images[IMG_SHIP01]->h/8;
    tuxship.x3 = images[IMG_SHIP01]->w/8;
    tuxship.y3 = images[IMG_SHIP01]->h-(images[IMG_SHIP01]->h/8);

    /*  --- reset all controls:  ---  */
    left_pressed = 0;
    right_pressed = 0;
    up_pressed = 0;
    shift_pressed = 0;
    shoot_pressed = 0;
    button_pressed = 0;
    SDL_quit_received = 0; 
    score = 0;
    wave = 0;
    escape_received = 0;
    game_status = FF_IN_PROGRESS;

    FF_add_level();

    for (i = 0; i < MAX_LASER; i++)
        laser[i].alive = 0;

    wait_for_input();

    return 1;
}


static void FF_handle_ship(void)
{
    /****************** Ship center... ******************/

    tuxship.centerx = tuxship.x;
    tuxship.centery = tuxship.y;

    /******************* Ship live *********************/

    if(tuxship.hurt)
    {
        static float timer = 0;

        if((timer+=FC_time_elapsed) >= 0.13f)
        {
            tuxship.hurt_count--;
            if(tuxship.hurt_count <= 0)
                tuxship.hurt = 0;
        }
    }
    /****************** Rotate Ship *********************/

    if(right_pressed || left_pressed)
    {
        if(roto_speed < 10)
        {
            roto_speed = roto_speed + 2;
        }
    }
    else
    {
        roto_speed = 1;
    }

    if (right_pressed)
    {
        tuxship.angle -= DEG_PER_ROTATION * roto_speed * 15 * FC_time_elapsed;
        if (tuxship.angle < 0)
            tuxship.angle = tuxship.angle + 360;

        tuxship.x1= fast_cos(DEG_PER_ROTATION*-roto_speed) * tuxship.centerx
            -fast_sin(DEG_PER_ROTATION*-roto_speed) * tuxship.centery;
        tuxship.y1= fast_sin(DEG_PER_ROTATION*-roto_speed) * tuxship.centerx
            +fast_cos(DEG_PER_ROTATION*-roto_speed) * tuxship.centery;

    }
    else if (left_pressed)
    {
        tuxship.angle += DEG_PER_ROTATION * roto_speed * 15 * FC_time_elapsed;
        if (tuxship.angle >= 360)
            tuxship.angle = tuxship.angle - 360;

        tuxship.x1= fast_cos(DEG_PER_ROTATION*roto_speed) * tuxship.centerx
            -fast_sin(DEG_PER_ROTATION*roto_speed) * tuxship.centery;
        tuxship.y1= fast_sin(DEG_PER_ROTATION*roto_speed * tuxship.centerx
                +fast_cos(DEG_PER_ROTATION*roto_speed)) * tuxship.centery;

    }

    /**************** Mouse Rotation ************************/
    tuxship.angle = (tuxship.angle + DEG_PER_ROTATION * -mouseroto) % 360;
    tuxship.angle += tuxship.angle < 0 ? 360 : 0;

    tuxship.x1= fast_cos(DEG_PER_ROTATION*roto_speed) * tuxship.centerx
        -fast_sin(DEG_PER_ROTATION*roto_speed) * tuxship.centery;
    tuxship.y1= fast_sin(DEG_PER_ROTATION*roto_speed * tuxship.centerx
            +fast_cos(DEG_PER_ROTATION*roto_speed)) * tuxship.centery;

    /**************** Move, and increse speed ***************/


    if (up_pressed && (tuxship.lives > 0))
    {
        tuxship.xspeed += ((fast_cos(tuxship.angle >> 3) * 3) >> 10)*5;
        tuxship.yspeed -= ((fast_sin(tuxship.angle >> 3) * 3) >> 10)*5;

        //Google Code-In 2010 Task: Add sound for ship's thrust
        //Sound taken from http://www.freesound.org 20/12/2010
        playsound(SND_ENGINE);
        tuxship.thrust = 1;
    }
    else
    {
        static float timer = 0;

        if ((timer+=FC_time_elapsed) >= 0.13)
        {
            tuxship.xspeed = tuxship.xspeed * TUXSHIP_DECEL;
            tuxship.yspeed = tuxship.yspeed * TUXSHIP_DECEL;
            tuxship.thrust = 0;
            timer = 0;
        }
    }

    tuxship.x += tuxship.xspeed * FC_time_elapsed;
    tuxship.y += tuxship.yspeed * FC_time_elapsed;

    /*************** Wrap ship around edges of screen ****************/

    if(tuxship.x >= (screen->w))
        tuxship.x = tuxship.x - (screen->w);
    else if (tuxship.x < -60)
        tuxship.x = tuxship.x + (screen->w);

    if(tuxship.y >= (screen->h))
        tuxship.y = tuxship.y - (screen->h);
    else if (tuxship.y < -60)
        tuxship.y = tuxship.y + (screen->h);

    /**************** Shoot ***************/
    if(shoot_pressed)
    {
        FF_add_laser();
        shoot_pressed=0;
    }
}


static void FF_handle_asteroids(void){

    SDL_Surface* surf;
    int i, found=0;
    for (i = 0; i < MAX_ASTEROIDS; i++){
        if (asteroid[i].alive)
        {

            found=1;

            /*************** Rotate asteroid ****************/

            asteroid[i].angle += ROUND(asteroid[i].angle_speed*FC_time_elapsed);

            // Wrap rotation angle...

            if (asteroid[i].angle < 0)
                asteroid[i].angle = asteroid[i].angle + 360;
            else if (asteroid[i].angle >= 360)
                asteroid[i].angle = asteroid[i].angle - 360;

            /**************Move the astroids ****************/
            surf=get_asteroid_image(asteroid[i].size,asteroid[i].angle);

            asteroid[i].rx += ROUND(asteroid[i].xspeed*FC_time_elapsed);
            asteroid[i].ry += ROUND(asteroid[i].yspeed*FC_time_elapsed);

            asteroid[i].x  = (asteroid[i].rx - (surf->w/2));
            asteroid[i].y  = (asteroid[i].ry - (surf->h/2));

            // Wrap asteroid around edges of screen:

            if (asteroid[i].x >= (screen->w))
                asteroid[i].rx = asteroid[i].rx - (screen->w);
            else if (asteroid[i].x < 0)
                asteroid[i].rx = asteroid[i].rx + (screen->w);

            if (asteroid[i].y >= (screen->h))
                asteroid[i].ry = asteroid[i].ry - (screen->h);
            else if (asteroid[i].ry < 0)
                asteroid[i].ry = asteroid[i].ry + (screen->h);
            /**************Center Asteroids**************/

            asteroid[i].centerx=((surf->w)/2)+(asteroid[i].x-5);
            asteroid[i].centery=((surf->h)/2)+(asteroid[i].y-5);

            /*************** Collisions! ****************/

            if(AsteroidColl(surf->w, surf->h, asteroid[i].x, asteroid[i].y, tuxship.centerx, tuxship.centery) &&
                    !(bonus == TB_CLOAKING && bonus_time > 0))
            {
                if(!tuxship.hurt)
                {
                    asteroid[i].xdead=asteroid[i].centerx;
                    asteroid[i].ydead=asteroid[i].centery;

                    if(!(bonus == TB_FORCEFIELD && bonus_time > 0)) {
                        tuxship.lives--;
                        tuxship.hurt=1;
                        tuxship.hurt_count=50;
                    }
                    FF_destroy_asteroid(i, tuxship.xspeed, tuxship.yspeed);
                    playsound(SND_EXPLOSION);

                }
            }
        }
    }
    if(!found)
        FF_add_level();
}

static void FF_handle_answer(void)
{

    num = (digits[0] * 100 +
            digits[1] * 10 +
            digits[2]);
    /* negative answer support DSB */
    if (neg_answer_picked)
    {
        num = -num;
    }

    if (!doing_answer)
    {
        return;
    }

    doing_answer = 0;

    neg_answer_picked = 0;

}


/*Tree rectangle vs a point collitions
  returns 1 if the collitions is detected
  and 0 if not*/

int AsteroidColl(int astW,int astH,int astX,int astY,
        int x, int y)
{
    int astWq=astW/8;
    int astHq=astH/8;
    int x1, y1, x2, y2;

    x1=astX+astWq*3;
    y1=astY;

    x2=astX+astWq*6;
    y2=astY+astH;

    if(x>x1 && x<x2 && y>y1 && y<y2)
        return 1;

    x1=astX;
    y1=astY+astHq*3;

    x2=astW;
    y2=astY+astHq*6;

    if(x>x1 && x<x2 && y>y1 && y<y2)
        return 1;

    x1=astX+astWq;
    y1=astY+astHq;

    x2=astX+astWq*7;
    y2=astY+astHq*7;

    if(x>x1 && x<x2 && y>y1 && y<y2)
        return 1;

    return 0;
}

// Returns x % w but in the range [-w/2, w/2]
static int modwrap(int x,int w)
{
    x = x % w;
    if (x > (w/2))
        x -= w;
    else if (x < -(w/2))
        x += w;
    return x;
}

static void FF_add_level(void)
{
    int i = 0;
    int x, y, xvel, yvel, dx, dy;
    int ok;
    int width;
    int safety_radius2, speed2;
    int max_speed;
    SDL_Rect rect;

    wave++;

    // New lives per wave!
    if (wave%5==0)
    {
        tuxship.lives++;
    }

    // Clear all bonuses obtained in a wave
    bonus_time = BONUS_NOTUSED; // Reset the timer for the bonus
    // And now reward a new bonus
    bonus = rand()%TB_SIZE;

    // Set active number to newly added prime
    int wave_i = wave - 1;
    if(wave>=PRIME_MAX_LIMIT) {
        wave_i = PRIME_MAX_LIMIT - 1;
    }
    int c_prime = prime_numbers[wave_i];
    digits[2] = c_prime % 10;
    digits[1] = c_prime / 10;

    //Limit the new asteroids
    if(NUM_ASTEROIDS<MAX_ASTEROIDS)
        NUM_ASTEROIDS=NUM_ASTEROIDS+wave;
    else
        NUM_ASTEROIDS=MAX_ASTEROIDS;

    width = screen->w;
    if (screen->h < width)
        width = screen->h;

    // Define the "safety radius" as one third of the screen width
    safety_radius2 = width/3;
    safety_radius2 = safety_radius2*safety_radius2; // the square distance

    // Define the max speed in terms of the screen width
    max_speed = width/6;
    if (max_speed == 0)
        max_speed = 1;

    for (i=0; i<MAX_ASTEROIDS; i++)
        asteroid[i].alive=0;
    for (i=0; i<NUM_ASTEROIDS && NUM_ASTEROIDS<MAX_ASTEROIDS; i++){
        // Generate the new position, avoiding the location of the ship
        ok = 0;
        while (!ok) {
            x = rand()%(screen->w);
            y = rand()%(screen->h);
            dx = modwrap(x - tuxship.x,screen->w);
            dy = modwrap(y - tuxship.y,screen->h);
            if (dx*dx + dy*dy > safety_radius2)
                ok = 1;
        }
        // Generate the new speed, making none of them stationary but none
        // of them too fast
        ok = 0;
        while (!ok) {
            xvel = rand()%(2*max_speed+1) - max_speed;
            yvel = rand()%(2*max_speed+1) - max_speed;
            speed2 = xvel*xvel + yvel*yvel;
            if (speed2 != 0 && speed2 < max_speed*max_speed)
                ok = 1;
        }
        if(FF_game == FACTOROIDS_GAME){
            FF_add_asteroid(x,y,
                    xvel,yvel,
                    rand()%2,
                    rand()%360, rand()%150-75,
                    generatenumber(wave),
                    0, 0,
                    1);
        }
        else if(FF_game==FRACTIONS_GAME){
            FF_add_asteroid(x,y,
                    xvel,yvel,
                    rand()%2,
                    rand()%360, rand()%150-75,
                    0,
                    (rand()%(31+(wave*2))), (rand()%(80+(wave*wave))),
                    1);
        }
    }

    if(wave != 1)
    {
        SDL_Event event;
        float timer = 0;

        while((timer+=FC_time_elapsed) < 1)
        {
            FC_frame_begin();

            rect.x=(screen->w/2)-(images[IMG_GOOD]->w/2);
            rect.y=(screen->h/2)-(images[IMG_GOOD]->h/2);
            factoroids_draw(asteroid, &tuxship, laser, bonus, bonus_time, digits, wave, score, num, tux_img, button_pressed);
            SDL_BlitSurface(images[IMG_GOOD],NULL,screen,&rect);
            SDL_Flip(screen);

            FC_frame_end();
        }

        //Empty the message queue
        while(SDL_PollEvent(&event));

        factoroids_draw(asteroid, &tuxship, laser, bonus, bonus_time, digits, wave, score, num, tux_img, button_pressed);
        factoroids_level_message(wave);
        /* Reset frame counter */
        FC_init();
        /* Escape key disable in overview screen */
         escape_received = 0;
    }
}

static int FF_over(int game_status)
{
    SDL_Rect dest_message;
    SDL_Event event;


    /* TODO: need better "victory" screen with animation, special music, etc., */
    /* as well as options to review missed questions, play again using missed  */
    /* questions as question list, etc.                                        */
    /* TODO: also, some of these cases just redraw the background on every     */
    /* frame with nothing else - just copy-and-pasted code without much        */
    /* further attention.                                                      */

    /* Turn mouse cursor back on before we go back to menus: */
    SDL_ShowCursor(1);
    SDL_WM_GrabInput(SDL_GRAB_OFF);


    switch (game_status)
    {
        case FF_OVER_WON:
            {
                int looping = 1;

                DEBUGMSG(debug_factoroids, "Loop exited with GAME_OVER_WON\n");

                /* set up victory message: */
                dest_message.x = (screen->w - images[IMG_GAMEOVER_WON]->w) / 2;
                dest_message.y = (screen->h - images[IMG_GAMEOVER_WON]->h) / 2;
                dest_message.w = images[IMG_GAMEOVER_WON]->w;
                dest_message.h = images[IMG_GAMEOVER_WON]->h;

                SDL_BlitSurface(images[IMG_GAMEOVER_WON], NULL, screen, &dest_message);
                SDL_Flip(screen);

                wait_for_input();
                break;
            }

        case FF_OVER_ERROR:
            {
                DEBUGMSG(debug_factoroids, "Loop exited with  FF_OVER_ERROR\n");
            }
        case FF_OVER_LOST:
        case FF_OVER_OTHER:
            {
                DEBUGMSG(debug_factoroids, "Loop exited with FF_OVER_LOST or FF_OVER_OTHER\n");

                /* set up GAMEOVER message: */
                dest_message.x = (screen->w - images[IMG_GAMEOVER]->w) / 2;
                dest_message.y = (screen->h - images[IMG_GAMEOVER]->h) / 2;
                dest_message.w = images[IMG_GAMEOVER]->w;
                dest_message.h = images[IMG_GAMEOVER]->h;

                SDL_BlitSurface(images[IMG_GAMEOVER], NULL, screen, &dest_message);
                SDL_Flip(screen);

                wait_for_input();
                break;
            }

        case FF_OVER_ESCAPE:
            {
                DEBUGMSG(debug_factoroids, "Loop exited with FF_OVER_ESCAPE\n");
                break;
            }

        case FF_OVER_SDL_QUIT:
            {
                DEBUGMSG(debug_factoroids, "Loop exited with FF_OVER_SDL_QUIT\n");
                break;
            }

        default:
            {
                DEBUGMSG(debug_factoroids, "Loop exited with unrecognized status value: %dn", game_status);
            }
    }

    FF_exit_free();

    /* Save score in case needed for high score table: */
    Opts_SetLastScore(score);

    /* Return the chosen command: */
    if (GAME_OVER_WINDOW_CLOSE == game_status)
    {
        /* program exits: */
        FF_exit_free();;
        return 1;
    }
    else
    {
        /* return to title() screen: */
        return 0;
    }
}


static void FF_exit_free()
{
    free(asteroid);
    factoroids_cleanup_graphics();

    /* Resume "normal" settings when we leave:
    */
    SDL_ShowCursor(1);
    SDL_WM_GrabInput(SDL_GRAB_OFF);
}

/******************* Math Funcs ***********************/

/* Return 1 if the number is prime and 0 if its not */
int is_prime(int num)
{
    int i;
    if (num==0 || num==1 || num==-1) return 1;
    else if (num > 0)
    {

        for(i = 2; i < num; i++)
        {
            if(num%i == 0) return 0;
        }
    }
    else if (num < 0)
    {
        for(i = 2; i > num; i--)
        {
            if(num%i == 0) return 0;
        }
    }
    return 1;
}

int is_simplified(int a, int b)
{
    int i;
    for(i=2; i<1000; i++)
        if(((a%i)==0)&&((b%i)==0))
            return 0;
    return 1;
}
/*** Fast cos by Bill***/

int fast_cos(int angle)
{
    angle = (angle % 45);

    if (angle < 12)
        return(trig[angle]);
    else if (angle < 23)
        return(-trig[10 - (angle - 12)]);
    else if (angle < 34)
        return(-trig[angle - 22]);
    else
        return(trig[45 - angle]);
}


/*** Sine based on fast cosine..., by Bill ***/

int fast_sin(int angle)
{
    return(- fast_cos((angle + 11) % 45));
}

/*** fact_number generator by aviraldg ***/

static int generatenumber(int wave) {
    if(wave > PRIME_MAX_LIMIT) wave = PRIME_MAX_LIMIT;
    int n=1, i;
    for(i=0; i<wave; i++)
        n *= pow(prime_numbers[i], rand()%prime_power_limit[i]);
    /* If we somehow got a bogus number, try again: */
    if(validate_number(n, wave))
        return n;
    else
    {
        if (n > 1)  /* 1 can be generated without bugs and is innocuous */
            DEBUGMSG(debug_factoroids, "generatenumber() - wrn - invalid number: %d\n", n);
        return generatenumber(wave);
    }
}

/*** For some reason, we have sometimes seen rocks with numbers */
/*** that are not multiples of the desired primes.  Here we     */
/*** factor those primes out and see what's left.               */
/*** Returns 0 (false) if number is invalid.    DSB             */
static int validate_number(int num, int wave)
{
    int i = 0;
    if(num < 2)
        return 0;
    if(wave > PRIME_MAX_LIMIT)
        wave = PRIME_MAX_LIMIT;
    for(i = 0; i < wave; i++)
    {
        while(num % prime_numbers[i] == 0)
            num /= prime_numbers[i];
    }

    /* If we aren't left with 1, the number is invalid: */
    if(num == 1)
        return 1;
    else
        return 0;
}

//implementation of the powerbomb powerup
void _tb_PowerBomb (int num) {
    int i;

    for(i=0; i<MAX_ASTEROIDS; i++) {
        if(asteroid[i].alive == 1) {
            if((FF_game==FACTOROIDS_GAME && (asteroid[i].isprime && ((num==asteroid[i].fact_number)||(num==0)))) ||
                    (FF_game==FRACTIONS_GAME && (asteroid[i].isprime && num==0))) {
                FF_destroy_asteroid(i, 0, 0);
            } else if((FF_game==FACTOROIDS_GAME && num > 1 && ((asteroid[i].fact_number%num)==0) && (num!=asteroid[i].fact_number)) ||
                    (FF_game==FRACTIONS_GAME && num > 1 && ((asteroid[i].a%num)==0) && ((asteroid[i].b%num)==0) && (num!=asteroid[i].fact_number))) {
                FF_destroy_asteroid(i, 0, 0);
            }
        }
    }
}

/******************* LASER FUNCTIONS *********************/
/*Return -1 if no laser is available*/
int FF_add_laser(void)
{
    int i, k, zapIndex, zapScore;
    float ux, uy, s, smin,dx,dy,dx2, dy2, d2, thresh;
    int screensize;
    SDL_Surface *asteroid_image;

    const float inside_factor = 0.9*0.9;

    screensize = screen->w;
    if (screensize < screen->h)
        screensize = screen->h;

    for(i=0; i<MAX_LASER; i++)
    {
        if(laser[i].alive==0)
        {
            // Fire the laser
            laser[i].alive=1;
            laser[i].x=tuxship.centerx;
            laser[i].y=tuxship.centery;
            laser[i].angle=tuxship.angle;
            laser[i].count=15;
            laser[i].n = num;

            ux = cos((float)laser[i].angle * DEG_TO_RAD);
            uy = -sin((float)laser[i].angle * DEG_TO_RAD);
            laser[i].destx = laser[i].x + (int)(ux * screensize);
            laser[i].desty = laser[i].y + (int)(uy * screensize);

            // Check to see if it hits asteroids---we only check when it
            // just starts firing, "drift" later doesn't count!
            // We describe the laser path as p = p0 + s*u, where
            //   p0 = (x0,y0) is the initial position vector (i.e., the ship)
            //   u = (ux,uy) is the unit vector of the laser's direction
            //   s (a scalar) is the distance along the laser (s >= 0)
            // With this parametrization, it's easy to calculate the
            // closest approach to the asteroid center, etc.
            zapIndex = -1;  // keep track of the closest "hit" asteroid
            zapScore = 0;
            smin = 10*screensize;


            for (k=0; k<MAX_ASTEROIDS; k++)
            {
                if (!asteroid[k].alive)
                    continue;
                asteroid_image = get_asteroid_image(asteroid[k].size,asteroid[k].angle);
                dx = asteroid[k].x + asteroid_image->w/2 - laser[i].x;
                dy = asteroid[k].y + asteroid_image->h/2 - laser[i].y;
                // Find distance along laser of closest approach to asteroid center
                s = dx*ux + dy*uy;
                if (s >= 0)  // don't worry about it if it's in the opposite direction! (i.e., behind the ship)
                {
                    // Find the distance to the asteroid center at closest approach
                    dx2 = dx - s*ux;
                    dy2 = dy - s*uy;
                    d2 = dx2*dx2 + dy2*dy2;
                    thresh = (asteroid_image->h)/2;
                    thresh = thresh*thresh*inside_factor;
                    if (d2 < thresh)
                    {
                        // The laser intersects the asteroid. Check to see if
                        // the answer works

                        if( (FF_game==FACTOROIDS_GAME && (asteroid[k].isprime && ((num==asteroid[k].fact_number)||(num==0)))) ||
                                (FF_game==FRACTIONS_GAME && (asteroid[k].isprime && num==0))
                          )
                        {
                            // It's valid, check to see if it's closest
                            if (s < smin)
                            {
                                // It's the closest yet examined but has not score
                                smin = s;
                                zapIndex = k;
                                zapScore = 0;
                            }
                        }
                        else if((FF_game==FACTOROIDS_GAME && num > 1 && ((asteroid[k].fact_number%num)==0) && (num!=asteroid[k].fact_number)) ||
                                (FF_game==FRACTIONS_GAME && num > 1 && ((asteroid[k].a%num)==0) && ((asteroid[k].b%num)==0) && (num!=asteroid[k].fact_number)))
                        {
                            // It's valid, check to see if it's closest
                            if (s < smin)
                            {
                                // It's the closest yet examined and has socre
                                smin = s;
                                zapIndex = k;
                                zapScore = 1;
                            }
                        }
                    }
                }
            }

            // Handle the destruction, score, and extra lives
            if (zapIndex >= 0)  // did we zap one?
            {
                asteroid[zapIndex].isdead = 1;
                laser[i].destx = laser[i].x + (int)(ux * smin);
                laser[i].desty = laser[i].y + (int)(uy * smin);
                FF_destroy_asteroid(zapIndex,30*ux,30*uy);
                playsound(SND_SIZZLE);

                if (floor((float)score/100) < floor((float)(score+num)/100))
                    tuxship.lives++;
                if(zapScore)
                {
                    score += num;
                }
            }
            return 1;
        }
    }
    DEBUGMSG(debug_factoroids, "Laser could't be created!\n");
    return -1;
}

/******************* ASTEROIDS FUNCTIONS *******************/



static int FF_add_asteroid(int x, int y, int xspeed, int yspeed, int size, int angle, int angle_speed, int fact_number, int a, int b, int new_wave)
{
    int i;
    for(i=0; i<MAX_ASTEROIDS; i++){
        if(asteroid[i].alive==0)
        {
            asteroid[i].alive=1;
            asteroid[i].rx=x;
            asteroid[i].ry=y;
            asteroid[i].angle=angle;
            asteroid[i].y=(asteroid[i].ry - tuxship_img_h(asteroid[i].angle/DEG_PER_ROTATION)/2);
            asteroid[i].x=(asteroid[i].rx - tuxship_img_w(asteroid[i].angle/DEG_PER_ROTATION)/2);
            asteroid[i].xdead = 0;
            asteroid[i].ydead = 0;
            asteroid[i].isdead = 0;
            asteroid[i].countdead = 0;

            //Without this on higher frame rates moevement in pixels per frame could be less than 1
            if(xspeed < 16 && xspeed >= 0)
                asteroid[i].xspeed = (rand()%4+1)*16;
            else if(xspeed > -16 && xspeed <= 0)
                asteroid[i].xspeed = -(rand()%4+1)*16;
            else
                asteroid[i].xspeed = xspeed;

            if(yspeed < 16 && yspeed >= 0)
                asteroid[i].yspeed = (rand()%4+1)*16;
            else if(yspeed > -16 && yspeed <= 0)
                asteroid[i].yspeed = -(rand()%4+1)*16;
            else
                asteroid[i].yspeed = yspeed;

            if(angle_speed < 16 && angle_speed >= 0)
                asteroid[i].angle_speed = (rand()%4+1)*16;
            else if(angle_speed > -16 && angle_speed <= 0)
                asteroid[i].angle_speed = -(rand()%4+1)*16;
            else
                asteroid[i].angle_speed = angle_speed;

            if(FF_game==FACTOROIDS_GAME){

                if(!validate_number(fact_number, wave))
                {
                    DEBUGMSG(debug_factoroids, "Invalid asteroid number: %d\n", fact_number);
                    return -1;
                }
                //while(!asteroid[i].fact_number)
                //  asteroid[i].fact_number=rand()%80;

                asteroid[i].fact_number=fact_number;
                asteroid[i].isprime=is_prime(asteroid[i].fact_number);

            }else if(FF_game==FRACTIONS_GAME){

                asteroid[i].a=a;
                asteroid[i].b=b;

                while(!asteroid[i].a)
                    asteroid[i].a=rand()%80;
                while(!asteroid[i].b)
                    asteroid[i].b=rand()%80;

                asteroid[i].isprime=is_simplified(asteroid[i].a,asteroid[i].b);
            }

            if(new_wave){
                if(tuxship.x-50<asteroid[i].x+80 &&
                        tuxship.x+50>asteroid[i].x &&
                        tuxship.y-50<asteroid[i].y+80 &&
                        tuxship.y+50>asteroid[i].y &&
                        tuxship.lives>0 &&
                        asteroid[i].alive){
                    asteroid[i].rx=asteroid[i].rx+300;
                    asteroid[i].ry=asteroid[i].ry+300;
                }
            }

            if(asteroid[i].isprime)
            {
                asteroid[i].size=0;
                asteroid[i].centerx=(images[IMG_ASTEROID1]->w/2)+asteroid[i].x;
                asteroid[i].centery=(images[IMG_ASTEROID1]->h/2)+asteroid[i].y;
                asteroid[i].radius=(images[IMG_ASTEROID1]->h/2);

            }
            else if(!asteroid[i].isprime)
            {
                asteroid[i].size=1;
                asteroid[i].centerx=(images[IMG_ASTEROID2]->w/2)+asteroid[i].x;
                asteroid[i].centery=(images[IMG_ASTEROID2]->h/2)+asteroid[i].y;
                asteroid[i].radius=(images[IMG_ASTEROID1]->h/2);
            }

            while (asteroid[i].xspeed==0)
            {
                asteroid[i].xspeed = ((rand() % 3) - 1)*2;
            }
            return 1;
        }
    }
    fprintf(stderr, "Asteroid could't be created!\n");
    return -1;
}

int FF_destroy_asteroid(int i, float xspeed, float yspeed)
{
    if(asteroid[i].alive==1){
        asteroid[i].isdead=1;
        asteroid[i].xdead=asteroid[i].x;
        asteroid[i].ydead=asteroid[i].y;
        if(asteroid[i].size>0){
            /* Break the rock into two smaller ones! */
            if(num!=0){



                if(FF_game==FACTOROIDS_GAME){
                    FF_add_asteroid(asteroid[i].rx,
                            asteroid[i].ry,
                            asteroid[i].xspeed + (xspeed - yspeed)/2,
                            asteroid[i].yspeed + (yspeed + xspeed)/2,
                            0,
                            rand()%360, rand()%150-75, (int)(asteroid[i].fact_number/num),
                            0, 0,
                            0);

                    FF_add_asteroid(asteroid[i].rx,
                            asteroid[i].ry,
                            asteroid[i].xspeed + (xspeed + yspeed)/2,
                            asteroid[i].yspeed + (yspeed - xspeed)/2,
                            0,
                            rand()%360, rand()%150-75, num,
                            0, 0,
                            0);
                }
                else if(FF_game==FRACTIONS_GAME){
                    FF_add_asteroid(asteroid[i].rx,
                            asteroid[i].ry,
                            ((asteroid[i].xspeed + xspeed) / 2),
                            (asteroid[i].yspeed + yspeed),
                            0,
                            rand()%360, rand()%150-75, 0,
                            (int)(asteroid[i].a/num), (int)(asteroid[i].b/num),
                            0);

                    FF_add_asteroid(asteroid[i].rx,
                            asteroid[i].ry,
                            (asteroid[i].xspeed + xspeed),
                            ((asteroid[i].yspeed + yspeed) / 2),
                            0,
                            rand()%360, rand()%150-75, 0,
                            (int)(asteroid[i].b/num), (int)(asteroid[i].a/num),
                            0);
                }
            }
        }

        /* Destroy the old asteroid */

        asteroid[i].alive=0;
        return 1;
    }
    return 0;
}


void game_handle_user_events(void)
{
    SDL_Event event;
    SDLKey key;
    int roto = 0; //rotation flag

    while (SDL_PollEvent(&event) > 0)
    {
        T4K_HandleStdEvents(&event);
        if (event.type == SDL_QUIT)
        {
            SDL_quit_received = 1;
            quit = 1;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
        {
            key = game_mouse_event(event);
            //the code transforms a mouse event into a keyboard event,
            //so the same modification should be made with the event structure
            // -- aviraldg 14/12/10
            int state = event.button.state;
            event.key.keysym.sym = key;
            event.type = state == SDL_PRESSED ? SDL_KEYDOWN : SDL_KEYUP;
        }
        if(event.type == SDL_MOUSEMOTION) {
            //NOTE: in SDL 1.2 this repositioning is only needed for
            //OS-X.  Not sure if it will be needed at all in SDL 1.3

            //If we are near edge of the screen, reset mouse to center
            if(event.motion.x <= 10
                    || event.motion.x >= (screen->w - 10)
                    || event.motion.y <= 10
                    || event.motion.y >= (screen->h - 10))
            {
                mouse_reset = 1;
                SDL_WarpMouse(screen->w/2, screen->h/2);
                continue;
            }
            //If this was an event generated by our reset-to-center,
            //ignore it:
            if(mouse_reset)
            {
                mouse_reset = 0;
                continue;
            }
            //otherwise, valid event - handle mouse rotation
            roto = 1;
            mouseroto = game_mouseroto(event) / MOUSE_SENSITIVITY;
        }
        if (event.type == SDL_KEYDOWN ||
                event.type == SDL_KEYUP)
        {
            key = event.key.keysym.sym;

            if (event.type == SDL_KEYDOWN)
            {
                if (key == SDLK_ESCAPE)
                {
                    // Return to menu!
                    escape_received = 1;

                }

                // Key press...

                if (key == SDLK_RIGHT)
                {
                    // Rotate CW

                    left_pressed = 0;
                    right_pressed = 1;
                }
                else if (key == SDLK_LEFT)
                {
                    // Rotate CCW

                    left_pressed = 1;
                    right_pressed = 0;
                }
                else if (key == SDLK_UP)
                {
                    // Thrust!

                    up_pressed = 1;
                }

                if (key == SDLK_LSHIFT || key == SDLK_RSHIFT)
                {
                    // Respawn now (if applicable)
                    shift_pressed = 1;
                }

                if (key == SDLK_TAB || key == SDLK_p)
                {
                    /* [TAB] or [P]: Pause! (if settings allow) */
                    if (Opts_AllowPause())
                    {
                        paused = 1;
                    }
                }

                if (key == CTRL_NEXT) {
                    int n = prime_next[digits[1]*10 + digits[2]];
                    if(n <= prime_numbers[wave - 1]) {
                        digits[2] = n % 10;
                        digits[1] = n / 10;
                        tux_pressing = 1;
                        playsound(SND_SHIELDSDOWN);
                    } else {
                        digits[2] = 2;
                        digits[1] = 0;
                        tux_pressing = 1;
                        playsound(SND_SHIELDSDOWN);
                    }
                }

                if (key == CTRL_PREV) {
                    int n = prime_prev[digits[1]*10 + digits[2]];
                    if(n <= prime_numbers[wave - 1]) {
                        digits[2] = n % 10;
                        digits[1] = n / 10;
                        tux_pressing = 1;
                        playsound(SND_SHIELDSDOWN);
                    } else {
                        digits[2] = prime_numbers[wave - 1] % 10;
                        digits[1] = prime_numbers[wave - 1] / 10;
                        tux_pressing = 1;
                        playsound(SND_SHIELDSDOWN);
                    }
                }
                // TODO this code only deals with the first 6 primes, we'd probably want a
                // more generic solution
                int _tmp = digits[0]*100 + digits[1]*10 + digits[2];
                int digit, exec_digits = 0;
                if(key >= SDLK_0 && key <= SDLK_9) {
                    digit = key - SDLK_0;
                    tux_pressing = 1;
                    exec_digits = 1;
                    playsound(SND_SHIELDSDOWN);
                } else if (key >= SDLK_KP0 && key <= SDLK_KP9) {
                    digit = key - SDLK_KP0;
                    tux_pressing = 1;
                    exec_digits = 1;
                    playsound(SND_SHIELDSDOWN);
                }
                if(exec_digits == 1) {
                    if(digits[1] == 1 && (digit == 2 || digit == 5 || digit == 7)) {
                        digits[1] = 0;
                        digits[2] = digit;
                    } else if(digits[1] == 1 && digit == 1) {
                        digits[2] = 1;
                    } else if(digit==1) {
                        digits[1] = 1;
                        digits[2] = 0;
                    }
                    if(digit == 2 || digit == 3 || digit == 5 || digit == 7) {
                        digits[2] = digit;
                    }
                }
                //cancel if requested digit forms larger prime than allowed
                if((digits[1]*10 + digits[2]) > prime_numbers[wave - 1]) {
                    digits[2] = _tmp % 10;
                    digits[1] = _tmp / 10;
                }

                /* activate bonus/powerup */
                if((key == SDLK_LSHIFT || key == SDLK_RSHIFT) && bonus_time == -1) {
                    playsound(SND_HARP);
                    bonus_time = SDL_GetTicks() + 10000; //10sec bonus

                    //special handling for the powerbomb, since it happens "at once"
                    if(bonus == TB_POWERBOMB) {
                        _tb_PowerBomb(digits[1]*10 + digits[2]);
                        bonus_time = SDL_GetTicks() + 1000;
                        /* FIXME ugly hack to allow multiple lasers to display */
                    }
                }
                /* support for negative answer input DSB */
                else if ((key == SDLK_MINUS || key == SDLK_KP_MINUS))
                    //&& MC_AllowNegatives())  /* do nothing unless neg answers allowed */
                {
                    /* allow player to make answer negative: */
                    neg_answer_picked = 1;
                    tux_pressing = 1;
                    playsound(SND_SHIELDSDOWN);
                }
                else if ((key == SDLK_PLUS || key == SDLK_KP_PLUS))
                    //&& MC_AllowNegatives())  /* do nothing unless neg answers allowed */
                {
                    /* allow player to make answer positive: */
                    neg_answer_picked = 0;
                    tux_pressing = 1;
                    playsound(SND_SHIELDSDOWN);
                }
                else if (key == SDLK_RETURN ||
                        key == SDLK_KP_ENTER ||
                        key == SDLK_SPACE)
                {
                    shoot_pressed = 1;
                    doing_answer = 1;
                    button_pressed = 1;
                    playsound(SND_LASER);
                }


            }
            else if (event.type == SDL_KEYUP)
            {
                // Key release...

                if (key == SDLK_RIGHT)
                {
                    right_pressed = 0;
                }
                else if (key == SDLK_LEFT)
                {
                    left_pressed = 0;
                }
                else if (key == SDLK_UP)
                {
                    up_pressed = 0;
                }
                if (key == SDLK_LSHIFT ||
                        key == SDLK_RSHIFT)
                {
                    // Respawn now (if applicable)
                    shift_pressed = 0;
                }
                if ( key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE )
                {
                    button_pressed = 0;
                }
            }
        }

#ifdef JOY_YES
        else if (event.type == SDL_JOYBUTTONDOWN &&
                player_alive)
        {
            if (event.jbutton.button == JOY_B)
            {
                shoot_pressed = 1;
            }
            else if (event.jbutton.button == JOY_A)
            {
                // Thrust:

                up_pressed = 1;
            }
            else
            {
                shift_pressed = 1;
            }
        }
        else if (event.type == SDL_JOYBUTTONUP)
        {
            if (event.jbutton.button == JOY_A)
            {
                // Stop thrust:

                up_pressed = 0;
            }
            else if (event.jbutton.button != JOY_B)
            {
                shift_pressed = 0;
            }
        }
        else if (event.type == SDL_JOYAXISMOTION)
        {
            if (event.jaxis.axis == JOY_X)
            {
                if (event.jaxis.value < -256)
                {
                    left_pressed = 1;
                    right_pressed = 0;
                }
                else if (event.jaxis.value > 256)
                {
                    left_pressed = 0;
                    right_pressed = 1;
                }
                else
                {
                    left_pressed = 0;
                    right_pressed = 0;
                }
            }
        }
#endif

    }
    if(roto == 0) mouseroto = 0;
}


static int game_mouse_event(SDL_Event event)
{
    if(event.button.button == SDL_BUTTON_LEFT) return  SDLK_RETURN;
    else if(event.button.button == SDL_BUTTON_MIDDLE) return SDLK_LSHIFT;
    else if(event.button.button == SDL_BUTTON_RIGHT) return SDLK_UP;
    else if(event.button.button == SDL_BUTTON_WHEELUP) return CTRL_NEXT;
    else if(event.button.button == SDL_BUTTON_WHEELDOWN) return CTRL_PREV;
    else return SDLK_UNKNOWN;
}


static int check_exit_conditions(void)
{
    if(SDL_quit_received)
    {
        return FF_OVER_SDL_QUIT;
    }

    if(escape_received)
    {
        return FF_OVER_ESCAPE;
    }
    if(tuxship.lives<=0)
    {
        return FF_OVER_LOST;
    }
    if(wave > 6 )
    {
        return FF_OVER_WON;
    }

    /* if we made it to here, the game goes on! */
    return FF_IN_PROGRESS;
}


void wait_for_input(void)
{
    SDL_Event event = {SDL_NOEVENT};

    while(1)
    {
        if(!SDL_PollEvent(&event))
        {
            SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
            SDL_EventState(SDL_JOYAXISMOTION, SDL_IGNORE);

            SDL_WaitEvent(&event);

            SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);
            SDL_EventState(SDL_JOYAXISMOTION, SDL_ENABLE);
        }

        if (event.type == SDL_QUIT)
        {
            SDL_quit_received = 1;
            quit = 1;
            break;
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN ||
                event.type == SDL_JOYBUTTONDOWN)
        {
            break;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_ESCAPE)
                escape_received = 1;
            break;
        }
    }
}


static int cockpit_tux_image( int prime )
{
    switch( prime )
    {
        case 2:
            return IMG_TUX1;
        case 3:
            return IMG_TUX2;
        case 5:
            return IMG_TUX3;
        case 7:
            return IMG_TUX4;
        case 11:
            return IMG_TUX5;
        case 13:
            return IMG_TUX6;
        default:
            return IMG_TUX1;
    }
}
