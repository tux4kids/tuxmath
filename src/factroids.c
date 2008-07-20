/************************************************************
 *  factroids.c                                             *
 *                                                          *
 *  Description: Code for the factor and fraction activity  *
 *                                                          *
 *  Autor:       Jesus M. Mager H. (fongog@gmail.com) 2008  *
 *  Copyright:   GPL v3 or later                            *
 *                                                          *
 *  Code based on the work made by:                         *
 *               Bull kendrick (vectoroids 1.1.0)          *
 *               and Bill Kendrick avid Bruce, Tim Holy    *
 *               and others (Tuxmath 1.6.3)                *
 *                                                          *
 *  TuxMath                                                 *
 *  Part of "Tux4Kids" Project                              *
 *  http://tux4kids.alioth.debian.org/                      *
 ************************************************************/

#include "tuxmath.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"
#ifndef NOSOUND
#include "SDL_mixer.h"
#endif
#include "SDL_image.h"
  

#include "game.h"
#include "fileops.h"
#include "setup.h"
#include "mathcards.h"
#include "titlescreen.h"
#include "options.h"
#include "SDL_extras.h"

#define FPS 15                     /* 15 frames per second */
#define MS_PER_FRAME (1000 / FPS)

#define MAX_LASER 5
#define MAX_ASTEROIDS 50
#define NUM_TUXSHIPS 2
#define NUM_SPRITES 11
#define TUXSHIP_LIVES 3

#define DEG_TO_RAD 0.0174532925
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))

/********* Enumerations ***********/

enum{
  FACTROIDS_GAME,
  FRACTIONS_GAME
};

/********* Structures *********/

typedef struct colorRGBA_type {
  Uint8 r;
  Uint8 g;
  Uint8 b;
  Uint8 a;
} ColorRGBA_type;

typedef struct asteroid_type {
  int alive, size;
  int angle, angle_speed;
  int xspeed, yspeed;
  int x, y;
  int centerx, centery;
  int fact_number;
  int isprime;
  int a, b; /*  a / b */
  MC_FlashCard flashcard;
  Uint32 time_started;
} asteroid_type;


typedef struct tuxship_type {
  int lives, size;
  int xspeed, yspeed;
  int x, y;
  int centerx, centery;
  int angle;
  int hurt, hurt_count;
} tuxship_type;

typedef struct FF_laser_type{
  int alive;
  int x, y;
  int destx,desty;
  int r, g, b;
  int count;
  int angle;
  int m;
} FF_laser_type;

typedef struct {
  int x_is_blinking;
  int extra_life_is_blinking;
  int laser_enabled;
} help_controls_type;

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

static char operchars[4] = {
   "+-*/"
};

// ControlKeys
static int left_pressed;
static int right_pressed;
static int up_pressed;
static int shift_pressed;
static int shoot_pressed;

// GameControl
static int points;
static int game_status;
static int gameover_counter;
static int escape_received;

//SDL Variables
static SDL_Surface *bg; //The background

static SDL_Rect bgSrc, bgScreen2, bgScreen;

// Game type

static int FF_game;

// Game vars
static int score;
static int wave;
static int paused;
static int key_pressed;
static int escape_received;
static int gameover_counter;
static int game_status;
static int SDL_quit_received;
static int done, quit;
static int digits[3];
static int num;

static int neg_answer_picked;
static int tux_pressing;
static int doing_answer;
static int level_start_wait;
//static int FF_level;

static asteroid_type* asteroid = NULL;
static tuxship_type tuxship;
static FF_laser_type laser[MAX_LASER];

static int NUM_ASTEROIDS;
static int bkg_h, counter;
static int xdead, ydead, isdead, countdead;

//static int SDL_Surface *screen2;

static int FF_init(void);
static void FF_intro(void);
static void FF_handle_ship(void);
static void FF_handle_asteroids(void);
static void FF_handle_answer(void);
static void FF_draw(void);
static void FF_add_level(void);
static void FF_loose(void);
static void FF_win(void);
static void FF_exit_free(void);
static int FF_add_laser(void);
static int FF_add_asteroid(int x, int y, int xspeed, int yspeed, int size, int angle, int 				   angle_speed, int fact_num, int a, int b, int new_wave);
static int FF_destroy_asteroid(int i, int xspeed, int yspeed);

static int is_prime(int num);
static int fast_cos(int angle);
static int fast_sin(int angle);


void factors(void){


  Uint32 last_time, now_time; 
  
  done = 0;
  quit = 0;
  counter = 0;
  

  
  #ifdef TUXMATH_DEBUG
     fprintf(stderr, "Entering factors():\n");
  #endif

  FF_game=FACTROIDS_GAME;
  
  if (!FF_init())
  {
    fprintf(stderr, "FF_init() failed!\n");
    FF_exit_free();
    return;
  } 
  
  FF_intro();
  
  while (!done){
      last_time = SDL_GetTicks();
      counter++;
      if(tuxship.lives<=0)
        done=1;
      
   
    game_handle_user_events();

    FF_handle_ship();
    FF_handle_asteroids();
    FF_handle_answer();
    FF_draw();

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
	    Mix_PlayMusic(musics[MUS_GAME + (rand() % 3)], 0);
      }  
    }
