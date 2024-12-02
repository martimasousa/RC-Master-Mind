#ifndef CLIENT_CORE_H
#define CLIENT_CORE_H

#include "constants.h"

int start_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND]);
int try_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND], int PLID, int nT);
int quit_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND], int PLID);
int exit_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND], int PLID);
int debug_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND]);

int show_trials_function(struct addrinfo *tcp_res, char cmd[MAX_PLAYER_COMMAND], char type[MAX_PLAYER_COMMAND], int PLID);
int scoreboard_function(struct addrinfo *tcp_res, char cmd[MAX_PLAYER_COMMAND], char type[MAX_PLAYER_COMMAND]);

#endif