#ifndef FOR
#define FOR
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "cmd.h"
#define SYNTAX_ERROR "For invalide.\n"
#define TOO_MANY_OPT "Nombre d'options invalide.\n"
#define INVALID_OPT "Options invalides.\n"
#define INVALID_EXT "Extension invalide.\n"
#define INVALID_TYPE "Option -t type invalide.\n"
#define FILE_SIZE 512
#define DIR_SIZE 512
#define MAX_OPTIONS 8 //On compte aussi les dépendances des options ex: -e EXT = 2 
#define OPTIONS_SIZE 512

enum FOR_TYPE { FOR_FILE, FOR_DIRECTORY };

typedef struct for_info {
    char * directory;
    char * command;
    char ** options;
    int opt_size;
    //Des flags pour les options.
    int opt_a;
    int opt_r;

    int opt_ext;
    char * ext;

    int * opt_type;
    char file_type;

    int opt_p;
    int p_max;
    enum FOR_TYPE for_type;
} for_info;

for_info * for_info_init();
void destroy_for_info(for_info *);

for_info * handle_for_line(char *);
void handle_for_cmd(for_info *, char *);
char * replace_var(char *, char *);

#endif
