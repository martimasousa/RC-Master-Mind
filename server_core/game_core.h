#ifndef GAME_CORE_H
#define GAME_CORE_H

#include "constants.h"
#include "aux_game.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include "constants.h"

void process_command_udp(int udp_fd, struct sockaddr_in *client_addr, const char *command);
void process_command_tcp(int client_fd, const char *command);

void process_sng_command(int udp_fd, struct sockaddr_in *client_addr, const char *command);
void process_try_command(int udp_fd, struct sockaddr_in *client_addr, const char *command);
void process_qut_command(int udp_fd, struct sockaddr_in *client_addr, const char *command);
void process_dbg_command(int udp_fd, struct sockaddr_in *client_addr, const char *command);
void process_udp_uknown_command(int udp_fd, struct sockaddr_in *client_addr);


void process_str_command(int client_fd, const char *command);
void process_ssb_command(int client_fd, const char *command);
void process_tcp_uknown_command(int client_fd);

#endif
