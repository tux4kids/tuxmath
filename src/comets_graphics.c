#include "comets_graphics.h"

#include "fileops.h"
#include "frame_counter.h"
#include "globals.h"
#include "options.h"
#include "multiplayer.h"
#include "tuxmath.h"


void comets_draw_background(SDL_Surface *bkgd, int wave)
{
    static int old_wave = 0; //update wave immediately
    static Uint32 bgcolor, fgcolor = 0;
    SDL_Rect dest;

    if (fgcolor == 0)
        fgcolor = SDL_MapRGB(screen->format, 64, 96, 64);
    if (old_wave != wave)
    {
        DEBUGMSG(debug_game,"Wave %d\n", wave);
        old_wave = wave;
        bgcolor = SDL_MapRGB(screen->format,
                64,
                64 + ((wave * 32) % 192),
                128 - ((wave * 16) % 128) );
        DEBUGMSG(debug_game,"Filling screen with color %d\n", bgcolor);
    }

    if (bkgd == NULL || (bkgd->w != screen->w && bkgd->h != screen->h))
    {
        dest.x = 0;
        dest.y = 0;
        dest.w = screen->w;
        dest.h = ((screen->h) / 4) * 3;

        SDL_FillRect(screen, &dest, bgcolor);


        dest.y = ((screen->h) / 4) * 3;
        dest.h = (screen->h) / 4;

        SDL_FillRect(screen, &dest, fgcolor);
    }

    if (bkgd)
    {
        dest.x = (screen->w - bkgd->w) / 2;
        dest.y = (screen->h - bkgd->h) / 2;
        SDL_BlitSurface(bkgd, NULL, screen, &dest);
    }
}

/* Draw comets: */
/* NOTE bonus comets split into separate pass to make them */
/* draw last (i.e. in front), as they can overlap          */
void comets_draw_comets(const comet_type *comets)
{

    int i;
    bool answered, num_draw;
    SDL_Surface* img = NULL;
    SDL_Rect dest;

    /* First draw regular comets: */
    for (i = 0; i < MAX_MAX_COMETS; i++)
    {
        if (comets[i].alive && !comets[i].bonus)
        {
            if (comets[i].expl == -1)
            {
                answered = 0;
                /* Decide which image to display: */
                img = sprites[IMG_COMET]->frame[(FC_sprite_counter + i) % sprites[IMG_COMET]->num_frames];
                /* Display the formula (flashing, in the bottom half
                   of the screen) */
                if (comets[i].y < screen->h / 2 || FC_sprite_counter % 8 < 6)
                    num_draw = 1;
                else
                    num_draw = 0;
            }
            else
            {
                /* show each frame of explosion twice */
                img = sprites[IMG_COMET_EXPL]->frame[comets[i].expl / 2];
                num_draw = 1;
                answered = 1;
            }

            /* Draw it! */
            dest.x = comets[i].x - (img->w / 2);
            dest.y = comets[i].y - img->h;
            dest.w = img->w;
            dest.h = img->h;
            SDL_BlitSurface(img, NULL, screen, &dest);

            if (num_draw)
            {
                comets_draw_comet_nums(&comets[i], answered, &white);
            }
        }
    }

    /* Now draw any bonus comets: */
    for (i = 0; i < MAX_MAX_COMETS; i++)
    {
        if (comets[i].alive && comets[i].bonus)
        {
            if (comets[i].expl == -1)
            {
                answered = 0;
                /* Decide which image to display: */
                img = sprites[IMG_BONUS_COMET]->frame[(FC_sprite_counter + i) % sprites[IMG_BONUS_COMET]->num_frames];
                /* Display the formula (flashing, in the bottom half
                   of the screen) */
                if (comets[i].y < screen->h / 2 || FC_sprite_counter % 8 < 6)
                    num_draw = 1;
                else
                    num_draw = 0;
            }
            else
            {
                answered = 1;
                num_draw = 1;
                img = sprites[IMG_BONUS_COMET_EXPL]->frame[comets[i].expl / 2];
            }

            /* Draw it! */
            dest.x = comets[i].x - (img->w / 2);
            dest.y = comets[i].y - img->h;
            dest.w = img->w;
            dest.h = img->h;
            SDL_BlitSurface(img, NULL, screen, &dest);
            if (num_draw)
                comets_draw_comet_nums(&comets[i], answered, &white);
        }
    }
}



