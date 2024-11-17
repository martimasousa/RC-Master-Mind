#include "utils.h"
#include "constants.H"
#include <stdio.h>
#include <ctype.h>
#include <arpa/inet.h>

// Função para verificar se a string é um número inteiro
int is_integer(const char *str) {
    while (*str) {
        if (!isdigit(*str)) return FALSE;
        str++;
    }
    return TRUE;
}

// Função para verificar se a string é um IP válido
int is_valid_ip(const char *ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) != 0;
}
