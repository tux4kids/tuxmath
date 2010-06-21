/*
  schoolmode.c

  For TuxMath
  Contains code for implementing school mode in Tuxmath.

  by Vikas Singh
  vikassingh008@gmail.com 


  Part of "Tux4Kids" Project
  http://www.tux4kids.com/

 
*/

#include "menu.h"
#include "SDL_extras.h"
#include "titlescreen.h"
#include "highscore.h"
#include "factoroids.h"
#include "credits.h"
#include "multiplayer.h"
#include "mathcards.h"
#include "campaign.h"
#include "game.h"
#include "options.h"
#include "fileops.h"
#include "setup.h"
#include "throttle.h"
#include "schoolmode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



SDL_Surface *tux4kids_logo=NULL;
SDL_Rect logo_rect1,tux4kids_logo_rect;
SDL_Rect bkgd_rect;

SDL_Surface* win_bkgd = NULL;
SDL_Surface* fs_bkgd= NULL;


const char* tux4kids_standby_path = "status/standby.svg";
const char* school_bkg_path    = "schoolmode/school_bkg.jpg";


SDL_Surface* current_bkgd()
  { return screen->flags & SDL_FULLSCREEN ? fs_bkgd : win_bkgd; }


/* Local function prototypes: */
void display_screen(void);
void ShowMsg(char*,char*,char*,char*);


int schoolmode(void)
{
display_screen();
parse_xmlLesson();

return 0;
}


void display_screen()
{

SDL_Surface* srfc = NULL;
 SDL_Rect text_rect, button_rect , dest;



  Uint32 timer = 0;


  if (Opts_UsingSound())
  {
    Opts_SetGlobalOpt(MENU_SOUND, 1);
    Opts_SetGlobalOpt(MENU_MUSIC, 1);
  }

  tux4kids_logo = LoadImage(tux4kids_standby_path, IMG_REGULAR);

  /* display the Standby screen */
  if(tux4kids_logo)
  {
    /* Center horizontally and vertically */
    tux4kids_logo_rect.x = (screen->w - tux4kids_logo->w) / 2;
    tux4kids_logo_rect.y = (screen->h - tux4kids_logo->h) / 2;

    tux4kids_logo_rect.w = tux4kids_logo->w;
    tux4kids_logo_rect.h = tux4kids_logo->h;

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_BlitSurface(tux4kids_logo, NULL, screen, &tux4kids_logo_rect);
    SDL_UpdateRect(screen, 0, 0, 0, 0);
    /* Play "harp" greeting sound lifted from Tux Paint */
    playsound(SND_HARP);
    SDL_FreeSurface(tux4kids_logo);
  }

SDL_Delay(2000);



 LoadBothBkgds(school_bkg_path, &fs_bkgd, &win_bkgd);
  if(fs_bkgd == NULL || win_bkgd == NULL)
  {
    fprintf(stderr, "Backgrounds were not properly loaded, exiting");
    if(fs_bkgd)
      SDL_FreeSurface(fs_bkgd);
    if(win_bkgd)
      SDL_FreeSurface(win_bkgd);
  }



bkgd_rect = current_bkgd()->clip_rect;
    bkgd_rect.x = (screen->w - bkgd_rect.w) / 2;
    bkgd_rect.y = (screen->h - bkgd_rect.h) / 2;




if(current_bkgd())
  {
    /* FIXME not sure trans_wipe() works in Windows: */
    trans_wipe(current_bkgd(), RANDOM_WIPE, 10, 20);
    /* Make sure background gets drawn (since trans_wipe() doesn't */
    /* seem to work reliably as of yet):                          */
    SDL_BlitSurface(current_bkgd(), NULL, screen, &bkgd_rect);
  }



/*



IMG_school=LoadImage(school_bkg_path,IMG_REGULAR);
if(IMG_school)
 SDL_BlitSurface(IMG_school, NULL, screen, NULL);
*/


ShowMsg(_("SCHOOL MODE"),
		   _("MISSIONS"),
		   _("BEST OF LUCK !"),
		   _("PRESS ENTER WHEN READY ..."));


        srfc = BlackOutline(_("Level 1"), 55, &black);
        if (srfc)
        {
          button_rect.x = text_rect.x = (screen->w)/2 - (srfc->w)/2 + 50;
          button_rect.y = text_rect.y = (screen->h/12)+60;
          button_rect.w = text_rect.w = srfc->w+400;
          button_rect.h = text_rect.h = srfc->h ;
          /* add margin to button and draw: */
          //button_rect.x -= 10;
          //button_rect.w += 20;
          DrawButton(&button_rect, 15, 255, 255, 255, 255);
          /* Now blit text and free surface: */
          SDL_BlitSurface(srfc, NULL, screen, &text_rect);
       

         srfc = BlackOutline(_("Level 2"), 55, &gray);

          button_rect.x = text_rect.x = (screen->w)/2 - (srfc->w)/2 + 50;
          button_rect.y = text_rect.y =(screen->h/12)+160;
          button_rect.w = text_rect.w = srfc->w+400;
          button_rect.h = text_rect.h = srfc->h;
              

          DrawButton(&button_rect, 15,  255, 171, 0, 255);
          /* Now blit text and free surface: */
          SDL_BlitSurface(srfc, NULL, screen, &text_rect);


         srfc = BlackOutline(_("Level 3"), 55, &gray);

          button_rect.x = text_rect.x = (screen->w)/2 - (srfc->w)/2 + 50;
          button_rect.y = text_rect.y =(screen->h/12)+260;
          button_rect.w = text_rect.w = srfc->w+400;
          button_rect.h = text_rect.h = srfc->h;
              

          DrawButton(&button_rect, 15, 255, 255, 255, 100);
          /* Now blit text and free surface: */
          SDL_BlitSurface(srfc, NULL, screen, &text_rect);
  

         srfc = BlackOutline(_("Level 4"), 55, &gray);

          button_rect.x = text_rect.x = (screen->w)/2 - (srfc->w)/2 + 50;
          button_rect.y = text_rect.y =(screen->h/12)+360;
          button_rect.w = text_rect.w = srfc->w+400;
          button_rect.h = text_rect.h = srfc->h;
              

          DrawButton(&button_rect, 15, 255, 255, 255, 100);
          /* Now blit text and free surface: */
          SDL_BlitSurface(srfc, NULL, screen, &text_rect);


         srfc = BlackOutline(_("Level 5"), 55, &gray);

          button_rect.x = text_rect.x = (screen->w)/2 - (srfc->w)/2 + 50;
          button_rect.y = text_rect.y =(screen->h/12)+460;
          button_rect.w = text_rect.w = srfc->w+400;
          button_rect.h = text_rect.h = srfc->h;
              

          DrawButton(&button_rect, 15, 255, 255, 255, 100);
          /* Now blit text and free surface: */
          SDL_BlitSurface(srfc, NULL, screen, &text_rect);



          SDL_FreeSurface(srfc);
          srfc = NULL;
        }





dest.x = (screen->w - images[IMG_CORRECT]->w);
    dest.y = (screen->h - images[IMG_CORRECT]->h) ;
    dest.w = images[IMG_CORRECT]->w;
    dest.h = images[IMG_CORRECT]->h;

    SDL_BlitSurface(images[IMG_CORRECT], NULL, screen, &dest);
/*
dest.x = (screen->w - images[IMG_DEMO]->w) / 2;
    dest.y = (screen->h - images[IMG_DEMO]->h) / 2;
    dest.w = images[IMG_DEMO]->w;
    dest.h = images[IMG_DEMO]->h;

    SDL_BlitSurface(images[IMG_DEMO], NULL, screen, &dest);

*/
SDL_Flip(screen); 
while(1)
  {
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT)
    {
      //SDL_quit_received = 1;
    //  quit = 1;
      return;
    }
    else if (event.type == SDL_MOUSEBUTTONDOWN)
    {
      return;
    }
    else if (event.type == SDL_KEYDOWN)
    {
      if (event.key.keysym.sym == SDLK_RETURN)
        //parse();
         
        //escape_received = 1;
      return;
    }
    Throttle(20, &timer);

  }


}

