#include <string.h>
#include <stdio.h>
#include <netdb.h>

#include "constants.h"
#include "client_core.h"
#include "aux_client.h"
#include "utils.h"

int start_function(int udp_fd, struct addrinfo *udp_res, char cmd[MAX_PLAYER_COMMAND]) {

    char PLID_arg[PLID_DIGITS];
    char max_playtime_arg[TIME_DIGITS];

    // Valida o comando usando a nova função
    if (validate_start_command(cmd, PLID_arg, max_playtime_arg) == ERROR) {
        return ERROR;
    }

    // Create message to send
    char msg[15];
    snprintf(msg, sizeof(msg), "SNG %s %s", PLID_arg, max_playtime_arg);

    // Send message to server
    ssize_t n = sendto(udp_fd, msg, sizeof(msg), 0, udp_res->ai_addr, udp_res->ai_addrlen);
    if (n == ERROR) {
        fprintf(stderr, "Error while sending message.\n");
        return ERROR;
    }

    // Receive response from server
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char response[8];
    n = recvfrom(udp_fd, response, sizeof(response), 0, (struct sockaddr*)&addr, &addrlen);
    if (n == ERROR) {
        fprintf(stderr, "Error while receiving message.\n");
        return ERROR;
    }

    char response_status[RESPONSE_STATUS];
    sscanf(response, "RSG %s", response_status);

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