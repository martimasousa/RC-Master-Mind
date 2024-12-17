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

// STRUCTURES GIVEN BY THE UC ---------------------------------------------------

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

/**
 * This function checks if the given string is a number (integer).
 * @param str Given string to evaulate.
 * @returns TRUE if the given string is a number and FALSE otherwise.
 */
int is_integer(const char *str);

/**
 * This function checks if the given string is a valid port number.
 * @param str Given string to evaluate.
 * @returns TRUE if the given string represents a valid port number (0 to 65535), and FALSE otherwise.
 */
int is_valid_port(const char *str);

/**
 * This function checks if the given string is a valid hostname.
 * @param hostname Given string to evaluate.
 * @returns TRUE if the given string is a valid hostname, and FALSE otherwise.
 */
int is_valid_hostname(const char *hostname);

/**
 * This function checks if the given string is a valid IPv4 address.
 * @param ip Given string to evaluate.
 * @returns TRUE if the given string is a valid IPv4 address, and FALSE otherwise.
 */
int is_valid_ip(const char *ip);

/**
 * This function checks if the given string is a valid IP address or hostname.
 * @param input Given string to evaluate.
 * @returns TRUE if the given string is a valid IP address or hostname, and FALSE otherwise.
 */
int is_valid_address(const char *input);

/**
 * This function checks if the given character is a valid color code.
 * @param C Given character to evaluate.
 * @returns TRUE if the given character is one of the valid color codes
 * ('R', 'G', 'B', 'Y', 'O', 'P'), and FALSE otherwise.
 */
int is_valid_color(char C);

/**
 * This function checks if a file exists at the given path.
 * @param path The path of the file to check.
 * @returns TRUE if the file exists, and FALSE otherwise.
 */
int file_exists(const char *path);

/**
 * This function validates a command to start a game, checking for correct structure, argument
 * types, and value ranges.
 * @param cmd The command string to validate.
 * @param PLID_arg A pointer to store the validated PLID argument.
 * @param max_playtime_arg A pointer to store the validated max playtime argument.
 * @param execution_side An integer indicating the execution side (SERVER_SIDE or PLAYER_SIDE).
 * @returns OK if the command is valid, and ERROR if the command is malformed or contains invalid
 * arguments.
 */
int validate_start_command(const char *cmd, char *PLID_arg, char *max_playtime_arg, const int execution_side);

/**
 * This function validates a "try" command, checking for correct structure, argument types, and
 * value ranges.
 * @param cmd The command string to validate.
 * @param PLID_arg A pointer to store the validated PLID argument.
 * @param C1 A pointer to store the validated first color argument.
 * @param C2 A pointer to store the validated second color argument.
 * @param C3 A pointer to store the validated third color argument.
 * @param C4 A pointer to store the validated fourth color argument.
 * @param nT A pointer to store the validated nT argument (only for the server side).
 * @param execution_side An integer indicating the execution side (SERVER_SIDE or PLAYER_SIDE).
 * @returns OK if the command is valid, and ERROR if the command is malformed or contains invalid
 * arguments.
 */
int validate_try_command(const char *cmd, char *PLID_arg, char *C1, char *C2, char *C3, char *C4, char *nT, const int execution_side);

/**
 * This function validates a "quit" or "exit" command, checking for correct structure, argument
 * types, and value ranges.
 * @param cmd The command string to validate.
 * @param PLID_arg A pointer to store the validated PLID argument (only for the server side).
 * @param execution_side An integer indicating the execution side (SERVER_SIDE or PLAYER_SIDE).
 * @param context_flag An integer flag to determine the expected command for the player side
 * (QUIT_CONTEXT or other).
 * @returns OK if the command is valid, and ERROR if the command is malformed or contains invalid
 * arguments.
 */
int validate_quit_command(const char *cmd, char *PLID_arg, const int execution_side, const int context_flag);

/**
 * This function validates a "debug" command, checking for correct structure, argument types, and
 * value ranges.
 * @param cmd The command string to validate.
 * @param PLID_arg A pointer to store the validated PLID argument.
 * @param max_playtime_arg A pointer to store the validated max playtime argument.
 * @param C1 A pointer to store the validated first color argument.
 * @param C2 A pointer to store the validated second color argument.
 * @param C3 A pointer to store the validated third color argument.
 * @param C4 A pointer to store the validated fourth color argument.
 * @param execution_side An integer indicating the execution side (SERVER_SIDE or PLAYER_SIDE).
 * @returns OK if the command is valid, and ERROR if the command is malformed or contains invalid
 * arguments.
 */
int validate_debug_command(const char *cmd, char *PLID_arg, char *max_playtime_arg, char *C1, char *C2, char *C3, char *C4, const int execution_side);

/**
 * This function validates a "show_trials" or "STR" command, checking for correct structure,
 * argument types, and value ranges.
 * @param cmd The command string to validate.
 * @param PLID A pointer to store the validated PLID argument (for SERVER_SIDE).
 * @param execution_side An integer indicating the execution side (SERVER_SIDE or PLAYER_SIDE).
 * @returns OK if the command is valid, and ERROR if the command is malformed or contains invalid
 * arguments.
 */
int validate_showtrials_command(const char *cmd, char *PLID, const int execution_side);

