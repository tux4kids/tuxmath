/* fileops_media.c
   ±±±±
   Load media files from disk.

   Copyright 2006, 2007, 2008, 2009, 2010, 2011.
Author: David Bruce, Tim Holy, Boleslaw Kulbabinski, Brendan Luchen.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org


fileops.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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
#include "fileops.h"
#include "options.h"

int glyph_offset;

/*****************************************************************/
/*   Loading of data files for images and sounds.                */
/*   These functions also draw some user feedback to             */
/*   display the progress of the loading.                        */
/*****************************************************************/

/* returns 1 if all data files successfully loaded, 0 otherwise. */

/* TODO load only "igloo" or "city" files, not both.             */
/* TODO get rid of files no longer used.                         */

int load_image_data()
{
    int i;

    static char* image_filenames[NUM_IMAGES] = {
        "status/title.png",
        "status/left.png",
        "status/left_gray.png",
        "status/right.png",
        "status/right_gray.png",
        "status/tux4kids.png",
        "status/nbs.png",
        "cities/city-blue.png",
        "cities/csplode-blue-1.png",
        "cities/csplode-blue-2.png",
        "cities/csplode-blue-3.png",
        "cities/csplode-blue-4.png",
        "cities/csplode-blue-5.png",
        "cities/cdead-blue.png",
        "cities/city-green.png",
        "cities/csplode-green-1.png",
        "cities/csplode-green-2.png",
        "cities/csplode-green-3.png",
        "cities/csplode-green-4.png",
        "cities/csplode-green-5.png",
        "cities/cdead-green.png",
        "cities/city-orange.png",
        "cities/csplode-orange-1.png",
        "cities/csplode-orange-2.png",
        "cities/csplode-orange-3.png",
        "cities/csplode-orange-4.png",
        "cities/csplode-orange-5.png",
        "cities/cdead-orange.png",
        "cities/city-red.png",
        "cities/csplode-red-1.png",
        "cities/csplode-red-2.png",
        "cities/csplode-red-3.png",
        "cities/csplode-red-4.png",
        "cities/csplode-red-5.png",
        "cities/cdead-red.png",
        "cities/shields.png",
        "comets/mini_comet1.png",
        "comets/mini_comet2.png",
        "comets/mini_comet3.png",
        "status/nums.png",
        "status/lednums.png",
        "status/led_neg_sign.png",
        "status/paused.png",
        "status/demo.png",
        "status/demo-small.png",
        "status/keypad.png",
        "status/keypad_no_neg.png",
        "tux/console_led.png",
        "tux/console_bash.png",
        "tux/tux-console1.png",
        "tux/tux-console2.png",
        "tux/tux-console3.png",
        "tux/tux-console4.png",
        "tux/tux-relax1.png",
        "tux/tux-relax2.png",
        "tux/tux-egypt1.png",
        "tux/tux-egypt2.png",
        "tux/tux-egypt3.png",
        "tux/tux-egypt4.png",
        "tux/tux-drat.png",
        "tux/tux-yipe.png",
        "tux/tux-yay1.png",
        "tux/tux-yay2.png",
        "tux/tux-yes1.png",
        "tux/tux-yes2.png",
        "tux/tux-sit.png",
        "tux/tux-fist1.png",
        "tux/tux-fist2.png",
        "penguins/flapdown.png",
        "penguins/flapup.png",
        "penguins/incoming.png",
        "penguins/grumpy.png",
        "penguins/worried.png",
        "penguins/standing-up.png",
        "penguins/sitting-down.png",
        "penguins/walk-on1.png",
        "penguins/walk-on2.png",
        "penguins/walk-on3.png",
        "penguins/walk-off1.png",
        "penguins/walk-off2.png",
        "penguins/walk-off3.png",
        "igloos/melted3.png",
        "igloos/melted2.png",
        "igloos/melted1.png",
        "igloos/half.png",
        "igloos/intact.png",
        "igloos/rebuilding1.png",
        "igloos/rebuilding2.png",
        "igloos/steam1.png",
        "igloos/steam2.png",
        "igloos/steam3.png",
        "igloos/steam4.png",
        "igloos/steam5.png",
        "igloos/cloud.png",
        "igloos/snow1.png",
        "igloos/snow2.png",
        "igloos/snow3.png",
        "igloos/extra_life.png",
        "status/wave.png",
        "status/score.png",
        "status/stop.png",
        "status/numbers.png",
        "status/gameover.png",
        "status/gameover_won.png",
        "factoroids/gbstars.png",
        "factoroids/asteroid1.png",
        "factoroids/asteroid2.png",
        "factoroids/asteroid3.png",
        "factoroids/ship.png",
        "factoroids/ship-cloaked.png",
        "factoroids/powerbomb.png",
        "factoroids/shield.png",
        "factoroids/stealth.png",
        "factoroids/factoroids.png",
        "factoroids/factors.png",
        "factoroids/tux.png",
        "factoroids/good.png",
        "tux/cockpit_tux1.png",
        "tux/cockpit_tux2.png",
        "tux/cockpit_tux3.png",
        "tux/cockpit_tux4.png",
        "tux/cockpit_tux5.png",
        "tux/cockpit_tux6.png",
        "factoroids/button_2.png",
        "factoroids/button_3.png",
        "factoroids/button_5.png",
        "factoroids/button_7.png",
        "factoroids/button_11.png",
        "factoroids/button_13.png",
        "factoroids/cockpit.png",
        "factoroids/forcefield.png",
        "factoroids/ship-thrust.png",
        "factoroids/ship-thrust-cloaked.png",
        "status/arrows.png"
    };

    static char* sprite_filenames[NUM_SPRITES] = {
        "comets/comet",
        "comets/bonus_comet",
        "comets/cometex",
        "comets/bonus_cometex",
        "comets/left_powerup_comet",
        "comets/right_powerup_comet",
        "comets/powerup_cometex",
        "tux/bigtux"
    };

    /* Load static images: */
    for (i = 0; i < NUM_IMAGES; i++)
    {
        images[i] = T4K_LoadImage(image_filenames[i], IMG_ALPHA);

        if (images[i] == NULL)
        {
            fprintf(stderr,
                    "\nError: I couldn't load a graphics file:\n"
                    "%s\n"
                    "The Simple DirectMedia error that occured was:\n"
                    "%s\n\n", image_filenames[i], SDL_GetError());
            return 0;
        }
    }

    /* Load animated graphics: */
    for (i = 0; i < NUM_SPRITES; i++)
    {
        sprites[i] = T4K_LoadSprite(sprite_filenames[i], IMG_ALPHA);

        if (sprites[i] == NULL)
        {
            fprintf(stderr,
                    "\nError: I couldn't load a graphics file:\n"
                    "%s\n"
                    "The Simple DirectMedia error that occured was:\n"
                    "%s\n\n", sprite_filenames[i], SDL_GetError());
            return 0;
        }
    }

    glyph_offset = 0;

#ifdef REPLACE_WAVESCORE
    /* Replace the "WAVE" and "SCORE" with translate-able versions */
    SDL_FreeSurface(images[IMG_WAVE]);
    images[IMG_WAVE] = T4K_SimpleTextWithOffset(_("WAVE"), 28, &white, &glyph_offset);
    SDL_FreeSurface(images[IMG_SCORE]);
    images[IMG_SCORE] = T4K_SimpleTextWithOffset(_("SCORE"), 28, &white, &glyph_offset);
    glyph_offset++;
#endif

    /* If we make it to here OK, return 1: */
    return 1;
}





