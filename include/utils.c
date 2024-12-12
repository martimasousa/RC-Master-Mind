#include "utils.h"

/*
    STATUS VERIFICATION auxiliar functions 
*/

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

int file_exists(const char *path) {
    FILE *file = fopen(path, "r");
    if (file) {
        fclose(file);
        return TRUE;
    }
    return FALSE;
}


int validate_start_command(const char *cmd, char *PLID_arg, char *max_playtime_arg, const int execution_side) {
    char *expected_command_name;
    char input_command[INPUT_ARGUMENT_LEN];
    char temp_PLID[INPUT_ARGUMENT_LEN];
    char temp_time[INPUT_ARGUMENT_LEN];

    if (execution_side == SERVER_SIDE) expected_command_name = "SNG"; 
    else if (execution_side == PLAYER_SIDE) expected_command_name = "start";
    else {
        fprintf(stderr, "Error: Invalid execution side.\n");
        return ERROR;
    }

    // Usar sscanf para ler os valores da string cmd
    int items_read = sscanf(cmd, "%s %s %s", input_command, temp_PLID, temp_time);
    if (items_read < 3) {
        fprintf(stderr, "Error: Command is malformed or missing arguments.\n");
        return ERROR;
    }

    // Verificar se o comando corresponde ao esperado
    if (strcmp(input_command, expected_command_name) != 0) {
        fprintf(stderr, "Error: Command is malformed. It should start with '%s'.\n", expected_command_name);
        return ERROR;
    }

    // Validar PLID
    if (strlen(temp_PLID) != PLID_DIGITS || !is_integer(temp_PLID)) {
        fprintf(stderr, "Error: PLID must be an integer with 6 digits.\n");
        return ERROR;
    }

    // Validar max_playtime
    if (strlen(temp_time) > TIME_DIGITS || !is_integer(temp_time)) {
        fprintf(stderr, "Error: Time must be an integer with less or equal than 3 digits.\n");
        return ERROR;
    }

    int max_playtime = atoi(temp_time);
    if (max_playtime > 600 || max_playtime <= 0) {
        fprintf(stderr, "Error: max_playtime must be a value between 1 and 600 seconds.\n");
        return ERROR;
    }

    // Verificar se há argumentos extras
    const char *remaining = cmd + strlen(input_command) + 1 + strlen(temp_PLID) + 1 + strlen(temp_time);
    while (*remaining == ' ') remaining++;
    if (*remaining != '\0' && *remaining != '\n') {
        fprintf(stderr, "Error: Command contains extra arguments or is malformed.\n");
        return ERROR;
    }

    // Copiar valores para as variáveis de saída
    strcpy(PLID_arg, temp_PLID);
    strcpy(max_playtime_arg, temp_time);

    return OK;
}

