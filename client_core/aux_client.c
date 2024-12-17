#include "aux_client.h"

/*
* UDP related functions
*/
ssize_t send_receive_udp(int udp_fd, struct addrinfo *udp_res, const char *msg, size_t msg_len, char *response, size_t resp_len) {
    // Send message
    ssize_t n = sendto(udp_fd, msg, msg_len, 0, udp_res->ai_addr, udp_res->ai_addrlen);
    if (n == ERROR) {
        fprintf(stderr, "Error: Error while sending UDP message.\n");
        return ERROR;
    }

    // Receive response
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    n = recvfrom(udp_fd, response, resp_len, 0, (struct sockaddr*)&addr, &addrlen);
    if (n == ERROR) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            fprintf(stderr, "Error: Receive operation timed out.\n");
        } else {
            fprintf(stderr, "Error: Error while receiving UDP message.\n");
        }
        return ERROR;
    }

    return n;
}

/*
* TCP related functions
*/
int create_tcp_socket(int *tcp_fd) {
    // Create TCP socket
    *tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*tcp_fd == ERROR) {
        fprintf(stderr, "Error: Error while creating TCP socket.");
        return ERROR;
    }

    // Configuring socket timeout
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SECONDS;
    timeout.tv_usec = 0;
    if (setsockopt(*tcp_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        fprintf(stderr, "Error: setsockopt(SO_RCVTIMEO) failed");
        return ERROR;
    }

    return OK;
}

int connect_to_server(int tcp_fd, struct addrinfo *tcp_res) {

    // Connect to the server
    ssize_t n = connect(tcp_fd, tcp_res->ai_addr, tcp_res->ai_addrlen);
    if (n == ERROR) {
        fprintf(stderr, "Error: Error while trying to establish a connection with GS.\n");
        return ERROR;
    }
    return OK;
}

int send_message(int tcp_fd, char *msg) {

    // Send message to server
    if (tcp_write(tcp_fd, msg)) {
        fprintf(stderr, "Error: Error while writing to TCP socket.\n");
        return ERROR;
    }
    return OK;
}

int receive_response(int tcp_fd, char **response) {

    // Receive server response with server formated response
    if (tcp_read_until_delimiter(tcp_fd, response, ' ', 2)) {
        return ERROR;
    }
    return OK;
}

int store_file_local(char *Fname, char *Fdata) {
    char file_path[BUFFER_SIZE] = "./CLIENT_CACHE/";

    // Create the directory
    if (mkdir(file_path, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Error: Error while creating '%s' directory.\n", file_path);
        free(Fdata);
        free(Fname);
        return ERROR;
    }

    // Write on the local file
    strcat(file_path, Fname);
    if (write_to_file(file_path, Fdata) == -1) {
        fprintf(stderr, "Error: Error while writing to local file.\n");
        free(Fdata);
        free(Fname);
        return ERROR;
    }

    printf("Local copy stored at: %s\n", file_path);

    return OK;
}

void show_show_trials_content(char *Fdata) {
    printf("------------- TRIES: -------------\n");
    printf("%s", Fdata);
    printf("----------------------------------\n");
}

void show_scoreboard_content(char *Fdata) {
    printf("---------- SCOREBOARD: ----------\n");
    printf("%s", Fdata);
    printf("---------------------------------\n");
}