#ifndef NOSOUND
int load_sound_data(void)
{
    int i = 0;

    static char* sound_filenames[NUM_SOUNDS] = {
        DATA_PREFIX "/sounds/harp.wav",
        DATA_PREFIX "/sounds/pop.wav",
        DATA_PREFIX "/sounds/tock.wav",
        DATA_PREFIX "/sounds/laser.wav",
        DATA_PREFIX "/sounds/buzz.wav",
        DATA_PREFIX "/sounds/alarm.wav",
        DATA_PREFIX "/sounds/shieldsdown.wav",
        DATA_PREFIX "/sounds/explosion.wav",
        DATA_PREFIX "/sounds/sizzling.wav",
        DATA_PREFIX "/sounds/towerclock.wav",
        DATA_PREFIX "/sounds/cheer.wav",
        DATA_PREFIX "/sounds/engine.wav"
    };


    /* skip loading sound files if sound system not available: */
    if (Opts_UsingSound())
    {
        for (i = 0; i < NUM_SOUNDS; i++)
        {
            sounds[i] = Mix_LoadWAV(sound_filenames[i]);

            if (sounds[i] == NULL)
            {
                fprintf(stderr,
                        "\nError: I couldn't load a sound file:\n"
                        "%s\n"
                        "The Simple DirectMedia error that occured was:\n"
                        "%s\n\n", sound_filenames[i], SDL_GetError());
                return 0;
            }
        }
    }

    //NOTE - no longer load musics here - they are loaded as needed
    return 1;
}

#endif /* NOSOUND */
