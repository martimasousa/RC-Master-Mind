#ifndef CONSTANTS_H
#define CONSTANTS_H

#define FALSE               0
#define TRUE                1

#define SNG_CMD             "SNG"
#define TRY_CMD             "TRY"
#define QUT_CMD             "QUT"
#define DBG_CMD             "DBG"

#define localhost           "127.0.0.1"
#define GSPORT              "58000"
#define GROUP_NUMBER        0               // TODO: Put the right value once we know our GROUP_NUMBER

#define PLID_DIGITS         6 + 1           // '\0'
#define TIME_DIGITS         3 + 1           // '\0'

#define ONGOING_GAME_FILE_NAME 13           // '\0'

#define FOUND 1
#define NOT_FOUND 0

#define MAX_TRIALS 8

// First line file arguments index
#define ARG_PLID 0
#define ARG_MODE 1
#define ARG_SOLUTION 2
#define ARG_MAXTIME 3
#define ARG_DATE 4
#define ARG_TIME 5
#define ARG_ELAPSED_TIME 6


#define max(A,B) ((A)>=(B)?(A):(B))
#define BUFFER_SIZE 1024

#define MAX_PLAYER_COMMAND  100             // TODO: Verify

#endif