void ShowMsg(char* str1, char* str2, char* str3, char* str4)
{
  SDL_Surface *s1, *s2, *s3, *s4;
  SDL_Rect loc;

  s1 = s2 = s3 = s4 = NULL;
 if (str1)
    s1 = BlackOutline(str1, 40, &white);
  if (str2)
    s2 = BlackOutline(str2, 40, &white);
  if (str3)
    s3 = BlackOutline(str3, 25, &white);
  /* When we get going with i18n may need to modify following - see below: */
  if (str4)
    s4 = BlackOutline(str4, 25, &white);
  /* Draw lines of text (do after drawing Tux so text is in front): */
  if (s1)
  {
    loc.x = (screen->w / 6) - (s1->w/2); 
    loc.y = (screen->h / 12)-30 ;
    SDL_BlitSurface( s1, NULL, screen, &loc);
  }
  if (s2)
  {
    loc.x = (screen->w/2 ) + (s2->w); 
    loc.y = (screen->h / 12) - 30;
    SDL_BlitSurface( s2, NULL, screen, &loc);
  }
  if (s3)
  {
    loc.x = (screen->w / 4) ; 
    loc.y = (screen->h / 3)+30 ;
    SDL_BlitSurface( s3, NULL, screen, &loc);
  }
  if (s4)
  {
    loc.x = (screen->w / 6) - (s4->w/2); 
    loc.y = (screen->h ) - 100;
    SDL_BlitSurface( s4, NULL, screen, &loc);
  }

  /* and update: */
  SDL_UpdateRect(screen, 0, 0, 0, 0);


  SDL_FreeSurface(s1);
  SDL_FreeSurface(s2);
  SDL_FreeSurface(s3);
  SDL_FreeSurface(s4);
}

