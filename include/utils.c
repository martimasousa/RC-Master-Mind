#include "utils.h"
#include "game_core.h"
#include "constants.h"
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>    // Para verificar erros com errno
#include <string.h>
#include <math.h>

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

char* get_player_folder_path(const char *PLID) {
    size_t path_length = strlen("./GAMES/") + strlen(PLID) + 1;

    char *file_path = malloc(path_length);
    if (file_path == NULL) {
        perror("Error allocating memory for the path!\n");
        return NULL;
    }
    snprintf(file_path, path_length, "./GAMES/%s", PLID);
    return file_path;
}

int create_directory(const char *directory) {
    int result = mkdir(directory, 0777); // Tenta criar o diretório

    if (result == 0) {
        printf("Diretoria '%s' criada com sucesso.\n", directory);
        return 0; // Sucesso
    }

    // Se houver erro, verifica o valor de errno
    if (errno == EEXIST) {
        printf("Diretoria '%s' já existe.\n", directory);
        return 0; // Não tratamos como erro, pois o diretório já existe
    } else {
        perror("Erro ao criar diretoria"); // Mostra o erro apropriado
        return -1; // Falha
    }
}

int move_file(const char *PLID, const char endGameType) {
    char *destinationDirectoryPath = get_player_folder_path(PLID);
    char *sourceFilePath = get_game_folder_path(PLID);
    char *newFileName = get_end_game_name(endGameType);
    time_t now = time(NULL);
    struct tm *current_time = gmtime(&now);

    size_t path_length = strlen(destinationDirectoryPath) + 1 + strlen(newFileName) + 1;
    char *destination_path = malloc(path_length);

    snprintf(destination_path, path_length, "%s/%s", destinationDirectoryPath, newFileName);

    char *message = malloc(sizeof(char) * 50);
    snprintf(message, 50, "%4d-%02d-%02d %02d:%02d:%02d %d",
                                        current_time->tm_year + 1900,
                                        current_time->tm_mon + 1,
                                        current_time->tm_mday,
                                        current_time->tm_hour,
                                        current_time->tm_min,
                                        current_time->tm_sec,
                                        get_elapsed_time(PLID));
    
    write_game_line(PLID, message);

    if (rename(sourceFilePath, destination_path) == 0) {
        printf("Ficheiro '%s' movido para '%s' com sucesso.\n", PLID, destinationDirectoryPath);
        return 0;
    } else {
        perror("Erro ao mover o ficheiro");
        return -1;
    }
}

char* get_end_game_name(char const type) {
    time_t now = time(NULL);
    struct tm *current_time = gmtime(&now);

    char *res = malloc(sizeof(char) * 50);
    snprintf(res, 50, "%4d%02d%02d_%02d%02d%02d_%c",
                                        current_time->tm_year + 1900,
                                        current_time->tm_mon + 1,
                                        current_time->tm_mday,
                                        current_time->tm_hour,
                                        current_time->tm_min,
                                        current_time->tm_sec,
                                        type);

    return res;
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

int has_exceeded_max_turn(char trial_number) {
    return (trial_number - '0' > MAX_TRIALS);
}

char* get_max_turn_response(const char* PLID) {
    GameTry *game_solution = malloc(sizeof(GameTry));
    
    extract_game_colour(PLID, game_solution);

    char *response = malloc(sizeof(char) * 100);
    snprintf(response, 100, "RTR ENT %c %c %c %c\n", game_solution->colours[0],
                                                                game_solution->colours[1],
                                                                game_solution->colours[2],
                                                                game_solution->colours[3]);

    return response;
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


int has_exceeded_time(const char *PLID) {

    int max_time = atoi(extract_game_info(PLID, ARG_MAXTIME));
    
    return (get_elapsed_time(PLID) > max_time);
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


int tcp_read_until_delimiter(int fd, char** word, char separator) {
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
        if (c == separator || c == '\0') {
            break;
        }

        // Add the character to the word
        (*word)[i++] = c;
    }
    (*word)[i] = '\0';  // Null-terminate the word
    return 0;
}

int line_size(FILE *fp) {
    char c;
    int res = 0;

    while ((c = fgetc(fp)) != EOF) {
        if (c == '\n') {
            res++;
            break;
        }
        res++;
    }

    return res;
}

int get_data_size(FILE *file) {
    int first_line_size = line_size(file);
    fseek(file, 0L, SEEK_END);
    int file_size = ftell(file);
    fseek(file, first_line_size, SEEK_SET);
    
    return (file_size - first_line_size);

}

int hasWon(int *player_try_res) {
    return (player_try_res[0] == 4);
}

int calculateScore(char *PLID, int turnsPlayed) {


    float elapsed_time = get_elapsed_time(PLID);
    float max_time = (float)atoi(extract_game_info(PLID, ARG_MAXTIME));

    // Calcule o score
    float score = 100.0f * (((float)MAX_ALLOWED_PLAYS - turnsPlayed) / (float)MAX_ALLOWED_PLAYS) * 
                            (1.0f - (elapsed_time / (float)MAX_ALLOWED_TIME)) * 
                            ((float)MAX_ALLOWED_TIME / max_time);

    // Arredonde para 2 casas decimais
    return round(score);
}

char* getScoreFileName(int score, char *PLID) {
    char *res = "77";

    return res;

    // char string[BUFFER_SIZE];
    // time_t now = time(NULL);
    // struct tm *current_time = gmtime(&now);

    // sprintf(string, "%d_%s_%02d%02d%4d_%02d%02d%02d.txt", score, PLID, 
    //                                                       current_time->tm_mday,
    //                                                       current_time->tm_mon + 1,
    //                                                       current_time->tm_year + 1900,
    //                                                       current_time->tm_hour,
    //                                                       current_time->tm_min,
    //                                                       current_time->tm_sec);

    // char *res = malloc(sizeof(char) * strlen(string));
    // memcpy(res, string, strlen(string));
    // return res;
    
}

int directoryExists(char *filepath) {
    struct stat st;

    return (stat(filepath, &st) == 0 && S_ISDIR(st.st_mode));
}