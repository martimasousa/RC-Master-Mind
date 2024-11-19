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
#include "udp_io.h"


void process_sng_command(int udp_fd, struct sockaddr_in *client_addr, const char* command) {
    char *PLID, *time;
    char *response = "RSG OK";

    PLID = malloc(sizeof(char) * PLID_DIGITS);
    time = malloc(sizeof(char) * TIME_DIGITS);


    sscanf(command, "SNG %s %s", PLID, time);

    send_udp_response(udp_fd, response, client_addr);

    printf("PLID: %s\n", PLID);
    printf("time: %s\n", time);

    free(PLID);
    free(time);

    // TODO: Add Game Logic!
}

void process_try_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {
    char *PLID, C1, C2, C3, C4, nt;
    char *response = "RTR OK 1 1 1";

    PLID = malloc(sizeof(char) * PLID_DIGITS);

    sscanf(command, "TRY %s %c %c %c %c %c", PLID, &C1, &C2, &C3, &C4, &nt);
    
    send_udp_response(udp_fd, response, client_addr);

    printf("PLID: %s\n", PLID);
    printf("Colors: %c %c %c %c\n", C1, C2, C3, C4);
    printf("nt: %c\n", nt);

    free(PLID);

    // TODO: Add Game Logic!
}

void process_qut_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {
    char *PLID;
    char *response = "RQT OK";

    PLID = malloc(sizeof(char) * PLID_DIGITS);

    sscanf(command, "QUT %s", PLID);
    
    send_udp_response(udp_fd, response, client_addr);

    printf("PLID: %s\n", PLID);

    free(PLID);

    // TODO: Add Game Logic!
}

void process_dbg_command(int udp_fd, struct sockaddr_in *client_addr, const char *command) {
    char *PLID, *time, C1, C2, C3, C4;
    char *response = "RDB OK";

    PLID = malloc(sizeof(char) * PLID_DIGITS);
    time = malloc(sizeof(char) * TIME_DIGITS);
    
    printf("%s\n", command);
    sscanf(command, "DBG  %s %s %c %c %c %c", PLID, time, &C1, &C2, &C3, &C4);
    
    send_udp_response(udp_fd, response, client_addr);

    printf("PLID: %s\n", PLID);
    printf("time: %s\n", time);
    printf("Colors: %c %c %c %c\n", C1, C2, C3, C4);

    free(PLID);
    free(time);

    // TODO: Add Game Logic!
}

void process_command(int udp_fd, struct sockaddr_in *client_addr, const char *type, const char *command) {

    if (strcmp(SNG_CMD, type) == 0) {
        process_sng_command(udp_fd, client_addr, command);
        return;

    } else if (strcmp(TRY_CMD, type) == 0) {
        process_try_command(udp_fd, client_addr, command);
        return;

    } else if (strcmp(QUT_CMD, type) == 0) {
        process_qut_command(udp_fd, client_addr, command);
        printf("QUT!\n");
        return;

    } else if (strcmp(DBG_CMD, type) == 0) {
        process_dbg_command(udp_fd, client_addr, command);
        printf("QUT!\n");
        return;
    }
}

int handle_TCP_messages(int client_fd) 
{ 

    // TODO: Add commands logic

    return 1;
}

int gameLogic(int udp_fd)
{
    int playing = TRUE;
    char *command, *type;

    while (playing)
    {

        struct sockaddr_in *client_addr;
        command = malloc(sizeof(char) * BUFFER_SIZE);
        type = malloc(sizeof(char) * 3);

        int read = recv_udp_message(udp_fd, command, BUFFER_SIZE, client_addr);
        sscanf(command, "%s", type);
        process_command(udp_fd, client_addr, type, command);
        
        
        sleep(1);
        free(command);
    }
    
    return 1;
}


int handleServer(char *GSport, int is_verbose)
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
            }
            else if(pid==0){

                gameLogic(udp_fd);
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