void comets_draw_cities(int igloo_vertical_offset,
        const cloud_type *cloud, const city_type *cities,
        const penguin_type *penguins, const steam_type *steam)
{
    int i, j, current_layer, max_layer;
    SDL_Rect src, dest;
    SDL_Surface* this_image;

    if (Opts_GetGlobalOpt(USE_IGLOOS)) {
        /* We have to draw respecting layering */
        current_layer = 0;
        max_layer = 0;
        do {
            for (i = 0; i < NUM_CITIES; i++) {
                if (cities[i].status != CITY_GONE && cities[i].layer > max_layer)
                    max_layer = cities[i].layer;
                if (penguins[i].status != PENGUIN_OFFSCREEN && penguins[i].layer > max_layer)
                    max_layer = penguins[i].layer;
                if (steam[i].status == STEAM_ON && steam[i].layer > max_layer)
                    max_layer = steam[i].layer;
                if (cities[i].layer == current_layer &&
                        cities[i].img != 0) {
                    // Handle the blended igloo images, which are encoded
                    // (FIXME) with a negative image number
                    if (cities[i].img <= 0)
                        this_image = blended_igloos[-cities[i].img];
                    else
                        this_image = images[cities[i].img];
                    //this_image = blended_igloos[frame % NUM_BLENDED_IGLOOS];
                    dest.x = cities[i].x - (this_image->w / 2);
                    dest.y = (screen->h) - (this_image->h) - igloo_vertical_offset;
                    if (cities[i].img == IMG_IGLOO_MELTED3 ||
                            cities[i].img == IMG_IGLOO_MELTED2)
                        dest.y -= (images[IMG_IGLOO_MELTED1]->h - this_image->h)/2;
                    dest.w = (this_image->w);
                    dest.h = (this_image->h);
                    SDL_BlitSurface(this_image, NULL, screen, &dest);
                }
                if (penguins[i].layer == current_layer &&
                        penguins[i].status != PENGUIN_OFFSCREEN) {
                    this_image = images[penguins[i].img];
                    if (penguins[i].status == PENGUIN_WALKING_OFF ||
                            penguins[i].status == PENGUIN_WALKING_ON) {
                        /* With walking penguins, we have to use flipped images
                           when it's walking left. The other issue is that the
                           images are of different widths, so aligning on the
                           center produces weird forward-backward walking. The
                           reliable way is the align them all on the tip of the
                           beak (the right border of the unflipped image) */
                        dest.x = penguins[i].x - (this_image->w / 2);
                        dest.y = (screen->h) - (this_image->h);
                        if ((i<NUM_CITIES/2 && penguins[i].status==PENGUIN_WALKING_OFF) ||
                                (i>=NUM_CITIES/2 && penguins[i].status==PENGUIN_WALKING_ON)) {
                            /* walking left */
                            this_image = flipped_images[flipped_img_lookup[penguins[i].img]];
                            dest.x = penguins[i].x - images[IMG_PENGUIN_WALK_OFF2]->w/2;
                        } else
                            dest.x = penguins[i].x - this_image->w
                                + images[IMG_PENGUIN_WALK_OFF2]->w/2;   /* walking right */
                    }
                    else {
                        dest.x = penguins[i].x - (this_image->w / 2);
                        dest.y = (screen->h) - (5*(this_image->h))/4 - igloo_vertical_offset;
                    }
                    dest.w = (this_image->w);
                    dest.h = (this_image->h);
                    SDL_BlitSurface(this_image, NULL, screen, &dest);
                }
                if (steam[i].layer == current_layer &&
                        steam[i].status == STEAM_ON) {
                    this_image = images[steam[i].img];
                    dest.x = cities[i].x - (this_image->w / 2);
                    dest.y = (screen->h) - this_image->h - ((4 * images[IMG_IGLOO_INTACT]->h) / 7);
                    dest.w = (this_image->w);
                    dest.h = (this_image->h);
                    SDL_BlitSurface(this_image, NULL, screen, &dest);
                }
            }
            current_layer++;
        } while (current_layer <= max_layer);
        if (cloud->status == EXTRA_LIFE_ON) {
            /* Render cloud & snowflakes */
            for (i = 0; i < NUM_SNOWFLAKES; i++) {
                if (cloud->snowflake_y[i] > cloud->y &&
                        cloud->snowflake_y[i] < screen->h - igloo_vertical_offset) {
                    this_image = images[IMG_SNOW1+cloud->snowflake_size[i]];
                    dest.x = cloud->snowflake_x[i] - this_image->w/2 + cloud->x;
                    dest.y = cloud->snowflake_y[i] - this_image->h/2;
                    dest.w = this_image->w;
                    dest.h = this_image->h;
                    SDL_BlitSurface(this_image, NULL, screen, &dest);
                }
            }
            this_image = images[IMG_CLOUD];
            dest.x = cloud->x - this_image->w/2;
            dest.y = cloud->y - this_image->h/2;
            dest.w = this_image->w;
            dest.h = this_image->h;
            SDL_BlitSurface(this_image, NULL, screen, &dest);
        }
    }
    else {
        /* We're drawing original city graphics, for which there are no
           layering issues, but has special handling for the shields */
        for (i = 0; i < NUM_CITIES; i++) {
            this_image = images[cities[i].img];
            dest.x = cities[i].x - (this_image->w / 2);
            dest.y = (screen->h) - (this_image->h);
            dest.w = (this_image->w);
            dest.h = (this_image->h);
            SDL_BlitSurface(this_image, NULL, screen, &dest);

            /* Draw sheilds: */
            if (cities[i].hits_left > 1) {
                for (j = (FC_sprite_counter % 3); j < images[IMG_SHIELDS]->h; j = j + 3) {
                    src.x = 0;
                    src.y = j;
                    src.w = images[IMG_SHIELDS]->w;
                    src.h = 1;

                    dest.x = cities[i].x - (images[IMG_SHIELDS]->w / 2);
                    dest.y = (screen->h) - (images[IMG_SHIELDS]->h) + j;
                    dest.w = src.w;
                    dest.h = src.h;

                    SDL_BlitSurface(images[IMG_SHIELDS], &src, screen, &dest);
                }
            }
        }
    }


}



