#include "cd.h"
#include <errno.h>

int cd(cmd_info *cmd) {
    static char last_dir[PATH_MAX] = "";
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) { // Récupère le répertoire courant
        perror("getcwd");
        return EXIT_FAILURE;    
    }

    if (strlen(cmd->args) == 0 && strlen(cmd->opt) == 0) { // Cas sans argument/option : retour à HOME
        const char *home = getenv("HOME");
        if (home && chdir(home) == 0) {
            strncpy(last_dir, cwd, PATH_MAX);
            return EXIT_SUCCESS;
        }
        perror("cd");
        return EXIT_FAILURE;
    }
    if (cmd->opt && strcmp(cmd->opt, "-") == 0) { // Cas option '-' : return au dernier répertoire
        if (strlen(last_dir) > 0 && chdir(last_dir) == 0) {
            strncpy(last_dir, cwd, PATH_MAX);
            return EXIT_SUCCESS;
        }
        fprintf(stderr, "cd: Aucun répertoire précédent enregistré\n");
        return EXIT_FAILURE;
    }

    if (cmd->args) { // Cas avec argument (chemin vers un répertoire)
        if (chdir(cmd->args) == 0) {
            strncpy(last_dir, cwd, PATH_MAX);
            return EXIT_SUCCESS;
        }
        perror("cd");
        return EXIT_FAILURE;
    }

    return EXIT_FAILURE;
}