#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "constants.h"
#include "utils.h"


int start_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND]) {
    char PLID_arg[7];
    char max_playtime_arg[4];
    char extra[100];

    // Read arguments and check for errors
    int result = sscanf(cmd, "start %s %s %s", PLID_arg, max_playtime_arg, extra);

    if (result != 2) {
        fprintf(stderr, "Error: Command format should be 'start PLID max_playtime'.\n");
        return -1;
    }

    if (!is_integer(PLID_arg) || !is_integer(max_playtime_arg)) {
        fprintf(stderr, "Error: Command format should be 'start PLID max_playtime'.\nHint: Make sure PLID and max_playtime are integers.\n");
        return -1;
    }

    // Convert arguments into integers to check if are valid
    int PLID = atoi(PLID_arg);
    if (PLID < 100000 || PLID > 999999) {
        fprintf(stderr, "Error: Command format should be 'start PLID max_playtime'.\nHint: PLID has only 6 digits.\n");
        return -1;
    }

    int max_playtime = atoi(max_playtime_arg);
    if (max_playtime > 600) {
        fprintf(stderr, "Error: Command format should be 'start PLID max_playtime'.\nHint: max_playtime cannot exceed 600s.\n");
        return -1;
    }


    // Create message to send
    char msg[15];
    snprintf(msg, sizeof(msg), "SNG %s %s", PLID_arg, max_playtime_arg);

    printf("[TEST] Message: %s\n", msg); // TODO: REMOVE

    // Send message to server
    ssize_t n = sendto(udp_fd, msg, sizeof(msg), 0, udp_res->ai_addr, udp_res->ai_addrlen);
    if (n == -1) {
        fprintf(stderr, "Error while sending message.\n");
        return -1;
    }

    // Receive response from server
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char response[8];
    n = recvfrom(udp_fd, response, sizeof(response), 0, (struct sockaddr*)&addr, &addrlen);
    if (n == -1) {
        fprintf(stderr, "Error while receiving message.\n");
        return -1;
    }

    char response_status[4];
    sscanf(response, "RSG %s", response_status);

    // Deal with response
    if (!strcmp(response_status, "OK")) {
        return PLID;
    } else if (!strcmp(response_status, "NOK")) {
        fprintf(stderr, "Error: You cannot start a new game while playing another.\n");
        return -1;
    } else if (!strcmp(response_status, "ERR")) {
        fprintf(stderr, "Error: Incorrect request syntax.\n");
        return -1;
    } else {
        fprintf(stderr, "Error: Response format/status not known.\n");
        return -1;
    }
}

/**
 * Handles try command.
 * Returns: 1 (in case of error) | 0 (in case of non-correct try) | -1 (in case of correct try)
 */
