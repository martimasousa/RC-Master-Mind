#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "constants.h"
#include "utils.h"
#include "udp_io.h"
#include "game_core.h"


int handleServer(char *GSport, int is_verbose) {
    int tcp_fd, udp_fd, client_fd, maxfd;
    struct sockaddr_in tcp_addr, udp_addr; //, client_addr;
    fd_set rfds, allfds;
    pid_t pid;

    // Creating and configuring TCP socket
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd < 0) {
        perror("Error creating TCP socket");
        exit(1);
    }
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_addr.sin_port = htons(atoi(GSport));
    if (bind(tcp_fd, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr)) < 0) {
        perror("Error binding TCP socket");
        exit(1);
    }
    listen(tcp_fd, SOMAXCONN);

    // Creating and configuring UDP socket
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        perror("Erro ao criar socket UDP");
        exit(1);
    }
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(atoi(GSport));
    if (bind(udp_fd, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0) {
        perror("Erro ao associar socket UDP");
        exit(1);
    }

    // Configuring fd set
    FD_ZERO(&allfds);
    FD_SET(tcp_fd, &allfds);
    FD_SET(udp_fd, &allfds);
    maxfd = max(tcp_fd, udp_fd);

    while (1) {
        // Client connection handler 
        rfds = allfds;
        int ready = select(maxfd + 1, &rfds, NULL, NULL, NULL); // TODO: Verify the arguments of this function
        if (ready < 0) {
            perror("Erro no select");
            exit(1);
        }

        // Verify new TCP connections
        if (FD_ISSET(tcp_fd, &rfds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            client_fd = accept(tcp_fd, (struct sockaddr *)&client_addr, &client_len);

            if (client_fd >= 0) {
                handle_TCP_messages(client_fd);
            }
        }

        // Verify UDP messages (Game Logic)
        if (FD_ISSET(udp_fd, &rfds)) {

            if ((pid = fork()) == -1) {
                perror("Error creating process with fork");
                exit(1);
            } else if (pid==0) { // CHILD PROCESS
                GameInfo *gameInfo = malloc(sizeof(GameInfo));
                gameInfo->udp_fd = udp_fd;
                gameInfo->playing = TRUE;
                gameInfo->client_addr = malloc(sizeof(struct sockaddr_in));
                char *command = malloc(sizeof(char) * BUFFER_SIZE);
                char *type = malloc(sizeof(char) * 3);

                while (gameInfo->playing) {
                    if (recv_udp_message(gameInfo->udp_fd, command, BUFFER_SIZE, gameInfo->client_addr) == -1) {
                        perror("Error reading command");
                        exit(1);
                    }
                    process_command(gameInfo, command);

                    printf("[gameLogic] Solution: %c %c %c %c\n", gameInfo->game_solution.colours[0], gameInfo->game_solution.colours[1], gameInfo->game_solution.colours[2], gameInfo->game_solution.colours[3]);
                }

                free(command);
                free(type);
                free(gameInfo->client_addr);
                free(gameInfo);
                
                //gameLogic(&gameInfo);
                //printf("[MAIN] Solution: %c %c %c %c\n", gameInfo->game_solution.colours[0], gameInfo->game_solution.colours[1], gameInfo->game_solution.colours[2], gameInfo->game_solution.colours[3]);

            } else { // PARENT PROCESS
                int status;
                wait(&status); // Wait for the child process to finish
                if (WIFEXITED(status)) {
                    // printf("Child exited with status %d\n", WEXITSTATUS(status));
                }
            }
        }
    }
    close(tcp_fd);
    close(udp_fd);
    return 0;
}

int main(int argc, char* argv[]) {

    char *GSport = GSPORT;
    int is_verbose = FALSE;

    if (!(argc == 1 || 
        (argc == 2 && strcmp(argv[1], "-v") == 0) || 
        (argc == 3 && strcmp(argv[1], "-p") == 0 && is_integer(argv[2])) || 
        (argc == 4 && strcmp(argv[1], "-p") == 0 && is_integer(argv[2]) && strcmp(argv[3], "-v") == 0))) {
        fprintf(stderr, "Error while reading arguments.\nUsage: ./GS [-p GSport] [-v]\n");
        return 0;
    }

    if (argc == 2 || argc == 4) {
        is_verbose = TRUE;
    }

    if (argc == 3 || argc == 4) {
        GSport = argv[2];
    }

    handleServer(GSport, is_verbose);

    printf("[Arguments]\nGSport: %s\nIs verbose: %d\n", GSport, is_verbose);
    return 0;
}