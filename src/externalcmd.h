#ifndef EXTERNALCMD
#define EXTERNALCMD 
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "cmd.h"


void destroy_split(char **, int *);
char ** split(cmd_info * , int * );
int externalcmd(cmd_info *);

#endif