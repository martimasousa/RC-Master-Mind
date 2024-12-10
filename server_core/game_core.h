#ifndef GAME_CORE_H
#define GAME_CORE_H

#include <netinet/in.h>
#include"constants.h"

typedef struct GameTry {
    char colours[4];
} GameTry;


void generate_solution(GameTry* game_solution);

void process_sng_command(int udp_fd, struct sockaddr_in *client_addr, const char *command);
void process_try_command(int udp_fd, struct sockaddr_in *client_addr, const char *command);
void process_qut_command(int udp_fd, struct sockaddr_in *client_addr, const char *command);
void process_dbg_command(int udp_fd, struct sockaddr_in *client_addr, const char *command);
void process_command(int udp_fd, struct sockaddr_in *client_addr, const char *command);
void handle_TCP_messages(int client_fd, const char *command);

void process_str_command(int client_fd, const char *command);
void process_ssb_command(int client_fd, const char *command);
//int gameLogic(GameInfo **gameInfo);

#endif
