#include <stdio.h>
#include "game_core.h"

#ifndef UTILS_H
#define UTILS_H

// Função para verificar se a string é um número inteiro
int is_integer(const char *str);

// Função para verificar se a string é um IP válido
int is_valid_ip(const char *ip);

// Função para verificar se é uma cor válida
int is_valid_color(char C);

// Returns true if PLID has an ongoing game
int has_ongoing_game(const char *PLID);

// Generate the path of the file that contains the player's information.
char* get_game_folder_path(const char *PLID);

char* get_player_folder_path(const char *PLID);

int create_directory(const char *directory);

int move_file(const char *source_file, const char endGameType);

int end_game(const char *PLID, const char endGameType);

int create_score_file(const char *PLID);

char* get_end_game_name(char const type);

// Write a new line in the player's game file (representing the command)
void write_game_line(const char *PLID, const char *message);

// Create a game file and write the first line (command that initialized the game)
void create_game_log_timestamp(const char *PLID, GameTry *game_solution, char *time, char mode);

// For a certain file, extract the game solution
int extract_game_colour(const char *PLID, GameTry *game);

// For a certain file, extract the game initial and max time
char* extract_game_info(const char *PLID, const char arg_type);

// Check if the game is within the time limit
int has_exceeded_time(const char *PLID);

// Check if there was a try with the same code (DUP)
int compare_game_try(const GameTry *input, const GameTry *line);

// Função para iterar pelas linhas do ficheiro
// Retorna true se já houver alguma linha igual
int is_duplicated(const char *PLID, GameTry *game_try);

int get_elapsed_time(const char *PLID);

int has_exceeded_max_turn(char trial_number);

char* get_max_turn_response(const char* PLID);

void write_try(const char *PLID, GameTry game_try, int *game_try_res);

// Reads delimited string from the file descriptor.
int tcp_read_until_delimiter(int fd, char** word, char separator);

int line_size(FILE *fp);

int get_data_size(FILE *file);

int hasWon(int *player_try_res);

int calculateScore(const char *PLID, int turnsPlayed);

char* getScoreFileName(int score, char *PLID);

int directoryExists(char *filepath);

#endif
