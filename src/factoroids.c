/************************************************************
 *  factoroids.c                                             *
 *                                                          *
 *  Description:  Code for the factor and fraction activity *
 *                                                          *
 *  Author:       Jesus M. Mager H. (fongog@gmail.com) 2008 *
 *  Copyright:    GPL v3 or later                           *
 *  							    *
 *  Also significantly enhanced by Tim Holy - 2008          *
 *                                                          *
 *  Code based on the work made by:                         *
 *               Bill Kendrick (vectoroids 1.1.0)           *
 *               and Bill Kendrick, David Bruce, Tim Holy   *
 *               and others (Tuxmath 1.6.3)                 *
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
#include "SDL_rotozoom.h"
#include "SDL_extras.h"

#include "game.h"
#include "fileops.h"
#include "setup.h"
#include "mathcards.h"
#include "loaders.h"
#include "titlescreen.h"
#include "options.h"

#define FPS 15                     /* 15 frames per second */
#define MS_PER_FRAME (1000 / FPS)
#define BASE_RES_X 1280 

#define MAX_LASER 5
#define MAX_ASTEROIDS 50
#define NUM_TUXSHIPS 2
#define NUM_SPRITES 11
#define TUXSHIP_LIVES 3
#define DEG_PER_ROTATION 2
#define NUM_OF_ROTO_IMGS 360/DEG_PER_ROTATION
/* TUXSHIP_DECEL controls "friction" - 1 means ship glides infinitely, 0 stops it instantly */
#define TUXSHIP_DECEL 0.8
#define DEG_TO_RAD 0.0174532925
#define MAX(a,b)           (((a) > (b)) ? (a) : (b))

/********* Enumerations ***********/

