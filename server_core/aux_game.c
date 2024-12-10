#include "utils.h"
#include "game_core.h"
#include "constants.h"
#include "aux_game.h"
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <stdlib.h>

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
    return (trial_number - '0' > MAX_TRIALS);
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
    GameTry *game_solution = malloc(sizeof(GameTry));
    
    extract_game_colour(PLID, game_solution);

    size_t response_len = COMMAND_LEN + 1 + RESPONSE_LEN + 4*COLOR_LEN*2 + 1 + 1;
    char *response = malloc(sizeof(char) * response_len);
    snprintf(response, response_len, "%s %s %c %c %c %c\n", Command, Status, 
                                                   game_solution->colours[0],
                                                   game_solution->colours[1],
                                                   game_solution->colours[2],
                                                   game_solution->colours[3]);

    return response;
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
    size_t timestr_len = YEAR_LEN + 1 + MONTH_LEN + 1 + DAY_LEN + 1 + HOUR_LEN + 1 + MINUTES_LEN + 1 + SECONDS_LEN;
    char timestr[timestr_len + 1]; // + '\0'
    snprintf(timestr, sizeof(timestr), "%4d-%02d-%02d %02d:%02d:%02d",
                                        current_time->tm_year + 1900,
                                        current_time->tm_mon + 1,
                                        current_time->tm_mday,
                                        current_time->tm_hour,
                                        current_time->tm_min,
                                        current_time->tm_sec);

    // Creates the first line complete string
    size_t fullstr_len = PLID_DIGITS + 1 + MODE_LEN + 4*COLOR_LEN + 1 + TIME_DIGITS + 1 + timestr_len + NOW_TIME_LEN + 1;
    char fullstr[fullstr_len + 1]; // + '\0'
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