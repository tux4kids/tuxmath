/*
  options.c

  For TuxMath
  The options screen loop.

  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/


  Part of "Tux4Kids" Project
  http://www.tux4kids.org/
      
  August 26, 2001 - January 3, 2005
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "options.h"
#include "images.h"
#include "setup.h"
#include "sounds.h"
#include "playsound.h"
#include "game.h"

/* Local (to options.c) 'globals': */
/* moved to file scope to allow update_selected_option() to use them DSB */
static SDL_Rect dest;
static char tmp_str[10];
static unsigned char range_bits;
static int i, j, x, y;

int options(void)
{
  int opt, old_opt, done, quit, img, blinking;
  SDL_Event event;
  Uint32 last_time, now_time;
  SDLKey key;
  
  
  /* Clear window: */
  
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

  dest.x = (screen->w - images[IMG_OPTIONS]->w) / 2;
  dest.y = 0;
  SDL_BlitSurface(images[IMG_OPTIONS], NULL, screen, &dest);


  /* Draw options: */

  for (i = 0; i < NUM_OPTS; i++)
  {
    y = (images[IMG_OPTIONS]->h + 8 + (i * images[IMG_TUX_HELMET1]->h));
    
    dest.x = 32 + images[IMG_TUX_HELMET1]->w + 4;
    dest.y = y;
    
    SDL_BlitSurface(images[IMG_OPT_ADDITION + i], NULL, screen, &dest);


    /* Checkmarks for operators: */

    if (i >= OPT_OP_ADD && i < OPT_OP_ADD + NUM_OPERS)
    {
      dest.x = screen->w - images[IMG_OPT_CHECK]->w - 16;
      dest.y = y;
      
      SDL_BlitSurface(images[IMG_OPT_CHECK + opers[i]], NULL, screen, &dest);
    }
    else if (i == OPT_A_MAX)
    {
      /* Maximum answer: */

      snprintf(tmp_str, sizeof(tmp_str), "%04d", max_answer);
      draw_nums(tmp_str,
		screen->w - ((images[IMG_NUMS]->w / 14) * 2) - 16,
		y + images[IMG_OPT_MAX_ANSWER]->h);

      /* Using "* 2" instead of "* 4" (even though string is 4 digits long)
         because "draw_nums()" actually centers around x; not left-justifies */
    }
    else if (i == OPT_A_SPEED)
    {
      /* Maximum answer: */

      snprintf(tmp_str, sizeof(tmp_str), "%.1f", speed);
      draw_nums(tmp_str,
		screen->w - ((images[IMG_NUMS]->w / 14) * 2) - 16,
		y + images[IMG_OPT_SPEED]->h);

      /* Using "* 2" instead of "* 4" (even though string is 4 digits long)
         because "draw_nums()" actually centers around x; not left-justifies */
    }
    else if (i == OPT_Q_RANGE)
    {
      x = 32 + images[IMG_TUX_HELMET1]->w + 4 + 64;
      y = y + images[IMG_TUX_HELMET1]->h;

      for (j = 0; j < NUM_Q_RANGES; j++)
      {
	dest.x = x;
	dest.y = y;

	SDL_BlitSurface(images[IMG_OPT_RNG_1_5 + j * 2 + range_enabled[j]],
			NULL,
			screen, &dest);

	x = x + images[IMG_OPT_RNG_1_5 + j * 2 + range_enabled[j]]->w + 16;
      }
    }
  }


  /* Get bits of what ranges are available: */

  range_bits = 0;
  for (j = 0; j < NUM_Q_RANGES; j++)
  {
    range_bits = range_bits << 1;
    if (range_enabled[j])
      range_bits = range_bits | 1;
  }

  
  /* --- MAIN OPTIONS SCREEN LOOP: --- */

  blinking = 0;
  opt = 0;
  done = 0;
  quit = 0;
  
  do
    {
      last_time = SDL_GetTicks();
      old_opt = opt;
      
      
      /* Handle any incoming events: */
      
      while (SDL_PollEvent(&event) > 0)
	{
	  if (event.type == SDL_QUIT)
	    {
	      /* Window close event - quit! */
	      
	      quit = 1;
	      done = 1;
	    }

	  else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
              /* FIXME Options screen should have "OK" button to accept choices with mouse DSB */
              /* figure out opt based on mouse coordinates */
              if (/* event.button.x >= left && */ /* don't see any reason to filter on x values here*/
	          /* event.button.x <= left + width && */  
		  event.button.y >= images[IMG_OPTIONS]->h + 8 &&
		  event.button.y <= (images[IMG_OPTIONS]->h + 8 +
		                   NUM_OPTS * images[IMG_TUX_HELMET1]->h))
	      {
	        opt = ((event.button.y - (images[IMG_OPTIONS]->h + 8)) /
	               images[IMG_TUX_HELMET1]->h);
                update_selected_option(opt);
	      }
              /* handle ranges below list as last option */
              if (event.button.y > (images[IMG_OPTIONS]->h + 8 +
	      	                   NUM_OPTS * images[IMG_TUX_HELMET1]->h)
                  &&

                  event.button.y <= (images[IMG_OPTIONS]->h + 8 +
	      	                    NUM_OPTS * images[IMG_TUX_HELMET1]->h)
                                    + (images[IMG_OPT_RNG_1_5]->h))                          
              { 
                opt = OPT_Q_RANGE;
                update_selected_option(opt);
              }
            }

	  else if (event.type == SDL_KEYDOWN)
	    {
	      key = event.key.keysym.sym;
	      
	      if (key == SDLK_ESCAPE)
		{
                  /* Don't leave options screen if all operations deselected! - DSB */
                  /* fixes Debian bug #336272 */
                  int i;
                  int at_least_one_oper = 0;
                  for (i =0; i < NUM_OPERS; i++)
                  {
                    if (opers[i])
                      at_least_one_oper = 1;
                  }
                  if (at_least_one_oper)
                    done = 1;
		}
	      else if (key == SDLK_DOWN)
		{
		  if (opt < NUM_OPTS - 1)
		  {
		    opt++;

		    playsound(SND_POP);
		  }
		}
	      else if (key == SDLK_UP)
		{
		  if (opt > 0)
		  {
		    opt--;
		    playsound(SND_POP);
		  }
		}
	      else if (key == SDLK_RETURN ||
		       key == SDLK_KP_ENTER ||
		       key == SDLK_SPACE)
              {
              /* code moved into function taking opt as argument so the same code can be   */
              /* used to handle mouse events DSB */
                update_selected_option(opt);
              }
	    }
	}
      
      
      /* Erase Tux (cursor) */
      
      if (opt != old_opt)
	{
	  blinking = 0;
	  
	  dest.x = 32;
	  dest.y = (images[IMG_OPTIONS]->h + 8 + 
		    (old_opt * images[IMG_TUX_HELMET1]->h));
	  dest.w = images[IMG_TUX_HELMET1]->w;
	  dest.h = images[IMG_TUX_HELMET1]->h;
	  
	  SDL_FillRect(screen, &dest,
		       SDL_MapRGB(screen->format, 0, 0, 0));
	}
      
      
      /* Handling Tux (cursor) blinking: */
      
      if ((rand() % 50) == 0 && blinking == 0)
	{
	  blinking = 6;
	}
      
      if (blinking > 0)
	blinking--;
      
      
      /* Draw Tux (cursor) */
      
      dest.x = 32;
      dest.y = images[IMG_OPTIONS]->h + 8 + (opt * images[IMG_TUX_HELMET1]->h);
      dest.w = images[IMG_TUX_HELMET1]->w;
      dest.h = images[IMG_TUX_HELMET1]->h;
      
      img = IMG_TUX_HELMET1;
      
      if (blinking >= 4 || (blinking >= 1 && blinking < 2))
	img = IMG_TUX_HELMET2;
      else if (blinking >= 2 && blinking < 4)
	img = IMG_TUX_HELMET3;
     
      SDL_BlitSurface(images[img], NULL, screen, &dest);
      
      SDL_Flip(screen);

      
      /* Pause (keep frame-rate event) */
      
      now_time = SDL_GetTicks();
      if (now_time < last_time + (1000 / 20))
	{
	  SDL_Delay(last_time + (1000 / 20) - now_time);
	}
    }
  while (!done);


  /* Return the chosen command: */
  
  return quit;
}


