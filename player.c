#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    // TODO: Set default values
    int GSIP = 192;
    int GSport = 58000;

    if (argc != 1 && argc != 3 && argc != 5) {
        printf("Error while reading arguments.\nUsage: ./player [-n GSIP] [-p GSport]\n");
        return 0;
    }

    if (argc > 1 && strcmp(argv[1], "-n") && strcmp(argv[1], "-p")) {
        printf("Error while processing first argument.\nUsage: ./player [-n GSIP] [-p GSport]\n");
        return 0;
    }

    if (argc > 3 && strcmp(argv[3], "-p")) {
        printf("Error while processing second argument.\nUsage: ./player [-n GSIP] [-p GSport]\n");
        return 0;
    }

    if (argc == 3) {
        if (!strcmp(argv[1], "-n")) {
            GSIP = atoi(argv[2]);
        } else { // == "-p"
            GSport = atoi(argv[2]);
        }
    } else if (argc == 5) {
        GSIP = atoi(argv[2]);
        GSport = atoi(argv[4]);
    }

    printf("[Arguments]\nGSIP: %d\nGSport: %d\n", GSIP, GSport);
    return 0;
}