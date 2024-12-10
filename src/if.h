#ifndef IF
#define IF
#include "cmd.h"
#include <unistd.h>

#define SYNTAX_ERROR "If invalide.\n"

typedef struct if_info {
    cmd_info *test;         // Commande TEST
    cmd_info *cmd_true;     // Commande CMD ou CMD_1
    cmd_info *cmd_false;    // Commande CMD_2 (si 'else')
} if_info;

if_info *if_info_init();
void destroy_if_info(if_info *info);

int handle_if_line(char *line, if_info *info);
int execute_if(if_info *info);

#endif