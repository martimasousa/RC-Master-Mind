#ifndef CLIENT_CORE_H
#define CLIENT_CORE_H

#include "constants.h"

int start_function(int udp_fd, struct addrinfo *udp_res, char *cmd);
int try_function(int udp_fd, struct addrinfo *udp_res, char *cmd, int PLID, int nT);
int exit_quit_function(int udp_fd, struct addrinfo *udp_res, char *cmd, int PLID, int game_context);
int debug_function(int udp_fd, struct addrinfo *udp_res, char *cmd);

int show_trials_function(struct addrinfo *tcp_res, char *cmd, int PLID);
int scoreboard_function(struct addrinfo *tcp_res, char *cmd);

#endif