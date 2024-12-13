#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "constants.h"
#include "client_core.h"
#include "aux_client.h"
#include "utils.h"

/**
 * Handles start command.
 */
int start_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND]) {

    char PLID_arg[PLID_DIGITS + 1];
    char max_playtime_arg[TIME_DIGITS + 1];

    // Validate command syntax
    if (validate_start_command(cmd, PLID_arg, max_playtime_arg, PLAYER_SIDE) == ERROR) {
        return ERROR;
    }

    // Create message to send
    // Example: SNG 106324 100\n\0
    size_t msg_len = COMMAND_LEN + 1 + PLID_DIGITS + 1 + TIME_DIGITS + 1 + 1;
    char msg[msg_len];
    snprintf(msg, msg_len, "SNG %s %s\n", PLID_arg, max_playtime_arg);

    // Send message to server
    ssize_t n = sendto(udp_fd, msg, sizeof(msg), 0, udp_res->ai_addr, udp_res->ai_addrlen);
    if (n == ERROR) {
        fprintf(stderr, "Error while sending message.\n");
        return ERROR;
    }

    // Receive response from server
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    // Receive server response
    // Example: SNG ERR\n\0
    size_t resp_len = COMMAND_LEN + 1 + RESPONSE_LEN + 1 + 1;
    char response[resp_len];
    n = recvfrom(udp_fd, response, sizeof(response), 0, (struct sockaddr*)&addr, &addrlen);
    if (n == ERROR) {
        fprintf(stderr, "Error while receiving message.\n");
        return ERROR;
    }

    // Example: RSG ERR\0
    char response_status[RESPONSE_LEN + 1];
    sscanf(response, "RSG %s\n", response_status);

    int PLID = atoi(PLID_arg);

    // Deal with response
    if (!strcmp(response_status, "OK")) {
        return PLID;
    } else if (!strcmp(response_status, "NOK")) {
        fprintf(stderr, "Error: You cannot start a new game while playing another.\n");
        return ERROR;
    } else if (!strcmp(response_status, "ERR")) {
        fprintf(stderr, "Error: Incorrect request syntax.\n");
        return ERROR;
    } else {
        fprintf(stderr, "Error: Response format/status not known.\n");
        return ERROR;
    }
}


/**
 * Handles try command.
 */
