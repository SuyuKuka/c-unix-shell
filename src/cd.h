#ifndef CD
#define CD
#define SRC = $SRC

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_PATH_LENGTH PATH_MAX

int cd(char ** argv);
//int change_dir(const char *path);

#endif