#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

/*

multiplayer.h - Provides routines for organizing and running a turn-based
                multiplayer that can accommodate up to four players (more with
                a recompilation)

Author: B. Luchen

*/

#define MAX_PLAYERS 4

enum {
  PLAYERS,
  ROUNDS,
  DIFFICULTY,
  MODE,
  NUM_PARAMS
};

typedef enum {
  SCORE_SWEEP,
  ELIMINATION
} MP_Mode;

void mp_set_parameter(unsigned int param, int value);
void mp_run_multiplayer();
int mp_get_player_score(int playernum);
const char* mp_get_player_name(int playernum);
int mp_num_players();

#endif // MULTIPLAYER_H
