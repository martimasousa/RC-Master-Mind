#include "constants.h"
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <dirent.h>

#ifndef UTILS_H
#define UTILS_H

// FUNCTIONS GIVEN BY THE UC ---------------------------------------------------

#define CODE_DIGITS 4 + 1   // '\0'
#define SSB_SIZE    10

typedef struct {
    int score[SSB_SIZE];                    // Array to store scores
    char PLID[SSB_SIZE][PLID_DIGITS + 1];   // Array of strings for Player IDs
    char colcode[SSB_SIZE][CODE_DIGITS];    // Array of strings for color codes
    int notries[SSB_SIZE];                  // Array to store number of tries
    char mode[SSB_SIZE];                    // Array to store the mode for each entry
    int nscores;                            // Number of scores currently stored
} SCORELIST;

// -----------------------------------------------------------------------------

/*
    STATUS VERIFICATION auxiliar functions 
*/

// Função para verificar se a string é um número inteiro
int is_integer(const char *str);

// Check if the string is a valid PORT
int is_valid_port(const char *str);

// Check if the string is a valid hostname
int is_valid_hostname(const char *hostname);

// Check if the string is a valid IP
int is_valid_ip(const char *ip);

// Check if the string is a either a valid IP or hostname.
int is_valid_address(const char *input);

// Função para verificar se é uma cor válida
int is_valid_color(char C);

int file_exists(const char *path);

int validate_start_command(const char *cmd, char *PLID_arg, char *max_playtime_arg, const int execution_side);
int validate_try_command(const char *cmd, char *PLID_arg, char *C1, char *C2, char *C3, char *C4, char *nT, const int execution_side);
int validate_quit_command(const char *cmd, char *PLID_arg, const int execution_side, const int context_flag);
int validate_debug_command(const char *cmd, char *PLID_arg, char *max_playtime_arg, char *C1, char *C2, char *C3, char *C4, const int execution_side);
int validate_showtrials_command(const char *cmd, char *PLID, const int execution_side);
int validate_scoreboard_command(const char *cmd, const int execution_side);

/*
    GENERAL auxiliar functions
*/

char *special_inv_function(const char *PLID);

// Generate the path of the file that contains the player's game information
char* get_game_folder_path(const char *PLID);

// Generate the path of the file that contains the player's game history information
char* get_player_folder_path(const char *PLID);

// Creates the directory
int create_directory(const char *directory);

// This function creates or replaces a file named 'Fname' with the content defined as 'Fdata'.
int write_to_file(const char *Fname, const char *Fdata);

// Converts a char into an integer
int convert_char_to_int(const char number);

char *int_to_string(int number);

char *create_string(const char *components[], size_t count);

/*
    MESSAGE sending and receiving functions
*/

/**
 * This function reads content from a file until a certain delimiter. If reaches EOF or the socket
 * is closed (by the other peer), all the characters read until that moment will be returned.
 * @param fd TCP file descriptor to read data from.
 * @param word Word to store read data.
 * @param separator Char representing the character where we should stop reading (not inclusive).
 * @param n_times Number of times we want to ignore the delimiter (word will contain n_times-1 delimiters).
 */
int tcp_read_until_delimiter(int fd, char** word, char separator, int n_times);

// Writes some string to a tcp file descriptor
int tcp_write(int fd, char* to_write);

/**
 * Reads exactly `n` characters from the given file descriptor (fd).
 * 
 * @param fd The file descriptor to read from.
 * @param buffer A buffer where the read data will be stored.
 * @param n The exact number of characters to read.
 * @return 0 if successful, -1 if an error occurs (e.g., EOF or read failure).
 */
int tcp_read(int fd, char **buffer, size_t n);

// Função que recebe uma mensagem UDP e retorna o número de bytes lidos.
// Modifica o buffer com os dados recebidos.
ssize_t recv_udp_message(int udp_fd, char *buffer, size_t buffer_size, struct sockaddr_in *client_addr);

// Função que envia uma resposta UDP para o cliente especificado.
void send_udp_response(int udp_fd, const char *message, struct sockaddr_in *client_addr);

int FindLastGame(const char *PLID, char *fname);

int FindTopScores(SCORELIST *list);

#endif
