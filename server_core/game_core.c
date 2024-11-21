#include "game_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "utils.h"

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


void check_try(GameTry game_solution, GameTry player_try, int* res) {
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
}


void process_sng_command(GameInfo *gameInfo, const char* command) {

    char time[TIME_DIGITS];
    char PLID[PLID_DIGITS];

    sscanf(command, "SNG %s %s", PLID, time);
    // TODO: Verify arguments. In case they are wrongly formed return "RSG ERR\n"

    /* If there is an ongoing game, respond with "RSG NOK" */
    if (search_file(PLID)) {
        char *response = "RSG NOK";
        send_udp_response(gameInfo->udp_fd, response, gameInfo->client_addr);
        return;
    }

    /* If not, generate a new solution, create a new file and write the first line infos */
    GameTry *game_solution = malloc(sizeof(GameTry));
    generate_solution(game_solution);

    create_game_log_timestamp(PLID, game_solution, time,'P');

    char *response = "RSG OK";
    send_udp_response(gameInfo->udp_fd, response, gameInfo->client_addr);
}


void process_try_command(GameInfo *gameInfo, const char *command) {

    GameTry player_try;
    char PLID[PLID_DIGITS], nt;

    int *check_try_counter = malloc(2 * sizeof(int));
    check_try_counter[0] = 0;
    check_try_counter[1] = 0;


    sscanf(command, "TRY %s %c %c %c %c %c", PLID, &player_try.colours[0], &player_try.colours[1], 
                                                   &player_try.colours[2], &player_try.colours[3], &nt);


    // TODO: Verify Sintaxe!

    if (!inTime(PLID)) {
        //TODO: OUT OF TIME!
    }

    if (extract_colors_from_file(PLID, &player_try)) {
        //TODO: Duplicated!
    }



    // check_try(gameInfo->game_solution, player_try, check_try_counter);

    // snprintf(response, sizeof(response), "RTR OK %c %d %d", nt, check_try_counter[0], check_try_counter[1]);
    
    // send_udp_response(gameInfo->udp_fd, response, gameInfo->client_addr);


    // TODO: Add Game Logic!
}

void process_qut_command(GameInfo *gameInfo, const char *command) {

    char PLID[PLID_DIGITS];
    
    sscanf(command, "QUT %s", PLID);
    // TODO: Verify arguments. In case they are wrongly formed return "RQT ERR\n"

    /* If there is an ongoing game, respond with "RQT NOK" */
    if (!search_file(PLID)) {
        char *response = "RQT NOK";
        send_udp_response(gameInfo->udp_fd, response, gameInfo->client_addr);
        return;
    }

    /* Get the solution and send the message */
    GameTry *game_solution = malloc(sizeof(GameTry));
    
    extract_game_colour(PLID, game_solution);

    char response[100];
    snprintf(response, sizeof(response), "RQT OK %c %c %c %c\n", game_solution->colours[0],
                                                                 game_solution->colours[1],
                                                                 game_solution->colours[2],
                                                                 game_solution->colours[3]);

    send_udp_response(gameInfo->udp_fd, response, gameInfo->client_addr);
}

void process_dbg_command(GameInfo *gameInfo, const char *command) {

    char time[TIME_DIGITS];
    char PLID[PLID_DIGITS];
    GameTry *game_solution = malloc(sizeof(GameTry));

    // Informação não está a ser bem guardada na estrutura
    sscanf(command, "DBG  %s %s %c %c %c %c", PLID, time,
                                              &game_solution->colours[0],
                                              &game_solution->colours[1],
                                              &game_solution->colours[2],
                                              &game_solution->colours[3]);

    // TODO: Verify arguments. In case they are wrongly formed return "RSG ERR\n"


    /* If there is an ongoing game, respond with "RSG NOK" */
    if (search_file(PLID)) {
        char *response = "RDB NOK";
        send_udp_response(gameInfo->udp_fd, response, gameInfo->client_addr);
        return;
    }

    create_game_log_timestamp(PLID, game_solution, time,'D');

    char *response = "RDB OK";
    send_udp_response(gameInfo->udp_fd, response, gameInfo->client_addr);
}


void process_command(GameInfo *gameInfo, const char *command) {
    char type[3];
    sscanf(command, "%s", type);

    if (!strcmp(SNG_CMD, type)) {
        process_sng_command(gameInfo, command);
        return;

    } else if (!strcmp(TRY_CMD, type)) {
        process_try_command(gameInfo, command);
        return;

    } else if (!strcmp(QUT_CMD, type)) {
        process_qut_command(gameInfo, command);
        return;

    } else if (!strcmp(DBG_CMD, type)) {
        process_dbg_command(gameInfo, command);
        return;
    } else {
        printf("Error: %s\n", type);
        return;
    }
}


int handle_TCP_messages(int client_fd) { 
    // TODO: Add commands logic
    return 1;
}

/*
int gameLogic(GameInfo **gameInfo) {
    
    (*gameInfo)->playing = TRUE;
    (*gameInfo)->client_addr = malloc(sizeof(struct sockaddr_in));
    char *command = malloc(sizeof(char) * BUFFER_SIZE);
    char *type = malloc(sizeof(char) * 3);

    while ((*gameInfo)->playing) {
        if (recv_udp_message((*gameInfo)->udp_fd, command, BUFFER_SIZE, (*gameInfo)->client_addr) == -1) {
            perror("Error reading command");
            exit(1);
        }
        process_command(gameInfo, command);

        printf("[gameLogic] Solution: %c %c %c %c\n", (*gameInfo)->game_solution.colours[0], (*gameInfo)->game_solution.colours[1], (*gameInfo)->game_solution.colours[2], (*gameInfo)->game_solution.colours[3]);
    }

    free(command);
    free(type);
    free((*gameInfo)->client_addr);
    
    return 1;
}
*/
