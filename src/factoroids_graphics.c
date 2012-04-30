#include "factoroids_graphics.h"

#include "tuxmath.h"
#include "fileops.h"
#include "factoroids.h"
#include "frame_counter.h"
#include "draw_utils.h"
#include "SDL_rotozoom.h"

/* definitions for cockpit buttons */
#define BUTTONW 24
#define BUTTONH 24
#define BUTTON2_X 65
#define BUTTON2_Y 45
#define BUTTON3_X 53
#define BUTTON3_Y 65
#define BUTTON5_X 30
#define BUTTON5_Y 77
#define BUTTON7_X 8
#define BUTTON7_Y 77
#define BUTTON11_X 30
#define BUTTON11_Y 65
#define BUTTON13_X 45
#define BUTTON13_Y 45
#define NUMBUTTONS 6

#define BASE_RES_X 1280

#define MAX_LASER 5
#define MAX_ASTEROIDS 50
#define NUM_TUXSHIPS 2
#define NUM_SPRITES 11
#define TUXSHIP_LIVES 3
#define DEG_PER_ROTATION 2
#define NUM_OF_ROTO_IMGS 360/DEG_PER_ROTATION


/* definitions of level message */
#define MAX_CHAR_MSG 256
#define LVL_WIDTH_MSG 350
#define LVL_HEIGHT_MSG 200
#define LVL_OBJ_X_OFFSET 20
#define LVL_OBJ_Y_OFFSET 20
#define LVL_HINT_X_OFFSET 20
#define LVL_HINT_Y_OFFSET 130

//the prime set keeps increasing till its size reaches this value
#define PRIME_MAX_LIMIT 6



/* Structures for Buttons on Cockpit */
struct ButtonType
{
    int img_id;
    int x;
    int y;
    int prime;
};


static struct ButtonType buttons[NUMBUTTONS];

static int laser_coeffs[][3] = {
    {0, 0, 0},  // 0
    {0, 0, 0},  // 1
    {18, 0, 0}, // 2
    {0, 18, 0}, // 3
    {0, 0, 0},  // 4
    {0, 0, 18}, // 5
    {0, 0, 0},  // 6
    {18, 18, 0},// 7
    {0, 0, 0},  // 8
    {0, 0, 0},  // 9
    {0, 0, 0},  // 10
    {0, 18, 18},// 11
    {0, 0, 0},  // 12
    {18, 0, 18} // 13
};

int bonus_img_ids[] = {
    IMG_BONUS_CLOAKING, IMG_BONUS_FORCEFIELD, IMG_BONUS_POWERBOMB
};

static float zoom;

//SDL_Surfaces:
static SDL_Surface* IMG_lives_ship = NULL;
static SDL_Surface* IMG_tuxship[NUM_OF_ROTO_IMGS];
static SDL_Surface* IMG_tuxship_cloaked[NUM_OF_ROTO_IMGS];
static SDL_Surface* IMG_tuxship_thrust[NUM_OF_ROTO_IMGS];
static SDL_Surface* IMG_tuxship_thrust_cloaked[NUM_OF_ROTO_IMGS];
static SDL_Surface* IMG_asteroids1[NUM_OF_ROTO_IMGS];
static SDL_Surface* IMG_asteroids2[NUM_OF_ROTO_IMGS];

SDL_Surface* bkgd = NULL; //640x480 background (windowed)
SDL_Surface* scaled_bkgd = NULL; //native resolution (fullscreen)

extern int FF_game;


SDL_Surface* current_bkgd(void)
{
    return screen->flags & SDL_FULLSCREEN ? scaled_bkgd : bkgd;
}


