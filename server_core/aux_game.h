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

typedef struct GameTry {
    char colours[4];
} GameTry;


/*
    STATUS VERIFICATION auxiliar functions 
*/

/**
 * Checks if a player has an ongoing game.
 * @param PLID The player's identifier.
 * @return TRUE if there is an ongoing game, FALSE otherwise.
 */
int has_ongoing_game(const char *PLID);

/**
 * Determines if the player has reached the maximum number of allowed plays.
 * @param trial_number The current trial number.
 * @return 1 if the maximum number of plays has been reached, 0 otherwise.
 */
int has_reached_max_turn(const char trial_number);

/**
 * Checks if the player has exceeded the game's maximum allowed time.
 * @param PLID The player's identifier.
 * @return 1 if the elapsed time exceeds the allowed time, 0 otherwise.
 */
int has_exceeded_time(const char *PLID);

/**
 * Checks if the given game try is a duplicate of an earlier try.
 * @param PLID The player's identifier.
 * @param game_try The current game try being validated.
 * @return TRUE if a duplicate is found, FALSE otherwise.
 */
int is_duplicated(const char *PLID, GameTry *game_try);

/**
 * Determines if the player has won the game by checking the result of the last try.
 * @param player_try_res The results of the player's try.
 * @return 1 if the player has won (4 correct results), 0 otherwise.
 */
int has_won(const int *player_try_res);

/**
 * Checks if a directory exists.
 * @param filepath The path to the directory.
 * @return 1 if the directory exists, 0 otherwise.
 */
int directory_exists(const char *filepath);

/**
 * Compares two game tries to check if they are identical.
 * @param input The first game try to compare.
 * @param line The second game try to compare.
 * @return TRUE if the game tries are equal, FALSE otherwise.
 */
int are_equal_game_tries(const GameTry *input, const GameTry *line);


/*
    GENERAL auxiliar functions
*/

/**
 * Writes a message (line) into a file at the specified path.
 * @param file_path The path of the file where the message will be written.
 * @param message The message to write into the file.
 */
void write_line(const char *file_path, const char *message);

/**
 * Writes a message to the game file associated with the given player's identifier (PLID).
 * @param PLID The player's identifier.
 * @param message The message to append to the player's game file.
 */
void write_game_line(const char *PLID, const char *message);

/**
 * Builds a response string for the end of the game using the given command and status.
 * @param PLID The player's identifier.
 * @param Command The command associated with the response.
 * @param Status The status of the game or command execution.
 * @return A dynamically allocated string containing the end-game response (Command + Status + Solution).
 */
char* build_end_game_response(const char* PLID, const char* Command, const char* Status);

/**
 * Extracts specific game information from the player's game file.
 * @param PLID The player's identifier.
 * @param arg_type The specific game argument to extract (e.g., max time).
 * @return A dynamically allocated string containing the requested game info.
 */
char* extract_game_info(const char *PLID, const char arg_type);

/**
 * Extracts the solution (correct colour combination) from the player's game file.
 * @param PLID The player's identifier.
 * @return A pointer to a GameTry structure containing the solution colours.
 */
GameTry* extract_game_colour(const char *PLID);

/**
 * Extracts the colour combination from the player's last game try.
 * @param PLID The player's identifier.
 * @return A pointer to a GameTry structure containing the last play's colour combination.
 */
GameTry* extract_last_colour(const char *PLID);

/**
 * Calculates and returns the elapsed time for the current player's game.
 * @param PLID The player's identifier.
 * @return The elapsed time (in seconds) for the ongoing game.
 */
int get_elapsed_time(const char *PLID);

/**
 * Determines the size (in bytes) of the remaining data in an open game file and
 * moves the file pointer to the first byte of data.
 * @param file A pointer to the open file.
 * @return The size of the remaining data in the file.
 */
int get_data_size(FILE *file);

/**
 * Determines the size (in bytes) of the next line in an open file.
 * @param file A pointer to the open file.
 * @return The size of the next line in the file, including the newline character.
 */ 
int get_line_size(FILE *file);

/**
 * Retrieves the expected number of trials (plays) for the player's current game.
 * @param PLID The player's identifier.
 * @return The number of the expected trial in the player's game file.
 */
int get_expected_nt(const char *PLID);


/*
    START and DEBUG auxiliar functions
*/

/**
 * Initializes and starts a new game for the specified player, including setting the game mode and solution.
 * @param PLID The player's identifier.
 * @param time The max time given by the player.
 * @param mode The mode of the game.
 * @param game_solution A pointer to the structure holding the solution (colour combination) for the game.
 * @return OK if the game was successfully started.
 */
