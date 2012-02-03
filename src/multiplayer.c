/*

   multiplayer.c

   Routines for organizing and running a turn-based (as opposed to LAN)
   multiplayer that can accommodate up to four players (more with
   a recompilation)

   Copyright 2008, 2010, 2011.
Authors: David Bruce, Brendan Luchen.
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

multiplayer.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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
#include "multiplayer.h"
#include "game.h"
#include "options.h"
#include "fileops.h"
#include "highscore.h"
#include "credits.h"

int params[NUM_PARAMS] = {0, 0, 0, 0};

int inprogress = 0;
int pscores[MAX_PLAYERS];
char* pnames[MAX_PLAYERS];
int currentplayer = 0;

//local function decs
static void showWinners(int* order, int num); //show a sequence recognizing winner
static int initMP();
static void cleanupMP();

void mp_set_parameter(unsigned int param, int value)
{
    if (inprogress)
    {
        DEBUGMSG(debug_multiplayer, "Oops, tried to set param %d in the middle of a game\n", param);
        return;
    }
    if (param > NUM_PARAMS)
    {
        DEBUGMSG(debug_multiplayer, "Oops, param %d is illegal, must be < %d\n", param, NUM_PARAMS);
        return;
    }
    params[param] = value;
}

void mp_run_multiplayer()
{
    int i;
    int round = 1;
    int result = 0;
    int done = 0;
    int activeplayers = params[PLAYERS];
    int winners[MAX_PLAYERS];

    currentplayer = 0;
    for (i = 0; i < MAX_PLAYERS; ++i)
        winners[i] = -1;

    if (initMP() )
    {
        fprintf(stderr, "Initialization failed, bailing out\n");
        return;
    }

    //cycle through players until all but one has lost
    if (params[MODE] == ELIMINATION) 
    {
        while(!done)
        {
            //TODO maybe gradually increase difficulty
            game_set_start_message(pnames[currentplayer], "Go!", "", "");
            result = comets_game(local_game);

            if (result == GAME_OVER_LOST || result == GAME_OVER_ESCAPE)
            {
                //eliminate player
                pscores[currentplayer] = 0xbeef;
                winners[--activeplayers] = currentplayer;
            }

            do //move to the next player
            {
                ++currentplayer;
                currentplayer %= params[PLAYERS];
                if (currentplayer == 0)
                    ++round;
            } 
            while (pscores[currentplayer] == 0xbeef); //skip over eliminated players

            if (activeplayers <= 1) //last man standing!
            {
                DEBUGMSG(debug_multiplayer, "%d wins\n", currentplayer);
                winners[0] = currentplayer;
                done = 1;
            }
        }
    }
    //players take turns, accumulating score, and the highest score wins
    else if (params[MODE] == SCORE_SWEEP)
    {
        int hiscore = 0;
        int currentwinner = -1;

        //play through rounds
        for (round = 1; round <= params[ROUNDS]; ++round)
        {
            for (currentplayer = 0; currentplayer < params[PLAYERS]; ++currentplayer)
            {
                game_set_start_message(pnames[currentplayer], _("Go!"), NULL, NULL);
                result = comets_game(local_game);
                //pscores[currentplayer] += Opts_LastScore(); //add this player's score
                if (result == GAME_OVER_WON)
                    pscores[currentplayer] += 500; //plus a possible bonus
            }
        }

        //sort out winners
        for (i = 0; i < params[PLAYERS]; ++i)
        {
            int j = 0;
            hiscore = 0;
            for (j = 0; j < params[PLAYERS]; ++j)
            {
                if (pscores[j] >= hiscore)
                {
                    hiscore = pscores[j];
                    currentwinner = j;
                }
                winners[i] = currentwinner;
                pscores[currentwinner] = -1;
            }
        }
    }

    DEBUGMSG(debug_multiplayer, "Game over; showing winners\n");

    showWinners(winners, params[PLAYERS]);
    cleanupMP();
}

int mp_get_currentplayer(void)
{
    return currentplayer;
}

int mp_set_player_score(int playernum, int score)
{
    if (playernum < 0 || playernum > params[PLAYERS])
    {
        DEBUGMSG(debug_multiplayer, "No player %d!\n", playernum);
        return 0;
    }
    pscores[playernum] = score;
    return 1;
}

int mp_get_player_score(int playernum)
{
    if (playernum < 0 || playernum > params[PLAYERS])
    {
        DEBUGMSG(debug_multiplayer, "No player %d!\n", playernum);
        return 0;
    }
    return pscores[playernum];
}

const char* mp_get_player_name(int playernum)
{
    if (playernum < 0 || playernum > params[PLAYERS])
    {
        DEBUGMSG(debug_multiplayer, "No player %d!\n", playernum);
        return 0;
    }
    return pnames[playernum];
}

int mp_get_parameter(unsigned int param)
{
    if (param > NUM_PARAMS)
    {
        fprintf(stderr, "Invalid mp_param index: %d\n", param);
        return 0;
    }
    return params[param];
}

//TODO a nicer-looking sequence that also recognizes second place etc.
//FIXME doesn't sort correctly if the winner happens to be other than the
//first player (winner is correct, but others aren't).  We could use the
//game-over standings coded for the LAN game here.
void showWinners(int* winners, int num)
{
    int skip = 0;
    int i = 0;
    const int boxspeed = 3;
    int sectionlength = num * (HIGH_SCORE_NAME_LENGTH + strlen(" wins!\n"));
    char text[sectionlength];
    SDL_Rect box = {screen->w / 2, screen->h / 2, 0, 0};
    SDL_Rect center = box;
    SDL_Event evt;

    const char* winnername = (winners[0] == -1 ? "Nobody" : pnames[winners[0]] );

    snprintf(text, HIGH_SCORE_NAME_LENGTH + strlen(" wins!"),
            "%s wins!\n", winnername);
    for (i = 1; i < num; ++i)
    {
        snprintf(strchr(text, '\0'), sectionlength, _("Then %s\n"), pnames[winners[i]]);
    }

    DEBUGMSG(debug_multiplayer, "%s Win text: %s\n", pnames[winners[0]], text);

    T4K_DarkenScreen(1);

    while (box.h < screen->h || box.w < screen->w)
    {
        //expand black box
        box.x -= boxspeed;
        box.y -= boxspeed;
        box.h += boxspeed * 2;
        box.w += boxspeed * 2;

        //reveal text specifying the winner
        SDL_FillRect(screen, &box, 0);
        draw_text(text, center);
        SDL_UpdateRect(screen, box.x, box.y, box.w, box.h);

        while (SDL_PollEvent(&evt) )
            if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE)
                skip = 1;
        if (skip)
            break;
        SDL_Delay(50);
    }
    //in case we've skipped, cover the whole screen
    SDL_FillRect(screen, NULL, 0);
    draw_text(text, center);
    SDL_Flip(screen);
    T4K_WaitForEvent(SDL_KEYDOWNMASK | SDL_MOUSEBUTTONDOWNMASK);
}

int initMP()
{
    int i;
    int success = 1;
    char nrstr[HIGH_SCORE_NAME_LENGTH * 3];
    int nplayers = params[PLAYERS];

    const char* config_files[5] = {
        "multiplay/space_cadet",
        "multiplay/scout",
        "multiplay/ranger",
        "multiplay/ace",
        "multiplay/commando"
    };

    DEBUGMSG(debug_multiplayer, "Reading in difficulty settings...\n");

    success *= read_global_config_file(local_game);

    success *= read_named_config_file(local_game, "multiplay/mpoptions");

    success *= read_named_config_file(local_game, config_files[params[DIFFICULTY]]);

    if (!success)
    {
        fprintf(stderr, "Couldn't read in settings for %s\n",
                config_files[params[DIFFICULTY]] );
        return 1;
    }

    pscores[0] = pscores[1] = pscores[2] = pscores[3] = 0;
    pnames[0] = pnames[1] = pnames[2] = pnames[3] = NULL;

    //allocate and enter player names
    for (i = 0; i < nplayers; ++i)
        pnames[i] = malloc((1 + 3 * HIGH_SCORE_NAME_LENGTH) * sizeof(char) );
    for (i = 0; i < nplayers; ++i)
    {
        if (pnames[i])
        {
            if (i == 0) //First player
                NameEntry(pnames[i], N_("Who is playing first?"), N_("Enter your name:"), NULL);
            else //subsequent players
                NameEntry(pnames[i], N_("Who is playing next?"), N_("Enter your name:"), NULL);
        }
        else
        {
            fprintf(stderr, "Can't allocate name %d!\n", i);
            return 1;
        }
    }

    //enter how many rounds
    if (params[MODE] == SCORE_SWEEP)
    {
        while (params[ROUNDS] <= 0)
        {
            NameEntry(nrstr, N_("How many rounds will you play?"), N_("Enter a number"), NULL);
            params[ROUNDS] = atoi(nrstr);
        }
    }
    inprogress = 1; //now we can start the game
    return 0;
}

void cleanupMP()
{
    int i;

    for (i = 0; i < params[PLAYERS]; ++i)
        if (pnames[i])
            free(pnames[i]);

    for (i = 0; i < NUM_PARAMS; ++i)
        params[i] = 0;

    inprogress = 0;
    currentplayer = 0;
}
