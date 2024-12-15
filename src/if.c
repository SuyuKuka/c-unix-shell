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
        if (info->test) {
            destroy_cmd_info(info->test);
            info->test = NULL;
        }
        if (info->cmd_true) {
            destroy_cmd_info(info->cmd_true);
            info->cmd_true = NULL;
        }
        if (info->cmd_false) {
            destroy_cmd_info(info->cmd_false);
            info->cmd_false = NULL;
        }
        free(info);
    }
}

/*
* Analyse une ligne de commande, vérifie si il s'agit d'un if. 
* Return 0 si succès, 1 en cas d'erreur de syntaxe.
*/
int handle_if_line(char *line, if_info *info) { // check si parse_cmd() n'entraîne pas de pb vu que pointeurs 
    if(!line || !info) {
        fprintf(stderr, SYNTAX_ERR);
        return 1;
    }
    char *cursor = line;

    while (*cursor != '\0' && *cursor != ' ') cursor++; // Avance jusqu'au 1er espace puis check si mot=='if'
    if (strncmp(line, "if", cursor - line) != 0) {
        fprintf(stderr, SYNTAX_ERR);
        return 1;
    }
    cursor++; // on skip ' '

    char *test_start = cursor;
    while (*cursor != '\0' && *cursor != '{') cursor++; // Avance jusqu'à '{' puis check si on ne trouve pas fin
    if (*cursor == '\0') {
        fprintf(stderr, SYNTAX_ERR);
        return 1;
    }
    char *test_end = cursor;
    cursor++;

    // TEST
    size_t test_len = test_end - test_start;
    char *test = malloc(test_len + 1);  // +1 pour rajouter '\0'
    if (!test) {
        perror("malloc");
        return 1;
    }
    memcpy(test, test_start, test_len);
    test[test_len] = '\0';
    info->test = parse_cmd(test);
    free(test);
    if (!info->test) {  // Gestion échec pour TEST
        fprintf(stderr, "Erreur : parse_cmd a échoué pour le test.\n");
        destroy_if_info(info);
        return 1;
    }

    // CMD_TRUE
    char *cmd_true_start = cursor;
    while (*cursor != '\0' && *cursor != '}') cursor++; // Avance jusqu'à '}' puis check si on ne trouve pas fin
    if (*cursor == '\0') {
        fprintf(stderr, SYNTAX_ERR);
        return 1;
    }
    char *cmd_true_end = cursor;
    cursor++;

    size_t cmd_true_len = cmd_true_end - cmd_true_start;
    char *cmd_true = malloc(cmd_true_len + 1);
    if (!cmd_true) {
        perror("malloc");
        return 1;
    }
    memcpy(cmd_true, cmd_true_start, cmd_true_len);
    cmd_true[cmd_true_len] = '\0';
    info->cmd_true = parse_cmd(cmd_true);
    free(cmd_true);
    if (!info->cmd_true) {  // Gestion échec pour CMD_TRUE
        fprintf(stderr, "Erreur : parse_cmd a échoué pour la cmd_true.\n");
        destroy_if_info(info);
        return 1;
    }

    // Check si présence 'else'
    while (*cursor == ' ') cursor++; // Ignore espaces
    if (strncmp(cursor, "else", 4) == 0) {
        cursor += 4; // Skip le "else"
        while (*cursor == ' ') cursor++;

        // CMD_FALSE
        char *cmd_false_start = cursor;
        while (*cursor != '\0' && *cursor != '}') cursor++; // Avance jusqu'à '}' puis check si on ne trouve pas fin
        if (*cursor == '\0') {
            fprintf(stderr, SYNTAX_ERR);
            return 1;
        }
        char *cmd_false_end = cursor;

        size_t cmd_false_len = cmd_false_end - cmd_false_start;
        char *cmd_false = malloc(cmd_false_len + 1);
        if (!cmd_false) {
            perror("malloc");
            return 1;
        }
        memcpy(cmd_false, cmd_false_start, cmd_false_len);
        cmd_false[cmd_false_len] = '\0';
        info->cmd_false = parse_cmd(cmd_false);
        free(cmd_false);
        if (!info->cmd_false) {  // Gestion échec pour CMD_FALSE
            fprintf(stderr, "Erreur : parse_cmd a échoué pour la cmd_false.\n");
            destroy_if_info(info);
            return 1;
        }
    }

    return execute_if(info);
}

// Exec commande if. Return 0 si succès, 1 si échec.
int execute_if(if_info *info) {
    if (!info || !info->test || !info->cmd_true) { // Pas info, ou pas de condition ni de true
        fprintf(stderr, SYNTAX_ERR);
        return 1;
    }

    int test_result = execute_cmd(info->test);  // Exécute commande TEST
    if (test_result != 0) {
        if (info->cmd_false) {
            // TEST échoué et présence de CMD_FALSE -> CMD_FALSE
            return execute_cmd(info->cmd_false);
        }
        // TEST échoué, pas de CMD_FALSE
        return 0;
    }
    // TEST réussi -> CMD_TRUE
    return execute_cmd(info->cmd_true);
}

/*   Analyse une ligne de commande, vérifie si il s'agit d'un if.
* On scan token par token la ligne et remplit au fur et à mesure la struct if_info.
* Return 0 si succès, 1 en cas d'erreur de syntaxe.

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

    return execute_if(info);
}
*/