int start_game(const char *PLID, const char *time, char mode, GameTry *game_solution);

/**
 * Generates a new game solution (correct colour combination) and stores it in the provided GameTry structure.
 * @param game_solution A pointer to the GameTry structure where the generated solution will be stored.
 */
void generate_solution(GameTry* game_solution);

/**
 * Creates a log file for the game, storing the player identifier, solution, start time, and mode.
 * @param PLID The player's identifier.
 * @param game_solution A pointer to the GameTry structure containing the solution for the game.
 * @param time_value The max time given by the player.
 * @param mode The mode of the game.
 */
void create_game_log(const char *PLID, GameTry *game_solution, const char *time_value, const char mode);


/*
    TRY and QUIT auxiliar functions
*/

/**
 * Checks the status of the player's INV based on the trial number given by the player.
 * @param PLID The player's identifier.
 * @param sent_nt The trial number (nt) sent by the player.
 * @param player_try The player's current game attempt (guess).
 * @return A dynamically allocated string with the status message.
 */
char *check_inv_status(const char *PLID, char sent_nt, GameTry player_try);

/**
 * Generates the result message for the player's game try, including the number of correct colours and positions.
 * @param player_try_res The results array that stores the number of correct colours and positions.
 * @param nt The current trial number.
 * @return A dynamically allocated string containing the result message for the player's try.
 */
char *generate_try_result_message(int *player_try_res, char nt);

/**
 * Executes the logic for the player's game try.
 * @param PLID The player's identifier.
 * @param player_try The player's attempt (guess).
 * @return A pointer to an array holding the result of the try.
 */
int* make_try(const char *PLID, const GameTry player_try);

/**
 * Compares the player's try with the game solution and evaluates the number of correct colours and positions (nB) 
 * and the number of colours in the solution but placed in the wrong position (nW).
 * @param PLID The player's identifier.
 * @param player_try The player's attempt (guess).
 * @return A dynamically allocated array holding the results: nB (correct colours in the correct position) 
 *         and nW (correct colours in the wrong position).
 */
int* get_try_results(const char *PLID, GameTry player_try);

/**
 * Writes the player's game try (guess) along with the result into the player's current game file.
 * @param PLID The player's identifier.
 * @param game_try The player's game try.
 * @param player_try_res The results of the try (number of correct colours and positions).
 */
void write_try(const char *PLID, GameTry game_try, int *player_try_res);

/**
 * Ends the current game, performing all necessary actions related to finishing the game.
 * @param PLID The player's identifier.
 * @param end_game_type Specifies the type of game-ending action (e.g., win, timeout).
 * @param number_tries The total number of tries made by the player in the current game.
 * @return An integer indicating the success or failure of the operation.
 */
int end_game(const char *PLID, const char end_game_type, const int number_tries);

/**
 * Relocates the game status file to the directory of completed games for the player after the game ends.
 * @param PLID The player's identifier.
 * @param endGameType Specifies the type of game end.
 * @return An integer indicating the success or failure of relocating the game file.
 */
int relocate_completed_game(const char *PLID, const char endGameType);

/**
 * Builds the file name for a completed game based on the specified game type.
 * @param type The type of the completed game (e.g., win, timeout).
 * @return A dynamically allocated string containing the generated file name.
 */
char* get_completed_game_name(char const type);

/**
 * Creates a score file for the player, storing the results of the game.
 * @param PLID The player's identifier.
 * @param number_tries The number of tries the player made to solve the game.
 * @return An integer indicating the success or failure of creating the score file.
 */
int create_score_file(const char *PLID, const int number_tries);

/**
 * Calculates the player's score for the game.
 * @param PLID The player's identifier.
 * @param turnsPlayed The number of turns the player took to complete or fail the game.
 * @return The calculated score for the game.
 */
int calculate_score(const char *PLID, const int turnsPlayed);


/*
    SHOW_TRIALS and SCOREBOARD auxiliar functions
*/


/**
 * Executes the functionality of showing the player's game trials (attempts).
 * @param client_fd The file descriptor for the client socket.
 * @param filepath The path to the file containing the player's game trials.
 * @param PLID The player's identifier.
 */
void execute_show_trials(const int client_fd, const char* filepath, char* PLID);

/**
 * Builds a response string for the scoreboard based on the provided list of score files.
 * @param files A pointer to a SCORELIST structure containing the list of score files.
 * @return A dynamically allocated string containing the formatted scoreboard response.
 */
char *build_scoreboard_response(SCORELIST *files);
    
#endif