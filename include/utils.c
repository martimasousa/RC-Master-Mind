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
        char colour1, colour2, colour3, colour4;
        int result = sscanf(line, "T: %c%c%c%c", &colour1, &colour2, &colour3, &colour4);

        if (result == 4) {
            GameTry line_game;
            line_game.colours[0] = colour1;
            line_game.colours[1] = colour2;
            line_game.colours[2] = colour3;
            line_game.colours[3] = colour4;

            if (compare_game_try(game_try, &line_game)) {
                fclose(file);
                return TRUE;
            }
        }
        line_num++;
    }
    fclose(file);
    return FALSE;
}


int has_ongoing_game(const char *PLID) {

    char *file_path = get_game_folder_path(PLID);

    if (access(file_path, F_OK) == 0) {
        return FOUND;
    } else {
        return NOT_FOUND;
    }
}

int hasExceededMaxTurn(char trial_number) {
    return (trial_number - '0' > MAX_TRIALS);
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

    int max_time = atoi(extract_game_info(PLID, ARG_MAXTIME));
    
    return (get_elapsed_time(PLID) < max_time);
}


int get_elapsed_time(const char *PLID) {
    return (int)time(NULL) - atoi(extract_game_info(PLID, ARG_ELAPSED_TIME));
}


void write_try(const char *PLID, GameTry game_try, int *player_try_res) {

    char message[50], elapsed_time;

    elapsed_time = get_elapsed_time(PLID);

    sprintf(message, "T: %c%c%c%c %d %d %d\n", 
                                        game_try.colours[0],                                               
                                        game_try.colours[1],
                                        game_try.colours[2],
                                        game_try.colours[3],
                                        player_try_res[0],
                                        player_try_res[1],
                                        elapsed_time);

    write_game_line(PLID, message);
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


// Reads ' ' or '\0' separated word from the file descriptor.
int read_word_from_fd(int fd, char** word) {
    char c;
    size_t i = 0;  // Keep track of the current position in the word

    size_t capacity = 5;  // Initialize word capacity
    *word = malloc(capacity * sizeof(char));
    if (*word == NULL) {
        fprintf(stderr, "Memory allocation failed");
        return 1;
    }
    
    while (1) {
        ssize_t n = read(fd, &c, 1);  // Read one byte at a time
        if (n == -1) {
            fprintf(stderr, "Error while reading from TCP socket.\n");
            return 1;
        }
        if (n == 0) {  // End of stream (connection closed)
            fprintf(stderr, "Unexpected end of stream while reading word.\n");
            return 1;
        }

        // If the buffer is full, reallocate more memory
        if (i >= capacity - 1) {  // We reserve the last byte for the null-terminator
            capacity *= 2;  // Double the capacity
            *word = realloc(*word, capacity * sizeof(char));  // Reallocate memory
            if (*word == NULL) {
                fprintf(stderr, "Memory reallocation failed");
                return 1;
            }
        }

        // If we encounter a space or null character, stop reading
        if (c == ' ' || c == '\0') {
            break;
        }

        // Add the character to the word
        (*word)[i++] = c;
    }
    (*word)[i] = '\0';  // Null-terminate the word
    return 0;
}