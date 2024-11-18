#include "udp_io.h"

ssize_t recv_udp_message(int udp_fd, char *buffer, size_t buffer_size, struct sockaddr_in *client_addr) {
    socklen_t client_len = sizeof(*client_addr);
    ssize_t n = recvfrom(udp_fd, buffer, buffer_size - 1, 0, (struct sockaddr *)client_addr, &client_len);
    
    if (n > 0) {
        buffer[n] = '\0';
    } else return -1;

    return n;
}

void send_udp_response(int udp_fd, const char *message, struct sockaddr_in *client_addr) {
    socklen_t client_len = sizeof(*client_addr);
    sendto(udp_fd, message, strlen(message), 0, (struct sockaddr *)client_addr, client_len);
}