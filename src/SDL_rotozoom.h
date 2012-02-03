/* SDL_rotozoom.h


   The SDL_rotozoom sources were copied from the SDL_gfx library and
   are relicensed, only for the purposes of TuxMath, to GPL.  Original
   license was GNU Lesser Public License, Version 2 (or later).
   Thanks to Andreas Schiffler.

   SDL_gfx website: http://www.ferzkopp.net/Software/SDL_gfx-2.0/

   Slight modification and relicensing for tuxmath:
   Copyright (C) 2008, 2009, 2010, 2011.
Authors: David Bruce, Tim Holy, Brendan Luchen.
email: <tuxmath-devel@lists.sourceforge.net>



SDL_rotozoom.h is part of "Tux, of Math Command", a.k.a. "tuxmath".

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




#ifndef _SDL_rotozoom_h
#define _SDL_rotozoom_h

#include <math.h>

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef M_PI
#define M_PI    3.141592654
#endif

#include "SDL.h"

    /* ---- Defines */

#define SMOOTHING_OFF           0
#define SMOOTHING_ON            1

    /* ---- Structures */

    typedef struct tColorRGBA {
        Uint8 r;
        Uint8 g;
        Uint8 b;
        Uint8 a;
    } tColorRGBA;

    typedef struct tColorY {
        Uint8 y;
    } tColorY;


    /* ---- Prototypes */

    // #ifdef WIN32
    // #ifdef BUILD_DLL
    // #define DLLINTERFACE __declspec(dllexport)
    // #else
    // #define DLLINTERFACE __declspec(dllimport)
    // #endif
    // #else
    // #define DLLINTERFACE
    // #endif

    /* NOTE inactivating above declspec stuff because we are building */
    /* for our own tree                                               */
#define DLLINTERFACE
    /* 

       rotozoomSurface()

       Rotates and zoomes a 32bit or 8bit 'src' surface to newly created 'dst' surface.
       'angle' is the rotation in degrees. 'zoom' a scaling factor. If 'smooth' is 1
       then the destination 32bit surface is anti-aliased. If the surface is not 8bit
       or 32bit RGBA/ABGR it will be converted into a 32bit RGBA format on the fly.

*/

    DLLINTERFACE SDL_Surface *rotozoomSurface(SDL_Surface * src, double angle, double zoom, int smooth);

    DLLINTERFACE SDL_Surface *rotozoomSurfaceXY
        (SDL_Surface * src, double angle, double zoomx, double zoomy, int smooth);

    /* Returns the size of the target surface for a rotozoomSurface() call */

    DLLINTERFACE void rotozoomSurfaceSize(int width, int height, double angle, double zoom, int *dstwidth,
            int *dstheight);

    DLLINTERFACE void rotozoomSurfaceSizeXY
        (int width, int height, double angle, double zoomx, double zoomy, 
         int *dstwidth, int *dstheight);

    /* 

       zoomSurface()

       Zoomes a 32bit or 8bit 'src' surface to newly created 'dst' surface.
       'zoomx' and 'zoomy' are scaling factors for width and height. If 'smooth' is 1
       then the destination 32bit surface is anti-aliased. If the surface is not 8bit
       or 32bit RGBA/ABGR it will be converted into a 32bit RGBA format on the fly.

*/

    DLLINTERFACE SDL_Surface *zoomSurface(SDL_Surface * src, double zoomx, double zoomy, int smooth);

    /* Returns the size of the target surface for a zoomSurface() call */

    DLLINTERFACE void zoomSurfaceSize(int width, int height, double zoomx, double zoomy, int *dstwidth, int *dstheight);


    /* 
       shrinkSurface()

       Shrinks a 32bit or 8bit 'src' surface ti a newly created 'dst' surface.
       'factorx' and 'factory' are the shrinking ratios (i.e. 2=1/2 the size,
       3=1/3 the size, etc.) The destination surface is antialiased by averaging
       the source box RGBA or Y information. If the surface is not 8bit
       or 32bit RGBA/ABGR it will be converted into a 32bit RGBA format on the fly.
       */     

    DLLINTERFACE SDL_Surface *shrinkSurface(SDL_Surface * src, int factorx, int factory);

    /* 

       Other functions

*/

    DLLINTERFACE SDL_Surface* rotateSurface90Degrees(SDL_Surface* pSurf, int numClockwiseTurns);

    /* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif                          /* _SDL_rotozoom_h */
