#include "utils.h"
#include "game_core.h"
#include "constants.h"
#include <stdio.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <string.h>


int search_file(const char *file_name) {
    const char *directory = "./GAMES";
    char full_path[512];

    snprintf(full_path, sizeof(full_path), "%s/%s", directory, file_name);

    if (access(full_path, F_OK) == 0) {
        return FOUND;
    } else {
        return NOT_FOUND;
    }
}

void write_game_line(const char *file_path, const char *message) {
    FILE *file = fopen(file_path, "a");
    if (file == NULL) {
        perror("Erro ao abrir o ficheiro");
        return;
    }
    fprintf(file, "%s", message);
    fflush(file);
    fclose(file);
}

void create_game_log_timestamp(const char *file_path, const char *PLID, GameTry *game_solution, char *time_value, char mode) {

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

    char timestr[20];
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

    write_game_line(file_path, fullstr);
}

int extract_game_colour(const char *file_path, GameTry *game) {
    
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Erro ao abrir o ficheiro");
        return -1;
    }

    char line[256];
    if (fgets(line, sizeof(line), file) == NULL) {
        perror("Erro ao ler a linha do ficheiro");
        fclose(file);
        return -1;
    }

    // Dividir a linha em partes usando espaço como delimitador
    char *token = strtok(line, " ");
    int i = 0;
    
    // Avançar até o terceiro token (BYYO)
    while (token != NULL && i < 2) {
        token = strtok(NULL, " ");
        i++;
    }

    strncpy(game->colours, token, 4);
    fclose(file);

    return 0;
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
