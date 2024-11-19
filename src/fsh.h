#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "pwd.h"
#include "cmd.h"
#define BUFFER_SIZE 512
#define PROMPT_SIZE 30
#define DEFAULT_COLOR "\033[00m"
#define GREEN "\001\033[32m\002"
#define RED "\001\033[91m\002"
#define CYAN "\001\033[36m\002"
void cmd_format(char * line, cmd_info * cmd);
int call_function (cmd_info * cmd);
int handle_line(char * line);
void color_prompt(char * prompt, int var);
void make_prompt(char * prompt, int var);
int display_prompt(int var);
int fsh_exit(cmd_info * cmd);