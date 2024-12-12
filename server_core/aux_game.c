#include "aux_game.h"

/*
    STATUS VERIFICATION auxiliar functions 
*/

int has_ongoing_game(const char *PLID) {

    char *file_path = get_game_folder_path(PLID);

    if (access(file_path, F_OK) == 0) {
        return FOUND;
    } else {
        return NOT_FOUND;
    }
}

int has_exceeded_max_turn(const char trial_number) {
    return (trial_number - '0' > MAX_ALLOWED_PLAYS);
}

int has_exceeded_time(const char *PLID) {

    int max_time = atoi(extract_game_info(PLID, ARG_MAXTIME));    
    return (get_elapsed_time(PLID) > max_time);
}

int is_duplicated(const char *PLID, GameTry *game_try) {
    
    char *file_path = get_game_folder_path(PLID);

    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Error openning file.\n");
        return FALSE;
    }

    // Date String
    size_t timestr_len = YEAR_LEN + 1 + MONTH_LEN + 1 + DAY_LEN + 1 + HOUR_LEN + 1 + MINUTES_LEN + 1 + SECONDS_LEN;
    // First Line String
    size_t first_line_len = PLID_DIGITS + 1 + MODE_LEN + 4*COLOR_LEN + 1 + TIME_DIGITS + 1 + timestr_len + NOW_TIME_LEN + 1;

    char line[first_line_len + 1];
    int line_num = 0;

    // Ignores the first line
    if (fgets(line, sizeof(line), file) == NULL) {
        perror("Error reading from the file.\n");
        fclose(file);
        return FALSE;
    }

    // Iterates through the file lines
    while (fgets(line, sizeof(line), file) != NULL) {
        char colour1, colour2, colour3, colour4;
        int result = sscanf(line, "T: %c%c%c%c", &colour1, &colour2, &colour3, &colour4);

        if (result == 4) {
            GameTry line_game;
            line_game.colours[0] = colour1;
            line_game.colours[1] = colour2;
            line_game.colours[2] = colour3;
            line_game.colours[3] = colour4;

            if (are_equal_game_tries(game_try, &line_game)) {
                fclose(file);
                return TRUE;
            }
        }
        line_num++;
    }
    fclose(file);
    return FALSE;
}

int has_won(const int *player_try_res) {
    return (player_try_res[0] == 4);
}

int directory_exists(const char *filepath) {
    struct stat st;
    return (stat(filepath, &st) == 0 && S_ISDIR(st.st_mode));
}