enum{
  FACTOROIDS_GAME,
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
  int rx, ry;
  int centerx, centery;
  int radius;
  int fact_number;
  int isprime;
  int a, b; /*  a / b */
  int count;
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

// ControlKeys
static int left_pressed;
static int right_pressed;
static int up_pressed;
static int shift_pressed;
static int shoot_pressed;

// GameControl
static int game_status;
//static int gameover_counter;
static int escape_received;

//SDL_Surfaces:
static SDL_Surface* IMG_tuxship[NUM_OF_ROTO_IMGS];
static SDL_Surface* IMG_asteroids1[NUM_OF_ROTO_IMGS];
static SDL_Surface* IMG_asteroids2[NUM_OF_ROTO_IMGS];
static SDL_Surface* bkgd = NULL; //640x480 background (windowed)
static SDL_Surface* scaled_bkgd = NULL; //native resolution (fullscreen)


// Game type
static int FF_game;

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

static int neg_answer_picked;
static int tux_pressing;
static int doing_answer;
static int level_start_wait;
static int tux_img;
//static int FF_level;

static asteroid_type* asteroid = NULL;
static tuxship_type tuxship;
static FF_laser_type laser[MAX_LASER];

static int NUM_ASTEROIDS;
static int counter;
static int xdead, ydead, isdead, countdead;
static int roto_speed;

/*************** The Factor and Faraction Activiy game Functions ***************/

/* Local function prototypes: */

static int FF_init(void);
static void FF_intro(void);

static void FF_handle_ship(void);
static void FF_handle_asteroids(void);
static void FF_handle_answer(void);
static int check_exit_conditions(void);
static void FF_draw(void);
static void FF_draw_bkgr(void);
static void FF_draw_led_console(void);
static void draw_console_image(int i);

static SDL_Surface* current_bkgd()
  { return screen->flags & SDL_FULLSCREEN ? scaled_bkgd : bkgd; }

static void FF_add_level(void);
static int FF_over(int game_status);
static void FF_exit_free(void);

static int FF_add_laser(void);
static int FF_add_asteroid(int x, int y, int xspeed, int yspeed, int size, int angle, int 				   angle_speed, int fact_num, int a, int b, int new_wave);
static int FF_destroy_asteroid(int i, float xspeed, float yspeed);

static void FF_ShowMessage(char* str1, char* str2, char* str3, char* str4);

static SDL_Surface* get_asteroid_image(int size,int angle);
static int AsteroidColl(int astW,int astH,int astX,int astY,
                 int x, int y);
static int is_prime(int num);
static int fast_cos(int angle);
static int fast_sin(int angle);
static void game_handle_user_events(void);
static int game_mouse_event(SDL_Event event);

/************** factors(): The factor main function ********************/
void factors(void)
{
  Uint32 last_time, now_time; 
  
  quit = 0;
  counter = 0;
  tux_img = IMG_TUX_CONSOLE1;

  #ifdef TUXMATH_DEBUG
     fprintf(stderr, "Entering factors():\n");
  #endif

  FF_game = FACTOROIDS_GAME;
  
  if (!FF_init())
  {
    fprintf(stderr, "FF_init() failed!\n");
    FF_exit_free();
    return;
  } 

  while (game_status == GAME_IN_PROGRESS)
  {
    last_time = SDL_GetTicks();
    counter++; 
    
    if(counter%15 == 0)
    {
      if(tux_img<IMG_TUX_CONSOLE4)
        tux_img++;
      else 
        tux_img=IMG_TUX_CONSOLE1;
    }

    game_handle_user_events();

    FF_handle_ship();
    FF_handle_asteroids();
    FF_handle_answer();
    FF_draw();
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
  FF_over(game_status);
}


/************** fractions(): The fractions main function ********************/
void fractions(void)
{

  Uint32 last_time, now_time; 
  
  quit = 0;
  counter = 0;
  tux_img = IMG_TUX_CONSOLE1;

  
  #ifdef TUXMATH_DEBUG
     fprintf(stderr, "Entering factors():\n");
  #endif
  
  /*****Initalizing the Factor activiy *****/
  FF_game = FRACTIONS_GAME;

  if (!FF_init())
  {
    fprintf(stderr, "FF_init() failed!\n");
    FF_exit_free();
    return;
  } 

  /************ Main Loop **************/
  while (game_status == GAME_IN_PROGRESS)
  {
    last_time = SDL_GetTicks();
    counter++;
      
    if(counter%15 == 0)
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
    FF_draw();
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
      now_time = (last_time + MS_PER_FRAME) - now_time;  // this holds the delay
      if (now_time > MS_PER_FRAME)
        now_time = MS_PER_FRAME;
      SDL_Delay(now_time);
    }
  }
  FF_over(game_status);
}


/************ Initialize all vars... ****************/
static int FF_init(void)
{
  int i;
  float zoom;

  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
  SDL_Flip(screen);
  
  FF_intro();
  
  if(screen->h < 600 && screen->w < 800)
    zoom = 0.65;
  else
    zoom=(float)screen->w/(float)BASE_RES_X;

  printf("The zoome rate is: %f\n", zoom);

  /*************** Precalculating software rotation ***************/

  for(i = 0; i < NUM_OF_ROTO_IMGS; i++)
  {
    //rotozoomSurface (SDL_Surface *src, double angle, double zoom, int smooth);
    IMG_tuxship[i] = rotozoomSurface(images[IMG_SHIP01], i * DEG_PER_ROTATION, zoom, 1);

    if (IMG_tuxship[i] == NULL)
    {
      fprintf(stderr,
              "\nError: rotozoomSurface() of images[IMG_SHIP01] for i = %d returned NULL\n", i);
      return 0;
    }

    IMG_asteroids1[i] = rotozoomSurface(images[IMG_ASTEROID1], i * DEG_PER_ROTATION, zoom, 1);

    if (IMG_asteroids1[i] == NULL)
    {
      fprintf(stderr,
              "\nError: rotozoomSurface() of images[IMG_ASTEROID1] for i = %d returned NULL\n", i);
      return 0;
    }

    IMG_asteroids2[i] = rotozoomSurface(images[IMG_ASTEROID2], i*DEG_PER_ROTATION, zoom, 1);

    if (IMG_asteroids2[i] == NULL)
    {
      fprintf(stderr,
              "\nError: rotozoomSurface() of images[IMG_ASTEROID2] for i = %d returned NULL\n", i);
      return 0;
    }
  }


  /********   Set up properly scaled and optimized background surfaces: *********/
  /* NOTE - optimization code moved into LoadBothBkgds() so rest of program     */
  /* can take advantage of it - DSB                                             */

  LoadBothBkgds("factoroids/gbstars.png", &scaled_bkgd, &bkgd);

  if (bkgd == NULL || scaled_bkgd == NULL)
  {
    fprintf(stderr,
       "\nError: could not scale background\n");
    return 0;
  }


  // Allocate memory 
  asteroid = NULL;  // set in case allocation fails partway through
  asteroid = (asteroid_type *) malloc(MAX_ASTEROIDS * sizeof(asteroid_type));
  
  if (asteroid == NULL)
  {
    printf("Allocation of asteroids failed");
    return 0;
  }

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

  score = 1;
  wave = 0;
  xdead = 0;
  ydead = 0;
  isdead = 0;
  countdead = 0;
  escape_received = 0;
  game_status = GAME_IN_PROGRESS;

  FF_add_level();

  for (i = 0; i < MAX_LASER; i++)
    laser[i].alive = 0;

  // Wait for click or keypress to start (get out if user presses Esc) :
  while(1)
  {
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT)
    {
      SDL_quit_received = 1;
      quit = 1;
      return 1;
    }
    else if (event.type == SDL_MOUSEBUTTONDOWN)
    {
      return 1;
    }
    else if (event.type == SDL_KEYDOWN)
    {
      if (event.key.keysym.sym == SDLK_ESCAPE)
        escape_received = 1;
      return 1;
    }
  }
}


