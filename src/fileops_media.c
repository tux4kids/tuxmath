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
  DATA_PREFIX "/images/status/standby.png",
  DATA_PREFIX "/images/title/menu_bkg.jpg",
  DATA_PREFIX "/images/title/title1.png",
  DATA_PREFIX "/images/status/title.png",
  DATA_PREFIX "/images/status/left.png",
  DATA_PREFIX "/images/status/left_gray.png",
  DATA_PREFIX "/images/status/right.png",
  DATA_PREFIX "/images/status/right_gray.png",
  DATA_PREFIX "/images/status/tux4kids.png",
  DATA_PREFIX "/images/status/nbs.png",
  DATA_PREFIX "/images/cities/city-blue.png",
  DATA_PREFIX "/images/cities/csplode-blue-1.png",
  DATA_PREFIX "/images/cities/csplode-blue-2.png",
  DATA_PREFIX "/images/cities/csplode-blue-3.png",
  DATA_PREFIX "/images/cities/csplode-blue-4.png",
  DATA_PREFIX "/images/cities/csplode-blue-5.png",
  DATA_PREFIX "/images/cities/cdead-blue.png",
  DATA_PREFIX "/images/cities/city-green.png",
  DATA_PREFIX "/images/cities/csplode-green-1.png",
  DATA_PREFIX "/images/cities/csplode-green-2.png",
  DATA_PREFIX "/images/cities/csplode-green-3.png",
  DATA_PREFIX "/images/cities/csplode-green-4.png",
  DATA_PREFIX "/images/cities/csplode-green-5.png",
  DATA_PREFIX "/images/cities/cdead-green.png",
  DATA_PREFIX "/images/cities/city-orange.png",
  DATA_PREFIX "/images/cities/csplode-orange-1.png",
  DATA_PREFIX "/images/cities/csplode-orange-2.png",
  DATA_PREFIX "/images/cities/csplode-orange-3.png",
  DATA_PREFIX "/images/cities/csplode-orange-4.png",
  DATA_PREFIX "/images/cities/csplode-orange-5.png",
  DATA_PREFIX "/images/cities/cdead-orange.png",
  DATA_PREFIX "/images/cities/city-red.png",
  DATA_PREFIX "/images/cities/csplode-red-1.png",
  DATA_PREFIX "/images/cities/csplode-red-2.png",
  DATA_PREFIX "/images/cities/csplode-red-3.png",
  DATA_PREFIX "/images/cities/csplode-red-4.png",
  DATA_PREFIX "/images/cities/csplode-red-5.png",
  DATA_PREFIX "/images/cities/cdead-red.png",
  DATA_PREFIX "/images/cities/shields.png",
  DATA_PREFIX "/images/comets/comet1.png",
  DATA_PREFIX "/images/comets/comet2.png",
  DATA_PREFIX "/images/comets/comet3.png",
  DATA_PREFIX "/images/comets/cometex3.png",
  DATA_PREFIX "/images/comets/cometex3.png",
  DATA_PREFIX "/images/comets/cometex2.png",
  DATA_PREFIX "/images/comets/cometex2.png",
  DATA_PREFIX "/images/comets/cometex1a.png",
  DATA_PREFIX "/images/comets/cometex1a.png",
  DATA_PREFIX "/images/comets/cometex1.png",
  DATA_PREFIX "/images/comets/cometex1.png",
  DATA_PREFIX "/images/comets/mini_comet1.png",
  DATA_PREFIX "/images/comets/mini_comet2.png",
  DATA_PREFIX "/images/comets/mini_comet3.png",
  DATA_PREFIX "/images/comets/bonus_comet1.png",
  DATA_PREFIX "/images/comets/bonus_comet2.png",
  DATA_PREFIX "/images/comets/bonus_comet3.png",
  DATA_PREFIX "/images/comets/bonus_cometex3.png",
  DATA_PREFIX "/images/comets/bonus_cometex3.png",
  DATA_PREFIX "/images/comets/bonus_cometex2.png",
  DATA_PREFIX "/images/comets/bonus_cometex2.png",
  DATA_PREFIX "/images/comets/bonus_cometex1a.png",
  DATA_PREFIX "/images/comets/bonus_cometex1a.png",
  DATA_PREFIX "/images/comets/bonus_cometex1.png",
  DATA_PREFIX "/images/comets/bonus_cometex1.png",
  DATA_PREFIX "/images/status/nums.png",
  DATA_PREFIX "/images/status/lednums.png",
  DATA_PREFIX "/images/status/led_neg_sign.png",
  DATA_PREFIX "/images/status/paused.png",
  DATA_PREFIX "/images/status/demo.png",
  DATA_PREFIX "/images/status/demo-small.png",
  DATA_PREFIX "/images/status/keypad.png",
  DATA_PREFIX "/images/status/keypad_no_neg.png",
  DATA_PREFIX "/images/tux/console_led.png",
  DATA_PREFIX "/images/tux/console_bash.png",
  DATA_PREFIX "/images/tux/tux-console1.png",
  DATA_PREFIX "/images/tux/tux-console2.png",
  DATA_PREFIX "/images/tux/tux-console3.png",
  DATA_PREFIX "/images/tux/tux-console4.png",
  DATA_PREFIX "/images/tux/tux-relax1.png",
  DATA_PREFIX "/images/tux/tux-relax2.png",
  DATA_PREFIX "/images/tux/tux-egypt1.png",
  DATA_PREFIX "/images/tux/tux-egypt2.png",
  DATA_PREFIX "/images/tux/tux-egypt3.png",
  DATA_PREFIX "/images/tux/tux-egypt4.png",
  DATA_PREFIX "/images/tux/tux-drat.png",
  DATA_PREFIX "/images/tux/tux-yipe.png",
  DATA_PREFIX "/images/tux/tux-yay1.png",
  DATA_PREFIX "/images/tux/tux-yay2.png",
  DATA_PREFIX "/images/tux/tux-yes1.png",
  DATA_PREFIX "/images/tux/tux-yes2.png",
  DATA_PREFIX "/images/tux/tux-sit.png",
  DATA_PREFIX "/images/tux/tux-fist1.png",
  DATA_PREFIX "/images/tux/tux-fist2.png",
  DATA_PREFIX "/images/penguins/flapdown.png",
  DATA_PREFIX "/images/penguins/flapup.png",
  DATA_PREFIX "/images/penguins/incoming.png",
  DATA_PREFIX "/images/penguins/grumpy.png",
  DATA_PREFIX "/images/penguins/worried.png",
  DATA_PREFIX "/images/penguins/standing-up.png",
  DATA_PREFIX "/images/penguins/sitting-down.png",
  DATA_PREFIX "/images/penguins/walk-on1.png",
  DATA_PREFIX "/images/penguins/walk-on2.png",
  DATA_PREFIX "/images/penguins/walk-on3.png",
  DATA_PREFIX "/images/penguins/walk-off1.png",
  DATA_PREFIX "/images/penguins/walk-off2.png",
  DATA_PREFIX "/images/penguins/walk-off3.png",
  DATA_PREFIX "/images/igloos/melted3.png",
  DATA_PREFIX "/images/igloos/melted2.png",
  DATA_PREFIX "/images/igloos/melted1.png",
  DATA_PREFIX "/images/igloos/half.png",
  DATA_PREFIX "/images/igloos/intact.png",
  DATA_PREFIX "/images/igloos/rebuilding1.png",
  DATA_PREFIX "/images/igloos/rebuilding2.png",
  DATA_PREFIX "/images/igloos/steam1.png",
  DATA_PREFIX "/images/igloos/steam2.png",
  DATA_PREFIX "/images/igloos/steam3.png",
  DATA_PREFIX "/images/igloos/steam4.png",
  DATA_PREFIX "/images/igloos/steam5.png",
  DATA_PREFIX "/images/igloos/cloud.png",
  DATA_PREFIX "/images/igloos/snow1.png",
  DATA_PREFIX "/images/igloos/snow2.png",
  DATA_PREFIX "/images/igloos/snow3.png",
  DATA_PREFIX "/images/igloos/extra_life.png",
  DATA_PREFIX "/images/status/wave.png",
  DATA_PREFIX "/images/status/score.png",
  DATA_PREFIX "/images/status/stop.png",
  DATA_PREFIX "/images/status/numbers.png",
  DATA_PREFIX "/images/status/gameover.png",
  DATA_PREFIX "/images/status/gameover_won.png",
  DATA_PREFIX "/images/factoroids/gbstars.png",
  DATA_PREFIX "/images/factoroids/asteroid1.png",
  DATA_PREFIX "/images/factoroids/asteroid2.png",
  DATA_PREFIX "/images/factoroids/asteroid3.png",
  DATA_PREFIX "/images/factoroids/ship01.png",
  DATA_PREFIX "/images/factoroids/factoroids.png",
  DATA_PREFIX "/images/factoroids/factors.png",
  DATA_PREFIX "/images/factoroids/tux.png",
  DATA_PREFIX "/images/factoroids/good.png"
  };

  /* Load images: */
  for (i = 0; i < NUM_IMAGES; i++)
  {
    images[i] = LoadImageFromFile(image_filenames[i]);

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
