#ifndef AUX_GAME_H
#define AUX_GAME_H

#include "utils.h"
#include "constants.h"
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <stdlib.h>

/*
    Structures
*/

// FUNCTIONS GIVEN BY THE UC ---------------------------------------------------

#define CODE_DIGITS 4 + 1   // '\0'
#define SSB_SIZE    10

typedef struct {
    int score[SSB_SIZE];                    // Array to store scores
    char PLID[SSB_SIZE][PLID_DIGITS + 1];   // Array of strings for Player IDs
    char colcode[SSB_SIZE][CODE_DIGITS];    // Array of strings for color codes
    int notries[SSB_SIZE];                  // Array to store number of tries
    char mode[SSB_SIZE];                    // Array to store the mode for each entry
    int nscores;                            // Number of scores currently stored
} SCORELIST;

// -----------------------------------------------------------------------------

typedef struct GameTry {
    char colours[4];
} GameTry;


/*
    STATUS VERIFICATION auxiliar functions 
*/

int has_ongoing_game(const char *PLID);
int has_exceeded_max_turn(const char trial_number);
int has_exceeded_time(const char *PLID);
int is_duplicated(const char *PLID, GameTry *game_try);
int has_won(const int *player_try_res);
int directory_exists(const char *filepath);
int are_equal_game_tries(const GameTry *input, const GameTry *line);

/*
    GENERAL auxiliar functions
*/

// Writes the message into a file
void write_line(const char *file_path, const char *message);

// Write a message to the PLID game file
void write_game_line(const char *PLID, const char *message);

// Builds the end game response with the Command and Status given
char* build_end_game_response(const char* PLID, const char* Command, const char* Status);

// Extracts the game info arguments from the current player game
char* extract_game_info(const char *PLID, const char arg_type);

// Extracts the colour and returns it into the GameTry structure
GameTry* extract_game_colour(const char *PLID);

// Returns the current game elapsed time
int get_elapsed_time(const char *PLID);

// Gets the size of the data from a game file
int get_data_size(FILE *file);

// Gets the size of a line from a file 
int get_line_size(FILE *file);

/*
    START and DEBUG auxiliar functions
*/

// Executes the start of the game
int start_game(const char *PLID, const char *time, char mode, GameTry *game_solution);

// Generates a new solution and returns its pointer
void generate_solution(GameTry* game_solution);

// Creates the game log file with the respective informations
void create_game_log(const char *PLID, GameTry *game_solution, const char *time_value, const char mode);


/*
    TRY and QUIT auxiliar functions
*/
char *generate_try_result_message(char *PLID, int *player_try_res, int nt);

// Executes the try logic and returns the result
int* make_try(const char *PLID, const GameTry player_try);

// Compare the player's try with the solution and evaluate nB (correct in both colour and position)
// and nW (belong to the secret key but are incorrectly positioned).
int* get_try_results(const char *PLID, GameTry player_try);

// Writes the player try into the players current game file
void write_try(const char *PLID, GameTry game_try, int *player_try_res);

// Ends the game taking care of all the related procedures
int end_game(const char *PLID, const char end_game_type, const int number_tries);

// Relocates the game status file to the completed games directory of the player
int relocate_completed_game(const char *PLID, const char endGameType);

// Builds the completed game file name
char* get_completed_game_name(char const type);

// Creates the score file
int create_score_file(const char *PLID, const int number_tries);

// Calculates the score of the game
int calculate_score(const char *PLID, const int turnsPlayed);


/*
    SHOW_TRIALS and SCOREBOARD auxiliar functions
*/

// FUNCTIONS GIVEN BY THE UC ---------------------------------------------------

int FindTopScores(SCORELIST *list);

int FindLastGame(const char *PLID, char *fname);

// -----------------------------------------------------------------------------

// Executes the functionality of the show_trials
void execute_show_trials(const int client_fd, const char* filepath, char* PLID);

#endif