static void FF_intro(void)
{
  static SDL_Surface* IMG_factors;
  static SDL_Surface* IMG_fractions;

//  SDL_Event event;
  SDL_Rect rect;

  float zoom;

  if(screen->h < 600 && screen->w < 800)
    zoom = 0.65;
  else
    zoom=(float)screen->w/(float)BASE_RES_X;
  
  IMG_factors   = rotozoomSurface(images[IMG_FACTOROIDS], 0, zoom, 1);
  IMG_fractions = rotozoomSurface(images[IMG_FACTORS], 0, zoom, 1);

  FF_draw_bkgr();
  if(FF_game == FACTOROIDS_GAME)
  {

    rect.x = (screen->w/2) - (IMG_factors->w/2);
    rect.y = (screen->h)/7;
    SDL_BlitSurface(IMG_factors, NULL, screen, &rect);
    FF_ShowMessage(_("FACTOROIDS: to win, you need destroy all the asteroids."),
		   _("Use the arrow keys to turn or go forward.  Aim at an asteroid,"),
		   _("type one of its factors, and press space or return"),
		   _("to split it into its factors.  Rocks with prime numbers are destroyed!"));
    SDL_BlitSurface(IMG_asteroids1[3],NULL,screen,&rect);
  }
  else if (FF_game == FRACTIONS_GAME)
  {
    rect.x = (screen->w/2)-(IMG_fractions->w/2);
    rect.y = (screen->h)/7;
    SDL_BlitSurface(IMG_fractions,NULL,screen,&rect);
    FF_ShowMessage(_("FRACTIONS: to win, you need destroy all the asteroids"),
		   _("Use the arrow keys to turn or go forward.  Aim at an asteroid,"),
		   _("type a number that can simplify the fraction, and press space or return"),
		   _("to split it.  Destroy fractions that can not be further simplified in a single shot!"));
  }

  SDL_FreeSurface(IMG_factors);
  SDL_FreeSurface(IMG_fractions);
}

