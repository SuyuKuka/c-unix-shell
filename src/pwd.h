#ifndef PWD
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "cmd.h"
#define PWD
#define PWD_SIZE 512

int pwd(cmd_info *);

int redirect_pwd(int);

#endif 