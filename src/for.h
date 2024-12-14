#ifndef FOR
#define FOR
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "cmd.h"
#define SYNTAX_ERROR "For invalide.\n"
#define FILE_SIZE 512
#define DIR_SIZE 512

typedef struct for_info {
    char * directory;
    char * command;
} for_info;

for_info * for_info_init();
void destroy_for_info(for_info *);

for_info * handle_for_line(char *);
void handle_for_cmd(for_info *, char *);
int replace_var(char *, char *);

#endif
