#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "constants.h"
#include "utils.h"

#define BUFFER_SIZE 1024

int handleServer(int GSport, int is_verbose)
{
    int tcp_fd, udp_fd, client_fd, maxfd;
    struct sockaddr_in tcp_addr, udp_addr, client_addr;
    fd_set rfds, allfds;
    char buffer[BUFFER_SIZE];
    pid_t pid;

    // Creating and configuring TCP socket
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd < 0) {
        perror("Error creating TCP socket");
        exit(1);
    }
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_addr.sin_port = htons(GSport);
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
    udp_addr.sin_port = htons(GSport);
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
                printf("Nova conexão TCP de %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                // FD_SET(client_fd, &allfds); TODO: Verify if this line is necessary (not necessary if TCP requests are not used for real-time game commands) 
                //if (client_fd > maxfd) maxfd = client_fd; // Same as above
            }
        }

        // Verify UDP messages (Game Logic)
        if (FD_ISSET(udp_fd, &rfds)) {

            if ((pid = fork()) == -1) {
                perror("Error creating process with fork");
                exit(1);
            }
            else if(pid==0){

                // TODO: Eliminar o que está aqui: isto é apenas para testes
                // Nesta parte só vai estar a função do gameLogic()
                socklen_t client_len = sizeof(client_addr);
                int n = recvfrom(udp_fd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&client_addr, &client_len);
                if (n > 0) {
                    buffer[n] = '\0';
                    printf("Mensagem UDP de %s:%d - %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);
                    // sendto NOT WORKING!! TODO: Verify why (sending messages do client will be needed!)
                    sendto(udp_fd, "Mensagem recebida\n", 18, 0, (struct sockaddr *)&client_addr, client_len);
                }

                while (1) {
                    
                    // TODO: Add Game Logic in a function that has a loop like this one
                    printf("Handling Client\n");
                    sleep(1);
                }
            }
        }
    }
    close(tcp_fd);
    close(udp_fd);
    return 0;
}
int main(int argc, char* argv[]) {

    int GSport = GSPORT;
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
        GSport = atoi(argv[2]);
    }

    handleServer(GSport, is_verbose);

    printf("[Arguments]\nGSport: %d\nIs verbose: %d\n", GSport, is_verbose);
    return 0;
}