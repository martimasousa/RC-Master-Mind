#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "constants.h"
#include "utils.h"

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

    printf("[Arguments]\nGSport: %d\nIs verbose: %d\n", GSport, is_verbose);
    return 0;
}