static void FF_handle_ship(void)
{
//FIXME - am I missing something -- doesn't this just reduce to 
//"tuxship.centerx = tuxship.x" and likewise for y???
/****************** Ship center... ******************/

  tuxship.centerx = ((IMG_tuxship[tuxship.angle/DEG_PER_ROTATION]->w)/2) + 
        (tuxship.x - (IMG_tuxship[tuxship.angle/DEG_PER_ROTATION]->w/2));
  tuxship.centery = ((IMG_tuxship[tuxship.angle/DEG_PER_ROTATION]->h)/2) + 
        (tuxship.y - (IMG_tuxship[tuxship.angle/DEG_PER_ROTATION]->h/2));  

/******************* Ship live *********************/
  
  if(tuxship.hurt)
  {
    tuxship.hurt_count--;
    if(tuxship.hurt_count <= 0)
	tuxship.hurt = 0;
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
    tuxship.angle = tuxship.angle - DEG_PER_ROTATION * roto_speed;
    if (tuxship.angle < 0)
      tuxship.angle = tuxship.angle + 360;

    tuxship.x1= fast_cos(DEG_PER_ROTATION*-roto_speed) * tuxship.centerx
               -fast_sin(DEG_PER_ROTATION*-roto_speed) * tuxship.centery;
    tuxship.y1= fast_sin(DEG_PER_ROTATION*-roto_speed) * tuxship.centerx
               +fast_cos(DEG_PER_ROTATION*-roto_speed) * tuxship.centery;

  }
  else if (left_pressed)
  {
    tuxship.angle=tuxship.angle + DEG_PER_ROTATION * roto_speed;
    if (tuxship.angle >= 360)
      tuxship.angle = tuxship.angle - 360;

    tuxship.x1= fast_cos(DEG_PER_ROTATION*roto_speed) * tuxship.centerx
               -fast_sin(DEG_PER_ROTATION*roto_speed) * tuxship.centery;
    tuxship.y1= fast_sin(DEG_PER_ROTATION*roto_speed * tuxship.centerx
               +fast_cos(DEG_PER_ROTATION*roto_speed)) * tuxship.centery;

  }

/**************** Move, and increse speed ***************/

      
  if (up_pressed && (tuxship.lives > 0))
  {
     tuxship.xspeed = tuxship.xspeed + ((fast_cos(tuxship.angle >> 3) * 3) >> 10);
     tuxship.yspeed = tuxship.yspeed - ((fast_sin(tuxship.angle >> 3) * 3) >> 10);
  }
  else
  {
    if ((counter % 2) == 0)
    {
       tuxship.xspeed = tuxship.xspeed * TUXSHIP_DECEL; 
       tuxship.yspeed = tuxship.yspeed * TUXSHIP_DECEL;
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

  SDL_Surface* surf;
  int i, found=0;
      for (i = 0; i < MAX_ASTEROIDS; i++){
	  if (asteroid[i].alive)
	    {

	      found=1;

	      /*************** Rotate asteroid ****************/ 
	      
	      asteroid[i].angle = (asteroid[i].angle + asteroid[i].angle_speed);

	      // Wrap rotation angle... 
	      
	      if (asteroid[i].angle < 0)
		asteroid[i].angle = asteroid[i].angle + 360;
	      else if (asteroid[i].angle >= 360)
		asteroid[i].angle = asteroid[i].angle - 360;

             /**************Move the astroids ****************/ 
	      surf=get_asteroid_image(asteroid[i].size,asteroid[i].angle);

	      asteroid[i].rx = asteroid[i].rx + asteroid[i].xspeed;
	      asteroid[i].ry = asteroid[i].ry + asteroid[i].yspeed;

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

              if(AsteroidColl(surf->w, surf->h, asteroid[i].x, asteroid[i].y, tuxship.centerx, tuxship.centery))
	      {
		if(!tuxship.hurt)
		{
		  xdead=asteroid[i].centerx;
		  ydead=asteroid[i].centery;
		     
		  tuxship.lives--;
		  tuxship.hurt=1;
		  tuxship.hurt_count=50;
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
  
    /* Clear digits: */
  digits[0] = 0;
  digits[1] = 0;
  digits[2] = 0;
  neg_answer_picked = 0;

}

static SDL_Surface* get_asteroid_image(int size,int angle)
{
  if (size == 0)
    return IMG_asteroids1[angle/DEG_PER_ROTATION];
  else
    return IMG_asteroids2[angle/DEG_PER_ROTATION];
}

static void FF_draw(void){

  int i, offset;
  int xnum, ynum;
  char str[64];
  SDL_Surface* surf;
  SDL_Rect dest;
  
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

  /************ Draw Background ***************/ 

  FF_draw_bkgr();

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

  if(!tuxship.hurt || (tuxship.hurt && tuxship.hurt_count%2==0)){
     dest.x = (tuxship.x - (IMG_tuxship[tuxship.angle/DEG_PER_ROTATION]->w/2));
     dest.y = (tuxship.y - (IMG_tuxship[tuxship.angle/DEG_PER_ROTATION]->h/2));
     dest.w = IMG_tuxship[tuxship.angle/DEG_PER_ROTATION]->w;
     dest.h = IMG_tuxship[tuxship.angle/DEG_PER_ROTATION]->h;
	
     SDL_BlitSurface(IMG_tuxship[tuxship.angle/DEG_PER_ROTATION], NULL, screen, &dest);
  }
  /************* Draw Asteroids ***************/
  for(i=0; i<MAX_ASTEROIDS; i++){
    if(asteroid[i].alive>0){
     
     xnum=0;
     ynum=0;

     dest.x = asteroid[i].x;
     dest.y = asteroid[i].y; 

     surf=get_asteroid_image(asteroid[i].size,asteroid[i].angle);

     dest.w = surf->w;
     dest.h = surf->h;

     SDL_BlitSurface(surf, NULL, screen, &dest);

     // Wrap the numbers of the asteroids
     if((asteroid[i].centery)>23 && (asteroid[i].centery)<screen->h)
     {
       if((asteroid[i].centerx)>0 && (asteroid[i].centerx)<screen->w)
       {
         xnum=asteroid[i].centerx-3;
         ynum=asteroid[i].centery;
       }
       else if((asteroid[i].centerx)<=0){
         xnum=20;
         ynum=asteroid[i].centery;
       }
       else if((asteroid[i].centerx)<=screen->w){
         xnum=screen->w-20;
         ynum=asteroid[i].centery;
       }
     }
     else if((asteroid[i].centery)<=23)
     {
       xnum=asteroid[i].centerx;
       ynum=23;
     }
     else if((asteroid[i].centery)>=screen->h)
     {
       xnum=asteroid[i].centerx;
       ynum=screen->h-7;
     }

     //Draw Numbers
     if(FF_game==FACTOROIDS_GAME)
     {   
       sprintf(str, "%.1d", asteroid[i].fact_number);
       draw_nums(str, xnum, ynum);
     }
     else if (FF_game==FRACTIONS_GAME)
     {
       sprintf(str, "%d", asteroid[i].a);
       draw_nums(str, xnum, ynum); 
       draw_line(xnum, ynum + 4, xnum + 30, ynum + 4,
		 255, 255, 255);
       sprintf(str, "%d", asteroid[i].b);
       draw_nums(str, xnum, ynum + 35);
     }
    }
  }
  /*************** Draw Steam ***************/
  
  if(isdead)
  {
    dest.x = xdead;
    dest.y = ydead;
    SDL_BlitSurface(images[IMG_STEAM1+countdead], NULL, screen, &dest);
    countdead++;
    if(countdead > 5)
    {
      isdead = 0;
      countdead = 0;
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
  dest.x = (screen->w - ((images[IMG_NUMBERS]->w/10) * 7) -
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

   
  if(screen->w < 800 && screen->h < 600)
  {
    sprintf(str, "%.3d", num);
    draw_numbers(str, ((screen->w)/2) - 50, (screen->h) - 30);
  } 
  else
  {
    FF_draw_led_console();
    draw_console_image(tux_img);
  }

  /************** Draw lives ***************/
  dest.y = screen->h;
  dest.x = 0;

  for(i = 1; i <= tuxship.lives; i++)
  {
    if(tuxship.lives <= 5)
    {
      dest.y = dest.y - (images[IMG_TUX_LITTLE]->h);
      SDL_BlitSurface(images[IMG_TUX_LITTLE], NULL, screen, &dest);
    }
    else if(tuxship.lives > 4)
    {
      dest.y = screen->h - (images[IMG_TUX_LITTLE]->h);
      SDL_BlitSurface(images[IMG_TUX_LITTLE], NULL, screen, &dest);
      sprintf(str, "%d", tuxship.lives);
      draw_numbers(str, 10, (screen->h) - 30); 
    }
  }
}

/*Modified from game.c*/
void FF_draw_led_console(void)
{
  int i;
  SDL_Rect src, dest;
  int y;

  /* draw new console image with "monitor" for LED numbers: */
  draw_console_image(IMG_CONSOLE_LED);
  /* set y to draw LED numbers into Tux's "monitor": */
  y = (screen->h
     - images[IMG_CONSOLE_LED]->h
     + 4);  /* "monitor" has 4 pixel margin */

  /* begin drawing so as to center display depending on whether minus */
  /* sign needed (4 digit slots) or not (3 digit slots) DSB */
  if (MC_GetOpt(ALLOW_NEGATIVES) )
    dest.x = ((screen->w - ((images[IMG_LEDNUMS]->w) / 10) * 4) / 2);
  else
    dest.x = ((screen->w - ((images[IMG_LEDNUMS]->w) / 10) * 3) / 2);

  for (i = -1; i < MC_MAX_DIGITS; i++) /* -1 is special case to allow minus sign */
                              /* with minimal modification of existing code DSB */
  {
    if (-1 == i)
    {
      if (MC_GetOpt(ALLOW_NEGATIVES))
      {
        if (neg_answer_picked)
          src.x =  (images[IMG_LED_NEG_SIGN]->w) / 2;
        else
          src.x = 0;

        src.y = 0;
        src.w = (images[IMG_LED_NEG_SIGN]->w) / 2;
        src.h = images[IMG_LED_NEG_SIGN]->h;

        dest.y = y;
        dest.w = src.w;
        dest.h = src.h;

        SDL_BlitSurface(images[IMG_LED_NEG_SIGN], &src, screen, &dest);
        /* move "cursor" */
        dest.x += src.w;
      }
    }
    else
    {
      src.x = digits[i] * ((images[IMG_LEDNUMS]->w) / 10);
      src.y = 0;
      src.w = (images[IMG_LEDNUMS]->w) / 10;
      src.h = images[IMG_LEDNUMS]->h;

      /* dest.x already set */
      dest.y = y;
      dest.w = src.w;
      dest.h = src.h;

      SDL_BlitSurface(images[IMG_LEDNUMS], &src, screen, &dest);
      /* move "cursor" */
      dest.x += src.w;
    }
  }
}

/* Draw image at lower center of screen: */
void draw_console_image(int i)
{
  SDL_Rect dest;

  dest.x = (screen->w - images[i]->w) / 2;
  dest.y = (screen->h - images[i]->h);
  dest.w = images[i]->w;
  dest.h = images[i]->h;

  SDL_BlitSurface(images[i], NULL, screen, &dest);
}

static void FF_draw_bkgr(void)
{

  SDL_BlitSurface(current_bkgd(), NULL, screen, NULL);
  //if(bgSrc.y>bkg_h)
  //  SDL_BlitSurface(images[BG_STARS], NULL, screen, &bgScreen);

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
  Uint32 now_time, last_time;
  SDL_Rect rect;

  last_time = now_time = SDL_GetTicks();

  wave++;
  
  // New lives per wave!
  if (wave%5==0)
  {
    tuxship.lives++;
  }
  
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
  max_speed = width/100;
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
   //int FF_add_asteroid(int x, int y, int xspeed, int yspeed, int size, int angle, int angle_speed, int fact_number, int a, int b, int new_wave)
   if(FF_game == FACTOROIDS_GAME){
     FF_add_asteroid(x,y,
		    xvel,yvel,
		    rand()%2,
		    rand()%360, rand()%3,
		    (rand()%(31+(wave*wave))), 
		    0, 0,
		    1);
   }
   else if(FF_game==FRACTIONS_GAME){
     FF_add_asteroid(x,y,
		     xvel,yvel,
                     rand()%2, 
		     rand()%360, rand()%3,
                     0, 
		     (rand()%(31+(wave*2))), (rand()%(80+(wave*wave))),
		     1);
   }
  }

  if(wave != 1)
  {
    while(i < 35)
    {
      i++;
      rect.x=(screen->w/2)-(images[IMG_GOOD]->w/2);
      rect.y=(screen->h/2)-(images[IMG_GOOD]->h/2);
      FF_draw();
      SDL_BlitSurface(images[IMG_GOOD],NULL,screen,&rect);
      SDL_Flip(screen);

      last_time = now_time;
      now_time = SDL_GetTicks();

      if (now_time < last_time + MS_PER_FRAME)
      {
        now_time = (last_time + MS_PER_FRAME) - now_time;  // this holds the delay
        if (now_time > MS_PER_FRAME)
 	  now_time = MS_PER_FRAME;
        SDL_Delay(now_time);
      }
    }
  }
}

static int FF_over(int game_status)
{
  Uint32 last_time, now_time; 
  SDL_Rect dest_message;
  SDL_Event event;

#ifdef TUXMATH_DEBUG
  //print_exit_conditions();
#endif

  /* TODO: need better "victory" screen with animation, special music, etc., */
  /* as well as options to review missed questions, play again using missed  */
  /* questions as question list, etc.                                        */
  switch (game_status)
  {
    case GAME_OVER_WON:
    {
      int looping = 1;
//      int frame;
      /* set up victory message: */
      dest_message.x = (screen->w - images[IMG_GAMEOVER_WON]->w) / 2;
      dest_message.y = (screen->h - images[IMG_GAMEOVER_WON]->h) / 2;
      dest_message.w = images[IMG_GAMEOVER_WON]->w;
      dest_message.h = images[IMG_GAMEOVER_WON]->h;

      do
      {
        //frame++;
        last_time = SDL_GetTicks();

        /* draw flashing victory message: */
        //if (((frame / 2) % 4))
        //{
          SDL_BlitSurface(images[IMG_GAMEOVER_WON], NULL, screen, &dest_message);
        //}


        SDL_Flip(screen);

        while (1)
        {
	  SDL_PollEvent(&event);
          if  (event.type == SDL_QUIT
            || event.type == SDL_KEYDOWN
            || event.type == SDL_MOUSEBUTTONDOWN)
          {
            looping = 0;
	    break;
          }
        }

        now_time = SDL_GetTicks();

        if (now_time < last_time + MS_PER_FRAME)
	  SDL_Delay(last_time + MS_PER_FRAME - now_time);
      }
      while (looping);
      break;
    }

    case GAME_OVER_ERROR:
    {
#ifdef TUXMATH_DEBUG
      printf("\ngame() exiting with error");
#endif
    }
    case GAME_OVER_LOST:
    case GAME_OVER_OTHER:
    {
      int looping = 1;

      /* set up GAMEOVER message: */
      dest_message.x = (screen->w - images[IMG_GAMEOVER]->w) / 2;
      dest_message.y = (screen->h - images[IMG_GAMEOVER]->h) / 2;
      dest_message.w = images[IMG_GAMEOVER]->w;
      dest_message.h = images[IMG_GAMEOVER]->h;

      do
      {
        //frame++;
        last_time = SDL_GetTicks();

        SDL_BlitSurface(images[IMG_GAMEOVER], NULL, screen, &dest_message);
        SDL_Flip(screen);

        while (1)
        {
	  SDL_PollEvent(&event);
          if  (event.type == SDL_QUIT
            || event.type == SDL_KEYDOWN
            || event.type == SDL_MOUSEBUTTONDOWN)
          {
            looping = 0;
	    break;
          }
        }

        now_time = SDL_GetTicks();

        if (now_time < last_time + MS_PER_FRAME)
	  SDL_Delay(last_time + MS_PER_FRAME - now_time);
      }
      while (looping);

      break;
    }

    case GAME_OVER_ESCAPE:
    {
      break;
    }

    case GAME_OVER_WINDOW_CLOSE:
    {
      break;
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
  int i = 0;

  free(asteroid);

  for(i = 0; i < NUM_OF_ROTO_IMGS; i++)
  {
    if (IMG_tuxship[i])
    {
      SDL_FreeSurface(IMG_tuxship[i]);
      IMG_tuxship[i] = NULL;
    }
    if (IMG_asteroids1[i])
    {
      SDL_FreeSurface(IMG_asteroids1[i]);
      IMG_asteroids1[i] = NULL;
    }
    if (IMG_asteroids2[i])
    {
      SDL_FreeSurface(IMG_asteroids2[i]);
      IMG_asteroids2[i] = NULL;
    }
  }

//  SDL_FreeSurface(*IMG_asteroids1);
//  SDL_FreeSurface(*IMG_asteroids2);
//  SDL_FreeSurface(*IMG_tuxship);

  if (bkgd)
  {
    SDL_FreeSurface(bkgd);
    bkgd = NULL;
  }
  if (scaled_bkgd)
  {
    SDL_FreeSurface(scaled_bkgd);
    scaled_bkgd = NULL;
  }
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

/******************* LASER FUNCTIONS *********************/

/*Return -1 if no laser is available*/
int FF_add_laser(void)
{
  int i, k, zapIndex;
  float ux, uy, s, smin,dx,dy,dx2, dy2, d2, thresh;
  int screensize;
  SDL_Surface *asteroid_image;

  const float inside_factor = 0.9*0.9;

  screensize = screen->w;
  if (screensize < screen->h)
    screensize = screen->h;

  for(i=0; i<=MAX_LASER; i++)
  {
    if(laser[i].alive==0)
    {
      // Fire the laser
      laser[i].alive=1;
      laser[i].x=tuxship.centerx;
      laser[i].y=tuxship.centery;
      laser[i].angle=tuxship.angle;
      laser[i].count=15;
      
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
	    if((asteroid[k].isprime && ((num==asteroid[k].fact_number)||(num==0))) ||
	       (FF_game==FACTOROIDS_GAME && num > 1 && ((asteroid[k].fact_number%num)==0) && (num!=asteroid[k].fact_number)) ||
	       (FF_game==FRACTIONS_GAME && num > 1 && ((asteroid[k].a%num)==0) && ((asteroid[k].a%num)==0) && (num!=asteroid[k].fact_number)))
	    {
	      // It's valid, check to see if it's closest
	      if (s < smin)
	      {
		// It's the closest yet examined
		smin = s;
		zapIndex = k;
	      }
	    }
	  }
	}
      }
      
      // Handle the destruction, score, and extra lives
      if (zapIndex >= 0)  // did we zap one?
      {
	isdead = 1;
	laser[i].destx = laser[i].x + (int)(ux * smin);
	laser[i].desty = laser[i].y + (int)(uy * smin);
	FF_destroy_asteroid(zapIndex,2*ux,2*uy);
	playsound(SND_SIZZLE);

	if (floor((float)score/100) < floor((float)(score+num)/100))
	  tuxship.lives++;
	score += num;
      }
      return 1;
    }
  }
  fprintf(stderr, "Laser could't be created!\n");
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
      asteroid[i].angle_speed=angle_speed;
      asteroid[i].y=(asteroid[i].ry - (IMG_tuxship[asteroid[i].angle/DEG_PER_ROTATION]->h/2));
      asteroid[i].x=(asteroid[i].rx - (IMG_tuxship[asteroid[i].angle/DEG_PER_ROTATION]->w/2));
      asteroid[i].yspeed=yspeed;
      asteroid[i].xspeed=xspeed;
      
      if(FF_game==FACTOROIDS_GAME){

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
    isdead=1;
    xdead=asteroid[i].x;
    ydead=asteroid[i].y;
     if(asteroid[i].size>0){
      /* Break the rock into two smaller ones! */
      if(num!=0){


//static int FF_add_asteroid(int x, int y, int xspeed, int yspeed, int size, int angle, int
//                           angle_speed, int fact_number, int a, int b, int new_wave

        if(FF_game==FACTOROIDS_GAME){
          FF_add_asteroid(asteroid[i].rx,
	  	          asteroid[i].ry,
	  	          asteroid[i].xspeed + (xspeed - yspeed)/2,
	  	          asteroid[i].yspeed + (yspeed + xspeed)/2,
	  	          0,
	  	          rand()%360, rand()%3, (int)(asteroid[i].fact_number/num),
		          0, 0,
                          0);
      
          FF_add_asteroid(asteroid[i].rx,
	  	          asteroid[i].ry,
	  	          asteroid[i].xspeed + (xspeed + yspeed)/2,
	  	          asteroid[i].yspeed + (yspeed - xspeed)/2,
	  	          0,
	  	          rand()%360, rand()%3, num,
                          0, 0,
                          0);
        }
        else if(FF_game==FRACTIONS_GAME){
          FF_add_asteroid(asteroid[i].rx,
	  	          asteroid[i].ry,
	  	          ((asteroid[i].xspeed + xspeed) / 2),
	  	          (asteroid[i].yspeed + yspeed),
	  	          0,
	  	          rand()%360, rand()%3, 0,
		          (int)(asteroid[i].a/num), (int)(asteroid[i].b/num),
                          0);
      
          FF_add_asteroid(asteroid[i].rx,
	  	          asteroid[i].ry,
	  	          (asteroid[i].xspeed + xspeed),
	  	          ((asteroid[i].yspeed + yspeed) / 2),
	  	          0,
	  	          rand()%360, rand()%3, 0,
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

/************** MODIFIED FUNCS FROM game.c and titlescreen.c ******************/

void FF_ShowMessage(char* str1, char* str2, char* str3, char* str4)
{
  SDL_Surface *s1, *s2, *s3, *s4;
  SDL_Rect loc;

  s1 = s2 = s3 = s4 = NULL;

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "ShowMessage() - creating text\n" );
#endif

  if (str1)
    s1 = BlackOutline(str1, default_font, &white);
  if (str2)
    s2 = BlackOutline(str2, default_font, &white);
  if (str3)
    s3 = BlackOutline(str3, default_font, &white);
  /* When we get going with i18n may need to modify following - see below: */
  if (str4)
    s4 = BlackOutline(str4, default_font, &white);

#ifdef TUXMATH_DEBUG
  fprintf(stderr, "NotImplemented() - drawing screen\n" );
#endif


  /* Draw lines of text (do after drawing Tux so text is in front): */
  if (s1)
  {
    loc.x = (screen->w / 2) - (s1->w/2); 
    loc.y = (screen->h / 2) + 10;
    SDL_BlitSurface( s1, NULL, screen, &loc);
  }
  if (s2)
  {
    loc.x = (screen->w / 2) - (s2->w/2); 
    loc.y = (screen->h / 2) + 80;
    SDL_BlitSurface( s2, NULL, screen, &loc);
  }
  if (s3)
  {
    loc.x = (screen->w / 2) - (s3->w/2); 
    loc.y = (screen->h / 2) + 130;
    SDL_BlitSurface( s3, NULL, screen, &loc);
  }
  if (s4)
  {
    loc.x = (screen->w / 2) - (s4->w/2); 
    loc.y = (screen->h / 2) + 180;
    SDL_BlitSurface( s4, NULL, screen, &loc);
  }

  /* and update: */
  SDL_UpdateRect(screen, 0, 0, 0, 0);


  SDL_FreeSurface(s1);
  SDL_FreeSurface(s2);
  SDL_FreeSurface(s3);
  SDL_FreeSurface(s4);
}


void game_handle_user_events(void)
{
  SDL_Event event;
  SDLKey key;

  while (SDL_PollEvent(&event) > 0)
  {
    if (event.type == SDL_QUIT)
    {
      SDL_quit_received = 1;
      quit = 1;
    }
    if (event.type == SDL_MOUSEBUTTONDOWN)
    {
      key = game_mouse_event(event);
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

static int game_mouse_event(SDL_Event event)
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
    quit = 1;
    return -1;
  } 

  /* get out unless we really are using keypad */
  if ( level_start_wait 
    || Opts_DemoMode()
    || !Opts_GetGlobalOpt(USE_KEYPAD))
  {
    return -1;
  }


  /* make sure keypad image is valid and has non-zero dimensions: */
  /* FIXME maybe this checking should be done once at the start */
  /* of game() rather than with every mouse click */
  if (1)//MC_AllowNegatives())
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


static int check_exit_conditions(void)
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
    return GAME_OVER_LOST;
  }
  if(score>=10000 || wave >= 30 )
  {
    return GAME_OVER_WON;
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