void comets_draw_misc(MC_MathGame *curr_game, int wave,
        int extra_life_earned, int bonus_comet_counter, int score,
        int total_questions_left, help_controls_type *help_controls)
{
    int i;
    int offset;
    SDL_Rect dest;
    char str[64];

    /* Draw "Demo" */
    if (Opts_DemoMode())
    {
        dest.x = (screen->w - images[IMG_DEMO]->w) / 2;
        dest.y = (screen->h - images[IMG_DEMO]->h) / 2;
        dest.w = images[IMG_DEMO]->w;
        dest.h = images[IMG_DEMO]->h;

        SDL_BlitSurface(images[IMG_DEMO], NULL, screen, &dest);
    }

    /* If we are playing through a defined list of questions */
    /* without "recycling", display number of remaining questions: */
    if (MC_GetOpt(curr_game, PLAY_THROUGH_LIST) )
    {
        comets_draw_question_counter(curr_game, total_questions_left);
    }

    if (extra_life_earned) {
        /* Draw extra life earned icon */
        dest.x = 0;
        dest.y = 0;
        dest.w = images[IMG_EXTRA_LIFE]->w;
        dest.h = images[IMG_EXTRA_LIFE]->h;
        SDL_BlitSurface(images[IMG_EXTRA_LIFE], NULL, screen, &dest);
    } else if (bonus_comet_counter) {
        /* Draw extra life progress bar */
        dest.x = 0;
        dest.y = images[IMG_EXTRA_LIFE]->h/4;
        dest.h = images[IMG_EXTRA_LIFE]->h/2;
        dest.w = ((Opts_BonusCometInterval() + 1 - bonus_comet_counter)
                * images[IMG_EXTRA_LIFE]->w) / Opts_BonusCometInterval();
        SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 255, 0));
    }

    /* Draw wave: */
    if (Opts_BonusCometInterval())
        offset = images[IMG_EXTRA_LIFE]->w + 5;
    else
        offset = 0;

    dest.x = offset;
    dest.y = glyph_offset;
    dest.w = images[IMG_WAVE]->w;
    dest.h = images[IMG_WAVE]->h;

    SDL_BlitSurface(images[IMG_WAVE], NULL, screen, &dest);

    sprintf(str, "%d", wave);
    draw_numbers(screen, str, offset+images[IMG_WAVE]->w + (images[IMG_NUMBERS]->w / 10), 0);

    if (Opts_KeepScore())
    {
        /* Draw "score" label: */
        dest.x = (screen->w - ((images[IMG_NUMBERS]->w / 10) * 7) -
                images[IMG_SCORE]->w -
                images[IMG_STOP]->w - 5);
        dest.y = glyph_offset;
        dest.w = images[IMG_SCORE]->w;
        dest.h = images[IMG_SCORE]->h;
        SDL_BlitSurface(images[IMG_SCORE], NULL, screen, &dest);

        /* In LAN mode, we show the server-generated score: */
        if(Opts_LanMode())
        {
            sprintf(str, "%.6d", LAN_PlayerScore(LAN_MyIndex()));
        }
        else
            sprintf(str, "%.6d", score);

        /* Draw score numbers: */
        draw_numbers(screen, str, screen->w - ((images[IMG_NUMBERS]->w / 10) * 6) - images[IMG_STOP]->w - 5, 0);
    }

    /* Draw other players' scores (turn-based single machine multiplayer) */
    if (mp_get_parameter(PLAYERS)) // && mp_get_parameter(MODE) == SCORE_SWEEP )
    {
        int i;
        SDL_Surface* score_surf = NULL;
        SDL_Rect loc;

        //Adjust font size for resolution:
        int fontsize = (int)(DEFAULT_MENU_FONT_SIZE * get_scale());

        for (i = 0; i < mp_get_parameter(PLAYERS); ++i)
        {
            snprintf(str, 64, "%s: %d", mp_get_player_name(i), mp_get_player_score(i));
            score_surf = T4K_BlackOutline(str, fontsize, &white);
            if(score_surf)
            {
                loc.x = 0;
                loc.y = score_surf->h * (i + 2);
                loc.w = score_surf->w;
                loc.h = score_surf->h;

                SDL_BlitSurface(score_surf, NULL, screen, &loc);
                SDL_FreeSurface(score_surf);
                score_surf = NULL;
            }
        }
    }

    /* Draw other players' scores (LAN game) */
    if (Opts_LanMode())
    {
        int entries = 0;
        SDL_Surface* score_surf = NULL;
        SDL_Rect loc;

        //Adjust font size for resolution:
        int fontsize = (int)(DEFAULT_MENU_FONT_SIZE * get_scale());

        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if(LAN_PlayerConnected(i))
            {
                snprintf(str, 64, "%s: %d",  LAN_PlayerName(i),  LAN_PlayerScore(i));
                if(LAN_PlayerMine(i))
                    score_surf = T4K_BlackOutline(str, fontsize, &yellow);
                else
                    score_surf = T4K_BlackOutline(str, fontsize, &white);
                if(score_surf)
                {
                    loc.w = score_surf->w;
                    loc.h = score_surf->h;
                    loc.x = 0;
                    loc.y = score_surf->h * (entries + 2);
                    SDL_BlitSurface(score_surf, NULL, screen, &loc);
                    entries++;
                    SDL_FreeSurface(score_surf);
                    score_surf = NULL;
                }
            }
        }
    }

    /* Draw stop button: */
    if (!help_controls->x_is_blinking || (FC_sprite_counter % 10 < 5)) {
        dest.x = (screen->w - images[IMG_STOP]->w);
        dest.y = 0;
        dest.w = images[IMG_STOP]->w;
        dest.h = images[IMG_STOP]->h;

        SDL_BlitSurface(images[IMG_STOP], NULL, screen, &dest);
    }
}