/* can be called by either keystroke or mouse click - moved into */
/* separate function to reduce code duplication - DSB */
void update_selected_option(int option)
{
  {
    if (option >= OPT_OP_ADD && option < OPT_OP_ADD + NUM_OPERS)
    {
      opers[option - OPT_OP_ADD] = !opers[option - OPT_OP_ADD];

      dest.x = screen->w - images[IMG_OPT_CHECK]->w - 16;
      dest.y = (images[IMG_OPTIONS]->h + 8 +
               ((option - OPT_OP_ADD) *
               images[IMG_TUX_HELMET1]->h));

      SDL_BlitSurface(images[IMG_OPT_CHECK + opers[option - OPT_OP_ADD]],
                      NULL,
                      screen,
                      &dest);
    }

    else if (option == OPT_A_MAX)
    {
      max_answer = (max_answer * 2) / 3;
      if (max_answer < 12)
        max_answer = 144;

      dest.x = screen->w - ((images[IMG_NUMS]->w / 14) * 4) - 16;
      dest.y = (images[IMG_OPTIONS]->h + 8 +
               ((option - OPT_OP_ADD) *
               images[IMG_TUX_HELMET1]->h));
      dest.w = ((images[IMG_NUMS]->w / 14) * 4);
      dest.h = images[IMG_OPT_MAX_ANSWER]->h;

      SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 0, 0));
      snprintf(tmp_str, sizeof(tmp_str), "%04d", max_answer);
      draw_nums(tmp_str,
                screen->w - ((images[IMG_NUMS]->w / 14) * 2) - 16,
                (images[IMG_OPTIONS]->h + 8 +
                ((option - OPT_OP_ADD) *
                images[IMG_TUX_HELMET1]->h)) +
                images[IMG_OPT_MAX_ANSWER]->h);
    }

    else if (option == OPT_A_SPEED)
    {
      speed = speed - 0.1;
      if (speed < 0.1)
        speed = 5;

      dest.x = screen->w - ((images[IMG_NUMS]->w / 14) * 4) - 16;
      dest.y = (images[IMG_OPTIONS]->h + 8 +
               ((option - OPT_OP_ADD) *
               images[IMG_TUX_HELMET1]->h));
      dest.w = ((images[IMG_NUMS]->w / 14) * 4);
      dest.h = images[IMG_OPT_SPEED]->h;

      SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 0, 0));

      snprintf(tmp_str, sizeof(tmp_str), "%.1f", speed);
      draw_nums(tmp_str,
                screen->w - ((images[IMG_NUMS]->w / 14) * 2) - 16,
                (images[IMG_OPTIONS]->h + 8 +
                ((option - OPT_OP_ADD) *
                images[IMG_TUX_HELMET1]->h)) +
                images[IMG_OPT_SPEED]->h);
    }

    else if (option == OPT_Q_RANGE)
    {
    /* Change which ranges are available: */
      range_bits = range_bits + 1;
      if (range_bits >= (1 << NUM_Q_RANGES))
        range_bits = 1;

      for (j = 0; j < NUM_Q_RANGES; j++)
      {
        if ((range_bits & (1 << j)) != 0)
          range_enabled[j] = 1;
        else
          range_enabled[j] = 0;
      }

      /* Redraw ranges: */
      x = 32 + images[IMG_TUX_HELMET1]->w + 4 + 64;
      y = (images[IMG_OPTIONS]->h + 8 +
          (option * images[IMG_TUX_HELMET1]->h)) +
           images[IMG_TUX_HELMET1]->h;

      for (j = 0; j < NUM_Q_RANGES; j++)
      {
        dest.x = x;
        dest.y = y;

        SDL_BlitSurface(images[IMG_OPT_RNG_1_5 + j * 2 +
                        range_enabled[j]],
                        NULL,
                        screen, 
                        &dest);

        x = x
            + images[IMG_OPT_RNG_1_5 + j * 2 + range_enabled[j]]-> w 
            + 16;
      }
    }
    /* same sound for all option updates */
    playsound(SND_LASER);
  }
}
