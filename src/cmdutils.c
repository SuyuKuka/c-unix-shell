#include "cmdutils.h"
/*
void trim(char *str) {
    if (str == NULL) return;
    while (isspace((unsigned char)*str)) str++; // Supprime les espaces à gauche
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;    // Supprime les espaces à droite
    *(end + 1) = '\0';  // Ajoute un caractère de fin de chaîne
}*/
char *trim(char *str) {
    if (!str) return NULL;
    char *start = str;
    while (isspace((unsigned char)*start)) start++;
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return start;  // Retourner un pointeur vers la partie "trimmed"
}

cmd_info *parse_cmd(const char *cmd) {
    if (!cmd || *cmd == '\0') {
    return NULL;    // CMD vide/invalide.
}
    cmd_info *info = malloc(sizeof(cmd_info));
    if (!info) {
        perror("malloc");
        return NULL;
    }
    info->name = NULL;
    info->opt = NULL;
    info->args = NULL;

    const char *cursor = cmd;   // Début du nom de la commande
    while (*cursor == ' ') {
        cursor++;
    }
    const char *name_start = cursor;
    while (*cursor != ' ' && *cursor != '\0') { // Fin du nom de la commande
        cursor++;
    }
    size_t name_len = cursor - name_start;

    if (name_len > 0) { // Allouer & copier nom
        info->name = malloc(name_len + 1);
        if (!info->name) {
            perror("malloc");
            destroy_cmd_info(info);
            return NULL;
        }
        memcpy(info->name, name_start, name_len);
        info->name[name_len] = '\0';
        info->name = trim(info->name);
    }

    while (*cursor == ' ') {    // Début de l'option
        cursor++;
    }
    const char *opt_start = cursor;
    while (*cursor != ' ' && *cursor != '\0') { // Fin de l'option
        cursor++;
    }
    size_t opt_len = cursor - opt_start;

    if (opt_len > 0) {  // Allouer & copier option
        info->opt = malloc(opt_len + 1);
        if (!info->opt) {
            perror("malloc");
            //free(info->name);
            destroy_cmd_info(info);
            return NULL;
        }
        memcpy(info->opt, opt_start, opt_len);
        info->opt[opt_len] = '\0';
        info->opt = trim(info->opt);
    }

    while (*cursor == ' ') {    // Début des arguments
        cursor++;
    }
    const char *args_start = cursor;
    if (*args_start != '\0') {  // Allouer & copier arguments (tout reste de la chaîne)
        size_t args_len = strlen(args_start);
        info->args = malloc(args_len + 1);
        if (!info->args) {
            perror("malloc");
            //free(info->name);
            //free(info->opt);
            printf("PB | destroy_cmd_info if(!info->args) dans parse\n");
            destroy_cmd_info(info);
            return NULL;
        }
        memcpy(info->args, args_start, args_len);
        info->args[args_len] = '\0';
        info->args = trim(info->args);
    }

    return info;
}


int execute_cmd(cmd_info *cmd) {
    if (!cmd || !cmd->name) {
        fprintf(stderr, "Commande invalide\n");
        return 1;
    }
    size_t argc = 0;
    if (cmd->opt) argc++;
    if (cmd->args) argc++;

    char **argv = malloc((argc + 2) * sizeof(char *));  // +1 pour le nom, +1 pour NULL
    if (!argv) {
        perror("malloc");
        return 1;
    }
    argv[0] = cmd->name;  // Nom commande
    size_t index = 1;
    if (cmd->opt) {
        argv[index] = cmd->opt;  // Option
        index++;
    }
    if (cmd->args) {
        argv[index] = cmd->args;  // Arguments
        index++;
    }
    argv[index] = NULL;  // Terminaison

    pid_t pid = fork(); // Création child process pour exe la commande
    if (pid < 0) {
        perror("fork");
        printf("free argv si pid < 0\n");
        free(argv);
        return 1;
    }
    if (pid == 0) {
        // Process child : exe la commande
        if (cmd->name) {
            printf("Commande à exécuter :%s\n", (char *)cmd->name);
        }
        for (size_t i = 0; argv[i] != NULL; i++) {
            printf("Argument %zu :%s\n", i, argv[i]);
        }
        execvp(cmd->name, argv);
        perror("execvp");  // Si execvp échoue
        printf("free argv dans pid == 0\n");
        free(argv);
        exit(1);
    }
    
    int status; // Process parent : wait fin de l'exe du child
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        printf("free argv dans waitpid\n");
        free(argv);
        return 1;
    }
    printf("free argv vers fin de execute_cmd\n");
    free(argv);

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return 0;   // succès
    }
    return 1;   // échec
}