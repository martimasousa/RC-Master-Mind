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

    printf("Message: %s\n", msg);


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

    printf("Message: %s\n", msg);


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
        }

        printf("nT: %d\nnB: %d\nnW: %d\n", nT, nB, nW);

        // TODO: "If nB = 4 the secret code has been correctly guessed and the player wins the game.
        // The same reply is provided if the trial number nT is the expected value minus 1, and the
        // secret key guess repeats the one of the previous message (it is a resend) – in this case
        // the number of trials is not increased" ?????????????????????????????????????????????????

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

int show_trials_function(int tcp_fd, struct addrinfo *tcp_res, char cmd[MAX_PLAYER_COMMAND], char type[MAX_PLAYER_COMMAND], int PLID) {
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


    // Create message to send
    char msg[11];
    snprintf(msg, 11, "STR %d", PLID);

    printf("Message: %s\n", msg);


    // TODO: Send message to server
    

    return 0;
}

int scoreboard_function(int tcp_fd, struct addrinfo *tcp_res, char cmd[MAX_PLAYER_COMMAND], char type[MAX_PLAYER_COMMAND]) {
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


    // Create message to send
    char msg[4];
    snprintf(msg, 4, "SSB");

    printf("Message: %s\n", msg);


    // TODO: Send message to server
    

    return 0;
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

    printf("Message: %s\n", msg);


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

    printf("Message: %s\n", msg);


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

    printf("Message: %s\n", msg);


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
    printf("[Arguments]\nGSIP: %s\nGSport: %s\n", GSIP, GSport);


    // Create UDP socket
    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd == -1) {
        fprintf(stderr, "Error while creating UDP socket.");
        exit(1);
    }

    // Find Game Server address (UDP) -> Will be stored in res
    struct addrinfo hints, *udp_res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP socket

    if (getaddrinfo(GSIP, GSPORT, &hints, &udp_res) != 0) {
        exit(1);   
    }



    // TODO: Create TCP socket and res
    int tcp_fd;
    struct addrinfo *tcp_res;


    int PLID = -1;
    int nT = 1;

    // Process commands
    while (TRUE) {
        // Read command
        char cmd[MAX_PLAYER_COMMAND];
        fgets(cmd, MAX_PLAYER_COMMAND, stdin);

        // Remove the trailing newline character, if any
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
            } else if (!try_function(udp_fd, udp_res, cmd, PLID, nT)) { // No error
                nT += 1;
            }
        } else if (!strcmp(type, "show_trials") || !strcmp(type, "st")) {
            if (PLID < 0) {
                fprintf(stderr, "Error: Please start a game before making a try.\n");
            } else if (show_trials_function(tcp_fd, tcp_res, cmd, type, PLID)) {
                //printf("Error\n");
            }
        } else if (!strcmp(type, "scoreboard") || !strcmp(type, "sb")) {
            if (scoreboard_function(tcp_fd, tcp_res, cmd, type)) {
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
                exit(1);
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