#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "aux_client.h"
#include "utils.h"
#include "constants.h"

int validate_start_command(const char *cmd, char *PLID_arg, char *max_playtime_arg) {

    // Use sscanf para extrair os dois argumentos
    int result = sscanf(cmd, "start %s %s", PLID_arg, max_playtime_arg);

    // Verifique se foram lidos exatamente 2 argumentos
    if (result != 2) {
        fprintf(stderr, "Error: Command format should be 'start PLID max_playtime'.\n");
        return ERROR;
    }

    // Calcula o final esperado do comando
    char *expected_end = (char *)cmd + strlen("start") + strlen(PLID_arg) + strlen(max_playtime_arg) + 3;

    // Verifica se o comando termina corretamente com '\n' ou '\0'
    while (*expected_end == ' ') expected_end++; // Ignora espaços extras
    if (*expected_end != '\n' && *expected_end != '\0') {
        fprintf(stderr, "Error: Command contains extra arguments or is malformed.\n");
        return ERROR;
    }

    // Verifica se PLID e max_playtime são números inteiros
    if (!is_integer(PLID_arg) || !is_integer(max_playtime_arg)) {
        fprintf(stderr, "Error: Command format should be 'start PLID max_playtime'.\nHint: Make sure PLID and max_playtime are integers.\n");
        return ERROR;
    }

    // Converte e valida os valores
    int PLID = atoi(PLID_arg);
    if (PLID < 100000 || PLID > 999999) {
        fprintf(stderr, "Error: Command format should be 'start PLID max_playtime'.\nHint: PLID has only 6 digits.\n");
        return ERROR;
    }

    int max_playtime = atoi(max_playtime_arg);
    if (max_playtime > 600 || max_playtime <= 0) {
        fprintf(stderr, "Error: Command format should be 'start PLID max_playtime'.\nHint: max_playtime must be between 1 and 600 seconds.\n");
        return ERROR;
    }

    return OK;
}


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
