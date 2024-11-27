#ifndef CMD
#define CMD
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define CMD_SIZE 512
#define OPTIONS_SIZE 512
#define ARG_SIZE 512
#define TOTAL 3*512

typedef struct cmd_info {
    char * name;
    char * opt;
    char * args;
} cmd_info;

cmd_info * cmd_info_init();

void destroy_cmd_info(cmd_info *);

void cmd_format(char *, cmd_info *);

#endif 