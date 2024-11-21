#ifndef GAME_CORE_H
#define GAME_CORE_H

#include <netinet/in.h>
#include"constants.h"
#include "udp_io.h"

typedef struct GameTry {
    char colours[4];
} GameTry;

typedef struct GameInfo {
    int udp_fd;
    time_t max_time;
    char PLID[PLID_DIGITS];
    struct sockaddr_in *client_addr;
    int playing;
    GameTry game_solution;

} GameInfo;

/**
 * Compare the player's try with the solution and evaluate nB (correct in both colour and position)
 * and nW (belong to the secret key but are incorrectly positioned).
 */
void check_try(GameTry game_solution, GameTry player_try, int* res);


void process_sng_command(GameInfo *gameInfo, const char* command);
void process_try_command(GameInfo *gameInfo, const char* command);
void process_qut_command(GameInfo *gameInfo, const char* command);
void process_dbg_command(GameInfo *gameInfo, const char* command);
void process_command(GameInfo *gameInfo, const char *command);
int handle_TCP_messages(int client_fd);
//int gameLogic(GameInfo **gameInfo);

#endif
