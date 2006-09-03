/*
  title.c

  For TuxMath
  The title screen function.

  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/


  Part of "Tux4Kids" Project
  http://www.tux4kids.org/
      
  August 26, 2001 - February 21, 2003
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "tuxmath.h"
#include "title.h"
#include "fileops.h"
#include "setup.h"
#include "playsound.h"


#define START_DEMO_COUNTDOWN 150  /* Some time unit.. not sure yet :) */

int title(void)
{
  int i, cmd, old_cmd, done, img, blinking, widest, left, width, demo_countdown;
  SDL_Rect dest;
  SDL_Event event;
  Uint32 last_time, now_time;
  SDLKey key;
  

  /* Determine widest option image size: */

  widest = 0;
  for (i = 0; i < NUM_CMDS; i++)
    {
      if (images[IMG_CMD_PLAY + i]->w > widest)
        widest = images[IMG_CMD_PLAY + i]->w;
    }

  width = widest + 4 + 4 + images[IMG_TUX_HELMET1] -> w;
  left = (screen->w - width) / 2;
  
  
  /* Clear window: */
  
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
  
  
  /* Draw title: */
  
  dest.x = (screen->w - images[IMG_TITLE]->w) / 2;
  dest.y = 0;
  dest.w = images[IMG_TITLE]->w;
  dest.h = images[IMG_TITLE]->h;
  
  SDL_BlitSurface(images[IMG_TITLE], NULL, screen, &dest);
  
  
  /* Draw options: */
  
  for (i = 4; i >= 0; i--)
    {
      // dest.x = (32 - 2) - i;
      dest.x = left + 4 - i;
      dest.y = (images[IMG_TITLE]->h + 2) + (4 - i);
      // dest.w = (screen->w) - ((32 - 2) * 2) + (i * 2);
      dest.w = width + (i * 2);
      dest.h = (NUM_CMDS * images[IMG_TUX_HELMET1]->h + 2) + (i * 2);
      
      SDL_FillRect(screen, &dest,
		   SDL_MapRGB(screen->format,
			      200 - (i * 32),
			      232 - (i * 32),
			      255 - (i * 32)));
    }
  
  
  for (i = 0; i < NUM_CMDS; i++)
    {
      // dest.x = 32 + (images[IMG_TUX_HELMET1]->w + 4);
      dest.x = left + (images[IMG_TUX_HELMET1]->w) + 4;
      dest.y = (images[IMG_TITLE]->h + 8 + 
		(i * images[IMG_TUX_HELMET1]->h));
      dest.w = images[IMG_TUX_HELMET1]->w;
      dest.h = images[IMG_TUX_HELMET1]->h;
      
      SDL_BlitSurface(images[IMG_CMD_PLAY + i], NULL, screen, &dest);
    }
  
  
  /* Draw "Tux4Kids" logo: */
  
  dest.x = (screen->w - images[IMG_TUX4KIDS]->w);
  dest.y = (screen->h - images[IMG_TUX4KIDS]->h);
  dest.w = images[IMG_TUX4KIDS]->w;
  dest.h = images[IMG_TUX4KIDS]->h;
  
  SDL_BlitSurface(images[IMG_TUX4KIDS], NULL, screen, &dest);


  /* Draw "New Breed Software" logo: */

  dest.x = 0;
  dest.y = (screen->h - images[IMG_NBS]->h);
  dest.w = images[IMG_NBS]->w;
  dest.h = images[IMG_NBS]->h;

  SDL_BlitSurface(images[IMG_NBS], NULL, screen, &dest);


  if (game_options->demo_mode)
  {
    dest.x = (screen->w - images[IMG_DEMO_SMALL]->w) / 2;
    dest.y = (screen->h - images[IMG_DEMO_SMALL]->h);
    dest.w = images[IMG_DEMO_SMALL]->w;
    dest.h = images[IMG_DEMO_SMALL]->h;

    SDL_BlitSurface(images[IMG_DEMO_SMALL], NULL, screen, &dest);
  }

  
  /* Flip the screen: */
  
  SDL_Flip(screen);
  
  
  
  /* --- MAIN TITLE SCREEN LOOP: --- */

  blinking = 0;
  cmd = 0;
  done = 0;
  demo_countdown = START_DEMO_COUNTDOWN;
  
  do
    {
      last_time = SDL_GetTicks();
      old_cmd = cmd;
      
      
      /* Handle any incoming events: */
       
      while (SDL_PollEvent(&event) > 0)
	{
	  if (event.type == SDL_QUIT)
	    {
	      /* Window close event - quit! */
	      
	      cmd = CMD_QUIT;
	      done = 1;
	    }
	  else if (event.type == SDL_KEYDOWN)
	    {
	      key = event.key.keysym.sym;
	      
	      if ((key == SDLK_ESCAPE)
                || (key == SDLK_q))
		{
		  /* Escape key or 'Q' - quit! */
		  
		  cmd = CMD_QUIT;
		  done = 1;
		}

	      if (key == SDLK_p)
		{
		  /* 'P'- play! */
		  
		  cmd = CMD_GAME;
		  done = 1;
		}

	      if (key == SDLK_o)
		{
		  /* 'O'- Options! */
		  
		  cmd = CMD_OPTIONS;
		  done = 1;
		}

	      if (key == SDLK_c)
		{
		  /* 'C'- Credits! */
		  
		  cmd = CMD_CREDITS;
		  done = 1;
		}

	      else if (key == SDLK_DOWN)
		{
		  demo_countdown = START_DEMO_COUNTDOWN;
		  
		  cmd++;
		  
		  if (cmd >= NUM_CMDS)
		    cmd = NUM_CMDS - 1;
		}
	      else if (key == SDLK_UP)
		{
		  demo_countdown = START_DEMO_COUNTDOWN;

		  cmd--;
		  
		  if (cmd < 0)
		    cmd = 0;
		}
	      else if ((key == SDLK_RETURN) 
                    || (key == SDLK_KP_ENTER)
                    || (key == SDLK_SPACE))
		{
		  done = 1;
		}
	    }
	  else if (event.type == SDL_MOUSEBUTTONDOWN)
	  {
            /* Mouse click: */
	
	    if (event.button.x >= left &&
	        event.button.x <= left + width &&
		event.button.y >= images[IMG_TITLE]->h + 8 &&
		event.button.y <= (images[IMG_TITLE]->h + 8 +
		                   NUM_CMDS * images[IMG_TUX_HELMET1]->h))
	    {
	      cmd = ((event.button.y - (images[IMG_TITLE]->h + 8)) /
	             images[IMG_TUX_HELMET1]->h);

	      done = 1;
	    }
	  }
	}
           
      
      
      /* Erase Tux (cursor) */
      
      if (cmd != old_cmd)
	{
	  blinking = 0;
	  
	  dest.x = left + 4;
	  dest.y = (images[IMG_TITLE]->h + 8 + 
		    (old_cmd * images[IMG_TUX_HELMET1]->h));
	  dest.w = images[IMG_TUX_HELMET1]->w;
	  dest.h = images[IMG_TUX_HELMET1]->h;
	  
	  SDL_FillRect(screen, &dest,
		       SDL_MapRGB(screen->format, 200, 232, 255));

	  playsound(SND_POP);
	}
      
      
      /* Handling Tux (cursor) blinking: */
      
      if ((rand() % 50) == 0 && blinking == 0)
	{
	  blinking = 6;
	}
      
      if (blinking > 0)
	blinking--;
      
      
      /* Draw Tux (cursor) */
      
      dest.x = left + 4;
      dest.y = images[IMG_TITLE]->h + 8 + (cmd * images[IMG_TUX_HELMET1]->h);
      dest.w = images[IMG_TUX_HELMET1]->w;
      dest.h = images[IMG_TUX_HELMET1]->h;
      
      img = IMG_TUX_HELMET1;
      
      if (blinking >= 4 || (blinking >= 1 && blinking < 2))
	img = IMG_TUX_HELMET2;
      else if (blinking >= 2 && blinking < 4)
	img = IMG_TUX_HELMET3;
      
      SDL_BlitSurface(images[img], NULL, screen, &dest);

      SDL_Flip(screen);


      /* Handle demo countdown: */

      if (game_options->demo_mode)
      {
	demo_countdown--;

	if (demo_countdown == 0)
	{
	  cmd = CMD_GAME;
	  done = 1;
	}
      }

      
      /* Pause (keep frame-rate event) */
      
      now_time = SDL_GetTicks();
      if (now_time < last_time + (1000 / 20))
	{
	  SDL_Delay(last_time + (1000 / 20) - now_time);
	}
    }
  while (!done);
  
  
  /* Return the chosen command: */
  
  return cmd;
}
