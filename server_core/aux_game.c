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
#include <errno.h>    // Para verificar erros com errno
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <stdlib.h>


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