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

int search_file(const char *PLID);

char* get_game_folder_path(const char *PLID);

void write_game_line(const char *PLID, const char *message);

void create_game_log_timestamp(const char *PLID, GameTry *game_solution, char *time, char mode);

int extract_game_colour(const char *PLID, GameTry *game);

char* extract_game_time(const char *PLID, const char time_type);

int inTime(const char *PLID);

int compare_game_try(const GameTry *input, const GameTry *line);

int extract_colors_from_file(const char *PLID, GameTry *game_try);

#endif