/* Draw numbers/symbols over the attacker: */
/* This draws the numbers related to the comets */
void comets_draw_comet_nums(const comet_type *comet, bool answered, SDL_Color *col)
{
    if(!comet || !col)
        return;

    SDL_Surface* surf = answered ? comet->answer_surf : comet->formula_surf;
    if(surf)
    {
        int w = T4K_GetScreen()->w;
        int x = comet->x;
        int y = comet->y;
        x -= surf->w/2;
        // Keep formula at least 8 pixels inside screen:
        if(surf->w + x > (w - 8))
            x = w - 8 - surf->w;
        if(x < 8)
            x = 8;
        //Draw numbers over comet:
        y -= surf->h;

        SDL_Rect pos = {x, y};
        SDL_BlitSurface(surf, NULL, T4K_GetScreen(), &pos);
    }
}


void comets_draw_question_counter(MC_MathGame* curr_game, int total_questions_left)
{
    int questions_left;
    int comet_img;
    int nums_width;
    int nums_x;
    int comet_width;
    int comet_x;

    char str[64];
    SDL_Rect dest;

    /* Calculate placement based on image widths: */
    nums_width = (images[IMG_NUMBERS]->w / 10) * 4; /* displaying 4 digits */
    comet_width = images[IMG_MINI_COMET1]->w;
    comet_x = (screen->w)/2 - (comet_width + nums_width)/2;
    nums_x = comet_x + comet_width;

    /* Draw mini comet symbol:            */
    /* Decide which image to display: */
    comet_img = IMG_MINI_COMET1 + (FC_sprite_counter % 3);
    /* Draw it! */
    dest.x = comet_x;
    dest.y = 0;
    dest.w = comet_width;
    dest.h = images[comet_img]->h;

    SDL_BlitSurface(images[comet_img], NULL, screen, &dest);

    /* draw number of remaining questions: */
    if(Opts_LanMode())
        questions_left = total_questions_left;
    else
        questions_left = MC_TotalQuestionsLeft(curr_game);

    sprintf(str, "%.4d", questions_left);
    draw_numbers(screen, str, nums_x, 0);
}


