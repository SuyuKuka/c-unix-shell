#ifndef FSH
#define FSH
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "pwd.h"
#include "cmd.h"
#include "cd.h"
#include "ftype.h"
#include "externalcmd.h"
#include "for.h"
#include "if.h"
#include "redirection.h"
#define BUFFER_SIZE 512
#define PROMPT_SIZE 30
#define DEFAULT_COLOR "\033[00m"
#define GREEN "\001\033[32m\002"
#define RED "\001\033[91m\002"
#define CYAN "\001\033[36m\002"
int call_function (cmd_info *);
int handle_line(char *);
void color_prompt(char *, int);
void make_prompt(char *, int);
int display_prompt(int);
int fsh_exit(cmd_info *);
int exec_for_cmd(for_info *);
int for_no_opt(cmd_info *);

#endif