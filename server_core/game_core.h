#ifndef GAME_CORE_H
#define GAME_CORE_H

#include <netinet/in.h>
#include"constants.h"
#include "udp_io.h"

typedef struct GameTry {
    char colours[4];
} GameTry;


/**
 * Compare the player's try with the solution and evaluate nB (correct in both colour and position)
 * and nW (belong to the secret key but are incorrectly positioned).
 */
void check_try(const char *PLID, GameTry player_try, int* res);


void process_sng_command(int udp_fd, struct sockaddr_in *client_addr, const char *command);
void process_try_command(int udp_fd, struct sockaddr_in *client_addr, const char *command);
void process_qut_command(int udp_fd, struct sockaddr_in *client_addr, const char *command);
void process_dbg_command(int udp_fd, struct sockaddr_in *client_addr, const char *command);
void process_command(int udp_fd, struct sockaddr_in *client_addr, const char *command);
int handle_TCP_messages(int client_fd);
//int gameLogic(GameInfo **gameInfo);

#endif
