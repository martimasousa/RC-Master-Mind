#ifndef CLIENT_CORE_H
#define CLIENT_CORE_H

#include "constants.h"

int start_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND]);
int try_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND], int PLID, int nT);

#endif