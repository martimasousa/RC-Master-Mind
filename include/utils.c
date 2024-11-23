#include "utils.h"
#include "game_core.h"
#include "constants.h"
#include <stdio.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

char* get_game_folder_path(const char *PLID) {
    size_t path_length = strlen("./GAMES/GAME_") + strlen(PLID) + 1;

    char *file_path = malloc(path_length);
    if (file_path == NULL) {
        perror("Error allocating memory for the path!\n");
        return NULL;
    }
    snprintf(file_path, path_length, "./GAMES/GAME_%s", PLID);
    return file_path;
}

// Função para comparar as cores
int compare_game_try(const GameTry *input, const GameTry *line) {
    for (int i = 0; i < 4; i++) {
        if (input->colours[i] != line->colours[i]) {
            return FALSE;  // Se alguma cor não bater, retorna false
        }
    }
    return TRUE;  // Se todas as cores coincidirem, retorna true
}

int is_duplicated(const char *PLID, GameTry *game_try) {
    
    char *file_path = get_game_folder_path(PLID);

    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Erro ao abrir o ficheiro");
        return FALSE;  // Erro ao abrir o ficheiro
    }

    char line[100];
    int line_num = 0;

    // Ignora a primeira linha
    if (fgets(line, sizeof(line), file) == NULL) {
        perror("Erro ao ler a primeira linha do ficheiro");
        fclose(file);
        return FALSE;
    }

    // Itera pelas linhas seguintes
    while (fgets(line, sizeof(line), file) != NULL) {
        // A linha está no formato: num: C C C C
        char colour1, colour2, colour3, colour4;

        // Usando sscanf para ler os dados da linha
        int result = sscanf(line, "T: %c%c%c%c", &colour1, &colour2, &colour3, &colour4);

        // Verifica se leu corretamente a linha
        if (result == 4) {
            // Criar a estrutura GameTry e armazenar as cores
            GameTry line_game;
            line_game.colours[0] = colour1;
            line_game.colours[1] = colour2;
            line_game.colours[2] = colour3;
            line_game.colours[3] = colour4;

            // Comparar a linha com o input
            if (compare_game_try(game_try, &line_game)) {
                fclose(file);
                return TRUE;  // Encontrou uma correspondência
            }
        }

        line_num++;
    }

    fclose(file);
    return FALSE;  // Não encontrou nenhuma correspondência
}


int has_ongoing_game(const char *PLID) {

    char *file_path = get_game_folder_path(PLID);

    if (access(file_path, F_OK) == 0) {
        return FOUND;
    } else {
        return NOT_FOUND;
    }
}

void write_game_line(const char *PLID, const char *message) {
    
    char *file_path = get_game_folder_path(PLID);

    FILE *file = fopen(file_path, "a");
    if (file == NULL) {
        perror("Erro ao abrir o ficheiro");
        return;
    }
    fprintf(file, "%s", message);
    fflush(file);
    fclose(file);
}

void create_game_log_timestamp(const char *PLID, GameTry *game_solution, char *time_value, char mode) {

    time_t now = time(NULL);
    if (now == -1) {
        perror("Erro ao obter o tempo atual");
        return;
    }

    struct tm *current_time = gmtime(&now);
    if (current_time == NULL) {
        perror("Erro ao converter o tempo");
        return;
    }

    char timestr[50];
    snprintf(timestr, sizeof(timestr), "%4d-%02d-%02d %02d:%02d:%02d",
                                        current_time->tm_year + 1900,
                                        current_time->tm_mon + 1,
                                        current_time->tm_mday,
                                        current_time->tm_hour,
                                        current_time->tm_min,
                                        current_time->tm_sec);

    char fullstr[100];
    snprintf(fullstr, sizeof(fullstr), "%s %c %c%c%c%c %s %s %ld\n", 
                                        PLID, mode, 
                                        game_solution->colours[0],
                                        game_solution->colours[1],
                                        game_solution->colours[2],
                                        game_solution->colours[3],
                                        time_value,
                                        timestr, now);

    write_game_line(PLID, fullstr);
}

int extract_game_colour(const char *PLID, GameTry *game) {

    char *colours = extract_game_info(PLID, ARG_SOLUTION);
    strncpy(game->colours, colours, 4);

    return 0;
}

// Alocar dinamicamente o espaço para a string e retornar
char* extract_game_info(const char *PLID, const char arg_type) {

    char *file_path = get_game_folder_path(PLID);

    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Erro ao abrir o ficheiro");
        return NULL;
    }

    char line[256];
    if (fgets(line, sizeof(line), file) == NULL) {
        perror("Erro ao ler a linha do ficheiro");
        fclose(file);
        return NULL;
    }

    // Extrair o token correspondente
    char *token = strtok(line, " ");
    int i = 0;
    while (token != NULL) {
        if (i == arg_type) {
            // Alocar dinamicamente a string para retornar
            char *time = malloc(strlen(token) + 1);
            if (time != NULL) {
                strcpy(time, token);
            }
            fclose(file);
            return time; // Retorna a string alocada dinamicamente
        }
        token = strtok(NULL, " ");
        i++;
    }

    fclose(file);
    return NULL;
}



int inTime(const char *PLID) {
    // Extrair os tempos do ficheiro
    char *startTimeStr = extract_game_info(PLID, ARG_ELAPSED_TIME);
    char *maxTimeStr = extract_game_info(PLID, ARG_MAXTIME);
    
    if (startTimeStr == NULL || maxTimeStr == NULL) {
        fprintf(stderr, "Erro ao extrair os tempos do ficheiro.\n");
        return 0;
    }

    int start_time = atoi(startTimeStr);
    int max_time = atoi(maxTimeStr);
    time_t now = time(NULL);
    int current_time = (int)now;
    
    if ((current_time - start_time) > max_time) return FALSE;
    
    return TRUE;
}

void write_try(const char *PLID, GameTry game_try, int *game_try_res) {

    char *file_path = get_game_folder_path(PLID);

    FILE *file = fopen(file_path, "a");
    if (file == NULL) {
        perror("Erro ao abrir o ficheiro");
        return;
    }

    char message[50];

    int *try_res = malloc(sizeof(int) * 2);

    GameTry game_solution;
    strncpy(game_solution.colours, extract_game_info(PLID, ARG_SOLUTION), sizeof(game_solution.colours));
    
    check_try(PLID, game_try, try_res);

    char *startTime = extract_game_info(PLID, ARG_ELAPSED_TIME);
    int elapsed_time = (int)time(NULL) - atoi(startTime);
    sprintf(message, "T: %c%c%c%c %d %d %d", 
                                        game_try.colours[0],                                               
                                        game_try.colours[1],
                                        game_try.colours[2],
                                        game_try.colours[3],
                                        game_try_res[0],
                                        game_try_res[1],
                                        elapsed_time);

    
    fprintf(file, "%s\n", message);
    fflush(file);
    fclose(file);
}

int is_integer(const char *str) {
    while (*str) {
        if (!isdigit(*str)) return FALSE;
        str++;
    }
    return TRUE;
}

int is_valid_ip(const char *ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) != 0;
}

int is_valid_color(char C) {
    if (C != 'R' && C != 'G' && C != 'B' && C != 'Y' && C != 'O' && C != 'P') {
        return FALSE;
    }

    return TRUE;
}
