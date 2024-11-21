#include "cd.h"


int cd(cmd_info *cmd) {
    static char last_dir[PATH_MAX] = ""; // ?
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL) { // Get le répertoire courant
        perror("getcwd");
        return EXIT_FAILURE;    
    }

    if (cmd->opt == NULL && cmd->args == NULL) {
        const char *src = getenv("SRC"); // Cas sans argument/option : aller à SRC
        if (src && chdir(src) == 0) {
            strncpy(last_dir, cwd, PATH_MAX);
            printf("Répertoire changé vers SRC : %s\n", src);
            return EXIT_SUCCESS;
        }
        fprintf(stderr, "Erreur : variable SRC non définie ou inaccessible.\n");
        return EXIT_FAILURE;
    }
    if (cmd->opt && strcmp(cmd->opt, "-") == 0) { // Cas option '-' : return au dernier répertoire
        if (strlen(last_dir) > 0 && chdir(last_dir) == 0) {
            strncpy(last_dir, cwd, PATH_MAX);
            printf("Répertoire changé vers le précédent : %s\n", last_dir);
            return EXIT_SUCCESS;
        }
        fprintf(stderr, "Erreur : aucun répertoire précédent défini.\n");
        return EXIT_FAILURE;
    }
    if (cmd->args) { // Cas avec un argument (chemin vers un répertoire)
        if (chdir(cmd->args) == 0) {
            strncpy(last_dir, cwd, PATH_MAX);
            printf("Répertoire changé vers : %s\n", cmd->args);
            return EXIT_SUCCESS;
        }
        fprintf(stderr, "Erreur : impossible d'accéder au répertoire '%s'.\n", cmd->args);
        return EXIT_FAILURE;
    }

    fprintf(stderr, "Erreur : commande invalide.\n");
    return EXIT_FAILURE;
}