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

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

#include <SDL_image.h> // For IMG_Load fallback

// Cache Structure
typedef struct CachedImageNode {
    char* filename;
    int width;
    int height;
    // char* gameStateTag; // Optional for future use
    SDL_Surface* surface;
    struct CachedImageNode* next;
} CachedImageNode;

static CachedImageNode* image_cache_head = NULL;

// Cache Management Functions
static SDL_Surface* Cache_GetSurface(const char* filename, int width, int height);
static void Cache_StoreSurface(const char* filename, int width, int height, SDL_Surface* surface);
static void Cache_UnloadAll(void);
// Public function to be called from setup.c or similar
void Fileops_UnloadAllCachedImages(void);


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
        "comets/mini_comet1.svg", // Changed to .svg
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
        "factoroids/gbstars.svg", // Changed to .svg
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
        char* themed_path = Fileops_GetThemedPath(image_filenames[i]);
        if (!themed_path) {
            fprintf(stderr, "\nError: Could not resolve themed path for: %s\n", image_filenames[i]);
            // Consider how to handle this - skip, try original, or error out
            // For now, try to load original relative path as a last resort (if Fileops_GetThemedPath returns it)
            // If Fileops_GetThemedPath returns NULL strictly on not found, then error here.
            // Based on current Fileops_GetThemedPath, it returns a strdup of a constructed legacy path if all themes fail.
            // So, themed_path should ideally not be NULL if the file exists in *any* of the checked locations.
            // If it IS null, it means strdup failed or relative_filename was null.
             if (!themed_path && image_filenames[i]) { // If Fileops_GetThemedPath could return NULL on file-not-found
                snprintf(themed_path_buffer, sizeof(themed_path_buffer), "%s/images/%s", DATA_PREFIX, image_filenames[i]);
                themed_path = strdup(themed_path_buffer); // Fallback for safety, though GetThemedPath should do this
                if (!themed_path) { return 0; /* Allocation error */ }
             } else if (!themed_path) {
                return 0; // filename was null
             }
        }

        // Check for SVG extension using the original relative filename, as themed_path might be complex
        if (strlen(image_filenames[i]) > 4 && strcmp(image_filenames[i] + strlen(image_filenames[i]) - 4, ".svg") == 0)
        {
            int target_w = 0;
            int target_h = 0;
            if (strcmp(image_filenames[i], "status/tux4kids.svg") == 0) {
                target_w = 128;
                target_h = 64;
            }
            // Add more cases for specific SVG sizes here if needed

            images[i] = TuxMath_LoadSVG(themed_path, target_w, target_h); // TuxMath_LoadSVG now handles caching internally
        }
        else
        {
            // Bitmap image loading with cache
            SDL_Surface* cached_surface = Cache_GetSurface(themed_path, 0, 0); // 0,0 for native size
            if (cached_surface) {
                images[i] = cached_surface;
            } else {
                // Load using IMG_Load directly with the themed path
                SDL_Surface* loaded_surface = IMG_Load(themed_path);
                if (loaded_surface) {
                    images[i] = SDL_DisplayFormatAlpha(loaded_surface);
                    SDL_FreeSurface(loaded_surface); // Free the original surface from IMG_Load
                    if (images[i]) {
                        Cache_StoreSurface(themed_path, images[i]->w, images[i]->h, images[i]);
                    } else {
                         fprintf(stderr, "Could not convert surface for %s to display format.\n", themed_path);
                    }
                } else {
                    images[i] = NULL; // IMG_Load failed
                }
            }
        }

        free(themed_path); // Free the path obtained from Fileops_GetThemedPath

        if (images[i] == NULL)
        {
            fprintf(stderr,
                    "\nError: I couldn't load a graphics file:\n"
                    "%s (tried path: %s - path from Fileops_GetThemedPath was used if not NULL)\n" // themed_path is freed, so show original for reference
                    "The Simple DirectMedia error that occured was (or SVG/IMG_Load loading error):\n"
                    "%s\n\n", image_filenames[i], "N/A after free", SDL_GetError());
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

// Internal SVG loading and rendering function
static SDL_Surface* TuxMath_LoadSVG_Internal(const char* filename, int target_width, int target_height) {
    NSVGimage* image = NULL;
    NSVGrasterizer* rast = NULL;
    unsigned char* img_data = NULL;
    SDL_Surface* sdl_surface = NULL;
    int w, h;

    DEBUGMSG(debug_fileops, "Attempting to load SVG: %s\n", filename);

    image = nsvgParseFromFile(filename, "px", 96.0f);
    if (!image) {
        fprintf(stderr, "Could not open or parse SVG image: %s\n", filename);
        return NULL;
    }

    w = (target_width > 0) ? target_width : (int)image->width;
    h = (target_height > 0) ? target_height : (int)image->height;

    if (w <= 0 || h <= 0) {
        fprintf(stderr, "SVG image %s has invalid dimensions: %dx%d (target: %dx%d)\n", filename, (int)image->width, (int)image->height, target_width, target_height);
        nsvgDelete(image);
        return NULL;
    }

    float scale_x = (target_width > 0) ? (float)target_width / image->width : 1.0f;
    float scale_y = (target_height > 0) ? (float)target_height / image->height : 1.0f;
    // For now, use a single scale factor, preferring one that fits (min_scale) or fills (max_scale)
    // To maintain aspect ratio, typically one would calculate target_w/h based on the other.
    // For this simple case, if one dim is provided, scale to that. If both, scale might distort.
    // Let's use the scale for width if provided, else height, else 1.0
    float scale = 1.0f;
    if (target_width > 0) scale = scale_x;
    else if (target_height > 0) scale = scale_y;

    // Adjust w, h based on scale if only one target dimension was provided
    if (target_width > 0 && target_height <= 0) {
        h = (int)(image->height * scale);
    } else if (target_height > 0 && target_width <= 0) {
        w = (int)(image->width * scale);
    } else if (target_width > 0 && target_height > 0) {
        // If both are provided, we are forcing to this size.
        // NanoSVG rasterizer will use the scale parameter to scale the path data before rendering to the w,h buffer.
        // We should provide a scale that makes sense. Let's use average or min scale.
        scale = (scale_x < scale_y) ? scale_x : scale_y; // Fit
         w = (int)(image->width * scale); // Re-calc w,h based on chosen uniform scale
         h = (int)(image->height * scale);
         if (target_width > 0 && target_height > 0) { // if specific target, use that
             w = target_width;
             h = target_height;
         }

    }


    if (w <= 0 || h <= 0) { // Recalculate if needed
        w = (target_width > 0) ? target_width : (int)(image->width * scale);
        h = (target_height > 0) ? target_height : (int)(image->height * scale);
        if (w <=0) w = (int)image->width; // fallback
        if (h <=0) h = (int)image->height; // fallback
    }


    rast = nsvgCreateRasterizer();
    if (!rast) {
        fprintf(stderr, "Could not init SVG rasterizer.\n");
        nsvgDelete(image);
        return NULL;
    }

    img_data = (unsigned char*)malloc(w * h * 4);
    if (!img_data) {
        fprintf(stderr, "Could not alloc image buffer for SVG.\n");
        nsvgDeleteRasterizer(rast);
        nsvgDelete(image);
        return NULL;
    }

    DEBUGMSG(debug_fileops, "Rasterizing SVG %s to %dx%d (scale: %f)\n", filename, w, h, scale);
    nsvgRasterize(rast, image, 0, 0, scale, img_data, w, h, w * 4);

    // Create SDL_Surface from RGBA data
    // SDL_CreateRGBSurfaceFrom uses the provided buffer directly if pitch matches.
    // For safety, ensure a copy if pixel format conversion or pitch mismatch occurs.
    // However, standard RGBA output from NanoSVG should be fine.
    sdl_surface = SDL_CreateRGBSurfaceFrom((void*)img_data, w, h, 32, w * 4,
                                           0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

    if (!sdl_surface) {
        fprintf(stderr, "Could not create SDL_Surface from SVG data: %s\n", SDL_GetError());
        free(img_data);
    } else {
        // SDL_CreateRGBSurfaceFrom does not own the pixel data if pitch matches.
        // To make sdl_surface independent, create a new surface and blit.
        SDL_Surface* formatted_surface = SDL_DisplayFormatAlpha(sdl_surface);
        SDL_FreeSurface(sdl_surface); // Free the surface that was using img_data
        free(img_data); // Now we can free img_data as formatted_surface has its own copy
        sdl_surface = formatted_surface;
        if (!sdl_surface) {
             fprintf(stderr, "Could not convert SVG SDL_Surface to display format: %s\n", SDL_GetError());
        }
    }

    nsvgDeleteRasterizer(rast);
    nsvgDelete(image);

    return sdl_surface;
}

// Cached SVG loader
static SDL_Surface* TuxMath_LoadSVG(const char* filename, int target_width, int target_height) {
    // Determine cache key dimensions. If target_width/height is 0, it means "native" size.
    // However, nsvgParseFromFile doesn't give us native size until after parsing,
    // and rasterization happens to target_width/height.
    // So, the cache will effectively store based on the *requested rasterization size*.
    // If target_w/h are 0, TuxMath_LoadSVG_Internal will use image->width/height.
    // We need to know these *actual* dimensions for an effective cache key if target_w/h are zero.

    // For simplicity in this step, if target_width or target_height is 0,
    // we will NOT use the cache for them, as determining the "actual" rendered size
    // without loading first is tricky for a generic cache key.
    // A more advanced cache key might involve parsing SVG first just for size.
    // For now, only cache if explicit dimensions are given.
    // OR: A better approach: the cache key IS (filename, target_w, target_h).
    // If target_w/h are 0, it means "render at native SVG size".
    // The actual w/h of the created surface will be stored with the cache entry.

    SDL_Surface* cached_surface = Cache_GetSurface(filename, target_width, target_height);
    if (cached_surface) {
        return cached_surface;
    }

    SDL_Surface* new_surface = TuxMath_LoadSVG_Internal(filename, target_width, target_height);
    if (new_surface) {
        // Store with the dimensions it was actually rendered at, which should match target_width/height
        // if they were non-zero, or the SVG's native if they were zero.
        // The internal loader already calculates the final w,h used for rasterization.
        Cache_StoreSurface(filename, new_surface->w, new_surface->h, new_surface);
    }
    return new_surface;
}


// Cache Management Function Implementations

static SDL_Surface* Cache_GetSurface(const char* filename, int width, int height) {
    CachedImageNode* current = image_cache_head;
    while (current) {
        // If width/height are 0, it implies native size.
        // For SVGs, this means native SVG size. For bitmaps, surface w/h.
        // The cache stores the rendered/loaded dimensions.
        if (strcmp(current->filename, filename) == 0 &&
            current->width == width &&
            current->height == height) {
            DEBUGMSG(debug_fileops, "Cache HIT for: %s (%dx%d)\n", filename, width, height);
            return current->surface;
        }
        current = current->next;
    }
    DEBUGMSG(debug_fileops, "Cache MISS for: %s (%dx%d)\n", filename, width, height);
    return NULL;
}

static void Cache_StoreSurface(const char* filename, int width, int height, SDL_Surface* surface) {
    if (!surface) return;

    // First, check if this exact surface (by pointer) is already cached to prevent issues
    // if this function is somehow called twice with the same surface.
    // More robustly, Cache_Get should be used first, and Store only if Get fails.
    CachedImageNode* existing_check = image_cache_head;
    while(existing_check) {
        if (existing_check->surface == surface) {
             DEBUGMSG(debug_fileops, "Surface for %s (%dx%d) already in cache by pointer. Not re-adding.\n", filename, width, height);
             return; // Already cached
        }
        existing_check = existing_check->next;
    }

    // Check if an entry for this filename/dims already exists (e.g. if Get was not called before Store)
    // This prevents adding a new node if an equivalent surface was already loaded and cached.
    // However, this specific Store function assumes it's a new, valid surface to cache.
    // If a different SDL_Surface* for the same file/dims exists, this will add another one.
    // Proper orchestration by the calling loader function is key.

    CachedImageNode* newNode = (CachedImageNode*)malloc(sizeof(CachedImageNode));
    if (!newNode) {
        fprintf(stderr, "Failed to allocate memory for cache node.\n");
        // Potentially free surface here if it's not going to be used? Or let caller handle.
        return;
    }
    newNode->filename = strdup(filename);
    if (!newNode->filename) {
        fprintf(stderr, "Failed to duplicate filename for cache.\n");
        free(newNode);
        return;
    }
    newNode->width = width;
    newNode->height = height;
    newNode->surface = surface; // The cache now "owns" this surface regarding lifetime unless caller keeps a ref
    newNode->next = image_cache_head;
    image_cache_head = newNode;
    DEBUGMSG(debug_fileops, "Cached: %s (%dx%d)\n", filename, width, height);
}

static void Cache_UnloadAll(void) {
    CachedImageNode* current = image_cache_head;
    CachedImageNode* next_node;
    DEBUGMSG(debug_fileops, "Unloading all cached images.\n");
    while (current) {
        next_node = current->next;
        DEBUGMSG(debug_fileops, "Freeing cached: %s (%dx%d)\n", current->filename, current->width, current->height);
        SDL_FreeSurface(current->surface);
        free(current->filename);
        free(current);
        current = next_node;
    }
    image_cache_head = NULL;
}

// Public wrapper for Cache_UnloadAll
void Fileops_UnloadAllCachedImages(void) {
    Cache_UnloadAll();
}