int factoroids_init_graphics(void)
{
    int i;

    if(screen->h < 600 && screen->w < 800)
        zoom = 0.65;
    else
        zoom=(float)screen->w/(float)BASE_RES_X;

    DEBUGCODE(debug_factoroids)
        fprintf(stderr, "The zoom factor is: %f\n", zoom);

    /********   Set up properly scaled and optimized background surfaces: *********/
    /* NOTE - optimization code moved into LoadBothBkgds() so rest of program     */
    /* can take advantage of it - DSB                                             */

    T4K_LoadBothBkgds("factoroids/gbstars.png", &scaled_bkgd, &bkgd);

    if (bkgd == NULL || scaled_bkgd == NULL)
    {
        fprintf(stderr,
                "\nError: could not scale background\n");
        return 0;
    }

    /*************** Precalculating software rotation ***************/

    for(i = 0; i < NUM_OF_ROTO_IMGS; i++)
    {
        //rotozoomSurface (SDL_Surface *src, double angle, double zoom, int smooth);
        IMG_tuxship[i] = rotozoomSurface(images[IMG_SHIP01], i * DEG_PER_ROTATION, zoom, 1);
        IMG_tuxship_cloaked[i] = rotozoomSurface(images[IMG_SHIP_CLOAKED], i * DEG_PER_ROTATION, zoom, 1);
        IMG_tuxship_thrust[i] = rotozoomSurface(images[IMG_SHIP_THRUST], i * DEG_PER_ROTATION, zoom, 1);
        IMG_tuxship_thrust_cloaked[i] = rotozoomSurface(images[IMG_SHIP_THRUST_CLOAKED], i * DEG_PER_ROTATION, zoom, 1);

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

    /* Create zoomed and scaled ship image for "lives" counter */
    IMG_lives_ship = rotozoomSurface(images[IMG_SHIP_CLOAKED], 90, zoom * 0.7, 1);
}


void factoroids_cleanup_graphics(void)
{
    int i;

    for(i = 0; i < NUM_OF_ROTO_IMGS; i++)
    {
        if (IMG_tuxship[i])
        {
            SDL_FreeSurface(IMG_tuxship[i]);
            IMG_tuxship[i] = NULL;
        }
        if (IMG_tuxship_thrust[i])
        {
            SDL_FreeSurface(IMG_tuxship_thrust[i]);
            IMG_tuxship_thrust[i] = NULL;
        }
        if (IMG_tuxship_cloaked[i])
        {
            SDL_FreeSurface(IMG_tuxship_cloaked[i]);
            IMG_tuxship_cloaked[i] = NULL;
        }
        if (IMG_tuxship_thrust_cloaked[i])
        {
            SDL_FreeSurface(IMG_tuxship_thrust_cloaked[i]);
            IMG_tuxship_thrust_cloaked[i] = NULL;
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

    if (IMG_lives_ship)
    {
        SDL_FreeSurface(IMG_lives_ship);
        IMG_lives_ship = NULL;
    }

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


void factoroids_intro(void)
{
    static SDL_Surface* IMG_factors;
    static SDL_Surface* IMG_fractions;

    SDL_Rect rect;

    IMG_factors   = rotozoomSurface(images[IMG_FACTOROIDS], 0, zoom, 1);
    IMG_fractions = rotozoomSurface(images[IMG_FACTORS], 0, zoom, 1);

    factoroids_draw_bkgr();
    if(FF_game == FACTOROIDS_GAME)
    {

        rect.x = (screen->w/2) - (IMG_factors->w/2);
        rect.y = (screen->h)/7;
        SDL_BlitSurface(IMG_factors, NULL, screen, &rect);
        factoroids_show_message(_("To win, you must destroy all the asteroids.\n"
                    "Turn: arrow keys or mouse movement.\n"
                    "Thrust: up arrow or right mouse button.\n"
                    "Shoot: [Enter], [Space], or left mouse button.\n"
                    "Switch Prime Number Gun: [D], [F], or mouse scroll wheel.\n"
                    "Activate Powerup: [Shift].\n"
                    "Shoot the rocks with their prime factors until they are all destroyed."));
        SDL_BlitSurface(IMG_asteroids1[3],NULL,screen,&rect);
    }
    else if (FF_game == FRACTIONS_GAME)
    {
        rect.x = (screen->w/2)-(IMG_fractions->w/2);
        rect.y = (screen->h)/7;
        SDL_BlitSurface(IMG_fractions,NULL,screen,&rect);
        factoroids_show_message(_("FRACTIONS: to win, you need destroy all the asteroids. "
                    "Use the arrow keys to turn or go forward.  Aim at an asteroid, "
                    "type a number that can simplify the fraction, and press space or return "
                    "to split it.  Destroy fractions that can not be further simplified in a single shot!"));
    }

    SDL_FreeSurface(IMG_factors);
    SDL_FreeSurface(IMG_fractions);
}


void factoroids_draw(asteroid_type *asteroid, tuxship_type *tuxship, FF_laser_type *laser,
        int bonus, int bonus_time, int *digits, int wave, int score, int num,
        int tux_img, int button_pressed)
{

    int i, offset;
    int xnum, ynum;
    char str[64];
    SDL_Surface* surf;
    SDL_Rect dest;

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

    /************ Draw Background ***************/

    factoroids_draw_bkgr();

    /******************* Draw laser *************************/
    for (i=0;i<MAX_LASER;i++){
        if(laser[i].alive)
        {
            if(laser[i].count>0)
            {
                laser[i].x += tuxship->xspeed*FC_time_elapsed;
                laser[i].y += tuxship->yspeed*FC_time_elapsed;
                laser[i].destx += tuxship->xspeed*FC_time_elapsed;
                laser[i].desty += tuxship->yspeed*FC_time_elapsed;

                draw_line(screen, laser[i].x, laser[i].y, laser[i].destx, laser[i].desty,
                        laser[i].count*laser_coeffs[laser[i].n][0], laser[i].count*laser_coeffs[laser[i].n][1], laser[i].count*laser_coeffs[laser[i].n][2]);

                laser[i].count -= 15*FC_time_elapsed;
            } else if (laser[i].count <= 0)
            {
                laser[i].alive=0;
            }
        }
    }
    /*************** Draw Ship ******************/

    if(!tuxship->hurt || (tuxship->hurt && tuxship->hurt_count%2==0)){
        dest.x = (tuxship->x - (IMG_tuxship[tuxship->angle/DEG_PER_ROTATION]->w/2));
        dest.y = (tuxship->y - (IMG_tuxship[tuxship->angle/DEG_PER_ROTATION]->h/2));
        dest.w = IMG_tuxship[tuxship->angle/DEG_PER_ROTATION]->w;
        dest.h = IMG_tuxship[tuxship->angle/DEG_PER_ROTATION]->h;

        //Change the image based on if the rocket is thrusting
        //Google code in task

        if(!tuxship->thrust) {
            SDL_Surface **_IMG_ship = bonus == TB_CLOAKING && bonus_time>0 ? IMG_tuxship_cloaked : IMG_tuxship;
            SDL_BlitSurface(_IMG_ship[tuxship->angle/DEG_PER_ROTATION], NULL, screen, &dest);
        } else {
            SDL_Surface **_IMG_ship = bonus == TB_CLOAKING && bonus_time>0 ? IMG_tuxship_thrust_cloaked : IMG_tuxship_thrust;
            SDL_BlitSurface(_IMG_ship[tuxship->angle/DEG_PER_ROTATION], NULL, screen, &dest);
        }



        if(bonus == TB_FORCEFIELD && bonus_time > 0) {
            SDL_Rect tmp = {tuxship->x - images[IMG_FORCEFIELD]->w/2, tuxship->y - images[IMG_FORCEFIELD]->h/2};
            SDL_BlitSurface(images[IMG_FORCEFIELD], NULL, screen, &tmp);
        }
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
                draw_nums(zoom, str, xnum, ynum, &white);
            }
            else if (FF_game==FRACTIONS_GAME)
            {
                sprintf(str, "%d", asteroid[i].a);
                draw_nums(zoom, str, xnum, ynum, &white);
                draw_line(screen, xnum, ynum + 4, xnum + 30, ynum + 4,
                        255, 255, 255);
                sprintf(str, "%d", asteroid[i].b);
                draw_nums(zoom, str, xnum, ynum + 35, &white);
            }
        }
    }
    /*************** Draw Steam ***************/
    for(i=0; i<MAX_ASTEROIDS; i++)
    {
        if(asteroid[i].isdead) {
            dest.x = asteroid[i].xdead;
            dest.y = asteroid[i].ydead;
            SDL_BlitSurface(images[IMG_STEAM1+asteroid[i].countdead], NULL, screen, &dest);
            if(bonus == TB_POWERBOMB && bonus_time > 0)
                draw_line(screen, asteroid[i].x, asteroid[i].y, tuxship->x, tuxship->y,
                        (5 - asteroid[i].countdead)*4*laser_coeffs[digits[1]*10+digits[2]][0],
                        (5 - asteroid[i].countdead)*4*laser_coeffs[digits[1]*10+digits[2]][1],
                        (5 - asteroid[i].countdead)*4*laser_coeffs[digits[1]*10+digits[2]][2]);
        }


        if(asteroid[i].isdead) {
            dest.x = asteroid[i].xdead;
            dest.y = asteroid[i].ydead;
            SDL_BlitSurface(images[IMG_STEAM1+asteroid[i].countdead], NULL, screen, &dest);
            asteroid[i].countdead++;
            if(asteroid[i].countdead > 5)
            {
                asteroid[i].isdead = 0;
                asteroid[i].countdead = 0;
            }
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
    draw_numbers(screen, str, offset+images[IMG_WAVE]->w + (images[IMG_NUMBERS]->w / 10), 0);

    /* Draw "score" label: */
    dest.x = (screen->w - ((images[IMG_NUMBERS]->w/10) * 7) -
            images[IMG_SCORE]->w -
            images[IMG_STOP]->w - 5);
    dest.y = 0;
    dest.w = images[IMG_SCORE]->w;
    dest.h = images[IMG_SCORE]->h;

    SDL_BlitSurface(images[IMG_SCORE], NULL, screen, &dest);

    sprintf(str, "%.6d", score);
    draw_numbers(screen, str,
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
        draw_numbers(screen, str, ((screen->w)/2) - 50, (screen->h) - 30);
    }
    else
    {
        factoroids_draw_led_console(wave, num, button_pressed);
        draw_console_image(tux_img);
    }

    /************** Draw lives ***************/
    dest.y = screen->h;
    dest.x = 0;

    for(i = 1; i <= tuxship->lives; i++)
    {
        if(tuxship->lives <= 5)
        {
            dest.y = dest.y - (IMG_lives_ship->h);
            SDL_BlitSurface(IMG_lives_ship, NULL, screen, &dest);
        }
        else if(tuxship->lives > 4)
        {
            dest.y = screen->h - (IMG_lives_ship->h);
            SDL_BlitSurface(IMG_lives_ship, NULL, screen, &dest);
            sprintf(str, "%d", tuxship->lives);
            draw_numbers(screen, str, 10, (screen->h) - 30);
        }
    }

    /*** Draw Bonus Indicator ***/
    static int blink = 0;
    if(bonus_time == 0)
        blink = 0;
    else if(bonus_time - SDL_GetTicks() > 3000)
        blink = 5;
    else
        blink = (blink + 1) % 10;
    if(bonus != -1 && blink>4) {
        SDL_Surface *indicator = images[bonus_img_ids[bonus]];
        SDL_Rect pos = {screen->w - indicator->w, screen->h - indicator->h};
        SDL_BlitSurface(indicator, NULL, screen, &pos);
    }
}


void factoroids_draw_led_console(int wave, int num, int button_pressed)
{
    int i;
    enum BUTTON_TYPE type;

    draw_console_image(IMG_COCKPIT);
    factoroids_init_buttons();

    for(i=0; i<6; i++)
    {
        if(i < wave)
        {
            if(button_pressed && num==buttons[i].prime)
            {
                type=PRESSED;
            }
            else if(num==buttons[i].prime)
            {
                type=SELECTED;
            }
            else
            {
                type = ACTIVE;
            }
        }
        else
        {
            type = DISABLED;
        }
        factoroids_draw_button(buttons[i].img_id,type,buttons[i].x,buttons[i].y);
    }
}


void factoroids_init_buttons(void)
{

    buttons[0].img_id = IMG_BUTTON2;
    buttons[0].x = screen->w/2 - BUTTON2_X;
    buttons[0].y = screen->h - BUTTON2_Y;
    buttons[0].prime = 2;

    buttons[1].img_id = IMG_BUTTON3;
    buttons[1].x = screen->w/2 - BUTTON3_X;
    buttons[1].y = screen->h - BUTTON3_Y;
    buttons[1].prime = 3;

    buttons[2].img_id = IMG_BUTTON5;
    buttons[2].x = screen->w/2 - BUTTON5_X;
    buttons[2].y = screen->h - BUTTON5_Y;
    buttons[2].prime = 5;

    buttons[3].img_id = IMG_BUTTON7;
    buttons[3].x = screen->w/2 + BUTTON7_X;
    buttons[3].y = screen->h - BUTTON7_Y;
    buttons[3].prime = 7;

    buttons[4].img_id = IMG_BUTTON11;
    buttons[4].x = screen->w/2 + BUTTON11_X;
    buttons[4].y = screen->h - BUTTON11_Y;
    buttons[4].prime = 11;

    buttons[5].img_id = IMG_BUTTON13;
    buttons[5].x = screen->w/2 + BUTTON13_X;
    buttons[5].y = screen->h - BUTTON13_Y;
    buttons[5].prime = 13;

}


void factoroids_draw_bkgr(void)
{

    SDL_BlitSurface(current_bkgd(), NULL, screen, NULL);
}


void factoroids_draw_button(int img_id, enum BUTTON_TYPE type, int x, int y)
{
    SDL_Rect rect, scr;
    rect.y = 0;
    rect.w = BUTTONW;
    rect.h = BUTTONH;

    scr.x = x;
    scr.y = y;

    if(type == ACTIVE)
    {
        rect.x = 0;
        SDL_BlitSurface(images[img_id], &rect, screen, &scr);
    }
    else if(type == SELECTED)
    {
        rect.x = BUTTONW;
        SDL_BlitSurface(images[img_id], &rect, screen, &scr);
    }
    else if(type == PRESSED)
    {
        rect.x = BUTTONW * 2;
        SDL_BlitSurface(images[img_id], &rect, screen, &scr);
    }
    else if(type == DISABLED)
    {
        rect.x = BUTTONW * 3;
        SDL_BlitSurface(images[img_id], &rect, screen, &scr);
    }
}


void factoroids_show_message(char* str)
{
    SDL_Surface* s1 = NULL;
    SDL_Rect loc;
    char wrapped_str[1024];
    int char_width;

    if(str == NULL)
        return;

    char_width = T4K_CharsForWidth(DEFAULT_MENU_FONT_SIZE, screen->w * 0.75);
    T4K_LineWrapInsBreaks(str, wrapped_str, char_width, 64, 64);
    s1 = T4K_BlackOutline(wrapped_str, DEFAULT_MENU_FONT_SIZE, &yellow);
    if (s1)
    {
        loc.x = screen->w/2 - s1->w/2;
        loc.y = screen->h/4;
        SDL_BlitSurface(s1, NULL, screen, &loc);
        SDL_FreeSurface(s1);
    }
    SDL_UpdateRect(screen, 0, 0, 0, 0);
}


void factoroids_level_objs_hints(char *label, char *contents, int x, int y )
{
    SDL_Surface *s1 = NULL, *s2 = NULL;
    SDL_Rect loc;
    char wrapped_label[256];
    char wrapped_contents[256];
    int char_width;

    if(label == NULL || contents == NULL)
        return;

    char_width = T4K_CharsForWidth(DEFAULT_MENU_FONT_SIZE, LVL_WIDTH_MSG);
    T4K_LineWrapInsBreaks(label, wrapped_label, char_width, 64, 64);
    T4K_LineWrapInsBreaks(contents, wrapped_contents, char_width, 64, 64);

    s1 = T4K_BlackOutline(wrapped_label, DEFAULT_MENU_FONT_SIZE, &white);
    s2 = T4K_BlackOutline(wrapped_contents, DEFAULT_MENU_FONT_SIZE, &white);

    if(s1)
    {
        loc.x = x;
        loc.y = y;
        SDL_BlitSurface(s1, NULL, screen, &loc);
    }
    if(s2)
    {
        loc.x = x;
        loc.y = s1->h + loc.y ;
        SDL_BlitSurface(s2, NULL, screen, &loc);
    }

    SDL_UpdateRect(screen, 0, 0, 0, 0);

    SDL_FreeSurface(s1);
    SDL_FreeSurface(s2);
}


void factoroids_level_message(int wave)
{
    SDL_Rect rect;
    SDL_Surface *bgsurf=NULL;
    int nwave;

    char objs_str[PRIME_MAX_LIMIT][MAX_CHAR_MSG] =
    {
        N_("Powers of 2"),
        N_("Products of 2 and 3"),
        N_("Products of 2, 3 and 5"),
        N_("Products of 2, 3, 5 and 7"),
        N_("Products of 2, 3, 5, 7, and 11"),
        N_("Products of 2, 3, 5, 7, 11 and 13")
    };

    char hints_str[PRIME_MAX_LIMIT][MAX_CHAR_MSG] =
    {
        N_("All multiples of 2 end in 2, 4, 6, 8, or 0"),
        N_("The digits of a multiple of 3 add up to a multiple of 3"),
        N_("All multiples of 5 end in 0 or 5"),
        N_("Sorry - there is no simple rule to identify multiples of 7."),
        N_("Under 100, multiples of 11 have equal digits, such as 55 or 88."),
        N_("Sorry - there is no simple rule to identify multiples of 13."),
    };

    rect.x = (screen->w/2)-(LVL_WIDTH_MSG/2);
    rect.y = (screen->h/2)-(LVL_HEIGHT_MSG/2);

    bgsurf = T4K_CreateButton(LVL_WIDTH_MSG,LVL_HEIGHT_MSG,12,19,19,96,96);

    if(bgsurf)
    {
        SDL_BlitSurface(bgsurf, NULL, screen, &rect );
        SDL_FreeSurface(bgsurf);
    }

    nwave = (wave > PRIME_MAX_LIMIT) ? PRIME_MAX_LIMIT : wave;

    factoroids_level_objs_hints(_("Objectives:"), _(objs_str[nwave-1]), rect.x+LVL_OBJ_X_OFFSET, rect.y+LVL_OBJ_Y_OFFSET);
    factoroids_level_objs_hints(_("Hints:"), _(hints_str[nwave-1]), rect.x+LVL_HINT_X_OFFSET, rect.y+LVL_HINT_Y_OFFSET-10);

    SDL_Flip(screen);

    wait_for_input();
}


int tuxship_img_h(int deg)
{
    return IMG_tuxship[deg]->h;
}


int tuxship_img_w(int deg)
{
    return IMG_tuxship[deg]->w;
}


SDL_Surface* get_asteroid_image(int size,int angle)
{
    if (size == 0)
        return IMG_asteroids1[angle/DEG_PER_ROTATION];
    else
        return IMG_asteroids2[angle/DEG_PER_ROTATION];
}