int validate_debug_command(const char *cmd, char *PLID_arg, char *max_playtime_arg, char *C1, char *C2, char *C3, char *C4, const int execution_side) {
    
    char *expected_command_name;
    char input_command[INPUT_ARGUMENT_LEN];
    char temp_PLID[INPUT_ARGUMENT_LEN];
    char temp_max_playtime[INPUT_ARGUMENT_LEN];

    // Determinar o comando esperado com base no lado de execução
    if (execution_side == SERVER_SIDE) expected_command_name = "DBG";
    else if (execution_side == PLAYER_SIDE) expected_command_name = "debug";
    else {
        fprintf(stderr, "Error: Invalid execution side.\n");
        return ERROR;
    }

    // Usar sscanf para extrair os valores da string cmd
    int items_read = sscanf(cmd, "%s %s %s %c %c %c %c",
                            input_command, temp_PLID, temp_max_playtime, C1, C2, C3, C4);

    // Verificar se os argumentos foram lidos corretamente
    if (items_read != 7) {
        fprintf(stderr, "Error: Command format should be '%s PLID max_playtime C1 C2 C3 C4'.\n", expected_command_name);
        return ERROR;
    }

    // Verificar se o comando inicial corresponde ao esperado
    if (strcmp(input_command, expected_command_name) != 0) {
        fprintf(stderr, "Error: Command is malformed. It should start with '%s'.\n", expected_command_name);
        return ERROR;
    }

    // Verificar se PLID é válido
    if (strlen(temp_PLID) != PLID_DIGITS || !is_integer(temp_PLID)) {
        fprintf(stderr, "Error: PLID must be an integer with 6 digits.\n");
        return ERROR;
    }

    // Verificar se max_playtime é válido
    if (strlen(temp_max_playtime) > TIME_DIGITS || !is_integer(temp_max_playtime)) {
        fprintf(stderr, "Error: max_playtime must be an integer with less or equal than 3 digits.\n");
        return ERROR;
    }

    int max_playtime = atoi(temp_max_playtime);
    if (max_playtime > 600 || max_playtime <= 0) {
        fprintf(stderr, "Error: max_playtime must be a value between 1 and 600 seconds.\n");
        return ERROR;
    }

    // Verificar se as cores são válidas
    if (!is_valid_color(*C1) || !is_valid_color(*C2) || !is_valid_color(*C3) || !is_valid_color(*C4)) {
        fprintf(stderr, "Error: The valid colors are: {red (R), green (G), blue (B), yellow (Y), orange (O), purple (P)}.\n");
        return ERROR;
    }

    // Verificar se há argumentos extras
    const char *remaining = cmd + strlen(input_command) + 1 + strlen(temp_PLID) + 1 +
                            strlen(temp_max_playtime) + 1 + 1 + 1 + 1 + 1;
    while (*remaining == ' ') remaining++;
    if (*remaining != '\0' && *remaining != '\n') {
        fprintf(stderr, "Error: Command contains extra arguments or is malformed.\n");
        return ERROR;
    }

    // Copiar valores para as variáveis de saída
    strcpy(PLID_arg, temp_PLID);
    strcpy(max_playtime_arg, temp_max_playtime);

    return OK;
}


int validate_showtrials_command(const char *cmd, char *PLID, const int execution_side) {
    
    char input_command[INPUT_ARGUMENT_LEN];
    char temp_PLID[INPUT_ARGUMENT_LEN];


    // Usar sscanf para ler os valores da string cmd
    int items_read = sscanf(cmd, "%s %s", input_command, temp_PLID);
    if (items_read < 1) {
        fprintf(stderr, "Error: Command is malformed or missing arguments.\n");
        return ERROR;
    }

    // Verificar se o comando corresponde ao esperado
    if (execution_side == SERVER_SIDE) {
        if (strcmp(input_command, "STR") != 0) {
            fprintf(stderr, "Error: Command should start with 'STR'.\n");
            return ERROR;
        }

        if (items_read != 2 || strlen(temp_PLID) != PLID_DIGITS || !is_integer(temp_PLID)) {
            fprintf(stderr, "Error: PLID must be an integer with 6 digits.\n");
            return ERROR;
        }

        strcpy(PLID, temp_PLID);
    } else if (execution_side == PLAYER_SIDE) {
        if (strcmp(input_command, "show_trials") != 0 && strcmp(input_command, "st") != 0) {
            fprintf(stderr, "Error: Command should start with 'show_trials' or 'st'.\n");
            return ERROR;
        }
    }

    // Verificar se há argumentos extras
    const char *remaining = cmd + strlen(input_command) + 1;
    if (execution_side == SERVER_SIDE) {
        remaining += strlen(temp_PLID) + 1;
    }
    while (*remaining == ' ') remaining++;
    if (*remaining != '\0' && *remaining != '\n') {
        fprintf(stderr, "Error: Command contains extra arguments or is malformed.\n");
        return ERROR;
    }

    return OK;
}

