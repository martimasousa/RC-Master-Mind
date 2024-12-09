#ifndef AUX_GAME_H
#define AUX_GAME_H

#include "constants.h"
#include "game_core.h"
#include "utils.h"
/*
    GENERAL auxiliar functions
*/

// Writes the message into a file
void write_line(const char *file_path, const char *message);

// Write a message to the PLID game file
void write_game_line(const char *PLID, const char *message);


/*
    START and DEBUG auxiliar functions
*/

// Executes the start of the game
int start_game(const char *PLID, const char *time, char mode, GameTry *game_solution);

// Generates a new solution and returns its pointer
void generate_solution(GameTry* game_solution);

// Creates the game log file with the respective informations
void create_game_log(const char *PLID, GameTry *game_solution, const char *time_value, const char mode);


#endif