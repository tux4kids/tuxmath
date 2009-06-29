#include "tuxmath.h"
#include "fileops.h"
#include "loaders.h"
#include "options.h"
#include "SDL_extras.h"

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
  "status/standby.png",
  "title/menu_bkg.jpg",
  "title/title1.png",
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
  "comets/comet1.svg",
  "comets/comet2.svg",
  "comets/comet3.svg",
  "comets/cometex3.png",
  "comets/cometex3.png",
  "comets/cometex2.png",
  "comets/cometex2.png",
  "comets/cometex1a.png",
  "comets/cometex1a.png",
  "comets/cometex1.png",
  "comets/cometex1.png",
  "comets/mini_comet1.png",
  "comets/mini_comet2.png",
  "comets/mini_comet3.png",
  "comets/bonus_comet1.png",
  "comets/bonus_comet2.png",
  "comets/bonus_comet3.png",
  "comets/bonus_cometex3.png",
  "comets/bonus_cometex3.png",
  "comets/bonus_cometex2.png",
  "comets/bonus_cometex2.png",
  "comets/bonus_cometex1a.png",
  "comets/bonus_cometex1a.png",
  "comets/bonus_cometex1.png",
  "comets/bonus_cometex1.png",
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
  "factoroids/ship01.png",
  "factoroids/factoroids.png",
  "factoroids/factors.png",
  "factoroids/tux.png",
  "factoroids/good.png"
  };

  /* Load images: */
  for (i = 0; i < NUM_IMAGES; i++)
  {
    images[i] = LoadImage(image_filenames[i], IMG_ALPHA);

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

  glyph_offset = 0;

#ifdef REPLACE_WAVESCORE  
  /* Replace the "WAVE" and "SCORE" with translate-able versions */
  SDL_FreeSurface(images[IMG_WAVE]);
  images[IMG_WAVE] = SimpleTextWithOffset(_("WAVE"), 28, &white, &glyph_offset);
  SDL_FreeSurface(images[IMG_SCORE]);
  images[IMG_SCORE] = SimpleTextWithOffset(_("SCORE"), 28, &white, &glyph_offset);
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
  DATA_PREFIX "/sounds/click.wav",
  DATA_PREFIX "/sounds/sizzling.wav",
  DATA_PREFIX "/sounds/towerclock.wav",
  DATA_PREFIX "/sounds/cheer.wav"
  };

  static char* music_filenames[NUM_MUSICS] = {
  DATA_PREFIX "/sounds/game.mod",
  DATA_PREFIX "/sounds/game2.mod",
  DATA_PREFIX "/sounds/game3.mod"
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


    for (i = 0; i < NUM_MUSICS; i++)
    {
      musics[i] = Mix_LoadMUS(music_filenames[i]);

      if (musics[i] == NULL)
      {
        fprintf(stderr,
                "\nError: I couldn't load a music file:\n"
                "%s\n"
                "The Simple DirectMedia error that occured was:\n"
                "%s\n\n", music_filenames[i], SDL_GetError());
        return 0;
      }

    }
  }
  return 1;
}

#endif /* NOSOUND */
