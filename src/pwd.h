#ifndef PWD
#define PWD
#define PWD_SIZE 512
#include "cmd.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int pwd(cmd_info *);

int redirect_pwd(int);

#endif 