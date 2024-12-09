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
    if (!directoryExists(directoryPath)) create_directory(directoryPath);

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