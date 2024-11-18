#ifndef UDP_FUNCTIONS_H
#define UDP_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Função que recebe uma mensagem UDP e retorna o número de bytes lidos.
// Modifica o buffer com os dados recebidos.
ssize_t recv_udp_message(int udp_fd, char *buffer, size_t buffer_size, struct sockaddr_in *client_addr);

// Função que envia uma resposta UDP para o cliente especificado.
void send_udp_response(int udp_fd, const char *message, struct sockaddr_in *client_addr);

#endif // UDP_FUNCTIONS_H
