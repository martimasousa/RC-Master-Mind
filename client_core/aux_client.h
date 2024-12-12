#ifndef AUX_CLIENT_CORE_H
#define AUX_CLIENT_CORE_H

#include "constants.h"

int validate_try_command(const char *cmd, char *C1, char *C2, char *C3, char *C4);
int validate_quit_command(const char *cmd);
int validate_exit_command(const char *cmd);


#endif