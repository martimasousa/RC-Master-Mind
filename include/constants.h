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

#define GAME_INFO_DATE_LEN (YEAR_LEN + 1 + MONTH_LEN + 1 + DAY_LEN + 1 + HOUR_LEN + 1 + MINUTES_LEN + 1 + SECONDS_LEN)
#define GAME_INFO_MAX_LEN (GAME_INFO_DATE_LEN + PLID_DIGITS + 1 + MODE_LEN + 4 * COLOR_LEN + 1 + TIME_DIGITS + 1 + GAME_INFO_DATE_LEN + NOW_TIME_LEN)

#define PLID_DIGITS         6               // '\0'
#define TIME_DIGITS         3               // '\0'

#define RESPONSE_LEN 3
#define COMMAND_LEN 3

#define ELAPSED_TIME_LEN 3
#define SCORE_MAX_LEN 3
#define TRIAL_MAX_LEN 1

#define COLOR_LEN 1
#define YEAR_LEN 4
#define MONTH_LEN 2
#define DAY_LEN 2
#define HOUR_LEN 2
#define MINUTES_LEN 2
#define SECONDS_LEN 2
#define MODE_LEN 1
#define NOW_TIME_LEN 19 // MAX len a number from function "now" can have


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


// Ways of End Game
#define END_WIN 'W'
#define END_FAIL 'F'
#define END_QUIT 'Q'
#define END_TIMEOUT 'T'


// Game Modes
#define DEBUG_MODE 'D'
#define PLAY_MODE 'P'


// Simple Functions
#define max(A,B) ((A)>=(B)?(A):(B))

#endif
