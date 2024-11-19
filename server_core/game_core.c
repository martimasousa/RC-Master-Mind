#include "game_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void process_sng_command(GameInfo *gameInfo, const char* command) {
    char time[TIME_DIGITS];

    char *response = "RSG OK";

    sscanf(command, "SNG %s %s", gameInfo->PLID, time);

    send_udp_response(gameInfo->udp_fd, response, gameInfo->client_addr);

    printf("PLID: %s\n", gameInfo->PLID);
    printf("time: %s\n", time);

    // TODO: Add Game Logic!
}

void process_try_command(GameInfo *gameInfo, const char *command) {
    char PLID[PLID_DIGITS], C1, C2, C3, C4, nt;
    char *response = "RTR OK 1 1 1";

    sscanf(command, "TRY %s %c %c %c %c %c", PLID, &C1, &C2, &C3, &C4, &nt);
    
    send_udp_response(gameInfo->udp_fd, response, gameInfo->client_addr);

    printf("PLID: %s\n", PLID);
    printf("Colors: %c %c %c %c\n", C1, C2, C3, C4);
    printf("nt: %c\n", nt);

    // TODO: Add Game Logic!
}

void process_qut_command(GameInfo *gameInfo, const char *command) {
    char PLID[PLID_DIGITS];
    char *response = "RQT OK";

    sscanf(command, "QUT %s", PLID);
    
    send_udp_response(gameInfo->udp_fd, response, gameInfo->client_addr);

    gameInfo->playing = FALSE;
    printf("PLID: %s\n", PLID);

    // TODO: Add Game Logic!
}

void process_dbg_command(GameInfo *gameInfo, const char *command) {

    GameSolution *game_solution = &gameInfo->game_solution;
    char time[TIME_DIGITS];
    char *response = "RDB OK";

    printf("%s\n", command);
    sscanf(command, "DBG  %s %s %c %c %c %c", gameInfo->PLID, time, 
                                              &game_solution->C1,
                                              &game_solution->C2,
                                              &game_solution->C3,
                                              &game_solution->C4);
    
    send_udp_response(gameInfo->udp_fd, response, gameInfo->client_addr);

    printf("PLID: %s\n", gameInfo->PLID);
    printf("time: %s\n", time);
    printf("Colors: %c %c %c %c\n", game_solution->C1,
                                    game_solution->C2,
                                    game_solution->C3,
                                    game_solution->C4);


    // TODO: Add Game Logic!
}

void process_command(GameInfo *gameInfo, const char *command) {

    char type[3];
    sscanf(command, "%s", type);

    if (strcmp(SNG_CMD, type) == 0) {
        process_sng_command(gameInfo, command);
        return;

    } else if (strcmp(TRY_CMD, type) == 0) {
        process_try_command(gameInfo, command);
        return;

    } else if (strcmp(QUT_CMD, type) == 0) {
        process_qut_command(gameInfo, command);
        return;

    } else if (strcmp(DBG_CMD, type) == 0) {
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

int gameLogic(GameInfo *gameInfo) {

    gameInfo->playing = TRUE;
    gameInfo->client_addr = malloc(sizeof(struct sockaddr_in));
    char *command = malloc(sizeof(char) * BUFFER_SIZE);
    char *type = malloc(sizeof(char) * 3);

    while (gameInfo->playing) {
        if (recv_udp_message(gameInfo->udp_fd, command, BUFFER_SIZE, gameInfo->client_addr) == -1) {
            perror("Error reading command");
            exit(1);
        }
        process_command(gameInfo, command);
    }

    free(command);
    free(type);
    free(gameInfo->client_addr);
    
    return 1;
}