int try_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND], int PLID, int nT) {
    char C1, C2, C3, C4;
    char extra[100];

    // Read arguments and check for errors
    int result = sscanf(cmd, "try %c %c %c %c %s", &C1, &C2, &C3, &C4, extra);

    if (result != 4) {
        fprintf(stderr, "Error: Command format should be 'try C1 C2 C3 C4'.\n");
        return 1;
    }

    if (!is_valid_color(C1) || !is_valid_color(C2) || !is_valid_color(C3) || !is_valid_color(C4)) {
        fprintf(stderr, "Error: Command format should be 'try C1 C2 C3 C4'.\nHint: The valid colours are: {red (R), green (G), blue (B), yellow (Y), orange (O) and purple (P)}.\n");
        return 1;
    }


    // Create message to send
    char msg[21];
    snprintf(msg, 21, "TRY %d %c %c %c %c %d", PLID, C1, C2, C3, C4, nT);

    printf("[TEST] Message: %s\n", msg); // TODO: REMOVE

    // Send message to server
    ssize_t n = sendto(udp_fd, msg, sizeof(msg), 0, udp_res->ai_addr, udp_res->ai_addrlen);
    if (n == -1) {
        fprintf(stderr, "Error while sending message.\n");
        return 1;
    }

    // Receive response from server
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char response[16];
    n = recvfrom(udp_fd, response, sizeof(response), 0, (struct sockaddr*)&addr, &addrlen);
    if (n == -1) {
        fprintf(stderr, "Error while receiving message.\n");
        return 1;
    }

    char response_status[4];
    sscanf(response, "RTR %s", response_status);

    // Deal with response
    if (!strcmp(response_status, "OK")) {
        // Read nT, nB and nW
        int nT, nB, nW;
        sscanf(response, "RTR OK %d %d %d", &nT, &nB, &nW);

        if (nB == 4) {
            // Player won the game
            printf("Congratulations! You won the game!\n");
            return -1;
        }

        printf("nT: %d\nnB: %d\nnW: %d\n", nT, nB, nW);

        // TODO: "If nB = 4 the secret code has been correctly guessed and the player wins the game.
        // The same reply is provided if the trial number nT is the expected value minus 1, and the
        // secret key guess repeats the one of the previous message (it is a resend) – in this case
        // the number of trials is not increased" ?????????????????????????????????????????????????
        // TODO: Check if the last try was the same. If yes, return 1. But what is the expected nT?

        return 0;
    } else if (!strcmp(response_status, "DUP")) {
        fprintf(stderr, "Error: Your secret key guess repeats a previous trial's guess.\n");
        return 1;
    } else if (!strcmp(response_status, "INV")) {
        fprintf(stderr, "Error: Invalid nT value.\n");
        return 1;
    } else if (!strcmp(response_status, "NOK")) {
        fprintf(stderr, "Error: Out of context trial.\n");
        return 1;
    } else if (!strcmp(response_status, "ENT")) {
        // Read C1, C2, C3, C4
        char C1, C2, C3, C4;
        sscanf(response, "RTR ENT %c %c %c %c", &C1, &C2, &C3, &C4);

        printf("You have no more attempts available!\nThe secret key is: %c %c %c %c\n", C1, C2, C3, C4);
        return 1;
    } else if (!strcmp(response_status, "ETM")) {
        // Read C1, C2, C3, C4
        char C1, C2, C3, C4;
        sscanf(response, "RTR ETM %c %c %c %c", &C1, &C2, &C3, &C4);

        printf("You have exceed the play time!\nThe secret key is: %c %c %c %c\n", C1, C2, C3, C4);
        return 1;
    } else if (!strcmp(response_status, "ERR")) {
        fprintf(stderr, "Error: Incorrect syntax, invalid PLID or invalid color.\n");
        return 1;
    } else {
        fprintf(stderr, "Error: Response format/status not known.\n");
        return 1;
    }
}

