#ifndef IF
#define IF
#include "cmd.h"
#include "cmdutils.h"
#include <unistd.h>

#define SYNTAX_ERR "If invalide.\n"

typedef struct if_info {
    cmd_info *test;         // Commande TEST; voir histoire de test pipeline
    char *cmd_true;     // Commande CMD ou CMD_1
    char *cmd_false;    // Commande CMD_2 (si 'else')
} if_info;

if_info *if_info_init();
void destroy_if_info(if_info *info);

if_info *handle_if_line(char *line);
int execute_if(if_info *info);

#endif