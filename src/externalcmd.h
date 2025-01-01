#ifndef EXTERNALCMD
#define EXTERNALCMD 
#include "cmd.h"

void destroy_split(char **, int *);
char ** split(cmd_info * , int * );
int externalcmd(cmd_info *);
int new_exec(cmd_info *);

#endif