#endif



      /* Pause (keep frame-rate event) */
    now_time = SDL_GetTicks();
    if (now_time < last_time + MS_PER_FRAME)
    {
      //Prevent any possibility of a time wrap-around
      // (this is a very unlikely problem unless there is an SDL bug
      //  or you leave tuxmath running for 49 days...)
      now_time = (last_time+MS_PER_FRAME) - now_time;  // this holds the delay
      if (now_time > MS_PER_FRAME)
	now_time = MS_PER_FRAME;
      SDL_Delay(now_time);
    }
  }
}

void fractions(void){

  Uint32 last_time, now_time; 
  
  done = 0;
  quit = 0;
  counter = 0;
  

  
  #ifdef TUXMATH_DEBUG
     fprintf(stderr, "Entering factors():\n");
  #endif
  
  /*****Initalizing the Factor activiy *****/
  FF_game=FRACTIONS_GAME;

  if (!FF_init())
  {
    fprintf(stderr, "FF_init() failed!\n");
    FF_exit_free();
    return;
  } 

  /************ Game Intro **************/
  
  FF_intro();

  /************ Main Loop **************/
  while (!done){
      last_time = SDL_GetTicks();
      counter++;
      if(tuxship.lives<=0)
        done=1;
      
   
    game_handle_user_events();

    FF_handle_ship();
    FF_handle_asteroids();
    FF_handle_answer();
    FF_draw();

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
	    Mix_PlayMusic(musics[MUS_GAME + (rand() % 3)], 0);
      }  
    }
#endif



      /* Pause (keep frame-rate event) */
    now_time = SDL_GetTicks();
    if (now_time < last_time + MS_PER_FRAME)
    {
      //Prevent any possibility of a time wrap-around
      // (this is a very unlikely problem unless there is an SDL bug
      //  or you leave tuxmath running for 49 days...)
      now_time = (last_time+MS_PER_FRAME) - now_time;  // this holds the delay
      if (now_time > MS_PER_FRAME)
	now_time = MS_PER_FRAME;
      SDL_Delay(now_time);
    }
  }
}

static int FF_init(void){
  int i;
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
  SDL_Flip(screen);
  bkg_h=(images[BG_STARS]->h)>>1;
  bgSrc.y=((images[BG_STARS]->h)>>1)-bkg_h;
  bgSrc.x=0;
  bgSrc.w=screen->w;
  bgSrc.h=screen->h;
  
  game_status = GAME_IN_PROGRESS;  
  
  // Allocate memory 
  asteroid = NULL;  // set in case allocation fails partway through
  
  asteroid = (asteroid_type *) malloc(MAX_ASTEROIDS * sizeof(asteroid_type));
  if (asteroid == NULL) {
    printf("Allocation of asteroids failed");
    return 0;
  }
  NUM_ASTEROIDS=4;
  FF_add_level();

  /**************Setting up the ship values! **************/

  tuxship.x=((screen->w)/2)-20;
  tuxship.y=((screen->h)/2)-20;
  tuxship.lives=TUXSHIP_LIVES;
  tuxship.hurt=0;
  tuxship.hurt_count=0;
  tuxship.angle=90;
  tuxship.xspeed=0;
  tuxship.yspeed=0;
  tuxship.centerx=(images[IMG_SHIP01]->w)/2;
  tuxship.centery=(images[IMG_SHIP01]->h)/2;
  shoot_pressed=0;
  score=1;
  wave=1;

  for (i=0; i<MAX_LASER; i++)
    laser[i].alive=0;
  return 1;
  
}


static void FF_intro(void){}

