#ifndef AUX_CLIENT_H
#define AUX_CLIENT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>

#include "constants.h"
#include "utils.h"

/*
* UDP related functions
*/
ssize_t send_receive_udp(int udp_fd, struct addrinfo *udp_res, const char *msg, size_t msg_len, char *response, size_t resp_len);

/*
* TCP related functions
*/
int create_tcp_socket(int *tcp_fd);
int connect_to_server(int tcp_fd, struct addrinfo *tcp_res);
int send_message(int tcp_fd, char *msg);
int receive_response(int tcp_fd, char **response);
void show_content(char *Fdata);
int store_file_local(char *Fname, char *Fdata);
#endif