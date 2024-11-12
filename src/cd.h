#ifndef CD
#define CD
#define SRC = $SRC

#include <limits.h>  // Pour PATH_MAX

#define MAX_PATH_LENGTH PATH_MAX

int cd(char ** argv);
int change_dir(const char *path);

#endif