int show_trials_function(struct addrinfo *tcp_res, char cmd[MAX_PLAYER_COMMAND], char type[MAX_PLAYER_COMMAND], int PLID) {
    char extra[100];

    // Read arguments and check for errors
    int result;
    if (!strcmp(type, "show_trials")) {
        result = sscanf(cmd, "show_trials %s", extra);
    } else {
        result = sscanf(cmd, "st %s", extra);
    }

    if (result == 1) {
        fprintf(stderr, "Error: Command format should be 'show_trials' or 'st'.\n");
        return 1;
    }


    // Create TCP socket
    int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd == -1) {
        fprintf(stderr, "Error while creating TCP socket.");
        exit(1);
    }

    // Open a TCP connection with the server
    ssize_t n = connect(tcp_fd, tcp_res->ai_addr, tcp_res->ai_addrlen);
    if (n == -1) {
        fprintf(stderr, "Error while trying to establish a connection with GS.\n");
        return 1;
    }


    // Create message to send
    char msg[11];
    snprintf(msg, 11, "STR %d", PLID);

    printf("[TEST] Message: %s\n", msg); // TODO: REMOVE

    // Send message to server
    n = write(tcp_fd, msg, sizeof(msg));
    if (n == -1) {
        fprintf(stderr, "Error while writing to TCP socket.\n");
        close(tcp_fd);
        return 1;
    }
    
    // Receive response from server
    char response[8];
    n = read(tcp_fd, response, 8);
    if (n == -1) {
        fprintf(stderr, "Error while reading from TCP socket.\n");
        close(tcp_fd);
        return 1;
    }

    char response_status[4];
    sscanf(response, "RST %s", response_status);

    // Deal with response
    if (!strcmp(response_status, "ACT")) {
        // Read Fname
        char *Fname = NULL;
        if (tcp_read_until_delimiter(tcp_fd, &Fname, ' ')) {
            free(Fname);
            close(tcp_fd);
            return 1;
        }

        // Read Fsize
        char *Fsize_char = NULL;
        if (tcp_read_until_delimiter(tcp_fd, &Fsize_char, ' ')) {
            free(Fname);
            free(Fsize_char);
            close(tcp_fd);
            return 1;
        }
        int Fsize = atoi(Fsize_char);
        free(Fsize_char);

        // Read file content
        char Fdata[Fsize + 1];
        n = read(tcp_fd, Fdata, Fsize);
        if (n == -1) {
            fprintf(stderr, "Error while reading from TCP socket.\n");
            free(Fname);
            close(tcp_fd);
            return 1;
        }
        Fdata[Fsize] = '\0';


        // Show file
        printf("Fname: %s\nFsize: %d\nFdata:\n-----------------\n%s\n-----------------\n", Fname, Fsize, Fdata);


        // Close TCP connection and frees allocated memory
        free(Fname);
        close(tcp_fd);
        return 0;
    } else if (!strcmp(response_status, "FIN")) {
        // Read Fname
        char *Fname = NULL;
        if (tcp_read_until_delimiter(tcp_fd, &Fname, ' ')) {
            free(Fname);
            close(tcp_fd);
            return 1;
        }

        // Read Fsize
        char *Fsize_char = NULL;
        if (tcp_read_until_delimiter(tcp_fd, &Fsize_char, ' ')) {
            free(Fname);
            free(Fsize_char);
            close(tcp_fd);
            return 1;
        }
        int Fsize = atoi(Fsize_char);
        free(Fsize_char);

        // Read file content
        char Fdata[Fsize + 1];
        n = read(tcp_fd, Fdata, Fsize);
        if (n == -1) {
            fprintf(stderr, "Error while reading from TCP socket.\n");
            free(Fname);
            close(tcp_fd);
            return 1;
        }
        Fdata[Fsize] = '\0';


        // Show file
        printf("Fname: %s\nFsize: %d\nFdata:\n-----------------\n%s\n-----------------\n", Fname, Fsize, Fdata);

        // TODO: Terminate game???
        

        // Close TCP connection and frees allocated memory
        free(Fname);
        close(tcp_fd);
        return 0;
    } else if (!strcmp(response_status, "NOK")) {
        fprintf(stderr, "Error: No active/finished games found.\n");
        close(tcp_fd);
        return 1;
    } else {
        fprintf(stderr, "Error: Response format/status not known.\n");
        close(tcp_fd);
        return 1;
    }
}

