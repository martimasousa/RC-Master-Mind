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

int is_valid_port(const char *str) {
    if (!is_integer(str)) {
        return FALSE;
    }

    int port = atoi(str);

    return port >= 0 && port <= 65535;
}

int is_valid_hostname(const char *hostname) {
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // Any socket type

    // Use getaddrinfo to resolve the hostname
    int status = getaddrinfo(hostname, NULL, &hints, &res);
    if (status == 0) {
        freeaddrinfo(res); // Free the linked list
        return 1;          // Valid hostname
    }

    return 0; // Invalid hostname
}

int is_valid_ip(const char *ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) != 0;
}

int is_valid_address(const char *input) {
    if (is_valid_ip(input)) {
        return 1; // It's a valid IP address
    }
    if (is_valid_hostname(input)) {
        return 1; // It's a valid hostname
    }
    return 0; // Neither valid IP nor hostname
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
    if (*remaining != '\n') {
        fprintf(stderr, "Error: Command contains extra arguments or is malformed.\n");
        return ERROR;
    }

    // Copiar valores para as variáveis de saída
    strcpy(PLID_arg, temp_PLID);
    strcpy(max_playtime_arg, temp_time);

    return OK;
}

int validate_try_command(const char *cmd, char *PLID_arg, char *C1, char *C2, char *C3, char *C4, char *nT, const int execution_side) {
    
    char *expected_command_name;
    char input_command[INPUT_ARGUMENT_LEN];
    char temp_PLID[INPUT_ARGUMENT_LEN];

    // Determinar o comando esperado com base no lado de execução
    if (execution_side == SERVER_SIDE) expected_command_name = "TRY";
    else if (execution_side == PLAYER_SIDE) expected_command_name = "try";
    else {
        fprintf(stderr, "Error: Invalid execution side.\n");
        return ERROR;
    }

    // Verificar os comandos do cliente e do servidor
    if (execution_side == SERVER_SIDE) {
        // Para o lado do servidor, o comando esperado é "TRY PLID C1 C2 C3 C4 nT"
        int items_read = sscanf(cmd, "%s %s %s %s %s %s %s",
                                input_command, temp_PLID, C1, C2, C3, C4, nT);

        if (items_read != 7) {
            fprintf(stderr, "Error: Command format should be '%s PLID C1 C2 C3 C4 nT'.\n", expected_command_name);
            return ERROR;
        }

        if (strcmp(input_command, expected_command_name) != 0) {
            fprintf(stderr, "Error: Command is malformed. It should start with '%s'.\n", expected_command_name);
            return ERROR;
        }

        // Validar o PLID
        if (strlen(temp_PLID) != PLID_DIGITS || !is_integer(temp_PLID)) {
            fprintf(stderr, "Error: PLID must be an integer with 6 digits.\n");
            return ERROR;
        }

        // Validar nT
        if (*nT <= '0' || *nT >= '9') {
            fprintf(stderr, "Error: nT must be a positive integer.\n");
            return ERROR;
        }

        // Copiar PLID para a variável de saída
        strcpy(PLID_arg, temp_PLID);

    } else if (execution_side == PLAYER_SIDE) {
        // Para o lado do cliente, o comando esperado é "try C1 C2 C3 C4"
        int items_read = sscanf(cmd, "%s %s %s %s %s", input_command, C1, C2, C3, C4);

        if (items_read != 5) {
            fprintf(stderr, "Error: Command format should be '%s C1 C2 C3 C4'.\n", expected_command_name);
            return ERROR;
        }

        if (strcmp(input_command, expected_command_name) != 0) {
            fprintf(stderr, "Error: Command is malformed. It should start with '%s'.\n", expected_command_name);
            return ERROR;
        }
    }

    // Validar as cores
    if (!is_valid_color(*C1) || !is_valid_color(*C2) || !is_valid_color(*C3) || !is_valid_color(*C4)) {
        fprintf(stderr, "Error: The valid colors are: red (R), green (G), blue (B), yellow (Y), orange (O), purple (P).\n");
        return ERROR;
    }

    // Verificar se há argumentos extras
    int total = 0;
    total += strlen(input_command) + 1;

    const char *remaining = cmd + strlen(input_command);
    if (execution_side == SERVER_SIDE) {
        remaining += 1 + strlen(temp_PLID) + 1 + (4 * COLOR_LEN* 2) + 1;
    } else if (execution_side == PLAYER_SIDE) {
        total += (4 * COLOR_LEN * 2);
        remaining += (4 * COLOR_LEN * 2);
    }

    while (*remaining == ' ') remaining++;
    if (*remaining != '\n') {
        fprintf(stderr, "Error: Command contains extra arguments or is malformed.\n");
        return ERROR;
    }

    return OK;
}

