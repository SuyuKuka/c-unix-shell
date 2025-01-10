#ifndef CMDUTILS
#define CMDUTILS
#include "cmd.h"
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include "fsh.h"

char *trim(char *str);

char *strip_brackets(char *);

char *custom_strndup(const char *, size_t);

char *extract_block(const char *, int *);

#endif