int scoreboard_function(struct addrinfo *tcp_res, char cmd[MAX_PLAYER_COMMAND], char type[MAX_PLAYER_COMMAND]) {
    char extra[100];

    // Read arguments and check for errors
    int result;
    if (!strcmp(type, "scoreboard")) {
        result = sscanf(cmd, "scoreboard %s", extra);
    } else {
        result = sscanf(cmd, "sb %s", extra);
    }

    if (result == 1) {
        fprintf(stderr, "Error: Command format should be 'scoreboard' or 'sb'.\n");
        return 1;
    }


    // Create TCP socket
    int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd == -1) {
        fprintf(stderr, "Error while creating TCP socket.");
        exit(1);
    }

    // Open a TCP connection with the server
    ssize_t n = connect(tcp_fd, tcp_res->ai_addr, tcp_res->ai_addrlen);
    if (n == -1) {
        fprintf(stderr, "Error while trying to establish a connection with GS.\n");
        return 1;
    }


    // Create message to send
    char msg[4];
    snprintf(msg, 4, "SSB");

    printf("[TEST] Message: %s\n", msg); // TODO: REMOVE

    // Send message to server
    n = write(tcp_fd, msg, sizeof(msg));
    if (n == -1) {
        fprintf(stderr, "Error while writing to TCP socket.\n");
        close(tcp_fd);
        return 1;
    }
    
    // Receive response from server
    char response[4];
    n = read(tcp_fd, response, 4);
    if (n == -1) {
        fprintf(stderr, "Error while reading from TCP socket.\n");
        close(tcp_fd);
        return 1;
    }

    char *response_status = NULL;
    if (tcp_read_until_delimiter(tcp_fd, &response_status, ' ')) {
        free(response_status);
        close(tcp_fd);
        return 1;
    }

    // Deal with response
    if (!strcmp(response_status, "OK")) {
        free(response_status);

        // Read Fname
        char *Fname = NULL;
        if (tcp_read_until_delimiter(tcp_fd, &Fname, ' ')) {
            free(Fname);
            close(tcp_fd);
            return 1;
        }

        // Read Fsize
        char *Fsize_char = NULL;
        if (tcp_read_until_delimiter(tcp_fd, &Fsize_char, ' ')) {
            free(Fname);
            free(Fsize_char);
            close(tcp_fd);
            return 1;
        }
        int Fsize = atoi(Fsize_char);
        free(Fsize_char);

        // Read file content
        char Fdata[Fsize + 1];
        n = read(tcp_fd, Fdata, Fsize);
        if (n == -1) {
            fprintf(stderr, "Error while reading from TCP socket.\n");
            free(Fname);
            close(tcp_fd);
            return 1;
        }
        Fdata[Fsize] = '\0';

        // Show file
        printf("Fname: %s\nFsize: %d\nFdata: %s\n", Fname, Fsize, Fdata);

        free(Fname);
        close(tcp_fd);
        return 0;
    } else if (!strcmp(response_status, "EMPTY")) {
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

int quit_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND], int PLID) {
    char extra[100];

    // Read arguments and check for errors
    int result = sscanf(cmd, "quit %s", extra);

    if (result == 1) {
        fprintf(stderr, "Error: Command format should be 'quit'.\n");
        return 1;
    }


    // Create message to send
    char msg[11];
    snprintf(msg, 11, "QUT %d", PLID);

    printf("[TEST] Message: %s\n", msg); // TODO: REMOVE

    // Send message to server
    ssize_t n = sendto(udp_fd, msg, sizeof(msg), 0, udp_res->ai_addr, udp_res->ai_addrlen);
    if (n == -1) {
        fprintf(stderr, "Error while sending message.\n");
        return -1;
    }

    // Receive response from server
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char response[16];
    n = recvfrom(udp_fd, response, sizeof(response), 0, (struct sockaddr*)&addr, &addrlen);
    if (n == -1) {
        fprintf(stderr, "Error while receiving message.\n");
        return -1;
    }

    char response_status[4];
    sscanf(response, "RQT %s", response_status);

    // Deal with response
    if (!strcmp(response_status, "OK")) {
        // Read nT, nB and nW
        char C1, C2, C3, C4;
        sscanf(response, "RQT OK %c %c %c %c", &C1, &C2, &C3, &C4);

        printf("The secret key is: %c %c %c %c\n", C1, C2, C3, C4);
        return 0;
    } else if (!strcmp(response_status, "NOK")) {
        fprintf(stderr, "Error: PLID does not have an ongoing game.\n");
        return 1;
    } else if (!strcmp(response_status, "ERR")) {
        fprintf(stderr, "Error: Incorrect request syntax.\n");
        return 1;
    } else {
        fprintf(stderr, "Error: Response format/status not known.\n");
        return 1;
    }
}

int exit_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND], int PLID) {
    char extra[100];

    // Read arguments and check for errors
    int result = sscanf(cmd, "exit %s", extra);

    if (result == 1) {
        fprintf(stderr, "Error: Command format should be 'exit'.\n");
        return 1;
    }


    // Create message to send
    char msg[11];
    snprintf(msg, 11, "QUT %d", PLID);

    printf("[TEST] Message: %s\n", msg); // TODO: REMOVE

    // Send message to server
    ssize_t n = sendto(udp_fd, msg, sizeof(msg), 0, udp_res->ai_addr, udp_res->ai_addrlen);
    if (n == -1) {
        fprintf(stderr, "Error while sending message.\n");
        return -1;
    }

    // Receive response from server
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char response[16];
    n = recvfrom(udp_fd, response, sizeof(response), 0, (struct sockaddr*)&addr, &addrlen);
    if (n == -1) {
        fprintf(stderr, "Error while receiving message.\n");
        return -1;
    }

    char response_status[4];
    sscanf(response, "RQT %s", response_status);

    // Deal with response
    if (!strcmp(response_status, "OK")) {
        // Read nT, nB and nW
        char C1, C2, C3, C4;
        sscanf(response, "RQT OK %c %c %c %c", &C1, &C2, &C3, &C4);

        printf("The secret key is: %c %c %c %c\n", C1, C2, C3, C4);
        return 0;
    } else if (!strcmp(response_status, "NOK")) {
        fprintf(stderr, "Error: PLID does not have an ongoing game.\n");
        return 1;
    } else if (!strcmp(response_status, "ERR")) {
        fprintf(stderr, "Error: Incorrect request syntax.\n");
        return 1;
    } else {
        fprintf(stderr, "Error: Response format/status not known.\n");
        return 1;
    }
}