static void FF_handle_ship(void){

/****************** Ship center... ******************/

  tuxship.centerx=((images[IMG_SHIP01]->w)/2)+tuxship.x;
  tuxship.centery=((images[IMG_SHIP01]->h)/2)+tuxship.y;  

/******************* Ship live *********************/

  if ((wave%5==0) || (score%100==0))
  {
    tuxship.lives++;
  }
  
  if(tuxship.hurt)
  {
    tuxship.hurt_count--;
    if(tuxship.hurt_count<=0)
	tuxship.hurt=0;
  }
/****************** Rotate Ship *********************/

  if (right_pressed)
  {
    tuxship.angle=tuxship.angle - 30;
    if (tuxship.angle < 0)
      tuxship.angle = tuxship.angle + 360;
  }
  else if (left_pressed)
  {
    tuxship.angle=tuxship.angle + 30;
    if (tuxship.angle >= 360)
      tuxship.angle = tuxship.angle - 360;
  }

/**************** Move, and increse speed ***************/

      
  if (up_pressed && (tuxship.lives>0))
  {
     tuxship.xspeed = tuxship.xspeed + ((fast_cos(tuxship.angle >> 3) * 3) >> 10);
     tuxship.yspeed = tuxship.yspeed - ((fast_sin(tuxship.angle >> 3) * 3) >> 10);
  }
  else
  {
    if ((counter % 8) == 0)
    {
       tuxship.xspeed = (tuxship.xspeed * 7) / 8;
       tuxship.yspeed = (tuxship.yspeed * 7) / 8;
    }
  }

  tuxship.x = tuxship.x + tuxship.xspeed;
  tuxship.y = tuxship.y + tuxship.yspeed;

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

  int i, found=0;
      for (i = 0; i < MAX_ASTEROIDS; i++){
	  if (asteroid[i].alive)
	    {

	      found=1;

              /**************Move the astroids ****************/

	      asteroid[i].x = asteroid[i].x + asteroid[i].xspeed;
	      asteroid[i].y = asteroid[i].y + asteroid[i].yspeed;
	      
	      // Wrap asteroid around edges of screen: 
	      
	      if (asteroid[i].x >= (screen->w))
		asteroid[i].x = asteroid[i].x - (screen->w);
	      else if (asteroid[i].x < 0)
		asteroid[i].x = asteroid[i].x + (screen->w);
	      
	      if (asteroid[i].y >= (screen->h))
		asteroid[i].y = asteroid[i].y - (screen->h);
	      else if (asteroid[i].y < 0)
		asteroid[i].y = asteroid[i].y + (screen->h);
	      
	      
	      // Rotate asteroid: 
	      
	      asteroid[i].angle = (asteroid[i].angle + asteroid[i].angle_speed);
	      
	      
	      // Wrap rotation angle... 
	      
	      if (asteroid[i].angle < 0)
		asteroid[i].angle = asteroid[i].angle + 360;
	      else if (asteroid[i].angle >= 360)
		asteroid[i].angle = asteroid[i].angle - 360;
            // Collitions!
              if(asteroid[i].size<=2){
	         if(tuxship.x+30<asteroid[i].x+80 && 
                    tuxship.x+30>asteroid[i].x && 
                    tuxship.y+30<asteroid[i].y+80 && 
                    tuxship.y+30>asteroid[i].y &&
                    tuxship.lives>0 &&
                    asteroid[i].alive){ 

		      if(!tuxship.hurt){
		         xdead=asteroid[i].x;
		         ydead=asteroid[i].y;
		      
		         tuxship.lives--;
		         tuxship.hurt=1;
		         tuxship.hurt_count=50;
		         FF_destroy_asteroid(i, tuxship.xspeed, tuxship.yspeed);

		      }
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
  
    /* Clear digits: */
  digits[0] = 0;
  digits[1] = 0;
  digits[2] = 0;
  neg_answer_picked = 0;

}

static void FF_draw(void){
  SDL_Rect dest;
  int i, offset;
  char str[64];
  
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

  /************ Draw Background ***************/ 

  SDL_BlitSurface(images[BG_STARS], NULL, screen, NULL);
  //if(bgSrc.y>bkg_h)
  //  SDL_BlitSurface(images[BG_STARS], NULL, screen, &bgScreen);
/******************* Draw laser *************************/
  for (i=0;i<MAX_LASER;i++){
    if(laser[i].alive)
    {
      if(laser[i].count>0)
      {
        laser[i].count--;
	laser[i].x=laser[i].x+tuxship.xspeed;
   	laser[i].y=laser[i].y+tuxship.yspeed;
	laser[i].destx=laser[i].destx+tuxship.xspeed;
	laser[i].desty=laser[i].desty+tuxship.yspeed;
        draw_line(laser[i].x, laser[i].y, laser[i].destx, laser[i].desty,
		  laser[i].count*18, 0, 0);
      } else if (laser[i].count <= 0)
      {
        laser[i].alive=0;
      }
    }
  }
  /*************** Draw Ship ******************/ 
  dest.x=tuxship.x;
  dest.y=tuxship.y;
  if(!tuxship.hurt || (tuxship.hurt && tuxship.hurt_count%2==0))
    for(i=0;i<12;i++)
      if((i*30<=tuxship.angle) && (tuxship.angle<((i*30)+30)))
        SDL_BlitSurface(images[IMG_SHIP01+i], NULL, screen, &dest);
 
  /************* Draw Asteroids ***************/
  for(i=0; i<MAX_ASTEROIDS; i++){
    if(asteroid[i].alive>0){
     dest.x=asteroid[i].x;
     dest.y=asteroid[i].y;
     if(asteroid[i].size==0){
        SDL_BlitSurface(images[IMG_ASTEROID1], NULL, screen, &dest);
     }
     if(asteroid[i].size==1){
        SDL_BlitSurface(images[IMG_ASTEROID2], NULL, screen, &dest);
     }
     if(asteroid[i].size==2){
        SDL_BlitSurface(images[IMG_ASTEROID2], NULL, screen, &dest);
     }
     if(FF_game==FACTROIDS_GAME)
     {   
       sprintf(str, "%.1d", asteroid[i].fact_number);
       draw_nums(str, asteroid[i].x+20,asteroid[i].y+10);
     }
     else if (FF_game==FRACTIONS_GAME)
     {
       sprintf(str, "%d", asteroid[i].a);
       draw_nums(str, asteroid[i].x+20,asteroid[i].y+20); 
       draw_line(asteroid[i].x+20,asteroid[i].y+25, asteroid[i].x+50,asteroid[i].y+25,
		 255, 255, 255);
       sprintf(str, "%d", asteroid[i].b);
       draw_nums(str, asteroid[i].x+20,asteroid[i].y+55);
     }
    }
  }
  /*************** Draw Strem ***************/
  
  if(isdead)
  {
    dest.x=xdead;
    dest.y=ydead;
    SDL_BlitSurface(images[IMG_STEAM1+countdead], NULL, screen, &dest);
    countdead++;
    if(countdead>5){
      isdead=0;
      countdead=0;
    }
  }
  /* Draw wave: */
  if (1)//Opts_BonusCometInterval())
    offset = images[IMG_EXTRA_LIFE]->w + 5;
  else
    offset = 0;

  dest.x = offset;

  dest.y = 0;
  dest.w = images[IMG_WAVE]->w;
  dest.h = images[IMG_WAVE]->h;

  SDL_BlitSurface(images[IMG_WAVE], NULL, screen, &dest);

  sprintf(str, "%d", wave);
  draw_numbers(str, offset+images[IMG_WAVE]->w + (images[IMG_NUMBERS]->w / 10), 0);

  /* Draw "score" label: */
  dest.x = (screen->w - ((images[IMG_NUMBERS]->w / 10) * 7) -
	        images[IMG_SCORE]->w -
                images[IMG_STOP]->w - 5);
  dest.y = 0;
  dest.w = images[IMG_SCORE]->w;
  dest.h = images[IMG_SCORE]->h;

  SDL_BlitSurface(images[IMG_SCORE], NULL, screen, &dest);
        
  sprintf(str, "%.6d", score);
  draw_numbers(str,
               screen->w - ((images[IMG_NUMBERS]->w / 10) * 6) - images[IMG_STOP]->w - 5,
               0);

  /* Draw stop button: */
//  if (!help_controls.x_is_blinking || (frame % 10 < 5)) {
    dest.x = (screen->w - images[IMG_STOP]->w);
    dest.y = 0;
    dest.w = images[IMG_STOP]->w;
    dest.h = images[IMG_STOP]->h;
    
    SDL_BlitSurface(images[IMG_STOP], NULL, screen, &dest);
 // }

    /************* Draw pre answer ************/

  
    sprintf(str, "%.3d", num);
    draw_numbers(str, ((screen->w)/2)-50, (screen->h)-30);
 
 
  /************ Doublebuffering.. ***********/
  SDL_Flip(screen);

}

static void FF_add_level(void)
{
  int i;
  wave++;
  NUM_ASTEROIDS=NUM_ASTEROIDS+wave;
  for (i=0; i<MAX_ASTEROIDS; i++)
    asteroid[i].alive=0;
  for (i=0; (i<(NUM_ASTEROIDS/2)) && NUM_ASTEROIDS<MAX_ASTEROIDS; i++){
   
   //int FF_add_asteroid(int x, int y, int xspeed, int yspeed, int size, int angle, int angle_speed, int fact_number, int a, int b, int new_wave)
   if(FF_game==FACTROIDS_GAME){
     FF_add_asteroid(rand()%(screen->w), rand()%(screen->h),
		     rand()%4, rand()%3,
                     rand()%2, 
		     0, 3,
                     (rand()%(31+(wave*wave))), 
		     0, 0,
		     1);

     FF_add_asteroid(rand()%(screen->w), rand()%(screen->h),
		    (-1*rand()%4), (-1*rand()%3),
                     rand()%2, 
		     0, 3,
                     (rand()%(31+(wave*wave))), 
		     0, 0,
		     1);
     }
   else if(FF_game==FRACTIONS_GAME){
     FF_add_asteroid(rand()%(screen->w), rand()%(screen->h),
		     rand()%4, rand()%3,
                     rand()%2, 
		     0, 3,
                     0, 
		     (rand()%(31+(wave*2))), (rand()%(80+(wave*wave))),
		     1);

     FF_add_asteroid(rand()%(screen->w), rand()%(screen->h),
		    (-1*rand()%4), (-1*rand()%3),
                     rand()%2, 
		     0, 3,
                     0, 
		     (rand()%(31+(wave*2))), (rand()%(80+(wave*wave))),
		     1);

     }
   } 
   if((NUM_ASTEROIDS%2)==1){
     if(FF_game==FACTROIDS_GAME){
       FF_add_asteroid(rand()%(screen->w), rand()%(screen->h),
		       (-1*rand()%4), (-1*rand()%3),
                       rand()%2, 
		       0, 3,
                       (rand()%(31+(wave*wave))), 
		       0, 0,
                       1);
     }
     else if(FF_game==FRACTIONS_GAME){
       FF_add_asteroid(rand()%(screen->w), rand()%(screen->h),
  		      (-1*rand()%4), (-1*rand()%3),
                      rand()%2, 
		      0, 3,
                      0, 
		      (rand()%(31+(wave*2))), (rand()%(80+(wave*wave))),
		      1);
     
     }
   }
}

static void FF_loose(void){
  
}
static void FF_win(void){
  
}

static void FF_exit_free()
{
  free(asteroid);
}

/******************* Math Funcs ***********************/

/* Return 1 if the number is prime and 0 if its not */
int is_prime(int num)
{
  int i;
  //int divisor=2;
  if (num==0 || num==1 || num==-1) return 1;
  else if (num>0)
  {
    for(i=2; i<num; i++)
    {
      if(num%i==0) return 0; 
    }
  }
  else if (num<0)
  {
    for(i=2; i>num; i--)
    {
      if(num%i==0) return 0; 
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

/******************* LASER FUNCTIONS *********************/

/*Return -1 if no laser is avable*/
int FF_add_laser(void)
{
  int i, k;
  int dx,dy;
  int xcount, ycount;
  float m, b;
  for(i=0; i<=MAX_LASER; i++)
  {
    if(laser[i].alive==0)
    {
      laser[i].alive=1;
      laser[i].x=tuxship.centerx;
      laser[i].y=tuxship.centery;
      laser[i].angle=tuxship.angle;
      laser[i].count=15;

      if(laser[i].angle>=0 && laser[i].angle<=360)
      {
	laser[i].m     = (int)sin((float)laser[i].angle * DEG_TO_RAD);
	laser[i].destx = laser[i].x + (int)(cos((float)laser[i].angle * DEG_TO_RAD) * 1400);
	laser[i].desty = laser[i].y - (int)(sin((float)laser[i].angle * DEG_TO_RAD) * 1400);
	
	xcount = laser[i].x;
	ycount = laser[i].y;

	dx = laser[i].destx - xcount;
	dy = laser[i].desty - ycount;
	if (dx != 0)
	{

    	  m = ((float) dy) / ((float) dx);
	  b = ycount  - m * xcount;

	  if (laser[i].destx > xcount) dx = 1;
	  else dx = -1;

	  while (xcount != laser[i].destx)
	  {
            xcount = xcount + dx;
	    ycount = m * xcount + b;
	    for(k=0; k<MAX_ASTEROIDS; k++)
	    {
		if(xcount<asteroid[k].x+70 && 
                   xcount>asteroid[k].x && 
                   ycount<asteroid[k].y+70 && 
                   ycount>asteroid[k].y &&
                   asteroid[k].alive)
	        {
                   if(asteroid[k].isprime)
 		   {
		     isdead=1;
		     xdead=asteroid[k].x;
		     ydead=asteroid[k].y;
		     if(tuxship.x<asteroid[k].x)
		       FF_destroy_asteroid(k, m, m);
		     if(tuxship.x>=asteroid[k].x)
		       FF_destroy_asteroid(k, -m, m);

		     laser[i].destx=xcount;
		     laser[i].desty=ycount;
		     return 1;
		   }
                   if (num!=0)
		   {  
		     if(FF_game==FACTROIDS_GAME)
		     {
                        if(((asteroid[k].fact_number%num)==0) && (num!=asteroid[k].fact_number))
		        {
		          isdead=1;
		          xdead=asteroid[k].x;
		          ydead=asteroid[k].y;

		          if(tuxship.x<asteroid[k].x)
		            FF_destroy_asteroid(k, m, m);
		          if(tuxship.x>=asteroid[k].x)
		            FF_destroy_asteroid(k, -m, m);
		          /*
			  if(tuxship.x<asteroid[k].x)
			     if(tuxship.y<asteroid[k].y)
		                FF_destroy_asteroid(k, 2, -2);
			     else if(tuxship.y>asteroid[k].y)
				FF_destroy_asteroid(k, 2, 2);
			  else if (tuxship.x>asteroid[k].x)
			     if(tuxship.y<asteroid[k].y)
		                FF_destroy_asteroid(k, -2, -2);
			     else if(tuxship.y>asteroid[k].y)
				FF_destroy_asteroid(k, -2, 2);*/
		          score=score+num;
		          //laser[i].destx=xcount;
		          //laser[i].desty=ycount;
		          return 1;
			}
		     }
		     else if (FF_game==FRACTIONS_GAME)
		     {
			if(((asteroid[k].a%num)==0) &&
			   ((asteroid[k].a%num)==0) &&
			   (num!=asteroid[k].fact_number))
		        {
			  isdead=1;
		          xdead=asteroid[k].x;
		          ydead=asteroid[k].y;
		          if(tuxship.x<asteroid[k].x)
		            FF_destroy_asteroid(k, m, m);
		          if(tuxship.x>=asteroid[k].x)
		            FF_destroy_asteroid(k, -m, m);
		          score=score+num;
		          laser[i].destx=xcount;
		          laser[i].desty=ycount;
		          return 1; 
			}  
		     }
		     
		   }
	         } 
               }
	    }
	  }
	  else
	  {
 	   while (ycount != laser[i].desty)
	   {
   	     if (laser[i].desty > ycount) dy = 1;
	     else dy = -1;
             ycount=ycount+dy;
  	     for(k=0; k<MAX_ASTEROIDS; k++)
	     {
                 if(xcount<asteroid[k].x+70 && 
                   xcount>asteroid[k].x && 
                   ycount<asteroid[k].y+70 && 
                   ycount>asteroid[k].y &&
                   asteroid[k].alive)
	         {

                   if(asteroid[k].isprime)
 		   {
		     isdead=1;
		     xdead=asteroid[k].x;
		     ydead=asteroid[k].y;
		     if(tuxship.x<asteroid[k].x)
		       FF_destroy_asteroid(k, m, m);
		     if(tuxship.x>=asteroid[k].x)
		       FF_destroy_asteroid(k, -m, m);

		     //laser[i].destx=xcount;
		     //laser[i].desty=ycount;
		     return 1;
		   }

		  if(FF_game==FACTROIDS_GAME)
		  {


                  if (num!=0)
		  {  
		   if(((asteroid[k].fact_number%num)==0) && (num!=asteroid[k].fact_number))
		   {

		     isdead=1;
		     xdead=asteroid[k].x;
		     ydead=asteroid[k].y;
		     if(tuxship.x<asteroid[k].x)
		       FF_destroy_asteroid(k, m, m);
		     if(tuxship.x>=asteroid[k].x)
		       FF_destroy_asteroid(k, -m, m);
		     score=score+num;
		     //laser[i].destx=xcount;
		     //laser[i].desty=ycount;
		     return 1;
		   }
                  }
	        }
		else if (FF_game==FRACTIONS_GAME)
		{
                  if (num!=0)
		  {  
		    if(((asteroid[k].a%num)==0) &&
		       ((asteroid[k].a%num)==0) &&
		       (num!=asteroid[k].fact_number))
		    {

		     isdead=1;
		     xdead=asteroid[k].x;
		     ydead=asteroid[k].y;
		     if(tuxship.x<asteroid[k].x)
		       FF_destroy_asteroid(k, m, m);
		     if(tuxship.x>=asteroid[k].x)
		       FF_destroy_asteroid(k, -m, m);
		     score=score+num;
		     //laser[i].destx=xcount;
		     //laser[i].desty=ycount;
		     return 1;
		   }
                  }
		}
	      }
            }
	  }
      }

      return 1;
    }
   }
  }
  fprintf(stderr, "Laser could't be created!\n");
  return -1;
}

/******************* ASTEROIDS FUNCTIONS *******************/



static int FF_add_asteroid(int x, int y, int xspeed, int yspeed, int size, int angle, int
                           angle_speed, int fact_number, int a, int b, int new_wave)
{
  int i;
  for(i=0; i<MAX_ASTEROIDS; i++){
    if(asteroid[i].alive==0)
    {
      asteroid[i].alive=1;
      asteroid[i].x=x;
      asteroid[i].y=y;
      asteroid[i].xspeed=xspeed;
      asteroid[i].yspeed=yspeed;
      asteroid[i].angle=angle;
      asteroid[i].angle_speed=angle_speed;
      
      if(FF_game==FACTROIDS_GAME){

         asteroid[i].fact_number=fact_number;

  	 while(!asteroid[i].fact_number)
	   asteroid[i].fact_number=rand()%80;

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
	       asteroid[i].x=asteroid[i].x+300;
	       asteroid[i].y=asteroid[i].y+300;
	    }
      }

      if(asteroid[i].isprime)
      {
        asteroid[i].size=0;
        asteroid[i].centerx=x+30;
        asteroid[i].centery=y+30;
      }
      else if(!asteroid[i].isprime)
      {
        asteroid[i].size=1;
        asteroid[i].centerx=x+40;
        asteroid[i].centery=y+40;
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

int FF_destroy_asteroid(int i, int xspeed, int yspeed)
{
  if(asteroid[i].alive==1){
     if(asteroid[i].size>0){
      /* Break the rock into two smaller ones! */
      if(num!=0){
        if(FF_game==FACTROIDS_GAME){
          FF_add_asteroid(asteroid[i].x,
	  	          asteroid[i].y,
	  	          ((asteroid[i].xspeed + xspeed) / 2),
	  	          (asteroid[i].yspeed + yspeed),
	  	          0,
	  	          0, 0, (int)(asteroid[i].fact_number/num),
		          0, 0,
                          0);
      
          FF_add_asteroid(asteroid[i].x,
	  	          asteroid[i].y,
	  	          (asteroid[i].xspeed + xspeed),
	  	          ((asteroid[i].yspeed + yspeed) / 2),
	  	          0,
	  	          0, 0, num,
                          0, 0,
                          0);
        }
        else if(FF_game==FRACTIONS_GAME){
          FF_add_asteroid(asteroid[i].x,
	  	          asteroid[i].y,
	  	          ((asteroid[i].xspeed + xspeed) / 2),
	  	          (asteroid[i].yspeed + yspeed),
	  	          0,
	  	          0, 0, 0,
		          (int)(asteroid[i].a/num), (int)(asteroid[i].b/num),
                          0);
      
          FF_add_asteroid(asteroid[i].x,
	  	          asteroid[i].y,
	  	          (asteroid[i].xspeed + xspeed),
	  	          ((asteroid[i].yspeed + yspeed) / 2),
	  	          0,
	  	          0, 0, 0,
                          (int)(asteroid[i].b/num), (int)(asteroid[i].a/num),
                          0); 
	}
      } 
    }

    /* Destroy the old asteroid */

    asteroid[i].alive=0;		  
    playsound(SND_EXPLOSION);
    return 1;
  }
  return 0;
}

/************** MODIFIED FUNCS FROM game.c ******************/


void game_handle_user_events(void)
{
  SDL_Event event;
  SDLKey key;

  while (SDL_PollEvent(&event) > 0)
  {
    if (event.type == SDL_QUIT)
    {
      SDL_quit_received = 1;
      done = 1;
      quit = 1;
    }
    if (event.type == SDL_MOUSEBUTTONDOWN)
    {
      key=game_mouse_event(event);
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
	            done = 1;
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
  /* The rest of the keys control the numeric answer console: */
	      
  if (key >= SDLK_0 && key <= SDLK_9)
  {
    /* [0]-[9]: Add a new digit: */
    digits[0] = digits[1];
    digits[1] = digits[2];
    digits[2] = key - SDLK_0;
    tux_pressing = 1;
    playsound(SND_SHIELDSDOWN);
  }
  else if (key >= SDLK_KP0 && key <= SDLK_KP9)
  {
    /* Keypad [0]-[9]: Add a new digit: */
    digits[0] = digits[1];
    digits[1] = digits[2];
    digits[2] = key - SDLK_KP0;
    tux_pressing = 1;
    playsound(SND_SHIELDSDOWN);
  }
  /* support for negative answer input DSB */
  else if ((key == SDLK_MINUS || key == SDLK_KP_MINUS)
        && MC_AllowNegatives())  /* do nothing unless neg answers allowed */
  {
    /* allow player to make answer negative: */
    neg_answer_picked = 1;
    tux_pressing = 1;
    playsound(SND_SHIELDSDOWN);
  }
  else if ((key == SDLK_PLUS || key == SDLK_KP_PLUS)
         && MC_AllowNegatives())  /* do nothing unless neg answers allowed */
  {
    /* allow player to make answer positive: */
    neg_answer_picked = 0;
    tux_pressing = 1;
    playsound(SND_SHIELDSDOWN);
  }
  else if (key == SDLK_BACKSPACE ||
           key == SDLK_CLEAR ||
	   key == SDLK_DELETE)
  {
    /* [BKSP]: Clear digits! */
    digits[0] = 0;
    digits[1] = 0;
    digits[2] = 0;
    tux_pressing = 1;
    playsound(SND_SHIELDSDOWN);
  }
 	else if (key == SDLK_RETURN ||
        	   key == SDLK_KP_ENTER ||
		   key == SDLK_SPACE)
 	 {
	       shoot_pressed = 1;
               doing_answer = 1;
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
  
}

int game_mouse_event(SDL_Event event)
{
  int keypad_w, keypad_h, x, y, row, column;
  SDLKey key = SDLK_UNKNOWN;

  keypad_w = 0;
  keypad_h = 0;

  /* Check to see if user clicked exit button: */
  /* The exit button is in the upper right corner of the screen: */
  if ((event.button.x >= (screen->w - images[IMG_STOP]->w))
    &&(event.button.y <= images[IMG_STOP]->h))
  {
    key = SDLK_ESCAPE;
    //game_key_event(key);
    escape_received = 1;
    done = 1;
    quit = 1;
    return -1;
  } 

  /* get out unless we really are using keypad */
  if ( level_start_wait 
    || Opts_DemoMode()
    || !Opts_UseKeypad())
  {
    return -1;
  }


  /* make sure keypad image is valid and has non-zero dimensions: */
  /* FIXME maybe this checking should be done once at the start */
  /* of game() rather than with every mouse click */
  if (MC_AllowNegatives())
  {
    if (!images[IMG_KEYPAD])
      return -1;
    else
    {
      keypad_w = images[IMG_KEYPAD]->w;
      keypad_h = images[IMG_KEYPAD]->h;
    }
  }
  else
  {
    if (!images[IMG_KEYPAD_NO_NEG])
      return -1;
    else
    {
      keypad_w = images[IMG_KEYPAD]->w;
      keypad_h = images[IMG_KEYPAD]->h;
    }
  }
 
  if (!keypad_w || !keypad_h)
  {
    return -1;
  }

  
  /* only proceed if click falls within keypad: */
  if (!((event.button.x >=
        (screen->w / 2) - (keypad_w / 2) &&
        event.button.x <=
        (screen->w / 2) + (keypad_w / 2) &&
        event.button.y >= 
        (screen->h / 2) - (keypad_h / 2) &&
        event.button.y <=
        (screen->h / 2) + (keypad_h / 2))))
  /* click outside of keypad - do nothing */
  {
    return -1;
  }
  
  else /* click was within keypad */ 
  {
    x = (event.button.x - ((screen->w / 2) - (keypad_w / 2)));
    y = (event.button.y - ((screen->h / 2) - (keypad_h / 2)));
 
  /* Now determine what onscreen key was pressed */
  /*                                             */
  /* The on-screen keypad has a 4 x 4 layout:    */
  /*                                             */
  /*    *********************************        */
  /*    *       *       *       *       *        */
  /*    *   7   *   8   *   9   *   -   *        */
  /*    *       *       *       *       *        */
  /*    *********************************        */
  /*    *       *       *       *       *        */
  /*    *   4   *   5   *   6   *       *        */
  /*    *       *       *       *       *        */
  /*    *************************   +   *        */
  /*    *       *       *       *       *        */
  /*    *   1   *   2   *   3   *       *        */
  /*    *       *       *       *       *        */
  /*    *********************************        */
  /*    *       *                       *        */
  /*    *   0   *         Enter         *        */
  /*    *       *                       *        */
  /*    *********************************        */
  /*                                             */
  /*  The following code simply figures out the  */
  /*  row and column based on x and y and looks  */
  /*  up the SDlKey accordingly.                 */

    column = x/((keypad_w)/4);
    row    = y/((keypad_h)/4);

    /* make sure row and column are sane */
    if (column < 0
     || column > 3
     || row    < 0
     || row    > 3)
    {
      printf("\nIllegal row or column value!\n");
      return -1;
    }

    /* simple but tedious - I am sure this could be done more elegantly */

    if (0 == row)
    {
      if (0 == column)
        key = SDLK_7;
      if (1 == column)
        key = SDLK_8;
      if (2 == column)
        key = SDLK_9;
      if (3 == column)
        key = SDLK_MINUS;
    } 
    if (1 == row)
    {
      if (0 == column)
        key = SDLK_4;
      if (1 == column)
        key = SDLK_5;
      if (2 == column)
        key = SDLK_6;
      if (3 == column)
        key = SDLK_PLUS;
    }     
    if (2 == row)
    {
      if (0 == column)
        key = SDLK_1;
      if (1 == column)
        key = SDLK_2;
      if (2 == column)
        key = SDLK_3;
      if (3 == column)
        key = SDLK_PLUS;
    } 
    if (3 == row)
    {
      if (0 == column)
        key = SDLK_0;
      if (1 == column)
        key = SDLK_RETURN;
      if (2 == column)
        key = SDLK_RETURN;
      if (3 == column)
        key = SDLK_RETURN;
    }     

    if (key == SDLK_UNKNOWN)
    {
      return -1;
    }

    /* now can proceed as if keyboard was used */
    //game_key_event(key);
    return key;
  }
}


int check_exit_conditions(void)
{
  if(SDL_quit_received)
  {
    return GAME_OVER_WINDOW_CLOSE;
  }

  if(escape_received)
  {
    return GAME_OVER_ESCAPE;
  }
  if(tuxship.lives<=0)
  {
     GAME_OVER_LOST;
  }
  /* determine if game lost (i.e. all cities blown up): */
  /*if (!num_cities_alive)
  {
    if (gameover_counter < 0)
      gameover_counter = GAMEOVER_COUNTER_START;
    gameover_counter--;
    if (gameover_counter == 0)
      return GAME_OVER_LOST;
  }*/
  
  /* determine if game won (i.e. all questions in mission answered correctly): */
  /*if (MC_MissionAccomplished())
  {
    return GAME_OVER_WON;
  }*/

  /* Could have situation where mathcards doesn't have more questions */
  /* even though not all questions answered correctly:                */
  /*if (!MC_TotalQuestionsLeft())
  {
    return GAME_OVER_OTHER;
  }*/

  /* Need to get out if no comets alive and MathCards has no questions left in list, */
  /* even though MathCards thinks there are still questions "in play".  */
  /* This SHOULD NOT HAPPEN and means we have a bug somewhere. */
 /* if (!MC_ListQuestionsLeft() && !num_comets_alive)
  {
    #ifdef TUXMATH_DEBUG
    printf("\nListQuestionsLeft() = %d", MC_ListQuestionsLeft());
    printf("\nnum_comets_alive = %d", num_comets_alive);
    #endif 
    return GAME_OVER_ERROR;
  }
  */
  /* If using demo mode, see if counter has run out: */ 
  /*if (Opts_DemoMode())
  {
    if (demo_countdown <= 0 )
      return GAME_OVER_OTHER;
  }*/

  /* if we made it to here, the game goes on! */
  return GAME_IN_PROGRESS;
}
