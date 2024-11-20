#include "game_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void check_try(GameTry game_solution, GameTry player_try, int* res) {
    int i, j;

    res[0] = 0; // Correct position and color
    res[1] = 0; // Correct color but wrong position

    char wrong_solutions[4];    // Solution colors that were not guessed
    char wrong_tries[4];        // Try colors that were wrong
    int wrongs = 0;             // Number of wrong guesses

    // First pass: Check for exact matches (correct color and position)
    for (i = 0; i < 4; i++) {
        if (game_solution.colours[i] == player_try.colours[i]) {
            res[0]++;
        } else {
            wrong_solutions[wrongs] = game_solution.colours[i];
            wrong_tries[wrongs] = player_try.colours[i];
            wrongs++;
        }
    }

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

void process_sng_command(GameInfo **gameInfo, const char* command) {
    char time[TIME_DIGITS];

    char *response = "RSG OK";

    sscanf(command, "SNG %s %s", (*gameInfo)->PLID, time);

    send_udp_response((*gameInfo)->udp_fd, response, (*gameInfo)->client_addr);

    printf("PLID: %s\n", (*gameInfo)->PLID);
    printf("time: %s\n", time);

    // TODO: Add Game Logic!
}

void process_try_command(GameInfo **gameInfo, const char *command) {

    GameTry player_try;
    char PLID[PLID_DIGITS], nt;

    int *check_try_counter = malloc(2 * sizeof(int));
    check_try_counter[0] = 0;
    check_try_counter[1] = 0;

    char response[MAX_PLAYER_COMMAND];

    sscanf(command, "TRY %s %c %c %c %c %c", PLID, &player_try.colours[0], &player_try.colours[1], 
                                                   &player_try.colours[2], &player_try.colours[3], &nt);

    printf("Player Try: %c %c %c %c\n", player_try.colours[0], player_try.colours[1], player_try.colours[2], player_try.colours[3]);
    printf("Solution: %c %c %c %c\n", (*gameInfo)->game_solution.colours[0], (*gameInfo)->game_solution.colours[1], (*gameInfo)->game_solution.colours[2], (*gameInfo)->game_solution.colours[3]);

    check_try((*gameInfo)->game_solution, player_try, check_try_counter);

    snprintf(response, sizeof(response), "RTR OK %d %d %c", check_try_counter[0], check_try_counter[1], nt);
    
    send_udp_response((*gameInfo)->udp_fd, response, (*gameInfo)->client_addr);


    /*
    printf("PLID: %s\n", PLID);
    printf("Colors: %c %c %c %c\n", player_try.colours[0], player_try.colours[1], 
                                    player_try.colours[2], player_try.colours[3]);
    printf("nt: %c\n", nt);
    */

    // TODO: Add Game Logic!
}

void process_qut_command(GameInfo **gameInfo, const char *command) {
    char PLID[PLID_DIGITS];
    char *response = "RQT OK";

    sscanf(command, "QUT %s", PLID);
    
    send_udp_response((*gameInfo)->udp_fd, response, (*gameInfo)->client_addr);

    (*gameInfo)->playing = FALSE;
    printf("PLID: %s\n", PLID);

    // TODO: Add Game Logic!
}

void process_dbg_command(GameInfo **gameInfo, const char *command) {

    //GameTry *game_solution = &gameInfo->game_solution;
    char time[TIME_DIGITS];
    char *response = "RDB OK";

    printf("%s\n", command);

    // Informação não está a ser bem guardada na estrutura
    sscanf(command, "DBG  %s %s %c %c %c %c", (*gameInfo)->PLID, time,
                                              &((*gameInfo)->game_solution.colours[0]),
                                              &((*gameInfo)->game_solution.colours[1]),
                                              &((*gameInfo)->game_solution.colours[2]),
                                              &((*gameInfo)->game_solution.colours[3]));
    
    send_udp_response((*gameInfo)->udp_fd, response, (*gameInfo)->client_addr);

    printf("PLID: %s\n", (*gameInfo)->PLID);
    printf("time: %s\n", time);
    /*
    printf("Colors: %c %c %c %c\n", game_solution->colours[0],
                                    game_solution->colours[1],
                                    game_solution->colours[2],
                                    game_solution->colours[3]);
    */


    // TODO: Add Game Logic!
}

void process_command(GameInfo *gameInfo, const char *command) {

    char type[3];
    sscanf(command, "%s", type);

    if (strcmp(SNG_CMD, type) == 0) {
        process_sng_command(&gameInfo, command);
        return;

    } else if (strcmp(TRY_CMD, type) == 0) {
        process_try_command(&gameInfo, command);
        return;

    } else if (strcmp(QUT_CMD, type) == 0) {
        process_qut_command(&gameInfo, command);
        return;

    } else if (strcmp(DBG_CMD, type) == 0) {
        process_dbg_command(&gameInfo, command);
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