int debug_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND]) {
    char PLID_arg[7];
    char max_playtime_arg[4];
    char C1, C2, C3, C4;
    char extra[100];

    // Read arguments and check for errors
    int result = sscanf(cmd, "debug %s %s %c %c %c %c %s", PLID_arg, max_playtime_arg, &C1, &C2, &C3, &C4, extra);

    if (result != 6) {
        fprintf(stderr, "Error: Command format should be 'debug PLID max_playtime C1 C2 C3 C4'.\n");
        return -1;
    }

    if (!is_integer(PLID_arg) || !is_integer(max_playtime_arg)) {
        fprintf(stderr, "Error: Command format should be 'debug PLID max_playtime C1 C2 C3 C4'.\nHint: Make sure PLID and max_playtime are integers.\n");
        return -1;
    }

    if (!is_valid_color(C1) || !is_valid_color(C2) || !is_valid_color(C3) || !is_valid_color(C4)) {
        fprintf(stderr, "Error: Command format should be 'debug PLID max_playtime C1 C2 C3 C4'.\nHint: The valid colours are: {red (R), green (G), blue (B), yellow (Y), orange (O) and purple (P)}.\n");
        return -1;
    }

    // Convert arguments into integers
    int PLID = atoi(PLID_arg);
    int max_playtime = atoi(max_playtime_arg);


    // Create message to send
    char msg[23];
    snprintf(msg, 23, "DBG %d %d %c %c %c %c", PLID, max_playtime, C1, C2, C3, C4);

    printf("[TEST] Message: %s\n", msg); // TODO: REMOVE

    // Send message to server
    ssize_t n = sendto(udp_fd, msg, sizeof(msg), 0, udp_res->ai_addr, udp_res->ai_addrlen);
    if (n == -1) {
        fprintf(stderr, "Error while sending message.\n");
        return -1;
    }

    // Receive response from server
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char response[8];
    n = recvfrom(udp_fd, response, sizeof(response), 0, (struct sockaddr*)&addr, &addrlen);
    if (n == -1) {
        fprintf(stderr, "Error while receiving message.\n");
        return -1;
    }

    char response_status[4];
    sscanf(response, "RDB %s", response_status);

    // Deal with response
    if (!strcmp(response_status, "OK")) {
        return PLID;
    } else if (!strcmp(response_status, "NOK")) {
        fprintf(stderr, "Error: You cannot start a new game while playing another.\n");
        return -1;
    } else if (!strcmp(response_status, "ERR")) {
        fprintf(stderr, "Error: Incorrect request syntax.\n");
        return -1;
    } else {
        fprintf(stderr, "Error: Response format/status not known.\n");
        return -1;
    }
}