int try_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND], int PLID, int nT) {
    char C1, C2, C3, C4;

    // Validate command syntax
    if (validate_try_command(cmd, NULL, &C1, &C2, &C3, &C4, NULL, PLAYER_SIDE) == ERROR) {
        return ERROR;
    }

    // Create message to send
    // Example: TRY 106324 Y Y Y Y 1\n\0
    size_t msg_len = COMMAND_LEN + 1 + PLID_DIGITS + 4*2 + 1 + 1 + 1 + 1;
    char msg[msg_len];
    snprintf(msg, msg_len, "TRY %d %c %c %c %c %d\n", PLID, C1, C2, C3, C4, nT);

    // Send message to server
    ssize_t n = sendto(udp_fd, msg, sizeof(msg), 0, udp_res->ai_addr, udp_res->ai_addrlen);
    if (n == ERROR) {
        fprintf(stderr, "Error while sending message.\n");
        return ERROR;
    }

    // Receive response from server
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    // Receive server response
    // Example: RTR ENT C1 C2 C3 C4\n\0 
    size_t resp_len = COMMAND_LEN + 1 + RESPONSE_LEN + 4*2 + 1 + 1;
    char response[resp_len];
    n = recvfrom(udp_fd, response, sizeof(response), 0, (struct sockaddr*)&addr, &addrlen);
    if (n == ERROR) {
        fprintf(stderr, "Error while receiving message.\n");
        return ERROR;
    }

    char response_status[RESPONSE_LEN + 1];
    sscanf(response, "RTR %s", response_status);

    // TODO: ADD return constants to deal with dup, inv etc etc
    // Verify server message sending, receiving: "RTR OK 0 0 2="
    // Prints when the game ends are coming with bugs

    // Deal with response
    if (!strcmp(response_status, "OK")) {

        int nT, nB, nW;
        sscanf(response, "RTR OK %d %d %d\n", &nT, &nB, &nW);

        if (nB == 4) {
            printf("Congratulations! You won the game!\n");
            return GAME_ENDED;
        }
        printf("nT: %d\nnB: %d\nnW: %d\n", nT, nB, nW);        
        return OK;
    } else if (!strcmp(response_status, "DUP")) {
        fprintf(stderr, "Error: Your secret key guess repeats a previous trial's guess.\n");
        return ERROR;
    } else if (!strcmp(response_status, "INV")) {
        fprintf(stderr, "Error: Invalid nT value.\n");
        return ERROR;
    } else if (!strcmp(response_status, "NOK")) {
        fprintf(stderr, "Error: Out of context trial.\n");
        return ERROR;
    } else if (!strcmp(response_status, "ENT")) {
        char C1, C2, C3, C4;
        printf("%s\n", response);
        sscanf(response, "RTR ENT %c %c %c %c\n", &C1, &C2, &C3, &C4);
        printf("You have no more attempts available!\nThe secret key is: %c %c %c %c\n", C1, C2, C3, C4);
        return GAME_ENDED;
    } else if (!strcmp(response_status, "ETM")) {
        char C1, C2, C3, C4;
        printf("%s\n", response);
        sscanf(response, "RTR ETM %c %c %c %c\n", &C1, &C2, &C3, &C4);
        printf("You have exceed the play time!\nThe secret key is: %c %c %c %c\n", C1, C2, C3, C4);
        return GAME_ENDED;
    } else if (!strcmp(response_status, "ERR")) {
        fprintf(stderr, "Error: Incorrect syntax, invalid PLID or invalid color.\n");
        return ERROR;
    } else {
        fprintf(stderr, "Error: Response format/status not known.\n");
        return ERROR;
    }
}


/**
 * Handles quit command.
 */
int quit_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND], int PLID) {

    if (validate_quit_command(cmd, NULL, PLAYER_SIDE, QUIT_CONTEXT) == ERROR) {
        return ERROR;
    }

    // Create message to send
    // Example: quit 106324\n\0
    size_t msg_len = COMMAND_LEN + 1 + PLID_DIGITS + 1 + 1;
    char msg[msg_len];
    snprintf(msg, msg_len, "QUT %d\n", PLID);


    // Send message to server
    ssize_t n = sendto(udp_fd, msg, sizeof(msg), 0, udp_res->ai_addr, udp_res->ai_addrlen);
    if (n == ERROR) {
        fprintf(stderr, "Error while sending message.\n");
        return ERROR;
    }

    // Receive response from server
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    // Receive server response
    // Example: RQT OK C1 C2 C3 C4\n\0
    size_t resp_len = COMMAND_LEN + 1 + RESPONSE_LEN + 4*2 + 1 + 1;
    char response[resp_len];
    n = recvfrom(udp_fd, response, sizeof(response), 0, (struct sockaddr*)&addr, &addrlen);
    if (n == ERROR) {
        fprintf(stderr, "Error while receiving message.\n");
        return ERROR;
    }

    char response_status[RESPONSE_LEN + 1];
    sscanf(response, "RQT %s", response_status);

    // Deal with response
    if (!strcmp(response_status, "OK")) {
        char C1, C2, C3, C4;
        sscanf(response, "RQT OK %c %c %c %c\n", &C1, &C2, &C3, &C4);
        printf("The secret key is: %c %c %c %c\n", C1, C2, C3, C4);
        return GAME_ENDED;
    } else if (!strcmp(response_status, "NOK")) {
        fprintf(stderr, "Error: PLID does not have an ongoing game.\n");
        return ERROR;
    } else if (!strcmp(response_status, "ERR")) {
        fprintf(stderr, "Error: Incorrect request syntax.\n");
        return ERROR;
    } else {
        fprintf(stderr, "Error: Response format/status not known.\n");
        return ERROR;
    }
}


