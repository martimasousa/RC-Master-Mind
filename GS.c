#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FALSE   0
#define TRUE    1

int main(int argc, char* argv[]) {
    // TODO: Set default values
    int GSport = 58000;
    int is_verbose = FALSE;

    if (argc != 1 && argc != 2 && argc != 3 && argc != 4) {
        printf("Error while reading arguments.\nUsage: ./GS [-p GSport] [-v]\n");
        return 0;
    }

    if (argc == 2 && strcmp(argv[1], "-v")) {
        printf("Error while reading arguments.\nUsage: ./GS [-p GSport] [-v]\n");
        return 0;
    }

    if (argc == 3 && strcmp(argv[1], "-p")) {
        printf("Error while reading arguments.\nUsage: ./GS [-p GSport] [-v]\n");
        return 0;
    }

    if (argc == 4 && (strcmp(argv[1], "-p") || strcmp(argv[3], "-v"))) {
        printf("Error while reading arguments.\nUsage: ./GS [-p GSport] [-v]\n");
        return 0;
    }

    // TODO: Check if arguments are integers

    if (argc == 2 || argc == 4) {
        is_verbose = TRUE;
    }

    if (argc == 3 || argc == 4) {
        GSport = atoi(argv[2]);
    }

    printf("[Arguments]\nGSport: %d\nIs verbose: %d\n", GSport, is_verbose);
    return 0;
}