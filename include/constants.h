#ifndef CONSTANTS_H
#define CONSTANTS_H

// Booleans and Function Return Valuess
#define FALSE               0
#define TRUE                1

#define FOUND 1
#define NOT_FOUND 0

#define ERROR -1
#define GAME_ENDED 1
#define OK 0


// Memory Related Constants
#define BUFFER_SIZE 1024
#define MAX_PLAYER_COMMAND  100
#define PLID_DIGITS         6               // '\0'
#define TIME_DIGITS         3               // '\0'
#define ONGOING_GAME_FILE_NAME 13           // '\0'
#define RESPONSE_LEN 3
#define COMMAND_LEN 3


// MAIN COMMANDS
#define SNG_CMD             "SNG"
#define TRY_CMD             "TRY"
#define QUT_CMD             "QUT"
#define DBG_CMD             "DBG"
#define STR_CMD             "STR"
#define SSB_CMD             "SSB"


// Connection related variables
#define localhost           "127.0.0.1"
#define GSPORT              "58000"
#define GROUP_NUMBER        28


// Player Loop Logic
#define NOT_PLAYING -1


// First Line File Arguments Index
#define ARG_PLID 0
#define ARG_MODE 1
#define ARG_SOLUTION 2
#define ARG_MAXTIME 3
#define ARG_DATE 4
#define ARG_TIME 5
#define ARG_ELAPSED_TIME 6


// Game Constants
#define MAX_ALLOWED_TIME 600
#define MAX_ALLOWED_PLAYS 8
#define MAX_TRIALS 8            // REMOVE THIS


// Ways of End Game
#define END_WIN 'W'
#define END_FAIL 'F'
#define END_QUIT 'Q'
#define END_TIMEOUT 'T'


// Simple Functions
#define max(A,B) ((A)>=(B)?(A):(B))

#endif
