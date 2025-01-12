#include "if.h"
/*
* Intialisation d'une struct if_info
*/
if_info * if_info_init() {
    if_info *info = malloc(sizeof(if_info));
    if (info == NULL) {
        perror("malloc");
        return NULL;
    }
    info -> test = NULL;
    info -> cmd_true = calloc(CMD_SIZE, sizeof(char));  // Commande effectuée si test = true
    if (info -> cmd_true == NULL) {
        destroy_if_info(info);
        return NULL;
    }
    info -> cmd_false = calloc(CMD_SIZE, sizeof(char)); // Si 'else', commande effectuée si test = false
    if (info -> cmd_false == NULL) {
        destroy_if_info(info);
        return NULL;
    }
    return info;
}


void destroy_if_info(if_info *info) {
    if (info != NULL) {
        if (info -> test != NULL) {
            //printf("destroy_if_info | clear info->test;\n");
            free(info -> test);
            info -> test = NULL;
        }
        if (info -> cmd_true != NULL) {
            free(info -> cmd_true);
            info -> cmd_true = NULL;
        }
        if (info -> cmd_false != NULL) {
            free(info -> cmd_false);
            info -> cmd_false = NULL;
        }
        free(info);
    }
}


int evaluate_test(char *condition) {
    if(condition == NULL) return 1;
        char **test_args = malloc(sizeof(char *) * 10); // Taille initiale (dynamique, agrandie si besoin)
        if (test_args == NULL) {
            perror("malloc");
            return 1;
        }
        test_args[0] = "/usr/bin/test"; // commande test
        int arg_index = 1;
        char *name = condition;
        int in_quote = 0;
        int start = 0;

        // Rempli le tableau test_args contenant les informations de la condition à évaluer.
        for (int i = 0; name[i] != '\0'; i++) {
            if (name[i] == '"' || name[i] == '\'') {
                in_quote = !in_quote;
                continue;
            }
            if (name[i] == ' ' && !in_quote) {
                name[i] = '\0';
                test_args[arg_index++] = &name[start];
                start = i + 1;

                if (arg_index % 10 == 0) { // Réallocation si nécessaire
                    test_args = realloc(test_args, sizeof(char *) * (arg_index + 10));
                    if (test_args == NULL) {
                        perror("realloc");
                        return 1;
                    }
                }
            }
        }
        if (name[start] != '\0') {
            test_args[arg_index++] = &name[start];
        }
        test_args[arg_index] = NULL; // NULL-termine

        // Exécution du test
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            free(test_args);
            return 1;
        } else if (pid == 0) {
            int null = open("/dev/null", O_WRONLY);
            if (null == 1) {
                perror("Erreur : /dev/null");
                exit(1);
            }
            if (dup2(null, STDERR_FILENO) == -1) {
                perror("dup2");
                exit(1);
            }
            close(null);
            
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

        if (WIFEXITED(status)) {    // Test réussi si le code de retour est 0, échec sinon.
            return WEXITSTATUS(status);
        }
        return 1;
}


/*
* Analyse une ligne de commande, vérifie si il s'agit d'un if. 
* Return 0 si succès, 1 en cas d'erreur de syntaxe.
*/
if_info * handle_if_line(char *line) {
    if (line == NULL) {
        write(STDERR_FILENO, SYNTAX_ERR, strlen(SYNTAX_ERR));
        return NULL;
    }
    if_info * info = if_info_init();
    if (info == NULL) {
        perror("if_info_init");
        return NULL;
    }
    char *cursor = line;

    // 1. Vérification de la syntaxe : mot-clé "if"
    while (*cursor != '\0' && isspace((unsigned char)*cursor)) cursor++; // Skip espaces
    if (strncmp(cursor, "if", 2) != 0 || !isspace(cursor[2])) {
        write(STDERR_FILENO, SYNTAX_ERR, strlen(SYNTAX_ERR));
        goto error;
    }
    cursor += 2; // Skip "if"

    // 2. Extraction de la condition (jusqu'à '{')
    while (isspace((unsigned char)*cursor)) cursor++;
    char * condition_start = cursor;
    while (*cursor != '\0' && *cursor != '{') cursor++;
    if (*cursor != '{') {
        write(STDERR_FILENO, SYNTAX_ERR, strlen(SYNTAX_ERR));
        goto error;
    }
    size_t condition_len = cursor - condition_start;
    char * condition = custom_strndup(condition_start, condition_len);
    if (condition == NULL) {
        perror("strndup");
        goto error;
    }

    free(info->test);
    info->test = malloc(strlen(condition) + 1);
    if (info->test == NULL) {
        free(condition);
        goto error;
    }
    memcpy(info->test, condition, strlen(condition) + 1);
    free(condition);

    // 3. Extraction de cmd_true (jusqu'à '}')
    while (isspace((unsigned char) * cursor)) cursor++;
    int start = cursor - line; // Position actuelle
    char * tmp = extract_block(line, &start); // Utilise extract_block avec pointeur sur {
    if (tmp == NULL) {
        goto error;
    }
    
    char * tmp2 = trim(tmp);
    free(info->cmd_true);
    info->cmd_true = malloc(strlen(tmp2) + 1);
    if (info->cmd_true == NULL) {
        write(STDERR_FILENO, SYNTAX_ERR, strlen(SYNTAX_ERR));
        goto error;
    }
    memcpy(info->cmd_true, tmp2, strlen(tmp2) + 1);
    free(tmp);
    free(tmp2);
    if (info->cmd_true == NULL) {
        write(STDERR_FILENO, SYNTAX_ERR, strlen(SYNTAX_ERR));
        goto error;
    }
    cursor = line + start;

    // 4. Vérification & extraction de cmd_false si "else" présent
    while (isspace((unsigned char) * cursor)) cursor++;
    if (strncmp(cursor, "else", 4) == 0) {
        cursor += 4; // Skip "else"
        while (isspace((unsigned char)*cursor)) cursor++;
        if (*cursor != '{') {
            write(STDERR_FILENO, SYNTAX_ERR, strlen(SYNTAX_ERR));
            goto error;
        }

        start = cursor - line;
        char * tmp = extract_block(line, &start); // Utilise extract_block avec pointeur sur {
        if (tmp == NULL) {
            goto error;
        }

        free(info->cmd_false);
        info->cmd_false = malloc(strlen(tmp) + 1);
        if (info->cmd_false == NULL) {
            write(STDERR_FILENO, SYNTAX_ERR, strlen(SYNTAX_ERR));
            goto error;
        }
        memcpy(info->cmd_false, tmp, strlen(tmp) + 1);
        free(tmp);
        if (info->cmd_false == NULL) {
            write(STDERR_FILENO, SYNTAX_ERR, strlen(SYNTAX_ERR));
            goto error;
        }
        cursor = line + start;
    }
    return info;

    error:
    if (tmp != NULL) free(tmp);
    if (condition != NULL) free(condition);
    if (info != NULL) {
        destroy_if_info(info);
        info = NULL;
    }
    return NULL;
}