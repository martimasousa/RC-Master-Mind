#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "aux_client.h"
#include "utils.h"
#include "constants.h"


int validate_try_command(const char *cmd, char *C1, char *C2, char *C3, char *C4) {
    // Use sscanf para extrair os 4 caracteres
    int result = sscanf(cmd, "try %c %c %c %c", C1, C2, C3, C4);

    // Certifique-se de que foram lidos exatamente 4 caracteres
    if (result != 4) {
        fprintf(stderr, "Error: Command format should be 'try C1 C2 C3 C4'.\n");
        return ERROR;
    }

    // Calcula o final esperado do comando
    const char *expected_end = cmd + strlen("try") + 4 * 2 + 1; // "try" + espaços e caracteres
    while (*expected_end == ' ') expected_end++;  // Ignora espaços adicionais

    // Verifica se o comando termina corretamente com '\n' ou '\0'
    if (*expected_end != '\n' && *expected_end != '\0') {
        fprintf(stderr, "Error: Command contains extra arguments or is malformed.\n");
        return ERROR;
    }

    // Verifica se os caracteres lidos são cores válidas
    if (!is_valid_color(*C1) || !is_valid_color(*C2) || !is_valid_color(*C3) || !is_valid_color(*C4)) {
        fprintf(stderr, "Error: Command format should be 'try C1 C2 C3 C4'.\n");
        fprintf(stderr, "Hint: Valid colors are: red (R), green (G), blue (B), yellow (Y), orange (O), and purple (P).\n");
        return ERROR;
    }

    // Tudo está válido
    return OK;
}


int validate_quit_command(const char *cmd) {
    // Verifica se o comando começa com "quit"
    if (strncmp(cmd, "quit", strlen("quit")) != 0) {
        fprintf(stderr, "Error: Command should start with 'quit'.\n");
        return ERROR;
    }

    // Calcula o final esperado do comando
    const char *expected_end = cmd + strlen("quit");
    while (*expected_end == ' ') expected_end++; // Ignora espaços adicionais

    // Verifica se o comando termina corretamente com '\n' ou '\0'
    if (*expected_end != '\n' && *expected_end != '\0') {
        fprintf(stderr, "Error: Command contains extra arguments or is malformed.\n");
        return ERROR;
    }

    // Comando válido
    return OK;
}


int validate_exit_command(const char *cmd) {
    // Verifica se o comando começa com "exit"
    if (strncmp(cmd, "exit", strlen("exit")) != 0) {
        fprintf(stderr, "Error: Command should start with 'exit'.\n");
        return ERROR;
    }

    // Calcula o final esperado do comando
    const char *expected_end = cmd + strlen("exit");
    while (*expected_end == ' ') expected_end++; // Ignora espaços adicionais

    // Verifica se o comando termina corretamente com '\n' ou '\0'
    if (*expected_end != '\n' && *expected_end != '\0') {
        fprintf(stderr, "Error: Command contains extra arguments or is malformed.\n");
        return ERROR;
    }

    // Comando válido
    return OK;
}
