#ifndef CD
#define CD
#define SRC = $SRC

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "cmd.h"

#define MAX_PATH_LENGTH PATH_MAX

int cd(cmd_info * cmd);

#endif