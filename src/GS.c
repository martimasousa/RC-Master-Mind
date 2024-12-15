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
#include <fcntl.h>
#include "constants.h"
#include "utils.h"
#include "game_core.h"


typedef struct {
    char ipv4[INET_ADDRSTRLEN];
    int port;
} IP_INFO;


IP_INFO get_sender_info(int sock_fd, int is_udp) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    IP_INFO result;

    // Initialize result with default "unknown" values
    strcpy(result.ipv4, "Unknown");
    result.port = -1;

    // Set socket to non-blocking mode
    int flags = fcntl(sock_fd, F_GETFL, 0);
    fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);

    if (is_udp) {
        // For UDP, use recvfrom to get sender's address
        char dummy_buffer[1]; // Minimal buffer to satisfy recvfrom
        ssize_t bytes_received = recvfrom(sock_fd, dummy_buffer, sizeof(dummy_buffer), MSG_PEEK,
                                           (struct sockaddr *)&addr, &addr_len);
        if (bytes_received == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No data available, return default "unknown"
                return result;
            }
            fprintf(stderr, "recvfrom failed");
            return result;
        }
    } else {
        // For TCP, use getpeername to get the peer's address
        if (getpeername(sock_fd, (struct sockaddr *)&addr, &addr_len) == -1) {
            fprintf(stderr, "getpeername failed");
            return result;
        }
    }

    // Convert the IP address to a human-readable string
    if (inet_ntop(AF_INET, &addr.sin_addr, result.ipv4, sizeof(result.ipv4)) == NULL) {
        fprintf(stderr, "inet_ntop failed");
        return result;
    }

    // Convert the port from network byte order to host byte order
    result.port = ntohs(addr.sin_port);

    return result;
}


int handleServer(char *GSport, int is_verbose) {
    int tcp_fd, udp_fd, client_fd, maxfd;
    struct sockaddr_in tcp_addr, udp_addr; //, client_addr;
    fd_set rfds, allfds;


    // ############################################################################################
    // ### TCP ####################################################################################
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


    // ############################################################################################
    // ### UDP ####################################################################################
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
            fprintf(stderr, "Error during select execution...");
            exit(1); // TODO: ???
        }

        // Verify new TCP connections
        if (FD_ISSET(tcp_fd, &rfds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            // Accept new connection
            client_fd = accept(tcp_fd, (struct sockaddr *)&client_addr, &client_len);
            
            if (client_fd >= 0) {
                // Create a child process to handle the client
                pid_t pid = fork();

                if (pid == 0) { // Child process
                    close(tcp_fd); // Close listening socket in child

                    // Get sender information
                    IP_INFO sender_info = get_sender_info(client_fd, FALSE);

                    // Read command (whole line)
                    char *command = NULL;
                    tcp_read_until_delimiter(client_fd, &command, '\n', 1);

                    // If verbose option set, print request details
                    if (is_verbose) {
                        // Get values from command
                        char req_type[COMMAND_LEN + 1];
                        char req_PLID[PLID_DIGITS + 1];
                        sscanf(command, "%s %s", req_type, req_PLID);

                        printf("[NEW REQUEST] PLID: %s | TYPE: %s | SENDER: %s:%d\n", req_PLID, req_type, sender_info.ipv4, sender_info.port);
                    }

                    // Handle client messages
                    process_command_tcp(client_fd, command);

                    free(command);
                    close(client_fd); // Close client socket
                    exit(0); // Exit child process
                } else if (pid > 0) { // Parent process
                    close(client_fd); // Close client socket in parent
                } else {
                    fprintf(stderr, "Error during fork execution...");
                    close(client_fd); // Clean up on fork failure
                }
            } else {
                fprintf(stderr, "Error during accept execution...");
            }
        }

        // Verify UDP messages (Game Logic)
        if (FD_ISSET(udp_fd, &rfds)) {
            struct sockaddr_in *client_addr = malloc(sizeof(struct sockaddr_in));
            char *command = malloc(sizeof(char) * BUFFER_SIZE);

            // Get sender information
            IP_INFO sender_info = get_sender_info(udp_fd, TRUE);

            if (recv_udp_message(udp_fd, command, BUFFER_SIZE, client_addr) == -1) {
                perror("Error reading command");
                exit(1);
            }

            // If verbose option set, print request details
            if (is_verbose) {
                // Get values from command
                char req_type[COMMAND_LEN + 1];
                char req_PLID[PLID_DIGITS + 1];
                sscanf(command, "%s %s", req_type, req_PLID);

                printf("[NEW REQUEST] PLID: %s | TYPE: %s | SENDER: %s:%d\n", req_PLID, req_type, sender_info.ipv4, sender_info.port);
            }

            process_command_udp(udp_fd, client_addr, command);

            free(command);
            free(client_addr);
            
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

    // printf("[Arguments]\nGSport: %s\nIs verbose: %d\n", GSport, is_verbose);
    return 0;
}