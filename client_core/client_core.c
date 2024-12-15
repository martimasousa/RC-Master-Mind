#include "client_core.h"


/**
 * Handles start command.
 */
int start_function(int udp_fd, struct addrinfo *udp_res, char *cmd) {

    char PLID_arg[PLID_DIGITS + 1];
    char max_playtime_arg[TIME_DIGITS + 1];

    // Validate command syntax
    if (validate_start_command(cmd, PLID_arg, max_playtime_arg, PLAYER_SIDE) == ERROR) {
        return ERROR;
    }

    // Create message to send
    // Example: SNG 106324 100\n\0
    const char *components[] = {SNG_CMD, SPACE, PLID_arg, SPACE, max_playtime_arg, NEWLINE};
    size_t count = sizeof(components) / sizeof(components[0]);
    char *msg = create_string(components, count);
    if (msg == NULL) return ERROR;
    
    // Receive server response
    // Example: SNG ERR\n\0
    size_t resp_len = COMMAND_LEN + 1 + RESPONSE_LEN + 1 + 1;
    char response[resp_len];

    if (send_receive_udp(udp_fd, udp_res, msg, strlen(msg), response, resp_len) == ERROR) {
        return ERROR;
    }

    if (!strcmp(response, "ERR\n")) {
        fprintf(stderr, "Error: Command not recognized\n");
        return ERROR;
    }
    
    // Example: RSG ERR\0
    char response_status[RESPONSE_LEN + 1];
    int result = sscanf(response, "RSG %s\n", response_status);

    if (result != 1) {
        fprintf(stderr, "Error: Failed to parse the response string.\n");
        return ERROR;
    }

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
int try_function(int udp_fd, struct addrinfo *udp_res, char *cmd, int PLID, int nT) {
    char C1[COLOR_LEN + 1], C2[COLOR_LEN + 1], C3[COLOR_LEN + 1], C4[COLOR_LEN + 1];
    char *number_tries = int_to_string(nT);
    char *PLID_arg = int_to_string(PLID);

    if (PLID_arg == NULL || number_tries == NULL) return ERROR;

    // Validate command syntax
    if (validate_try_command(cmd, NULL, C1, C2, C3, C4, NULL, PLAYER_SIDE) == ERROR) {
        return ERROR;
    }

    // Create message to send
    // Example: TRY 106324 Y Y Y Y 1\n\0
    const char *components[] = {TRY_CMD, SPACE, PLID_arg, SPACE, C1, SPACE, C2, SPACE, C3, SPACE, C4, SPACE, number_tries, NEWLINE};
    size_t count = sizeof(components) / sizeof(components[0]);
    char *msg = create_string(components, count);
    if (msg == NULL) return ERROR;

    // Receive server response
    // Example: RTR ENT C1 C2 C3 C4\n\0 
    size_t resp_len = COMMAND_LEN + 1 + RESPONSE_LEN + 4*2 + 1 + 1;
    char response[resp_len];

    if (send_receive_udp(udp_fd, udp_res, msg, strlen(msg), response, resp_len) == ERROR) {
        return ERROR;
    }

    if (!strcmp(response, "ERR\n")) {
        fprintf(stderr, "Error: Command not recognized\n");
        return ERROR;
    }

    char response_status[RESPONSE_LEN + 1];
    int result = sscanf(response, "RTR %s", response_status);

    if (result != 1) {
        fprintf(stderr, "Error: Failed to parse the response string.\n");
        return ERROR;
    }

    // Deal with response
    if (!strcmp(response_status, "OK")) {

        int nT, nB, nW;
        int result = sscanf(response, "RTR OK %d %d %d\n", &nT, &nB, &nW);

        if (result != 3) {
            fprintf(stderr, "Error: Failed to parse the response string.\n");
            return ERROR;
        }

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
        int result = sscanf(response, "RTR ENT %c %c %c %c\n", &C1, &C2, &C3, &C4);

        if (result != 4) {
            fprintf(stderr, "Error: Failed to parse the response string.\n");
            return ERROR;
        }

        printf("You have no more attempts available!\nThe secret key is: %c %c %c %c\n", C1, C2, C3, C4);
        return GAME_ENDED;
    } else if (!strcmp(response_status, "ETM")) {
        char C1, C2, C3, C4;
        int result = sscanf(response, "RTR ETM %c %c %c %c\n", &C1, &C2, &C3, &C4);

        if (result != 4) {
            fprintf(stderr, "Error: Failed to parse the response string.\n");
            return ERROR;
        }

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
 * Handles quit and exit command.
 */
int exit_quit_function(int udp_fd, struct addrinfo *udp_res, char *cmd, int PLID, int game_context) {

    char *PLID_arg = int_to_string(PLID);
    if (PLID_arg == NULL) return ERROR;
    

    // Validate command syntax
    if (validate_quit_command(cmd, NULL, PLAYER_SIDE, game_context) == ERROR) {
        return ERROR;
    }

    // Create message to send
    // Example: QUT 106324\n\0
    const char *components[] = {QUT_CMD, SPACE, PLID_arg, NEWLINE};
    size_t count = sizeof(components) / sizeof(components[0]);
    char *msg = create_string(components, count);
    if (msg == NULL) return ERROR;

    // Receive server response
    // Example: RQT OK C1 C2 C3 C4\n\0
    size_t resp_len = COMMAND_LEN + 1 + RESPONSE_LEN + 4*COLOR_LEN*2 + 1 + 1;
    char response[resp_len];
    
    if (send_receive_udp(udp_fd, udp_res, msg, strlen(msg), response, resp_len) == ERROR) {
        return ERROR;
    }

    if (!strcmp(response, "ERR\n")) {
        fprintf(stderr, "Error: Command not recognized\n");
        return ERROR;
    }

    char response_status[RESPONSE_LEN + 1];
    int result = sscanf(response, "RQT %s", response_status);

    if (result != 1) {
        fprintf(stderr, "Error: Failed to parse the response string.\n");
        return ERROR;
    }

    // Deal with response
    if (!strcmp(response_status, "OK")) {
        char C1, C2, C3, C4;
        int result = sscanf(response, "RQT OK %c %c %c %c\n", &C1, &C2, &C3, &C4);
        
        if (result != 4) {
            fprintf(stderr, "Error: Failed to parse the response string.\n");
            return ERROR;
        
        }
        printf("The secret key is: %c %c %c %c\n", C1, C2, C3, C4);
        return GAME_ENDED;
    } else if (!strcmp(response_status, "NOK") && game_context == QUIT_CONTEXT) {
        fprintf(stderr, "Error: PLID does not have an ongoing game.\n");
        return ERROR;
    } else if (!strcmp(response_status, "ERR")) {
        fprintf(stderr, "Error: Incorrect request syntax.\n");
        return ERROR;
    }

    return OK;
}


/**
 * Handles debug command.
 */
int debug_function(int udp_fd, struct addrinfo *udp_res, char *cmd) {
    char C1[COLOR_LEN + 1], C2[COLOR_LEN + 1], C3[COLOR_LEN + 1], 
         C4[COLOR_LEN + 1], max_playtime_arg[TIME_DIGITS + 1], PLID_arg[PLID_DIGITS + 1];

    // Validate command syntax
    if (validate_debug_command(cmd, PLID_arg, max_playtime_arg, C1, C2, C3, C4, PLAYER_SIDE) == ERROR) {
        return ERROR;
    }

    // Create message to send
    // Example: DBG 106324 100 Y Y Y Y\n\0
    const char *components[] = {DBG_CMD, SPACE, PLID_arg, SPACE, max_playtime_arg, SPACE, C1, SPACE, C2, SPACE, C3, SPACE, C4, NEWLINE};
    size_t count = sizeof(components) / sizeof(components[0]);
    char *msg = create_string(components, count);
    if (msg == NULL) return ERROR;

    // Receive server response
    // Example: SNG ERR\n\0
    size_t resp_len = COMMAND_LEN + 1 + RESPONSE_LEN + 1 + 1;
    char response[resp_len];

    if (send_receive_udp(udp_fd, udp_res, msg, strlen(msg), response, resp_len) == ERROR) {
        return ERROR;
    }

    if (!strcmp(response, "ERR\n")) {
        fprintf(stderr, "Error: Command not recognized\n");
        return ERROR;
    }

    char response_status[RESPONSE_LEN + 1];
    int result = sscanf(response, "RDB %s", response_status);
    
    if (result != 1) {
        fprintf(stderr, "Error: Failed to parse the response string.\n");
        return ERROR;
    }

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
 * Handles show_trials command.
 */
int show_trials_function(struct addrinfo *tcp_res, char *cmd, int PLID) {
    char *PLID_arg = int_to_string(PLID);

    // Validate command syntax
    if (validate_showtrials_command(cmd, NULL, PLAYER_SIDE) == ERROR) {
        return ERROR;
    }

    // Create message to send
    // Example: STR 106324\n\0
    const char *components[] = {STR_CMD, SPACE, PLID_arg, NEWLINE};
    size_t count = sizeof(components) / sizeof(components[0]);
    char *msg = create_string(components, count);
    if (msg == NULL) return ERROR;

    // Create the TCP socket and configure the timeout
    int tcp_fd;
    if (create_tcp_socket(&tcp_fd) == ERROR) {
        fprintf(stderr, "Error: Failed to create TCP socket.\n");
        return ERROR;
    }

    // Establish the server connection
    if (connect_to_server(tcp_fd, tcp_res) == ERROR) {
        fprintf(stderr, "Error: Failed to connect to the server.\n");
        close(tcp_fd);
        return ERROR;
    }

    // Send message
    if (send_message(tcp_fd, msg) == ERROR) {
        fprintf(stderr, "Error: Failed to send message.\n");
        close(tcp_fd);
        return ERROR;
    }
    free(msg);

    // Receive response
    char *response = NULL;
    if (receive_response(tcp_fd, &response) == ERROR) {
        fprintf(stderr, "Error: Failed to receive response.\n");
        close(tcp_fd);
        return ERROR;
    }

    if (!strcmp(response, "ERR\n")) {
        fprintf(stderr, "Error: Command not recognized\n");
        close(tcp_fd);
        return ERROR;
    }

    char response_status[RESPONSE_LEN + 1];
    int result = sscanf(response, "RST %s", response_status);

    if (result != 1) {
        fprintf(stderr, "Error: Failed to parse the response string.\n");
        free(response);
        close(tcp_fd);
        return ERROR;
    }
    free(response);

    // Deal with response
    if (!strcmp(response_status, "NOK")) {
        return ERROR;
    } else if (!strcmp(response_status, "ACT") || !strcmp(response_status, "FIN")) {

        char *Fname = NULL, *Fsize_char = NULL, *Fdata = NULL;

        // Read the complete response
        if (tcp_read_until_delimiter(tcp_fd, &Fname, ' ', 1) ||
            tcp_read_until_delimiter(tcp_fd, &Fsize_char, ' ', 1) ||
            tcp_read(tcp_fd, &Fdata, atoi(Fsize_char)) == ERROR) {
            free(Fname);
            free(Fsize_char);
            free(Fdata);
            return ERROR;
        }

        // Store the information on a local file
        if (store_file_local(Fname, Fdata) == ERROR) {
            free(Fname);
            free(Fsize_char);
            free(Fdata);
            return ERROR;
        }

        // Show content
        show_show_trials_content(Fdata);

        free(Fname);
        free(Fsize_char);
        free(Fdata);
        return OK;
    }

    return ERROR;
}


/**
 * Handles scoreboard command.
*/
int scoreboard_function(struct addrinfo *tcp_res, char *cmd) {

    // Validate command syntax
    if (validate_scoreboard_command(cmd, PLAYER_SIDE) == ERROR) {
        return ERROR;
    }

    // Create message to send
    // Example: SSB\n\0
    const char *components[] = {SSB_CMD, NEWLINE};
    size_t count = sizeof(components) / sizeof(components[0]);
    char *msg = create_string(components, count);
    if (msg == NULL) return ERROR;

    // Create the TCP socket and configure the timeout
    int tcp_fd;
    if (create_tcp_socket(&tcp_fd) == ERROR) {
        fprintf(stderr, "Error: Failed to create TCP socket.\n");
        return ERROR;
    }

    // Establish the server connection
    if (connect_to_server(tcp_fd, tcp_res) == ERROR) {
        fprintf(stderr, "Error: Failed to connect to the server.\n");
        close(tcp_fd);
        return ERROR;
    }

    // Send message
    if (send_message(tcp_fd, msg) == ERROR) {
        close(tcp_fd);
        free(msg);
        return ERROR;
    }
    free(msg);

    // Receive response
    char *response = NULL;
    if (receive_response(tcp_fd, &response) == ERROR) {
        fprintf(stderr, "Error: Failed to receive response.\n");
        close(tcp_fd);
        return ERROR;
    }

    if (!strcmp(response, "ERR\n")) {
        fprintf(stderr, "Error: Command not recognized\n");
        close(tcp_fd);
        return ERROR;
    }

    char response_status[RESPONSE_LEN + 1];
    int result = sscanf(response, "RSS %s", response_status);
    
    if (result != 1) {
        fprintf(stderr, "Error: Failed to parse the response string.\n");
        free(response);
        close(tcp_fd);
        return ERROR;
    }
    free(response);

    // Deal with response
    if (!strcmp(response_status, "EMPTY")) {
        printf("The scoreboard is still empty.\n");
        close(tcp_fd);
        return ERROR;
    } else if (!strcmp(response_status, "OK")) {

        char *Fname = NULL, *Fsize_char = NULL, *Fdata = NULL;

        // Read the complete response
        if (tcp_read_until_delimiter(tcp_fd, &Fname, ' ', 1) ||
            tcp_read_until_delimiter(tcp_fd, &Fsize_char, ' ', 1) ||
            tcp_read(tcp_fd, &Fdata, atoi(Fsize_char)) == ERROR) {
            free(Fname);
            free(Fsize_char);
            free(Fdata);
            return ERROR;
        }

        // Store the information on a local file
        if (store_file_local(Fname, Fdata) == ERROR) {
            free(Fname);
            free(Fsize_char);
            free(Fdata);
            return ERROR;
        }

        // Show the content
        show_scoreboard_content(Fdata);

        free(Fname);
        free(Fsize_char);
        free(Fdata);
        return OK;
    } 

    return ERROR;
}