int are_equal_game_tries(const GameTry *input, const GameTry *line) {
    for (int i = 0; i < 4; i++) {
        if (input->colours[i] != line->colours[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

/*
    GENERAL auxiliar functions
*/

void write_line(const char *file_path, const char *message) {

    FILE *file = fopen(file_path, "a");
    if (file == NULL) {
        perror("Erro ao abrir o ficheiro");
        return;
    }
    fprintf(file, "%s", message);
    fflush(file);
    fclose(file);
}

void write_game_line(const char *PLID, const char *message) {
    
    write_line(get_game_folder_path(PLID), message);
}

char* build_end_game_response(const char* PLID, const char* Command, const char* Status) {
    GameTry *game_solution = extract_game_colour(PLID);

    size_t response_len = COMMAND_LEN + 1 + RESPONSE_LEN + 4*COLOR_LEN*2 + 1 + 1;
    char *response = malloc(sizeof(char) * response_len);
    snprintf(response, response_len, "%s %s %c %c %c %c\n", Command, Status, 
                                                   game_solution->colours[0],
                                                   game_solution->colours[1],
                                                   game_solution->colours[2],
                                                   game_solution->colours[3]);

    return response;
}

char* get_completed_game_name(char const type) {
    time_t now = time(NULL);
    struct tm *current_time = gmtime(&now);

    char *message = malloc(sizeof(char) * GAME_COMPLETED_FILE_NAME_LEN + 1);
    snprintf(message, GAME_COMPLETED_FILE_NAME_LEN + 1, "%4d%02d%02d_%02d%02d%02d_%c",
                                        current_time->tm_year + 1900,
                                        current_time->tm_mon + 1,
                                        current_time->tm_mday,
                                        current_time->tm_hour,
                                        current_time->tm_min,
                                        current_time->tm_sec,
                                        type);

    return message;
}

char* extract_game_info(const char *PLID, const char arg_type) {

    char *file_path = get_game_folder_path(PLID);

    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Error openning the file");
        return NULL;
    }

    char line[GAME_INFO_MAX_LEN + 1];
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
            char *res = malloc(strlen(token) + 1);
            if (res != NULL) {
                strcpy(res, token);
            }
            fclose(file);
            return res; // Retorna a string alocada dinamicamente
        }
        token = strtok(NULL, " ");
        i++;
    }

    fclose(file);
    return NULL;
}

GameTry* extract_game_colour(const char *PLID) {

    GameTry *game_try = malloc(sizeof(GameTry));
    char *colours = extract_game_info(PLID, ARG_SOLUTION);
    strncpy(game_try->colours, colours, sizeof(game_try->colours));

    return game_try;
}

int get_elapsed_time(const char *PLID) {
    return (int)time(NULL) - atoi(extract_game_info(PLID, ARG_ELAPSED_TIME));
}

int get_data_size(FILE *file) {
    int first_line_size = get_line_size(file);
    fseek(file, 0L, SEEK_END);
    int file_size = ftell(file);
    fseek(file, first_line_size, SEEK_SET);
    
    return (file_size - first_line_size);
}

int get_line_size(FILE *file) {
    char c;
    int res = 0;

    while ((c = fgetc(file)) != EOF) {
        if (c == '\n') {
            res++;
            break;
        }
        res++;
    }

    return res;
}


/*
    START and DEBUG auxiliar functions
*/

int start_game(const char *PLID, const char *time, char mode, GameTry *game_solution) {

    /* Generate a new solution */
    if (mode == 'P') generate_solution(game_solution);

    /* Create a new file and write the first line infos */
    create_game_log(PLID, game_solution, time, mode);

    /* Create PLID game directory if not created */
    char *directoryPath = get_player_folder_path(PLID);
    if (!directory_exists(directoryPath)) create_directory(directoryPath);

    return OK;
}

void generate_solution(GameTry* game_solution) {
    char colors[] = {'R', 'G', 'B', 'Y', 'O', 'P'};
    int len_colors = sizeof(colors) / sizeof(colors[0]);
    int len_solution = sizeof(game_solution->colours) / sizeof(game_solution->colours[0]);

    srand((unsigned) time(NULL));

    for (int i = 0; i < len_solution; i++) {
        int random_index = rand() % len_colors;
        game_solution->colours[i] = colors[random_index];
    }
}

void create_game_log(const char *PLID, GameTry *game_solution, const char *time_value, const char mode) {

    time_t now = time(NULL);
    if (now == -1) {
        perror("Error obtaining the time.\n");
        return;
    }

    struct tm *current_time = gmtime(&now);
    if (current_time == NULL) {
        perror("Error converting the time.\n");
        return;
    }

    // Creates the Date string
    char timestr[GAME_INFO_DATE_LEN + 1]; // + '\0'
    snprintf(timestr, sizeof(timestr), "%4d-%02d-%02d %02d:%02d:%02d",
                                        current_time->tm_year + 1900,
                                        current_time->tm_mon + 1,
                                        current_time->tm_mday,
                                        current_time->tm_hour,
                                        current_time->tm_min,
                                        current_time->tm_sec);

    // Creates the first line complete string
    char fullstr[GAME_INFO_MAX_LEN + 1]; // + '\0'
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


/*
    TRY and QUIT auxiliar functions
*/

int* make_try(const char *PLID, const GameTry player_try) {
    int *player_try_res = get_try_results(PLID, player_try);
    write_try(PLID, player_try, player_try_res);

    return player_try_res;
}

int* get_try_results(const char *PLID, GameTry player_try) {

    int *res = malloc(sizeof(int) * 2);
    GameTry game_solution;

    strncpy(game_solution.colours, extract_game_info(PLID, ARG_SOLUTION), sizeof(game_solution.colours));

    int i, j;

    res[0] = 0; // Correct position and color
    res[1] = 0; // Correct color but wrong position

    char wrong_solutions[4];    // Solution colors that were not guessed
    char wrong_tries[4];        // Try colors that were wrong
    int wrongs = 0;             // Number of wrong guesses

    // First step: Check for exact matches (correct color and position)
    for (i = 0; i < 4; i++) {
        if (game_solution.colours[i] == player_try.colours[i]) {
            res[0]++;
        } else {
            wrong_solutions[wrongs] = game_solution.colours[i];
            wrong_tries[wrongs] = player_try.colours[i];
            wrongs++;
        }
    }

    // Second step: Find correct colors in wrong positions
    for (i = 0; i < wrongs; i++) {
        for (j = 0; j < wrongs; j++) {
            if (wrong_tries[i] == wrong_solutions[j]) {
                res[1]++;
                wrong_solutions[j] = 'N'; // No color
                break;
            }
        }
    }
    
    return res;
}

void write_try(const char *PLID, GameTry game_try, int *player_try_res) {

    int elapsed_time = get_elapsed_time(PLID);

    size_t message_len = strlen("T:") + 1 + 4*COLOR_LEN + 2*TRIAL_MAX_LEN*2 + 1 + ELAPSED_TIME_LEN + 2;
    char message[message_len];

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

int end_game(const char *PLID, const char end_game_type, const int number_tries) {
    if (end_game_type == END_WIN) create_score_file(PLID, number_tries);
    
    relocate_completed_game(PLID, end_game_type);

    return 0;
}

int relocate_completed_game(const char *PLID, const char endGameType) {
    char *destinationDirectoryPath = get_player_folder_path(PLID);
    char *sourceFilePath = get_game_folder_path(PLID);
    char *destinationFileName = get_completed_game_name(endGameType);

    time_t now = time(NULL);
    struct tm *current_time = gmtime(&now);

    size_t destination_path_length = strlen(destinationDirectoryPath) + 1 + strlen(destinationFileName) + 1;
    char *destination_path = malloc(destination_path_length);

    // Buils the path to the new file
    snprintf(destination_path, destination_path_length, "%s/%s", destinationDirectoryPath, destinationFileName);

    size_t message_len = YEAR_LEN + 1 + MONTH_LEN + 1 + DAY_LEN + 1 + HOUR_LEN + 1 +
                         MINUTES_LEN + 1 + SECONDS_LEN + 1 + ELAPSED_TIME_LEN + 2;
    char *message = malloc(sizeof(char) * message_len);

    // Creates the end game string
    snprintf(message, message_len, "%4d-%02d-%02d %02d:%02d:%02d %d\n",
                                        current_time->tm_year + 1900,
                                        current_time->tm_mon + 1,
                                        current_time->tm_mday,
                                        current_time->tm_hour,
                                        current_time->tm_min,
                                        current_time->tm_sec,
                                        get_elapsed_time(PLID));
    
    write_game_line(PLID, message);

    // Moves the file
    if (rename(sourceFilePath, destination_path) == 0) {
        return OK;
    } else {
        perror("Error relocating the file!");
        return ERROR;
    }
}

int create_score_file(const char *PLID, const int number_tries) {

    time_t now = time(NULL);
    struct tm *current_time = gmtime(&now);

    char *mode = extract_game_info(PLID, ARG_MODE);
    GameTry *solution = extract_game_colour(PLID);
    int score = calculate_score(PLID, 1);

    size_t file_path_len = strlen("./SCORES/") + SCORE_MAX_LEN + 1 + PLID_DIGITS +
                           DAY_LEN + MONTH_LEN + YEAR_LEN + 1 + HOUR_LEN + MINUTES_LEN +
                           SECONDS_LEN + strlen(".txt") + 1;
    char *file_path = malloc(sizeof(char) * file_path_len);

    // Builds the path string
    snprintf(file_path, file_path_len, "./SCORES/%d_%s_%02d%02d%4d_%02d%02d%02d.txt",
                                        score,
                                        PLID,
                                        current_time->tm_mday,
                                        current_time->tm_mon + 1,
                                        current_time->tm_year + 1900,
                                        current_time->tm_hour,
                                        current_time->tm_min,
                                        current_time->tm_sec);

    if (strcmp("P", mode) == 0) {
        mode = "PLAY";
    } else if (strcmp("D", mode) == 0) {
        mode = "DEBUG";
    }
    
    // Debug is the largest mode string
    size_t message_len = SCORE_MAX_LEN + 1 + PLID_DIGITS + 1 + 4*COLOR_LEN + 1 + TRIAL_MAX_LEN + 1 + strlen("DEBUG") + 1;
    char *message = malloc(sizeof(char) * message_len);
    snprintf(message, message_len, "%d %s %c%c%c%c %d %s", score, PLID, solution->colours[0],
                                                               solution->colours[1],
                                                               solution->colours[2],
                                                               solution->colours[3],
                                                               number_tries,
                                                               mode);

    FILE *file = fopen(file_path, "a");

    fprintf(file, "%s", message);
    fflush(file);
    fclose(file);
    free(solution);
    free(message);

    return 0;
}

int calculate_score(const char *PLID, const int turnsPlayed) {


    float elapsed_time = get_elapsed_time(PLID);
    float max_time = (float)atoi(extract_game_info(PLID, ARG_MAXTIME));

    // Calcule o score
    float score = 100.0f * (((float)MAX_ALLOWED_PLAYS - turnsPlayed) / (float)MAX_ALLOWED_PLAYS) * 
                            (1.0f - (elapsed_time / (float)MAX_ALLOWED_TIME)) * 
                            ((float)MAX_ALLOWED_TIME / max_time);

    // Arredonde para 2 casas decimais
    return round(score);
}


/*
    SHOW_TRIALS and SCOREBOARD auxiliar functions
*/

// FUNCTIONS GIVEN BY THE UC ---------------------------------------------------

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

// -----------------------------------------------------------------------------

void execute_show_trials(const int client_fd, const char* filepath, char* PLID) {

    FILE* file = fopen(filepath, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    // Gets the data_size and adjust the file pointer to the start of the second line
    int file_data_size = get_data_size(file);

    int total_size = 0;

    char line[GAME_INFO_MAX_LEN];
    char filedata[MAX_FSIZE];
    // int n_try = 1;
    while (fgets(line, sizeof(line), file)) {
        // Get only necessary data
        // char c1, c2, c3, c4;
        // int nB, nW;
        // sscanf(line, "T: %c%c%c%c %d %d", &c1, &c2, &c3, &c4, &nB, &nW);

        // char to_send[GAME_INFO_MAX_LEN];
        // sprintf(to_send, "T%d: %c %c %c %c %d %d\n", n_try, c1, c2, c3, c4, nB, nW);

        size_t len = strlen(line);
        memcpy(filedata + total_size, line, len);
        total_size += len;

        // n_try++;
    }

    // If game is active, send remaining time
    if (PLID != NULL) {
        // Get remaining time
        char *time_start_char = extract_game_info(PLID, ARG_ELAPSED_TIME);
        char *time_max_char = extract_game_info(PLID, ARG_MAXTIME);
        time_t time_start = (time_t)strtol(time_start_char, NULL, 10);
        time_t time_max = (time_t)strtol(time_max_char, NULL, 10);
        time_t time_limit = time_start + time_max;

        time_t remaining_time = time_limit - time(NULL);

        // Add remaining time at the end of the file
        sprintf(line, "Remaining time: %ld seconds\n", remaining_time);
        size_t len = strlen(line);
        memcpy(filedata + total_size, line, len);
        total_size += len;
    }

    filedata[total_size] = '\0';
    

    size_t message_len = COMMAND_LEN + 1 + RESPONSE_LEN + 1 + MAX_FNAME + 1 + MAX_FILESIZE_DIGITS + 1 + file_data_size + 2;
    char message[message_len];
    sprintf(message, "RST ACT MUDAR %d %s", file_data_size, filedata);
    // printf("%s\n", message);

    tcp_write(client_fd, message);
}