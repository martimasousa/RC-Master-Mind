#include "utils.h"


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


// FUNCTIONS GIVEN BY THE UC ----------------------------------------------------------------------

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


// Função para encontrar o último jogo de um jogador com base no PLID
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
                snprintf(fname, 256, "GAMES/%s/%s", PLID, filelist[nentries]->d_name);
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