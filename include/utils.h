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
int search_file(const char *PLID);

// Generate the path of the file that contains the player's information.
char* get_game_folder_path(const char *PLID);

// Write a new line in the player's game file (representing the command)
void write_game_line(const char *PLID, const char *message);

// Create a game file and write the first line (command that initialized the game)
void create_game_log_timestamp(const char *PLID, GameTry *game_solution, char *time, char mode);

// For a certain file, extract the game solution
int extract_game_colour(const char *PLID, GameTry *game);

// For a certain file, extract the game initial and max time
char* extract_game_time(const char *PLID, const char time_type);

// Check if the game is within the time limit
int inTime(const char *PLID);

// Check if there was a try with the same code (DUP)
int compare_game_try(const GameTry *input, const GameTry *line);

// Função para iterar pelas linhas do ficheiro
// Retorna true se já houver alguma linha igual
int extract_colors_from_file(const char *PLID, GameTry *game_try);

#endif
