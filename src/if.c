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

/*void destroy_if_info(if_info *info) {   // Libération de la mémoire associée à une struct if_info
    if (info) {
        if (info->test) {
            if (info->test->name) free(info->test->name); // Libère 'name', pob avec destroy_if_info de type
            if (info->test->opt) free(info->test->opt);
            if (info->test->args) free(info->test->args);
    
            printf("destroy_if_info | info->test\n");
            destroy_cmd_info(info->test);
            info->test = NULL;
        }
        if (info->cmd_true && info->cmd_true != info->test) {
            printf("destroy_if_info | info->cmd_true\n");
            destroy_cmd_info(info->cmd_true);
            info->cmd_true = NULL;
        }
        if (info->cmd_false && info->cmd_false != info->test && info->cmd_false != info->cmd_true) {
            printf("destroy_if_info | info->cmd_false\n");
            destroy_cmd_info(info->cmd_false);
            info->cmd_false = NULL;
        }
        free(info);
    }
}
*/
void destroy_if_info(if_info *info) {
    if (info) {
        if (info->test) {
            destroy_cmd_info(info->test);
            info->test = NULL;
        }
        if (info->cmd_true) {
            if (info->cmd_true != info->test) {
                printf("destroy_if_info | info->cmd_true\n");
                destroy_cmd_info(info->cmd_true);
            }
            info->cmd_true = NULL;
        }
        if (info->cmd_false) {
            if (info->cmd_false != info->test && info->cmd_false != info->cmd_true) {
                printf("destroy_if_info | info->cmd_false\n");
                destroy_cmd_info(info->cmd_false);
            }
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

    if (*cursor != '[') {
        fprintf(stderr, "Erreur : condition mal formée, '[' manquant.\n");
        return 1;
    }
    cursor++; // Passe '['

    char *test_start = cursor;
    size_t bracket_balance = 1;

    while (*cursor != '\0' && bracket_balance > 0) {
        if (*cursor == '[') bracket_balance++;
        if (*cursor == ']') bracket_balance--;
        cursor++;
    }
    if (bracket_balance != 0) {
        fprintf(stderr, "Erreur : problème de crochets dans la condition TEST.\n");
        return 1;
    }
    char *test_end = cursor - 1;  // Pos avant ']'
    if (*test_end != ']') {
        fprintf(stderr, "Erreur : ']' manquant à la fin de la condition.\n");
        return 1;
    }

    // TEST
    size_t test_len = test_end - test_start;
    char *test = malloc(test_len + 1);  // +1 pour rajouter '\0'
    if (!test) {
        perror("malloc");
        return 1;
    }
    memcpy(test, test_start, test_len);
    test[test_len] = '\0';
    // trim du bled revisité
    char *trim_start = test;
    while (isspace((unsigned char)*trim_start)) trim_start++; // Trim gauche
    char *trim_end = trim_start + strlen(trim_start) - 1;
    while (trim_end > trim_start && isspace((unsigned char)*trim_end)) trim_end--; // Trim droit
    *(trim_end + 1) = '\0';

    //
    cmd_info *new_cmd = malloc(sizeof(cmd_info));
    if (!new_cmd) {
        perror("malloc");
        free(test);
        return 1;
    }
    char *new_name = malloc(strlen(trim_start) + 1);  // Alloue mémoire pour la nouvelle copie
    if (!new_name) {
        perror("malloc");
        free(test);
        free(new_cmd);
        return 1;
    }
    memcpy(new_name, trim_start, strlen(trim_start) + 1);
    new_cmd->name = new_name;     // chaîne brute dans 'name'
    new_cmd->opt = NULL;
    new_cmd->args = NULL;
    info->test = new_cmd;
    free(test);

    printf("handle_if_line | Test atteint :%s\n", (char *)info->test->name);

    // CMD_TRUE
    while (*cursor == ' ') cursor++;
    if (*cursor != '{') {
        fprintf(stderr, "Erreur : CMD_TRUE mal formée, '{' manquant.\n");
        return 1;
    }
    cursor++;

    char *cmd_true_start = cursor;
    while (*cursor != '\0' && *cursor != '}') cursor++;
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
    // Nettoyage des crochets et des espaces
    size_t tstart = 0;
    while (cmd_true[tstart] == '{' || isspace((unsigned char)cmd_true[tstart])) tstart++;
    size_t tend = cmd_true_len - 1;
    while (tend > tstart && (cmd_true[tend] == '}' || isspace((unsigned char)cmd_true[tend]))) tend--;

    size_t new_len = tend - tstart + 1;
    char *clean_cmd = malloc(new_len + 1);
    if (!clean_cmd) {
        perror("malloc");
        free(cmd_true);
        return 1;
    }
    memcpy(clean_cmd, cmd_true + tstart, new_len);
    clean_cmd[new_len] = '\0';
    free(cmd_true);

    char *trimmed_cmd = trim(clean_cmd);
    free(clean_cmd);
    if (!trimmed_cmd) {
        return 1;
    }
    info->cmd_true = parse_cmd(trimmed_cmd);
    free(trimmed_cmd);
    if (!info->cmd_true) {
        fprintf(stderr, "Erreur : parse_cmd a échoué pour CMD_TRUE.\n");
        destroy_if_info(info);
        return 1;
    }
    printf("handle_if_line | cmd_true atteint :%s\n", (char *)info->cmd_true->name);

    // CMD_FALSE
    while (*cursor == ' ') cursor++;
    if (strncmp(cursor, "else", 4) == 0) {
        cursor += 4;
        while (*cursor == ' ') cursor++;
        if (*cursor != '{') {
            fprintf(stderr, "Erreur : CMD_FALSE mal formée, '{' manquant.\n");
            return 1;
        }
        cursor++;

        char *cmd_false_start = cursor;
        while (*cursor != '\0' && *cursor != '}') cursor++;
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

        size_t estart = 0;
        while (cmd_false[estart] == '{' || isspace((unsigned char)cmd_false[estart])) estart++;
        size_t eend = cmd_false_len - 1;
        while (eend > estart && (cmd_false[eend] == '}' || isspace((unsigned char)cmd_false[eend]))) eend--;

        size_t new_len_false = eend - estart + 1;
        char *clean_false = malloc(new_len_false + 1);
        if (!clean_false) {
            perror("malloc");
            free(cmd_false);
            return 1;
        }
        memcpy(clean_false, cmd_false + estart, new_len_false);
        clean_false[new_len_false] = '\0';
        free(cmd_false);
        info->cmd_false = parse_cmd(trim(clean_false));
        free(clean_false);
        if (!info->cmd_false) {
            fprintf(stderr, "Erreur : parse_cmd a échoué pour CMD_FALSE.\n");
            destroy_if_info(info);
            return 1;
        }
        printf("Cmd_false atteint :%s\n", (char *)info->cmd_false->name);
    }

    return execute_if(info);
}


// Exec commande if. Return 0 si succès, 1 si échec.
int execute_if(if_info *info) {
    if (!info || !info->test || !info->test->name) {
        fprintf(stderr, "Erreur : if_info mal formé.\n");
        return 1;
    }
    
    // Tableau d'arguments alloué dynamiquement
    char **test_args = malloc(sizeof(char *) * 10); // Taille initiale (agrandie si besoin)
    if (!test_args) {
        perror("malloc");
        return 1;
    }
    test_args[0] = "/usr/bin/test"; // commande test
    int arg_index = 1;
    char *name = info->test->name;
    int in_quote = 0; // Pour guillemets
    int start = 0;
    for (int i = 0; name[i] != '\0'; i++) {
        if (name[i] == '"' || name[i] == '\'') {
            in_quote = !in_quote; // Bascule dans ou hors des guillemets
            continue;
        }

        if (name[i] == ' ' && !in_quote) {
            name[i] = '\0';
            test_args[arg_index++] = &name[start];
            start = i + 1;

            if (arg_index % 10 == 0) { // Réallocation si nécessaire
                test_args = realloc(test_args, sizeof(char *) * (arg_index + 10));
                if (!test_args) {
                    perror("realloc");
                    return 1;
                }
            }
        }
    }
    if (name[start] != '\0') {
        test_args[arg_index++] = &name[start];
    }
    test_args[arg_index] = NULL; // NULL-terminer

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        free(test_args);
        return 1;
    }
    if (pid == 0) {
        // Child process: exe la commande "test"
        execvp(test_args[0], test_args);
        perror("execvp");  // Si execvp échoue
        exit(1);
    }
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        free(test_args);
        return 1;
    }
    free(test_args);
    // Test réussi si le code de retour est 0
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("execute_if | code de retour du test : %d\n", WEXITSTATUS(status));
        if (info->cmd_true) {
            return execute_cmd(info->cmd_true);
        }
    } else {
        if (info->cmd_false) {
            return execute_cmd(info->cmd_false);
        }
    }
    return 0;
}

