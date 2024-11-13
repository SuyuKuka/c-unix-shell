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
#define GREEN "\033[32m"
#define RED "\033[91m"
#define CYAN "\033[36m"