/**
 * This function validates a "scoreboard" or "SSB" command, checking for correct structure and
 * argument format.
 * @param cmd The command string to validate.
 * @param execution_side An integer indicating the execution side (SERVER_SIDE or PLAYER_SIDE).
 * @returns OK if the command is valid, and ERROR if the command is malformed or contains extra
 * arguments.
 */
int validate_scoreboard_command(const char *cmd, const int execution_side);


/*
    GENERAL auxiliar functions
*/

/**
 * This function retrieves the last used game file for a given player (PLID).
 * It first attempts to return the path to the active game. If no active game is found, it attempts
 * to find the last player's game file.
 * @param PLID The player ID to find the last used game file for.
 * @returns A string containing the file path of the last used game file.
 */
char *get_last_used_game_file(const char *PLID);

/**
 * This function generates the file path to a specific player's game file based on their PLID.
 * The generated path follows the format: "./GAMES/GAME_<PLID>", where <PLID> is the player's ID.
 * Note: If there is no active game for this player, the generated path will be invalid!
 * @param PLID The player ID used to generate the game file path.
 * @returns A string containing the file path to the player's game file.
 */
char* get_game_file_path(const char *PLID);

/**
 * This function generates the file path to a specific player's folder based on their PLID.
 * The generated path follows the format: "./GAMES/<PLID>", where <PLID> is the player's ID.
 * @param PLID The player ID used to generate the player folder path.
 * @returns A string containing the file path to the player's folder.
 */
char* get_player_folder_path(const char *PLID);

/**
 * This function attempts to create a new directory with the specified path.
 * If the directory already exists, it does not return an error.
 * @param directory The path of the directory to create.
 * @returns 0 if the directory was created successfully or already exists.
 *          -1 if there was an error creating the directory (other than it already existing).
 */
int create_directory(const char *directory);

/**
 * This function writes data to a file, overwriting the file if it already exists.
 * @param Fname The name of the file to write to.
 * @param Fdata The data to write into the file.
 * @returns 0 if the operation was successful, or -1 if there was an error (e.g., opening, writing,
 * or closing the file).
 */
int write_to_file(const char *Fname, const char *Fdata);

/**
 * This function converts a character representing a digit to its integer value.
 * @param number The character to convert, which should represent a digit.
 * @returns The integer value of the character.
 */
int convert_char_to_int(const char number);

/**
 * This function converts an integer to a dynamically allocated string representation.
 * @param number The integer to convert.
 * @returns A pointer to the dynamically allocated string representing the integer. The caller is
 * responsible for freeing the memory.
 */
char *int_to_string(int number);

/**
 * This function concatenates multiple strings into a single dynamically allocated string.
 * @param components An array of strings to concatenate.
 * @param count The number of strings in the components array.
 * @returns A pointer to the dynamically allocated string representing the concatenation of all
 * input strings. The caller is responsible for freeing the memory.
 */
char *create_string(const char *components[], size_t count);



/*
    MESSAGE sending and receiving functions
*/

/**
 * This function reads from a TCP socket until a specified delimiter is encountered a given number
 * of times.
 * @param fd The file descriptor of the TCP socket to read from.
 * @param word A pointer to a string where the read data will be stored.
 * @param separator The character delimiter that marks the end of a segment to read.
 * @param n_times The number of times the separator must appear before stopping the read operation.
 * @returns 0 if the operation was successful, 1 if an error occurred during reading or memory
 * allocation.
 */
int tcp_read_until_delimiter(int fd, char** word, char separator, int n_times);

/**
 * This function writes data to a TCP socket in chunks until the entire string is sent.
 * @param fd The file descriptor of the TCP socket to write to.
 * @param to_write The string to be written to the TCP socket.
 * @returns OK if the operation was successful, ERROR if an error occurred during the write operation.
 */
int tcp_write(int fd, char* to_write);

/**
 * This function reads `n` bytes from a TCP socket into a dynamically allocated buffer.
 * @param fd The file descriptor of the TCP socket from which data will be read.
 * @param buffer A pointer to a char pointer that will hold the address of the allocated buffer
 * containing the data.
 * @param n The number of bytes to be read from the socket.
 * @return Returns OK (0) on success, ERROR (-1) on failure. If an error occurs, the buffer is freed
 * and set to NULL.
 */
int tcp_read(int fd, char **buffer, size_t n);

/**
 * This function receives a message over UDP from a given socket and stores it in the provided
 * buffer.
 * @param udp_fd The file descriptor of the UDP socket to receive from.
 * @param buffer The buffer where the received message will be stored.
 * @param buffer_size The size of the buffer.
 * @param client_addr The sockaddr_in structure to store the client's address.
 * @returns The number of bytes received if successful, -1 if there was an error or no data was
 * received.
 */
ssize_t recv_udp_message(int udp_fd, char *buffer, size_t buffer_size, struct sockaddr_in *client_addr);

/**
 * This function sends a UDP response message to a client.
 * @param udp_fd The file descriptor of the UDP socket to send the message from.
 * @param message The message to be sent to the client.
 * @param client_addr The sockaddr_in structure containing the client's address to which the
 * message will be sent.
 */
void send_udp_response(int udp_fd, const char *message, struct sockaddr_in *client_addr);


// FUNCTIONS GIVEN BY THE UC ---------------------------------------------------
int FindLastGame(const char *PLID, char *fname);

int FindTopScores(SCORELIST *list);
// -----------------------------------------------------------------------------

#endif
