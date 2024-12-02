#ifndef AUX_CLIENT_CORE_H
#define AUX_CLIENT_CORE_H

#include "constants.h"

int validate_start_command(const char *cmd, char *PLID_arg, char *max_playtime_arg);
int validate_try_command(const char *cmd, char *C1, char *C2, char *C3, char *C4);


#endif