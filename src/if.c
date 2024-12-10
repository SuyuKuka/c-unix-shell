#include "if.h"

if_info *if_info_init() {   // Intialisation d'une struct if_info
    if_info *info = malloc(sizeof(if_info));
    if (!info) {
        perror("malloc");
        return NULL;
    }
    info->test = NULL;  // Test du if
    info->cmd_true = NULL;  // Commande effectuée si test = true
    info->cmd_false = NULL; // Si 'else', commande effectuée si test = false

    return info;
}

void destroy_if_info(if_info *info) {   // Libération de la mémoire associée à une struct if_info
    if (info) {
        if (info->test) destroy_cmd_info(info->test);
        if (info->cmd_true) destroy_cmd_info(info->cmd_true);
        if (info->cmd_false) destroy_cmd_info(info->cmd_false);
        free(info);
    }
}

/*
* Analyse une ligne de commande, vérifie si il s'agit d'un if. 
* On scan token par token la ligne et remplit au fur et à mesure la struct if_info.
* Return 0 si succès, 1 en cas d'erreur de syntaxe.
*/
int handle_if_line(char *line, if_info *info) {
    if (!line || !info) {   // Si l'un des deux arg est vide
        fprintf(stderr, SYNTAX_ERROR);
        return 1;
    }

    // Analyse (Adapter si besoin selon futur format des commandes structurées)
    char *if_token = strtok(line, " "); // Vérification 'if', strokk scan la ligne jusqu'au délimiteur ' ', if_token = if
    if (strcmp(if_token, "if") != 0) {
        fprintf(stderr, SYNTAX_ERROR);
        return 1;
    }

    // Récupération de la commande TEST
    char *test = strtok(NULL, "{"); // Vérification jusqu'à '{', test = commande de test
    if (!test) {
        fprintf(stderr, SYNTAX_ERROR);
        return 1;
    }
    info->test = parse_cmd(test);

    // Récupération de la commande CMD_TRUE
    char *cmd_true = strtok(NULL, "}"); // Vérification '}', cmd_true = commande à effectuer
    if (!cmd_true) {
        fprintf(stderr, SYNTAX_ERROR);
        return 1;
    }
    info->cmd_true = parse_cmd(cmd_true);

    // Vérifie s'il y a un "else"
    char *else_token = strtok(NULL, " ");   // Vérification jusqu'à ' '
    if (else_token && strcmp(else_token, "else") == 0) {    // Si else_token existe et = else,
        char *cmd_false = strtok(NULL, "}");    // Revérification jusqu'à '}'
        if (!cmd_false) {
            fprintf(stderr, SYNTAX_ERROR);
            return 1;
        }
        info->cmd_false = parse_cmd(cmd_false);
    }

    return 0;
}

// Exec commande if. Return 0 si succès, 1 si échec.
int execute_if(if_info *info) {
    if (!info || !info->test || !info->cmd_true) { // Pas info, ou pas de condition ni de true
        fprintf(stderr, SYNTAX_ERROR);
        return 1;
    }

    int test_result = execute_cmd(info->test);  // Exécute commande TEST
    if (test_result == 0) {
        // TEST réussi -> CMD_TRUE
        return execute_cmd(info->cmd_true);
    } else if (info->cmd_false) {
        // TEST échoué et présence de CMD_FALSE -> CMD_FALSE
        return execute_cmd(info->cmd_false);
    }

    // TEST échoué, pas de CMD_FALSE
    return 0;
}
