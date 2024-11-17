#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include "constants.h"
#include "utils.h"

int start_function(char cmd[MAX_PLAYER_COMMAND]) {
    char PLID_arg[6];
    char max_playtime_arg[3];
    char extra[100];

    // Read arguments and check for errors
    int result = sscanf(cmd, "start %s %s %s", PLID_arg, max_playtime_arg, extra);

    if (result != 2) {
        fprintf(stderr, "Error: Command format should be 'start PLID max_playtime'.\n");
        return 1;
    }

    if (!is_integer(PLID_arg) || !is_integer(max_playtime_arg)) {
        fprintf(stderr, "Error: Command format should be 'start PLID max_playtime'.\nHint: Make sure PLID and max_playtime are integers.\n");
        return 1;
    }

    // Convert arguments into integers
    int PLID = atoi(PLID_arg);
    int max_playtime = atoi(max_playtime_arg);

    if (max_playtime > 600) {
        fprintf(stderr, "Error: Command format should be 'start PLID max_playtime'.\nHint: max_playtime cannot exceed 600s.\n");
        return 1;
    }

    printf("%d %d\n", PLID, max_playtime);


    // TODO: Send message to server
    

    return 0;
}

int try_function(char cmd[MAX_PLAYER_COMMAND]) {
    char C1, C2, C3, C4;
    char extra[100];

    // Read arguments and check for errors
    int result = sscanf(cmd, "try %c %c %c %c %s", &C1, &C2, &C3, &C4, extra);

    if (result != 4) {
        fprintf(stderr, "Error: Command format should be 'try C1 C2 C3 C4'.\n");
        return 1;
    }

    if (!is_valid_color(C1) || !is_valid_color(C2) || !is_valid_color(C3) || !is_valid_color(C4)) {
        fprintf(stderr, "Error: Command format should be 'try C1 C2 C3 C4'.\nHint: The valid colours are: {red (R), green (G), blue (B), yellow (Y), orange (O) and purple (P)}.\n");
        return 1;
    }


    // TODO: Send message to server
    

    return 0;
}

int show_trials_function(char cmd[MAX_PLAYER_COMMAND], char type[MAX_PLAYER_COMMAND]) {
    char extra[100];

    // Read arguments and check for errors
    int result;
    if (!strcmp(type, "show_trials")) {
        result = sscanf(cmd, "show_trials %s", extra);
    } else {
        result = sscanf(cmd, "st %s", extra);
    }

    if (result == 1) {
        fprintf(stderr, "Error: Command format should be 'show_trials' or 'st'.\n");
        return 1;
    }


    // TODO: Send message to server
    

    return 0;
}

int scoreboard_function(char cmd[MAX_PLAYER_COMMAND], char type[MAX_PLAYER_COMMAND]) {
    char extra[100];

    // Read arguments and check for errors
    int result;
    if (!strcmp(type, "scoreboard")) {
        result = sscanf(cmd, "scoreboard %s", extra);
    } else {
        result = sscanf(cmd, "sb %s", extra);
    }

    if (result == 1) {
        fprintf(stderr, "Error: Command format should be 'scoreboard' or 'sb'.\n");
        return 1;
    }


    // TODO: Send message to server
    

    return 0;
}

int quit_function(char cmd[MAX_PLAYER_COMMAND]) {
    char extra[100];

    // Read arguments and check for errors
    int result = sscanf(cmd, "quit %s", extra);

    if (result == 1) {
        fprintf(stderr, "Error: Command format should be 'quit'.\n");
        return 1;
    }


    // TODO: Send message to server
    

    return 0;
}

int exit_function(char cmd[MAX_PLAYER_COMMAND]) {
    char extra[100];

    // Read arguments and check for errors
    int result = sscanf(cmd, "exit %s", extra);

    if (result == 1) {
        fprintf(stderr, "Error: Command format should be 'exit'.\n");
        return 1;
    }


    // TODO: Send message to server
    

    return 0;
}

int debug_function(char cmd[MAX_PLAYER_COMMAND]) {
    char PLID_arg[6];
    char max_playtime_arg[3];
    char C1, C2, C3, C4;
    char extra[100];

    // Read arguments and check for errors
    int result = sscanf(cmd, "debug %s %s %c %c %c %c %s", PLID_arg, max_playtime_arg, &C1, &C2, &C3, &C4, extra);

    if (result != 6) {
        fprintf(stderr, "Error: Command format should be 'debug PLID max_playtime C1 C2 C3 C4'.\n");
        return 1;
    }

    if (!is_integer(PLID_arg) || !is_integer(max_playtime_arg)) {
        fprintf(stderr, "Error: Command format should be 'debug PLID max_playtime C1 C2 C3 C4'.\nHint: Make sure PLID and max_playtime are integers.\n");
        return 1;
    }

    if (!is_valid_color(C1) || !is_valid_color(C2) || !is_valid_color(C3) || !is_valid_color(C4)) {
        fprintf(stderr, "Error: Command format should be 'debug PLID max_playtime C1 C2 C3 C4'.\nHint: The valid colours are: {red (R), green (G), blue (B), yellow (Y), orange (O) and purple (P)}.\n");
        return 1;
    }

    // Convert arguments into integers
    int PLID = atoi(PLID_arg);
    int max_playtime = atoi(max_playtime_arg);

    printf("%d %d\n", PLID, max_playtime);


    // TODO: Send message to server
    

    return 0;
}


int main(int argc, char* argv[]) {

    char *GSIP = localhost;
    int GSport = GSPORT;

    if (!((argc == 1) || 
        (argc == 3 && ((strcmp(argv[1], "-n") == 0 && is_valid_ip(argv[2])) || 
                        (strcmp(argv[1], "-p") == 0 && is_integer(argv[2])))) || 
        (argc == 5 && strcmp(argv[1], "-n") == 0 && is_valid_ip(argv[2]) && 
                        strcmp(argv[3], "-p") == 0 && is_integer(argv[4])))) {
        fprintf(stderr, "Error while reading arguments.\nUsage: ./player [-n GSIP] [-p GSport]\n");
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


    while (TRUE) {
        // Read command
        char cmd[MAX_PLAYER_COMMAND];
        fgets(cmd, MAX_PLAYER_COMMAND, stdin);

        // Remove the trailing newline character, if any
        cmd[strcspn(cmd, "\n")] = '\0';

        // Get command type
        char type[MAX_PLAYER_COMMAND];
        sscanf(cmd, "%s", type);

        // Execute respective function
        if (!strcmp(type, "start")) {
            if (start_function(cmd)) {
                printf("Error\n");
            }
        } else if (!strcmp(type, "try")) {
            if (try_function(cmd)) {
                printf("Error\n");
            }
        } else if (!strcmp(type, "show_trials") || !strcmp(type, "st")) {
            if (show_trials_function(cmd, type)) {
                printf("Error\n");
            }
        } else if (!strcmp(type, "scoreboard") || !strcmp(type, "sb")) {
            if (scoreboard_function(cmd, type)) {
                printf("Error\n");
            }
        } else if (!strcmp(type, "quit")) {
            if (quit_function(cmd)) {
                printf("Error\n");
            }
        } else if (!strcmp(type, "exit")) {
            if (exit_function(cmd)) {
                printf("Error\n");
            }
        } else if (!strcmp(type, "debug")) {
            if (debug_function(cmd)) {
                printf("Error\n");
            }
        } else {
            fprintf(stderr, "Error: Please provide a valid command.\n");
        }
    }

    return 0;
}