/**
 * Handles exit command.
 */
int exit_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND], int PLID) {
    
    if (validate_quit_command(cmd, NULL, PLAYER_SIDE, EXIT_CONTEXT) == ERROR) {
        return ERROR;
    }

    // Create message to send
    // Example: quit 106324
    size_t msg_len = COMMAND_LEN + 1 + PLID_DIGITS + 1 + 1;
    char msg[msg_len];
    snprintf(msg, msg_len, "QUT %d\n", PLID);

    // Send message to server
    ssize_t n = sendto(udp_fd, msg, sizeof(msg), 0, udp_res->ai_addr, udp_res->ai_addrlen);
    if (n == ERROR) {
        fprintf(stderr, "Error while sending message.\n");
        return ERROR;
    }

    // Receive response from server
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    // Receive server response
    // Example: RQT OK C1 C2 C3 C4\n\0
    size_t resp_len = COMMAND_LEN + 1 + RESPONSE_LEN + 4*2 + 1 + 1;
    char response[resp_len];
    n = recvfrom(udp_fd, response, sizeof(response), 0, (struct sockaddr*)&addr, &addrlen);
    if (n == ERROR) {
        fprintf(stderr, "Error while receiving message.\n");
        return ERROR;
    }

    char response_status[RESPONSE_LEN + 1];
    sscanf(response, "RQT %s", response_status);

    // Deal with response
    if (!strcmp(response_status, "OK")) {
        char C1, C2, C3, C4;
        sscanf(response, "RQT OK %c %c %c %c", &C1, &C2, &C3, &C4);
        printf("The secret key is: %c %c %c %c\n", C1, C2, C3, C4);
        return GAME_ENDED;
    } else if (!strcmp(response_status, "NOK")) {
        fprintf(stderr, "Error: PLID does not have an ongoing game.\n");
        return ERROR;
    } else if (!strcmp(response_status, "ERR")) {
        fprintf(stderr, "Error: Incorrect request syntax.\n");
        return ERROR;
    } else {
        fprintf(stderr, "Error: Response format/status not known.\n");
        return ERROR;
    }
}


/**
 * Handles debug command.
 */
int debug_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND]) {
    char PLID_arg[PLID_DIGITS + 1];
    char max_playtime_arg[TIME_DIGITS + 1];
    char C1, C2, C3, C4;

    // Valida o comando "debug"
    if (validate_debug_command(cmd, PLID_arg, max_playtime_arg, &C1, &C2, &C3, &C4, PLAYER_SIDE) == ERROR) {
        return ERROR;
    }

    // Converte argumentos para inteiros
    int PLID = atoi(PLID_arg);
    int max_playtime = atoi(max_playtime_arg);

    // Create message to send
    // Example: DBG 106324 100 Y Y Y Y\n\0
    size_t msg_len = COMMAND_LEN + 1 + PLID_DIGITS + 1 + TIME_DIGITS + 4*2 + 1 + 1;
    char msg[msg_len];
    snprintf(msg, msg_len, "DBG %d %d %c %c %c %c\n", PLID, max_playtime, C1, C2, C3, C4);

    // Send message to server
    ssize_t n = sendto(udp_fd, msg, sizeof(msg), 0, udp_res->ai_addr, udp_res->ai_addrlen);
    if (n == ERROR) {
        fprintf(stderr, "Error while sending message.\n");
        return ERROR;
    }

    // Receive response from server
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    // Receive server response
    // Example: SNG ERR\n\0
    size_t resp_len = COMMAND_LEN + 1 + RESPONSE_LEN + 1 + 1;
    char response[resp_len];
    n = recvfrom(udp_fd, response, sizeof(response), 0, (struct sockaddr*)&addr, &addrlen);
    if (n == ERROR) {
        fprintf(stderr, "Error while receiving message.\n");
        return ERROR;
    }

    char response_status[RESPONSE_LEN + 1];
    sscanf(response, "RDB %s", response_status);

    // Deal with response
    if (!strcmp(response_status, "OK")) {
        return PLID;
    } else if (!strcmp(response_status, "NOK")) {
        fprintf(stderr, "Error: You cannot start a new game while playing another.\n");
        return ERROR;
    } else if (!strcmp(response_status, "ERR")) {
        fprintf(stderr, "Error: Incorrect request syntax.\n");
        return ERROR;
    } else {
        fprintf(stderr, "Error: Response format/status not known.\n");
        return ERROR;
    }
}



