#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "constants.h"
#include "utils.h"
#include "game_core.h"
#include "client_core.h"


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

    if (getaddrinfo(GSIP, GSport, &udp_hints, &udp_res) != 0) {
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

    int running = TRUE;
    int PLID = NOT_PLAYING;
    int lastPLID = NOT_PLAYING;
    int nT = 1;

    // Process commands
    while (running) {

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
            if (PLID == NOT_PLAYING) {
                PLID = start_function(udp_fd, udp_res, cmd);
                lastPLID = PLID;
            } else {
                fprintf(stderr, "Error: You already have an ongoing game.\n");
            }
        } else if (!strcmp(type, "try")) {
            if (PLID == NOT_PLAYING) {
                fprintf(stderr, "Error: Please start a game before making a try.\n");
            } else {
                int try_res = try_function(udp_fd, udp_res, cmd, PLID, nT);

                if (try_res == OK) {
                    nT += 1;
                } else if (try_res == GAME_ENDED) {
                    PLID = NOT_PLAYING;
                    nT = 1;
                }
            }
        } else if (!strcmp(type, "quit")) {
            if (PLID == NOT_PLAYING) {
                fprintf(stderr, "Error: There is no active game.\n");
            } else {
                int quit_res = quit_function(udp_fd, udp_res, cmd, PLID);

                if (quit_res == GAME_ENDED) {
                    PLID = NOT_PLAYING;
                    nT = 1;
                }
            }
        } else if (!strcmp(type, "exit")) {
            if (PLID != NOT_PLAYING) {
                exit_function(udp_fd, udp_res, cmd, PLID);
            }
            running = FALSE;
        } else if (!strcmp(type, "debug")) {
            if (PLID == NOT_PLAYING) {
                PLID = debug_function(udp_fd, udp_res, cmd);
                lastPLID = PLID;
            } else {
                fprintf(stderr, "Error: You already have an ongoing game.\n");
            }
        } else if (!strcmp(type, "show_trials") || !strcmp(type, "st")) {
            if (lastPLID != NOT_PLAYING) {
                show_trials_function(tcp_res, cmd, type, lastPLID);
            } else {
                fprintf(stderr, "Error: You don't have an active/past game.\n");
            }
        } else if (!strcmp(type, "scoreboard") || !strcmp(type, "sb")) {
            scoreboard_function(tcp_res, cmd, type);
        } else {
            fprintf(stderr, "Error: Please provide a valid command.\n");
        }
    }

    freeaddrinfo(udp_res);
    close(udp_fd);

    return 0;
}