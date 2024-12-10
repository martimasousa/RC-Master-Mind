#include "constants.h"
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifndef UTILS_H
#define UTILS_H

/*
    STATUS VERIFICATION auxiliar functions 
*/

// Função para verificar se a string é um número inteiro
int is_integer(const char *str);

// Função para verificar se a string é um IP válido
int is_valid_ip(const char *ip);

// Função para verificar se é uma cor válida
int is_valid_color(char C);

int file_exists(const char *path);

/*
    GENERAL auxiliar functions
*/

// Generate the path of the file that contains the player's game information
char* get_game_folder_path(const char *PLID);

// Generate the path of the file that contains the player's game history information
char* get_player_folder_path(const char *PLID);

// Creates the directory
int create_directory(const char *directory);


/*
    MESSAGE sending and receiving functions
*/

// Reads delimited string from the file descriptor.
int tcp_read_until_delimiter(int fd, char** word, char separator);

// Writes some string to a tcp file descriptor
int tcp_write(int fd, char* to_write);

// Função que recebe uma mensagem UDP e retorna o número de bytes lidos.
// Modifica o buffer com os dados recebidos.
ssize_t recv_udp_message(int udp_fd, char *buffer, size_t buffer_size, struct sockaddr_in *client_addr);

// Função que envia uma resposta UDP para o cliente especificado.
void send_udp_response(int udp_fd, const char *message, struct sockaddr_in *client_addr);



#endif
