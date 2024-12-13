#ifndef CMDUTILS
#define CMDUTILS
#include "cmd.h"
#include <unistd.h>

cmd_info *parse_cmd(const char *cmd);

int execute_cmd(cmd_info *cmd);

#endif