int show_trials_function(struct addrinfo *tcp_res, char cmd[MAX_PLAYER_COMMAND], char type[MAX_PLAYER_COMMAND], int PLID) {
    
    // Syntax Validation
    if (validate_showtrials_command(cmd, NULL, PLAYER_SIDE) == ERROR) {
        return ERROR;
    }


    // ############################################################################################
    // ### TCP CONNECTION #########################################################################
    // Create TCP socket
    int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd == ERROR) {
        fprintf(stderr, "Error while creating TCP socket.");
        return ERROR;
    }

    // Open a TCP connection with the server
    ssize_t n = connect(tcp_fd, tcp_res->ai_addr, tcp_res->ai_addrlen);
    if (n == ERROR) {
        fprintf(stderr, "Error while trying to establish a connection with GS.\n");
        return ERROR;
    }


    // Create message to send (ex: STR 106324\n\0)
    size_t msg_len = COMMAND_LEN + 1 + PLID_DIGITS + 1 + 1;
    char msg[msg_len];
    snprintf(msg, msg_len, "STR %d\n", PLID);

    // Send message to server
    if (tcp_write(tcp_fd, msg)) {
        fprintf(stderr, "Error while writing to TCP socket.\n");
        close(tcp_fd);
        return ERROR;
    }
    
    // Receive response from server (just need to read the first 2 words)
    char *response;
    if (tcp_read_until_delimiter(tcp_fd, &response, ' ', 2)) {
        close(tcp_fd);
        return ERROR;
    }
    char response_status[RESPONSE_LEN + 1];
    sscanf(response, "RST %s", response_status);

    // Evaluate responses
    if (!strcmp(response_status, "NOK")) {
        close(tcp_fd);
        return ERROR;
    } else if (!strcmp(response_status, "ACT") || !strcmp(response_status, "FIN")) {
        // Read Fname
        char *Fname = NULL;
        if (tcp_read_until_delimiter(tcp_fd, &Fname, ' ', 1)) {
            free(Fname);
            close(tcp_fd);
            return ERROR;
        }

        // Read Fsize
        char *Fsize_char = NULL;
        if (tcp_read_until_delimiter(tcp_fd, &Fsize_char, ' ', 1)) {
            free(Fname);
            free(Fsize_char);
            close(tcp_fd);
            return 1;
        }
        int Fsize = atoi(Fsize_char);
        free(Fsize_char);

        // Read file content
        char *Fdata;
        if (tcp_read_until_delimiter(tcp_fd, &Fdata, '\0', 1)) {
            close(tcp_fd);
            return ERROR;
        }


        // ########################################################################################
        // ### Store as local file ################################################################
        char file_path[BUFFER_SIZE] = "./CLIENT_CACHE/";

        // Create the directory
        if (mkdir(file_path, 0755) != 0 && errno != EEXIST) { // Created successfully or already exists
            fprintf(stderr, "Error while creating \'%s\' directory.\n", file_path);
            free(Fdata);
            free(Fname);
            close(tcp_fd);
            return 1;
        }

        // Write Fdata into local file
        strcat(file_path, Fname);
        if (write_to_file(file_path, Fdata) == -1) {
            fprintf(stderr, "Error while writing to local file.\n");
            free(Fdata);
            free(Fname);
            close(tcp_fd);
            return 1;
        }


        // ########################################################################################
        // ### Show content #######################################################################
        printf("------------- TRIES: -------------\n");
        printf(Fdata);
        printf("----------------------------------\n");


        free(Fdata);
        free(Fname);
        close(tcp_fd);
        return OK;
    }

    return ERROR;
}


