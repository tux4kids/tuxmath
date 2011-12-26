#ifndef COMETS_H
#define COMETS_H

#include <SDL_video.h>

#include "mathcards.h"

#define MAX_COMETS 10
#define NUM_CITIES 4   /* MUST BE AN EVEN NUMBER! */

#define NUM_BKGDS 8

#define MAX_CITY_COLORS 4


typedef struct laser_type {
    float alive;
    int x1, y1;
    int x2, y2;
} laser_type;

/* Note: both igloos and the original "cities" graphics are handled
   with the "cities" structure.
   hits_left holds the number of hits it can withstand before
   being "dead". If using the original cities graphics,
   2 = with shield,
   1 = without shield,
   0 = dead
   If using the igloo graphics,
   2 = intact,
   1 = half-melted
   0 = melted
   */

/* For both cities/igloos & penguins, the animation state is
   controlled by "status", and "counter" is used for timing.  We also
   have "img" and "layer" so that the image can be pre-planned to have
   a specific rendering order (so that foreground/background issues
   are handled properly). Layer 0 is rendered first, then layer 1, and
   so on. */

typedef struct city_type {
    int hits_left;
    int status, counter;
    int threatened;   /* true if a comet is near */
    int x;
    int img,layer;
} city_type;

typedef struct penguin_type {
    int status, counter;
    int x;
    int img,layer;
} penguin_type;

typedef struct steam_type {
    int status, counter;
    int img,layer;
} steam_type;

#define NUM_SNOWFLAKES 100

typedef struct cloud_type {
    int status;
    int city;
    int x,y;
    int snowflake_x[NUM_SNOWFLAKES];
    int snowflake_y[NUM_SNOWFLAKES];
    int snowflake_size[NUM_SNOWFLAKES];
} cloud_type;

#define GAME_MESSAGE_LENGTH 100

typedef struct {
    int x,y;
    int alpha;
    char message[GAME_MESSAGE_LENGTH];
} game_message;

/* City animation status types */
enum {
    CITY_PRESENT,
    CITY_EXPLODING,
    CITY_EVAPORATING,
    CITY_REBUILDING,
    CITY_GONE
};

/* Penguin animation status types */
enum {
    PENGUIN_OFFSCREEN,
    PENGUIN_HAPPY,
    PENGUIN_FLAPPING,
    PENGUIN_DUCKING,
    PENGUIN_GRUMPY,
    PENGUIN_WORRIED,
    PENGUIN_WALKING_OFF,
    PENGUIN_WALKING_ON,
    PENGUIN_STANDING_UP,
    PENGUIN_SITTING_DOWN,
    PENGUIN_BOWING
};

/* Steam animation status types */
enum {
    STEAM_OFF,
    STEAM_ON
};

/* Cloud & snowflake animation types */
enum {
    EXTRA_LIFE_OFF,
    EXTRA_LIFE_ON
};

typedef enum {
    SMARTBOMB,
    NPOWERUP
} PowerUp_Type;

typedef enum {
    POWERUP_DIR_LEFT,
    POWERUP_DIR_RIGHT,
    POWERUP_DIR_UNKNOWN
} PowerUp_Direction;

typedef struct comet_type {
    int alive;
    int expl;
    int city;
    float x, y;
    int answer;
    int bonus;
    int zapped;
    MC_FlashCard flashcard;
    SDL_Surface* formula_surf;
    SDL_Surface* answer_surf;
    Uint32 time_started;
} comet_type;

typedef struct powerup_comet_type {
    comet_type comet;
    PowerUp_Direction direction;
    PowerUp_Type type;
    int inc_speed;
} powerup_comet_type;

typedef struct help_controls_type {
    int x_is_blinking;
    int extra_life_is_blinking;
    int laser_enabled;
} help_controls_type;


int comets_game(MC_MathGame* loc_game);
void game_set_start_message(const char*, const char*, const char*, const char*);


#endif
