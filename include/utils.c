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

// Função para iterar pelas linhas do ficheiro
// Retorna true se já houver alguma linha igual
int extract_colors_from_file(const char *PLID, GameTry *game_try) {
    
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
        int num;  // O número no início (não será utilizado, mas podemos ler se necessário)
        char colour1, colour2, colour3, colour4;

        // Usando sscanf para ler os dados da linha
        int result = sscanf(line, "%d: %c %c %c %c", &num, &colour1, &colour2, &colour3, &colour4);

        // Verifica se leu corretamente a linha
        if (result == 5) {
            // Criar a estrutura GameTry e armazenar as cores
            GameTry line_game;
            line_game.colours[0] = colour1;
            line_game.colours[1] = colour2;
            line_game.colours[2] = colour3;
            line_game.colours[3] = colour4;

            // Comparar a linha com o input
            if (compare_game_try(game_try, &line_game)) {
                printf("Linha %d - Encontrado! Cores: %c %c %c %c\n", line_num + 2, line_game.colours[0], line_game.colours[1], line_game.colours[2], line_game.colours[3]);
                fclose(file);
                return TRUE;  // Encontrou uma correspondência
            }
        } else {
            fprintf(stderr, "Formato inválido na linha %d\n", line_num + 2);
        }

        line_num++;
    }

    fclose(file);
    return FALSE;  // Não encontrou nenhuma correspondência
}


int search_file(const char *PLID) {

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

    write_game_line(PLID, fullstr);
}

int extract_game_colour(const char *PLID, GameTry *game) {

    char *file_path = get_game_folder_path(PLID);

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
    
    // Avançar até o terceiro token
    while (token != NULL && i < 2) {
        token = strtok(NULL, " ");
        i++;
    }

    strncpy(game->colours, token, 4);
    fclose(file);

    return 0;
}

// Alocar dinamicamente o espaço para a string e retornar
char* extract_game_time(const char *PLID, const char time_type) {

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

    int max = 0;
    if (time_type == MAX_GAME_TIME) {
        max = 3;
    } else if (time_type == START_TIME) {
        max = 6;
    }

    // Extrair o token correspondente
    char *token = strtok(line, " ");
    int i = 0;
    while (token != NULL) {
        if (i == max) {
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
    char *startTimeStr = extract_game_time(PLID, START_TIME);
    char *maxTimeStr = extract_game_time(PLID, MAX_GAME_TIME);

    printf("StartTimeStr: %s\n", startTimeStr);
    printf("MaxTimeStr: %s\n", maxTimeStr);  // Corrigir aqui

    
    if (startTimeStr == NULL || maxTimeStr == NULL) {
        fprintf(stderr, "Erro ao extrair os tempos do ficheiro.\n");
        return 0;
    }

    int start_time = atoi(startTimeStr);
    int max_time = atoi(maxTimeStr);
    time_t now = time(NULL);
    int current_time = (int)now;

    printf("1st Time %d\n", current_time);
    printf("2nd Time %d\n", start_time);

    printf("Diff: %d, Max: %d\n", current_time-start_time, max_time);
    if ((current_time - start_time) > max_time) return FALSE;
    
    return TRUE;
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