/* FIXME very confusing having this function draw console */
void comets_draw_led_console(MC_MathGame *curr_game, int neg_answer_picked, int *digits)
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
    if (MC_GetOpt(curr_game, ALLOW_NEGATIVES) )
        dest.x = ((screen->w - ((images[IMG_LEDNUMS]->w) / 10) * 4) / 2);
    else
        dest.x = ((screen->w - ((images[IMG_LEDNUMS]->w) / 10) * 3) / 2);

    for (i = -1; i < MC_MAX_DIGITS; i++) /* -1 is special case to allow minus sign */
        /* with minimal modification of existing code DSB */
    {
        if (-1 == i)
        {
            if (MC_GetOpt(curr_game, ALLOW_NEGATIVES))
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



void comets_draw_powerup(powerup_comet_type* powerup_comet)
{
    SDL_Surface* img = NULL;
    SDL_Rect dest;
    int imgid, answered, num_draw ;
    if(powerup_comet == NULL)
        return;

    if(!powerup_comet->comet.alive)
        return;

    if(powerup_comet->comet.expl == -1)
    {
        answered = 0;
        if(powerup_comet->direction == POWERUP_DIR_LEFT)
            imgid = IMG_LEFT_POWERUP_COMET;
        else
            imgid = IMG_RIGHT_POWERUP_COMET;

        img = sprites[imgid]->frame[FC_sprite_counter % sprites[imgid]->num_frames];
        if(!img)
            return;

        if(powerup_comet->comet.x >= img->w/2 &&
                powerup_comet->comet.x <= screen->w - img->w/2)
        {
            num_draw = 1;
        }
        else
        {
            num_draw = 0;
        }
    }
    else
    {
        answered = 1;
        num_draw = 1;
        /* show each frame of explosion twice */
        img = sprites[IMG_POWERUP_COMET_EXPL]->frame[powerup_comet->comet.expl / 2];
    }

    /* Draw it! */
    dest.x = powerup_comet->comet.x - (img->w/2);
    dest.y = powerup_comet->comet.y - img->h;
    dest.w = img->w;
    dest.h = img->h;

    SDL_BlitSurface(img, NULL, screen, &dest);
    if (num_draw)
    {
        comets_draw_comet_nums(&(powerup_comet->comet), answered, &white);
    }
}


void comets_draw_smartbomb(int smartbomb_alive)
{
    SDL_Surface* img;
    SDL_Rect rect;
    char* txt = "[Shift]";
    int fontsize = 18;
    if(!smartbomb_alive)
        return;
    img = images[IMG_BONUS_POWERBOMB];
    if(img)
    {
        rect.x = screen->w - img->w;
        rect.y = (screen->h * 0.7) - img->h;
        rect.w = img->w;
        rect.h = img->h;
        SDL_BlitSurface(img, NULL, screen, &rect);
    }

    img = T4K_BlackOutline(txt, fontsize, &white);
    if(img)
    {
        rect.y += rect.h;
        SDL_BlitSurface(img, NULL, screen, &rect);
        SDL_FreeSurface(img);
    }
}

