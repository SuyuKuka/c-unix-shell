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


void destroy_if_info(if_info *info) {
    if (info) {
        if (info->test) {
            destroy_cmd_info(info->test);
            info->test = NULL;
        }
        if (info->cmd_true) {
            if (info->cmd_true != info->test) {
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

char *strip_brackets(char *condition) {
    if (!condition) return NULL;

    size_t len = strlen(condition);
    size_t start = 0, end = len - 1;

    // Retirer les crochets et espaces autour
    while (start < len && (condition[start] == '[' || isspace((unsigned char)condition[start]))) start++;
    while (end > start && (condition[end] == ']' || isspace((unsigned char)condition[end]))) end--;

    size_t new_len = end - start + 1;
    char *clean_condition = malloc(new_len + 1);
    if (!clean_condition) {
        perror("malloc");
        return NULL;
    }

    memcpy(clean_condition, condition + start, new_len);
    clean_condition[new_len] = '\0';
    return clean_condition;
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
    
    // CONDITION
    char *condition_start = cursor;
    while (*cursor != '\0' && *cursor != '{') cursor++;
    if (*cursor != '{') {
        fprintf(stderr, "Erreur : '{' manquant après la condition.\n");
        return 1;
    }

    char *condition_end = cursor;
    size_t condition_len = condition_end - condition_start;

    // Extraire la condition
    char *condition = malloc(condition_len + 1);
    if (!condition) {
        perror("malloc");
        return 1;
    }
    memcpy(condition, condition_start, condition_len);
    condition[condition_len] = '\0';

    // Nettoyer les crochets autour de la condition
    char *clean_condition = strip_brackets(condition);
    free(condition);
    if (!clean_condition) {
        fprintf(stderr, "Erreur : nettoyage des crochets échoué.\n");
        return 1;
    }

    // Parser la condition comme une commande
    cmd_info *new_cmd = malloc(sizeof(cmd_info));
    if (!new_cmd) {
        perror("malloc");
        free(clean_condition);
        return 1;
    }
    char *new_name = malloc(strlen(clean_condition) + 1);  // Alloue mémoire pour la copie brute
    if (!new_name) {
        perror("malloc");
        free(new_cmd);
        free(clean_condition);
        return 1;
    }
    memcpy(new_name, clean_condition, strlen(clean_condition) + 1);  // Copie brute de la condition
    new_cmd->name = new_name;  // La chaîne brute est stockée dans 'name'
    new_cmd->opt = NULL;       // Pas d'options pour cette commande
    new_cmd->args = NULL;      // Pas d'arguments supplémentaires pour cette commande
    info->test = new_cmd;  // Stocke directement dans info->test
    //printf("info->test->name post 'parse' :%s\n", info->test->name);
    free(clean_condition);

    cursor++;
    while (isspace((unsigned char)*cursor)) cursor++;

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
            //printf("execute_if | contenu de test_args[%d] :%s\n", arg_index, test_args[arg_index]);
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
        execvp(test_args[0], test_args);
        perror("execvp");
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
    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        //printf("execute_if | Code de retour de test : %d\n", exit_status);

        if (exit_status == 0) {
            // Condition vraie
            if (info->cmd_true) {
                //printf("execute_if | Condition vraie, exécution de cmd_true\n");
                return execute_cmd(info->cmd_true);
            }
        } else {
            // Condition fausse
            if (info->cmd_false) {
                //printf("execute_if | Condition fausse, exécution de cmd_false\n");
                return execute_cmd(info->cmd_false);
            }
        }
    } else {
        // Si le processus enfant s'est mal terminé
        //fprintf(stderr, "execute_if | Processus enfant terminé anormalement.\n");
    }
    //printf("Condition fausse et pas de cmd_false, return 0\n");
    return 0;
}


/*  
if (is_test_command) { 
    // Cas 'test': avancer jusqu'à '{' sans gérer les crochets
    while (*cursor != '\0' && *cursor != '{' && !isspace((unsigned char)*cursor)) {
        cursor++;
    }
    test_end = cursor;
    while (isspace((unsigned char)*cursor)) cursor++;  // Sauter les espaces après la condition jusqu'à '{
    if (*cursor != '{') {
        fprintf(stderr, "CHECK avec test, '{' manquant.\n");
        return 1;
    }
    cursor++;
} else { 
    // Cas '[' : vérifier les crochets équilibrés
    size_t bracket_balance = 1;  // Initialise avec '[' déjà détecté
    while (*cursor != '\0' && bracket_balance > 0) {
        if (*cursor == '[') bracket_balance++;
        if (*cursor == ']') bracket_balance--;
        cursor++;
    }
    if (bracket_balance != 0) {
        fprintf(stderr, "Erreur : problème de crochets dans la condition TEST.\n");
        return 1;
    }
    test_end = cursor - 1;  // Pos avant ']'
    if (*test_end != ']') {
        fprintf(stderr, "Erreur : ']' manquant à la fin de la condition.\n");
        return 1;
    }
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

    //printf("handle_if_line | Test atteint :%s\n", (char *)info->test->name);  */

    /* CMD_TRUE
    while (isspace((unsigned char)*cursor)) cursor++; // Sauter les espaces
    if (*cursor != '{') {
        fprintf(stderr, "Erreur : CMD_TRUE mal formée, '{' manquant.\n");
        return 1;
    }
    cursor++; */