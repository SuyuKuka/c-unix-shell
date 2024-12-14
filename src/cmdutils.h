#ifndef CMDUTILS
#define CMDUTILS
#include "cmd.h"
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>   // includepath ?

cmd_info *parse_cmd(const char *cmd);

int execute_cmd(cmd_info *cmd);

#endif