#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include "constants.H"
#include "utils.h"

int main(int argc, char* argv[]) {

    char *GSIP = localhost;
    int GSport = GSPORT;

    if (!((argc == 1) || 
        (argc == 3 && ((strcmp(argv[1], "-n") == 0 && is_valid_ip(argv[2])) || 
                        (strcmp(argv[1], "-p") == 0 && is_integer(argv[2])))) || 
        (argc == 5 && strcmp(argv[1], "-n") == 0 && is_valid_ip(argv[2]) && 
                        strcmp(argv[3], "-p") == 0 && is_integer(argv[4])))) {
        printf("Error while reading arguments.\nUsage: ./player [-n GSIP] [-p GSport]\n");
        return 0;
    }

    if (argc == 3) {
        if (strcmp(argv[1], "-n") == 0) {
            GSIP = argv[2];
        } else {
            GSport = atoi(argv[2]);
        }
    } else if (argc == 5) {
        GSIP = argv[2];
        GSport = atoi(argv[4]);
    }

    printf("[Arguments]\nGSIP: %s\nGSport: %d\n", GSIP, GSport);
    return 0;
}