int validate_quit_command(const char *cmd, char *PLID_arg, const int execution_side, const int context_flag) {
    char input_command[INPUT_ARGUMENT_LEN], temp_PLID[INPUT_ARGUMENT_LEN];
    const char *expected_end;

    if (sscanf(cmd, "%s", input_command) != 1) {
        fprintf(stderr, "Error: Command is empty or malformed.\n");
        return ERROR;
    }

    if (execution_side == SERVER_SIDE) {
        // Validação no servidor
        if (strcmp(input_command, "QUT") != 0) {
            fprintf(stderr, "Error: Command should start with 'QUT'.\n");
            return ERROR;
        }
        if (sscanf(cmd, "QUT %s", temp_PLID) != 1 || strlen(temp_PLID) != PLID_DIGITS || !is_integer(temp_PLID)) {
            fprintf(stderr, "Error: PLID must be an integer with 6 digits.\n");
            return ERROR;
        }
        strcpy(PLID_arg, temp_PLID);
        expected_end = cmd + strlen("QUT") + 1 + strlen(temp_PLID);
    } else if (execution_side == PLAYER_SIDE) {
        // Validação no cliente
        const char *valid_command = (context_flag == QUIT_CONTEXT) ? "quit" : "exit";
        if (strcmp(input_command, valid_command) != 0) {
            fprintf(stderr, "Error: Command should be '%s'.\n", valid_command);
            return ERROR;
        }
        expected_end = cmd + strlen(valid_command);
    } else {
        fprintf(stderr, "Error: Invalid execution side.\n");
        return ERROR;
    }

    // Verifica espaços extras e final do comando
    while (*expected_end == ' ') expected_end++;
    if (*expected_end != '\n') {
        fprintf(stderr, "Error: Command contains extra arguments or is malformed.\n");
        return ERROR;
    }

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
    int items_read = sscanf(cmd, "%s %s %s %s %s %s %s",
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
                            strlen(temp_max_playtime) + 4*COLOR_LEN*2;
    while (*remaining == ' ') remaining++;
    if (*remaining != '\n') {
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
    const char *remaining = cmd + strlen(input_command);
    if (execution_side == SERVER_SIDE) {
        remaining += 1 + strlen(temp_PLID);
    }

    while (*remaining == ' ') remaining++;
    if (*remaining != '\n' && *remaining != '\0') {
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
char *special_inv_function(const char *PLID) {
    char *file_path = get_game_folder_path(PLID);

    if (access(file_path, F_OK) == 0) {
        return file_path;
    }
    
    file_path = realloc(file_path, sizeof(char) * BUFFER_SIZE);
    FindLastGame(PLID, file_path);
    return file_path;
}

char* get_game_folder_path(const char *PLID) {
    size_t path_length = strlen("./GAMES/GAME_") + strlen(PLID) + 1;

    char *file_path = malloc(sizeof(char) * path_length);
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

char *int_to_string(int number) {
    int len = 0, temp = number, is_negative = 0;

    // Tratar o número zero como caso especial
    if (number == 0) {
        char *result = malloc(2); // Um dígito + '\0'
        if (!result) return NULL; // Verificar alocação
        result[0] = '0';
        result[1] = '\0';
        return result;
    }

    // Lidar com números negativos
    if (number < 0) {
        is_negative = 1;
        number = -number;
    }

    // Contar o número de dígitos no número
    while (temp != 0) {
        len++;
        temp /= 10;
    }

    // Alocar memória para os dígitos e o terminador nulo
    char *result = malloc(len + is_negative + 1); // +1 para sinal e +1 para '\0'
    if (!result) return NULL; // Verificar alocação

    result[len + is_negative] = '\0'; // Terminar a string

    // Converter os dígitos para caracteres
    for (int i = len + is_negative - 1; i >= is_negative; i--) {
        result[i] = (number % 10) + '0';
        number /= 10;
    }

    // Adicionar o sinal negativo, se necessário
    if (is_negative) {
        result[0] = '-';
    }

    return result;
}

char *create_string(const char *components[], size_t count) {

    // Calcular o tamanho total da string
    size_t total_length = 0;
    for (size_t i = 0; i < count; i++) {
        total_length += strlen(components[i]);
    }
    // Alocar memória para a string resultante
    char *result = (char *)malloc(total_length + 1); // +1 para o terminador nulo
    if (!result) {
        perror("Erro ao alocar memória");
        return NULL;
    }

    size_t offset = 0;

    // Construir a string usando memcpy
    for (size_t i = 0; i < count; i++) {
        size_t len = strlen(components[i]);
        memcpy(result + offset, components[i], len);
        offset += len;
    }

    // Adicionar terminador nulo
    result[offset] = '\0';

    return result;
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
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                fprintf(stderr, "Read operation timed out.\n");
            } else {
                fprintf(stderr, "Error while reading from TCP socket.\n");
            }
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
            return ERROR;
        }
        total += n;  // Increment total by the number of bytes written
    }

    return OK;
}

int tcp_read(int fd, char **buffer, size_t n) {
    if (!buffer) {
        fprintf(stderr, "Invalid buffer pointer.\n");
        return ERROR;
    }

    // Alocar memória para o buffer
    *buffer = (char *)malloc(n + 1); // +1 para potencial terminador nulo
    if (!(*buffer)) {
        perror("Failed to allocate memory for buffer");
        return ERROR;
    }

    size_t total_read = 0;
    ssize_t bytes_read;

    // Loop para ler até `n` bytes ou erro
    while (total_read < n) {
        bytes_read = read(fd, *buffer + total_read, n - total_read);

        if (bytes_read < 0) {
            perror("Error reading from file descriptor");
            free(*buffer); // Liberar memória em caso de erro
            *buffer = NULL; // Prevenir dangling pointers
            return ERROR;
        }

        if (bytes_read == 0) {
            // EOF encontrado antes de ler `n` bytes
            fprintf(stderr, "Unexpected EOF encountered.\n");
            free(*buffer);
            *buffer = NULL;
            return ERROR;
        }

        total_read += bytes_read;
    }

    // Garantir que o buffer seja nulo-terminado, caso seja usado como string
    (*buffer)[n] = '\0';

    return OK; // Sucesso
}

char *read_until_null(int fd) {
    char buffer[BUFFER_SIZE];
    size_t length = 0; // Comprimento atual da string

    while (1) {
        char c;
        ssize_t bytes_read = read(fd, &c, 1);

        if (bytes_read < 0) {
            // Erro na leitura
            return NULL;
        }

        if (bytes_read == 0) {
            // Socket fechado antes de encontrar '\0'
            errno = EPIPE;
            return NULL;
        }

        if (length >= BUFFER_SIZE - 1) {
            // O buffer é limitado, estourou o tamanho permitido
            errno = ENOMEM;
            return NULL;
        }

        buffer[length++] = c;

        // Verifica se encontrou o caractere '\0'
        if (c == '\0') {
            break;
        }
    }

    // Cria uma string com tamanho exato
    char *result = malloc(length);
    if (!result) {
        errno = ENOMEM;
        return NULL;
    }

    memcpy(result, buffer, length);
    return result;
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


// FUNCTIONS GIVEN BY THE UC ---------------------------------------------------

int FindLastGame(const char *PLID, char *fname) {
    struct dirent **filelist;
    int nentries, found;
    char dirname[256]; // Buffer para armazenar o diretório

    // Construir o caminho do diretório com base no PLID
    snprintf(dirname, sizeof(dirname), "GAMES/%s/", PLID);

    // Ler as entradas do diretório, usando alphasort para ordenar os arquivos
    nentries = scandir(dirname, &filelist, NULL, alphasort);
    found = 0;

    // Se não há entradas no diretório ou ocorreu um erro
    if (nentries <= 0) {
        return 0;
    } else {
        // Percorrer as entradas do diretório de trás para frente
        while (nentries--) {
            // Ignorar arquivos ocultos (nomes que começam com '.')
            if (filelist[nentries]->d_name[0] != '.' && !found) {
                // Construir o caminho completo do arquivo
                snprintf(fname, BUFFER_SIZE, "GAMES/%s/%s", PLID, filelist[nentries]->d_name);
                found = 1; // Marca como encontrado
            }

            // Liberar a entrada de diretório
            free(filelist[nentries]);
        }

        // Liberar a lista de entradas
        free(filelist);
    }

    return found;
}

int FindTopScores(SCORELIST *list) {
    struct dirent **filelist;
    int nentries, ifile;
    char fname[300];
    FILE *fp;
    char mode[8];

    nentries = scandir("SCORES/", &filelist, 0, alphasort);
    if (nentries <= 0)
        return (0);
    else {
        ifile = 0;
        while (nentries--) {
            if (filelist[nentries]->d_name[0] != '.' && ifile < 10) {
                sprintf(fname, "SCORES/%s", filelist[nentries]->d_name);
                fp = fopen(fname, "r");
                if (fp != NULL) {
                    fscanf(fp, "%d %s %s %d %s",
                           &list->score[ifile],
                           list->PLID[ifile],
                           list->colcode[ifile],
                           &list->notries[ifile],
                           mode);

                    if (!strcmp(mode, "PLAY"))
                        list->mode[ifile] = PLAY_MODE;
                    if (!strcmp(mode, "DEBUG"))
                        list->mode[ifile] = DEBUG_MODE;

                    fclose(fp);
                    ++ifile;
                }
            }
            free(filelist[nentries]);
        }
        free(filelist);
    }
    list->nscores = ifile;
    return (ifile);
}

// -----------------------------------------------------------------------------