int validate_scoreboard_command(const char *cmd, const int execution_side) {

    char input_command[INPUT_ARGUMENT_LEN];

    // Usar sscanf para ler o comando
    int items_read = sscanf(cmd, "%s", input_command);
    if (items_read < 1) {
        fprintf(stderr, "Error: Command is malformed or missing.\n");
        return ERROR;
    }

    // Verificar se o comando corresponde ao esperado
    if (execution_side == SERVER_SIDE) {
        if (strcmp(input_command, "SSB") != 0) {
            fprintf(stderr, "Error: Command should start with 'SSB'.\n");
            return ERROR;
        }
    } else if (execution_side == PLAYER_SIDE) {
        if (strcmp(input_command, "scoreboard") != 0 && strcmp(input_command, "sb") != 0) {
            fprintf(stderr, "Error: Command should start with 'scoreboard' or 'sb'.\n");
            return ERROR;
        }
    }

    // Verificar se há argumentos extras
    const char *remaining = cmd + strlen(input_command);
    while (*remaining == ' ') remaining++;
    if (*remaining != '\0' && *remaining != '\n') {
        fprintf(stderr, "Error: Command contains extra arguments or is malformed.\n");
        return ERROR;
    }

    return OK;
}


/*
    GENERAL auxiliar functions
*/

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
    int result = mkdir(directory, 0777);

    if (result == 0) {
        // printf("Diretoria '%s' criada com sucesso.\n", directory);
        return 0; // Sucesso
    }

    // Se houver erro, verifica o valor de errno
    if (errno == EEXIST) {
        fprintf(stderr, "Diretoria '%s' já existe.\n", directory);
        return 0; // Não tratamos como erro, pois o diretório já existe
    } else {
        fprintf(stderr, "Erro ao criar diretoria"); // Mostra o erro apropriado
        return -1; // Falha
    }
}

int write_to_file(const char *Fname, const char *Fdata) {
    // Open the file for writing, overwriting if it exists
    FILE *file = fopen(Fname, "w");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    // Write Fdata to the file
    if (fputs(Fdata, file) == EOF) {
        perror("Error writing to file");
        fclose(file);
        return -1;
    }

    // Close the file
    if (fclose(file) == EOF) {
        perror("Error closing file");
        return -1;
    }

    return 0; // Success
}

int convert_char_to_int(const char number) {
    return number - '0';
}
/*
    MESSAGE sending and receiving functions
*/

int tcp_read_until_delimiter(int fd, char** word, char separator, int n_times) {
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
        if (n == 0) {  // End of stream (either due to EOF or socket closed by other peer)
            // Return content read until this point
            break;

            // fprintf(stderr, "Unexpected end of stream while reading word.\n");
            // return 1;
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
        if (c == separator) {
            n_times--;

            if (n_times == 0) break;
        }

        // Add the character to the word
        (*word)[i++] = c;
    }
    (*word)[i] = '\0';  // Null-terminate the word

    return 0;
}

int tcp_write(int fd, char* to_write) {
    int total = 0;
    int len = strlen(to_write);
    while (total < len) {
        int n = write(fd, to_write + total, len - total);
        if (n < 0) {
            fprintf(stderr, "Error while writing to tcp fd");
            return 1;
        }
        total += n;  // Increment total by the number of bytes written
    }

    return 0;
}

ssize_t recv_udp_message(int udp_fd, char *buffer, size_t buffer_size, struct sockaddr_in *client_addr) {
    socklen_t client_len = sizeof(*client_addr);
    ssize_t n = recvfrom(udp_fd, buffer, buffer_size - 1, 0, (struct sockaddr *)client_addr, &client_len);
    
    if (n > 0) {
        buffer[n] = '\0';
    } else return -1;

    return n;
}

void send_udp_response(int udp_fd, const char *message, struct sockaddr_in *client_addr) {
    socklen_t client_len = sizeof(*client_addr);
    sendto(udp_fd, message, strlen(message) + 1, 0, (struct sockaddr *)client_addr, client_len);
}