int main(int argc, char* argv[]) {
    char *GSIP = localhost;
    char *GSport = GSPORT;

    // Read command-line arguments
    if (!((argc == 1) || 
        (argc == 3 && ((strcmp(argv[1], "-n") == 0 && is_valid_ip(argv[2])) || 
                        (strcmp(argv[1], "-p") == 0 && is_integer(argv[2])))) || 
        (argc == 5 && strcmp(argv[1], "-n") == 0 && is_valid_ip(argv[2]) && 
                        strcmp(argv[3], "-p") == 0 && is_integer(argv[4])))) {
        fprintf(stderr, "Error while reading arguments.\nUsage: ./player [-n GSIP] [-p GSport]\n");
        return 0;
    }

    if (argc == 3) {
        if (strcmp(argv[1], "-n") == 0) {
            GSIP = argv[2];
        } else {
            GSport = argv[2];
        }
    } else if (argc == 5) {
        GSIP = argv[2];
        GSport = argv[4];
    }
    printf("[Arguments]\nGSIP: %s\nGSport: %s\n", GSIP, GSport); // TODO: REMOVE


    // ######################################## UDP ############################################### //
    // Create UDP socket
    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd == -1) {
        fprintf(stderr, "Error while creating UDP socket.");
        exit(1);
    }

    // Find Game Server address (UDP) -> Will be stored in res
    struct addrinfo udp_hints, *udp_res;

    memset(&udp_hints, 0, sizeof udp_hints);
    udp_hints.ai_family = AF_INET; //IPv4
    udp_hints.ai_socktype = SOCK_DGRAM; //UDP socket

    if (getaddrinfo(GSIP, GSPORT, &udp_hints, &udp_res) != 0) {
        fprintf(stderr, "Error while getting UDP Game Server address.");
        exit(1);   
    }
    // ######################################################################################### //


    // ######################################## TCP ############################################ //
    // Find Game Server address (TCP) -> Will be stored in res
    struct addrinfo tcp_hints, *tcp_res;

    memset(&tcp_hints, 0, sizeof tcp_hints);
    tcp_hints.ai_family = AF_INET; //IPv4
    tcp_hints.ai_socktype = SOCK_STREAM; //TCP socket

    if (getaddrinfo(GSIP, GSPORT, &tcp_hints, &tcp_res) != 0) {
        fprintf(stderr, "Error while getting TCP Game Server address.");
        exit(1);   
    }
    // ######################################################################################### //


    int PLID = -1;
    int nT = 1;

    // Process commands
    while (TRUE) {
        // Read command
        char cmd[MAX_PLAYER_COMMAND];
        fgets(cmd, MAX_PLAYER_COMMAND, stdin);

        // Remove the trailing newline character
        cmd[strcspn(cmd, "\n")] = '\0';

        // Get command type
        char type[MAX_PLAYER_COMMAND];
        sscanf(cmd, "%s", type);

        // Execute respective function
        if (!strcmp(type, "start")) {
            if (PLID == -1) {
                PLID = start_function(udp_fd, udp_res, cmd);
            } else {
                fprintf(stderr, "Error: You already have an ongoing game.\n");
            }
        } else if (!strcmp(type, "try")) {
            if (PLID < 0) {
                fprintf(stderr, "Error: Please start a game before making a try.\n");
            } else {
                int try_res = try_function(udp_fd, udp_res, cmd, PLID, nT);

                if (try_res == 0) {
                    nT += 1;
                } else if (try_res == -1) {
                    PLID = -1;
                } else {
                    //printf("Error\n");
                }
            }
        } else if (!strcmp(type, "show_trials") || !strcmp(type, "st")) {
            if (PLID < 0) {
                fprintf(stderr, "Error: Please start a game before showing trials.\n");
            } else if (show_trials_function(tcp_res, cmd, type, PLID)) {
                //printf("Error\n");
            }
        } else if (!strcmp(type, "scoreboard") || !strcmp(type, "sb")) {
            if (scoreboard_function(tcp_res, cmd, type)) {
                //printf("Error\n");
            }
        } else if (!strcmp(type, "quit")) {
            if (PLID < 0) {
                fprintf(stderr, "Error: There is no active game.\n");
            } else if (quit_function(udp_fd, udp_res, cmd, PLID)) {
                //printf("Error\n");
            }
        } else if (!strcmp(type, "exit")) {
            if (PLID < 0) {
                fprintf(stderr, "Error: There is no active game.\n");
            } else if (exit_function(udp_fd, udp_res, cmd, PLID)) {
                //printf("Error\n");
                freeaddrinfo(udp_res);
                close(udp_fd);
                return 1;   // exit(1) ???
            }
            break;
        } else if (!strcmp(type, "debug")) {
            if (PLID == -1) {
                PLID = debug_function(udp_fd, udp_res, cmd);
            } else {
                fprintf(stderr, "Error: You already have an ongoing game.\n");
            }
        } else {
            fprintf(stderr, "Error: Please provide a valid command.\n");
        }
    }

    freeaddrinfo(udp_res);
    close(udp_fd);

    return 0;
}