/*
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
*/

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

SECTION PRECEDENTE DU CMDTRUE DE HANDLE:
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
    // Enlever les crochets
    size_t tstart = 0;
    while (cmd_true[tstart] == '{' || cmd_true[tstart] == ' ') tstart++;
    size_t tend = cmd_true_len - 1;
    while (cmd_true[tend] == '}' || cmd_true[tend] == ' ') tend--;
    cmd_true[tend + 1] = '\0';
    //
    char *original_cmd_true = cmd_true;
    cmd_true = cmd_true + tstart;
    size_t new_len = tend - tstart + 1;
    char *clean_cmd = malloc(new_len + 1);
    if (!clean_cmd) {
        perror("malloc");
        free(original_cmd_true);
        return 1;
    }
    memcpy(clean_cmd, cmd_true, new_len);
    clean_cmd[new_len] = '\0';

    clean_cmd = trim(clean_cmd);
    info->cmd_true = parse_cmd(clean_cmd);
    printf("free(original_cmd_true)\n");
    free(original_cmd_true);
    printf("free(clean_cmd)\n");
    free(clean_cmd);
    if (!info->cmd_true) {  // Gestion échec pour CMD_TRUE
        fprintf(stderr, "Erreur : parse_cmd a échoué pour la cmd_true.\n");
        destroy_if_info(info);
        return 1;
    }
    printf("handle_if_line | cmd_true atteint :%s\n", (char *)info->cmd_true->name);



SECTION PRECEDENTE DU CMDFALSE DE HANDLE
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
        // Enlever les crochets
        size_t estart = 0;
        while (cmd_false[estart] == '{' || cmd_false[estart] == ' ') estart++;
        size_t eend = cmd_false_len - 1;
        while (cmd_false[eend] == '}' || cmd_false[eend] == ' ') eend--;
        cmd_false[eend + 1] = '\0';
        //
        info->cmd_false = parse_cmd(cmd_false);
        free(cmd_false);
        if (!info->cmd_false) {  // Gestion échec pour CMD_FALSE
            fprintf(stderr, "Erreur : parse_cmd a échoué pour la cmd_false.\n");
            destroy_if_info(info);
            return 1;
        }
        printf("Cmd_false:%s\n", (char *)info->cmd_false);
    }

*/
