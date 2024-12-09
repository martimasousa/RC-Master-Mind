#ifndef AUX_GAME_H
#define AUX_GAME_H

#include "constants.h"
#include "game_core.h"

/*
    Starts the new game, encapsulating all the logic behind.
    - Generates Solution
    - Creates File Game
    - Creates Player Directory if not created yet
*/
int start_game(const char *PLID, const char *time, char mode, GameTry *game_solution);

#endif