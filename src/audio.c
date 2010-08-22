/* audio.c -  description: this file contains audio related functions
   
   Copyright (C) 2003, 2006, 2007, 2008, 2009, 2010.
   Authors: Sam Hart, Jesse Andrews, David Bruce.
   Project email: <tuxmath-devel@lists.sourceforge.net>
   Project website: http://tux4kids.alioth.debian.org

audio.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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
#include "options.h"   //Needed for Opts_UsingSound()
#include "titlescreen.h"

Mix_Music *music;

void playsound(int snd)
{
#ifndef NOSOUND
  if (Opts_UsingSound())
    Mix_PlayChannel(-1, sounds[snd], 0);
#endif
}

Mix_Music *defaultMusic = NULL; // holds music for audioMusicLoad/unload



/* audioMusicLoad attempts to load and play the music file 
 * Note: loops == -1 means forever
 */
void audioMusicLoad(char *musicFilename, int loops)
{
  if (!Opts_UsingSound())
  {
    return;
  }

  audioMusicUnload(); // make sure defaultMusic is clear
  defaultMusic = LoadMusic(musicFilename);
  Mix_PlayMusic(defaultMusic, loops);
}


/* audioMusicUnload attempts to unload any music data that was
 * loaded using the audioMusicLoad function
 */
void audioMusicUnload( void ) {
  if (!Opts_UsingSound()) return;

  if ( defaultMusic )
    Mix_FreeMusic( defaultMusic );

  defaultMusic=NULL;
}

/* audioMusicPlay attempts to play the passed music data. 
 * if a music file was loaded using the audioMusicLoad
 * it will be stopped and unloaded
 * Note: loops == -1 means forever
 */
void audioMusicPlay( Mix_Music *musicData, int loops ) { 
  if (!Opts_UsingSound()) return;

  audioMusicUnload();        
  Mix_PlayMusic( musicData, loops );
}
