#ifndef CMD
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CMD
#define CMD_SIZE 64
#define OPTIONS_SIZE 64
#define ARG_SIZE 512
#define TOTAL 640

typedef struct cmd_info {
    char * name;
    char * opt;
    char * args;
} cmd_info;

cmd_info * cmd_info_init();

void destroy_cmd_info(cmd_info *);

#endif 