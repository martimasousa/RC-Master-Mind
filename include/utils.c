#include "utils.h"
#include "constants.h"
#include <stdio.h>
#include <ctype.h>
#include <arpa/inet.h>

int is_integer(const char *str) {
    while (*str) {
        if (!isdigit(*str)) return FALSE;
        str++;
    }
    return TRUE;
}

int is_valid_ip(const char *ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) != 0;
}

int is_valid_color(char C) {
    if (C != 'R' && C != 'G' && C != 'B' && C != 'Y' && C != 'O' && C != 'P') {
        return FALSE;
    }

    return TRUE;
}