int scoreboard_function(struct addrinfo *tcp_res, char cmd[MAX_PLAYER_COMMAND], char type[MAX_PLAYER_COMMAND]) {
    
    if (validate_scoreboard_command(cmd, PLAYER_SIDE) == ERROR) {
        return ERROR;
    }

    // ############################################################################################
    // ### TCP CONNECTION #########################################################################
    // Create TCP socket
    int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd == ERROR) {
        fprintf(stderr, "Error while creating TCP socket.");
        return ERROR;
    }

    // Open a TCP connection with the server
    ssize_t n = connect(tcp_fd, tcp_res->ai_addr, tcp_res->ai_addrlen);
    if (n == ERROR) {
        fprintf(stderr, "Error while trying to establish a connection with GS.\n");
        return ERROR;
    }
    

    // Create message to send
    size_t msg_len = COMMAND_LEN + 2;
    char msg[msg_len];
    snprintf(msg, msg_len, "SSB\n\0");

    // Send message to server
    if (tcp_write(tcp_fd, msg)) {
        fprintf(stderr, "Error while writing to TCP socket.\n");
        close(tcp_fd);
        return ERROR;
    }
    
    // Receive response from server
    char *response = NULL;
    if (tcp_read_until_delimiter(tcp_fd, &response, ' ', 1)) {
        free(response);
        close(tcp_fd);
        return 1;
    }
    free(response);

    char *response_status = NULL;
    if (tcp_read_until_delimiter(tcp_fd, &response_status, ' ', 1)) {
        free(response_status);
        close(tcp_fd);
        return 1;
    }

    // Evaluate responses
    if (!strcmp(response_status, "OK")) {
        free(response_status);

        // Read Fname
        char *Fname = NULL;
        if (tcp_read_until_delimiter(tcp_fd, &Fname, ' ', 1)) {
            free(Fname);
            close(tcp_fd);
            return 1;
        }

        // Read Fsize
        char *Fsize_char = NULL;
        if (tcp_read_until_delimiter(tcp_fd, &Fsize_char, ' ', 1)) {
            free(Fname);
            free(Fsize_char);
            close(tcp_fd);
            return 1;
        }
        int Fsize = atoi(Fsize_char);
        free(Fsize_char);

        // Read file content
        char *Fdata;
        if (tcp_read_until_delimiter(tcp_fd, &Fdata, '\0', 1)) {
            close(tcp_fd);
            return ERROR;
        }


        // ########################################################################################
        // ### Store as local file ################################################################
        char file_path[BUFFER_SIZE] = "./CLIENT_CACHE/";

        // Create the directory
        if (mkdir(file_path, 0755) != 0 && errno != EEXIST) { // Created successfully or already exists
            fprintf(stderr, "Error while creating \'%s\' directory.\n", file_path);
            free(Fname);
            close(tcp_fd);
            return 1;
        }

        // Write Fdata into local file
        strcat(file_path, Fname);
        if (write_to_file(file_path, Fdata) == -1) {
            fprintf(stderr, "Error while writing to local file.\n");
            free(Fname);
            close(tcp_fd);
            return 1;
        }


        // ########################################################################################
        // ### Show content #######################################################################
        int n_line = 1;
        
        printf("---------- SCOREBOARD: ----------\n");
        printf(Fdata);
        printf("---------------------------------\n");


        free(Fname);
        close(tcp_fd);
        return 0;
    } else if (!strcmp(response_status, "EMPTY\n")) {
        free(response_status);
        printf("The scoreboard is still empty.\n");
        close(tcp_fd);
        return 0;
    } else {
        free(response_status);
        fprintf(stderr, "Error: Response format/status not known.\n");
        close(tcp_fd);
        return 1;
    }
}