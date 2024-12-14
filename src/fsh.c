#include "fsh.h"

int last_return = -1;

int fsh_exit(cmd_info * cmd) {
    int flag = 1;
    size_t args_length = strlen(cmd -> args);
    for (size_t i = 0; i < args_length; i++) {
        if (!isdigit(cmd -> args[i])) {
            flag = 0;
        }
    }
    if (strlen(cmd -> opt) > 0 || flag == 0) {
        char * error = "Too many arguments or wrong value.\n";
        write(STDERR_FILENO, error, strlen(error));
        return 1;
    } else if (strlen(cmd -> args) > 0) {
        int n  = atoi(cmd -> args);
        destroy_cmd_info(cmd);
        exit(n);
    } else {
        destroy_cmd_info(cmd);
        exit(last_return);
    }
}

int call_function (cmd_info * cmd) {
    if (strcmp("pwd", cmd -> name) == 0) {
        return pwd(cmd);
    } else if (strcmp("exit", cmd -> name) == 0) {
        return fsh_exit(cmd);
    } else if (strcmp(cmd -> name, "ftype") == 0) {
        return ftype(cmd);
    } else if (strcmp(cmd -> name, "cd") == 0) { 
        return cd(cmd);
    } else {
        return externalcmd(cmd);
    }
}

int exec_for(for_info * f) {
    int result = 0;
    char * path = NULL; 
    struct dirent * entry = NULL;
    struct stat s; 
    int file_arg = -1;
    char * copy_args = NULL;

    //Ouverture du fichier.
    DIR * dirp = opendir(f -> directory); 
    if (dirp == NULL) {  
        perror("Référence invalide"); 
        result = 1;
        goto error;
    }

    //malloc d'un char * pour sauvegarder la référence courante.
    path = malloc(BUFFER_SIZE * sizeof(char));
    if (path == NULL) {
        result = 1;
        goto error;
    }
    
    //Vérification permettant de savoir si la variable F est présente.
    if (strstr(f -> command, "$F") != NULL) {
        file_arg = 1;
    }
    
    //Si le répertoire donné ne possède pas de '/' à la fin de la ligne on lui ajoute et on sauvegarde dans path.
    if (f->directory[strlen(f -> directory) - 1] != '/') {
        strcat(f -> directory, "/");
    }
    memmove(path, f -> directory, strlen(f -> directory) + 1);

    //Copie de la ligne de l'argument.
    if (file_arg == 1) {
        copy_args = calloc(strlen(f -> command) + 1, sizeof(char));
        if (copy_args == NULL) {
            result = 1;
            goto error;
        }
        memmove(copy_args, f -> command, strlen(f -> command) + 1);
    }

    //Boucle qui exécute le for.
    while((entry = readdir(dirp)) != NULL) {
        memmove(path + strlen(f -> directory), entry -> d_name, strlen(entry -> d_name) + 1);    
        if (lstat(path, &s) == 0) {
            if(entry -> d_name[0] != '.' && S_ISREG(s.st_mode)) {
                if  (file_arg == 1) {
                    if (replace_var(f -> command, path) != 0) {goto error;}
                }
                char * copy = malloc((strlen(f -> command) + 1) * sizeof(char));
                if (copy == NULL) {goto error;}
                memmove(copy, f -> command, strlen(f -> command) + 1);
                int n = handle_line(copy);
                if (file_arg == 1) {
                    memmove(f -> command, copy_args, strlen(copy_args) + 1);
                }
                if (n > result) {
                    result = n;
                }
            }
        } else {
            closedir(dirp);
            perror("erreur");
            goto error;
        }
    }

    free(path);
    free(copy_args);
    closedir(dirp);
    destroy_for_info(f);
    return result;

    error:
    free(path);
    free(copy_args);
    destroy_for_info(f);
    return result;
}

/** handle_line permet de s'occuper de la ligne donnée en argument.*/
int handle_line(char * line) { 
    char ** cmds = handle_cmd(line);
    if (cmds == NULL) {
        free(line);
        return EXIT_FAILURE;
    }

    int result = 0;
    cmd_info * cmd = NULL;
    for_info * f = NULL;
    for (size_t all = 0; cmds[all] != NULL; all++) {
        char * line_cmd = cmds[all];
        result = 0;
        cmd = cmd_info_init();
        if (cmd == NULL) {
            result = 1;
            goto error;
        }

        size_t pos = 0;
        while (isspace(line_cmd[pos]) != 0) { pos++; }

        size_t i = 0;
        while (isspace(line_cmd[pos]) == 0 && line_cmd[pos] != '\0') {
            cmd -> name[i++] = line_cmd[pos++];
        }
        cmd -> name[i] = '\0';

        if (handle_redirection(line_cmd, pos) != 0) {
            result = 1;
            goto error;
        }

        if (strcmp(cmd -> name, "") == 0) {
            result = 0;
        } else if (strcmp(cmd -> name, "for") != 0) {
            if (line_cmd[pos] != '\0') {
                memmove(line_cmd, line_cmd + pos + 1, strlen(line_cmd) - pos + 1);
                cmd_format(line_cmd, cmd);
            }
            result = call_function(cmd);
        } else {
            if (line_cmd[pos] != '\0') {
                memmove(line_cmd, line_cmd + pos + 1, strlen(line_cmd) - pos + 1);
            }
            f = handle_for_line(line_cmd);
            if (f == NULL) {
                result = 2;
                goto error;
            }
            result = exec_for(f);
        }

        close_redirection();
        if (cmd != NULL) {
            destroy_cmd_info(cmd);
            cmd = NULL;
        }
    }

    if (cmds != NULL) {
        for (size_t j = 0; cmds[j] != NULL; j++) {
            free(cmds[j]);
        }
        free(cmds);
    }
    free(line);
    return result;

error:
    close_redirection();
    if (cmd != NULL) {
            destroy_cmd_info(cmd);
            cmd = NULL;
        }
    if (cmds != NULL) {
        for (size_t j = 0; cmds[j] != NULL; j++) {
            free(cmds[j]);
        }
        free(cmds);
    }
    free(line);
    return result;
}

char ** handle_cmd(char * line) {
    size_t pos = 0, nb_cmd = 0, pos_cmd = 0, for_cmd = 0;
    char ** cmd = calloc(BUFFER_SIZE, sizeof(char *));
    if (cmd == NULL) {
        perror("calloc");
        return NULL;
    }

    while (line[pos] != '\0') {
        if (cmd[nb_cmd] == NULL) {
            cmd[nb_cmd] = calloc(BUFFER_SIZE, sizeof(char));
            if (cmd[nb_cmd] == NULL) {
                perror("calloc cmd");
                for (size_t i = 0; i < nb_cmd; i++) {
                    free(cmd[i]);
                }
                free(cmd);
                return NULL;
            }
        }

        if (line[pos] == '{') {
            for_cmd++;
            cmd[nb_cmd][pos_cmd++] = line[pos++];
            continue;
        } else if (line[pos] == '}') {
            for_cmd--;
            cmd[nb_cmd][pos_cmd++] = line[pos++];
            continue;
        }

        if (line[pos] == ';' && for_cmd == 0) {
            cmd[nb_cmd][pos_cmd] = '\0';
            pos++;
            nb_cmd++;
            pos_cmd = 0;
            while (isspace(line[pos]) != 0) {
                pos++;
            }
            continue;
        }
        cmd[nb_cmd][pos_cmd++] = line[pos++];
    }

    if (cmd[nb_cmd] != NULL) {
        cmd[nb_cmd][pos_cmd] = '\0';
    }

    cmd[nb_cmd + 1] = NULL;
    return cmd;
}

int handle_redirection(char * line, size_t i) {
    int result = 0;
    int redirection_result = 0;
    char * redirection_type = NULL;
    char * redirection_file = NULL;
    size_t pos = i;
    size_t size = 0;

    while (line[pos] != '\0') {
        if (line[pos] == '>' || line[pos] == '<' || (line[pos] == '2' && (line[pos + 1] == '>' || line[pos + 1] == '<'))) {
            // On gere le type de redirection
            redirection_type = malloc(BUFFER_SIZE * sizeof(char));
            if (redirection_type == NULL) {
                result = 1;
                goto error;
            }
            size = 0;
            redirection_type[size++] = line[pos++];

            while (!isspace(line[pos]) && line[pos] != '\0') {
                redirection_type[size++] = line[pos++];
            }
            redirection_type[size] = '\0';

            // Ignorer les espaces avant le file
            while (isspace(line[pos]) != 0) { pos++; }

            // On gere le file de redirection
            redirection_file = malloc(BUFFER_SIZE * sizeof(char));
            if (redirection_file == NULL) {
                result = 1;
                goto error;
            }

            size = 0;
            while (line[pos] != '\0' && !isspace(line[pos])) {
                redirection_file[size++] = line[pos++];
            }
            redirection_file[size] = '\0';

            // On applique la redirection
            redirection_result = redirection(redirection_type, redirection_file);
            if (redirection_result != 0) {
                result = 1;
                goto error;
            }

            // Redirection multiples
            if (handle_redirection(line, pos) != 0) {
                result = 1;
                goto error;
            }

            // Suppresion de la redirection du line
            size_t redirection_size = pos - strlen(redirection_file) - strlen(redirection_type) - 1;
            while (redirection_size > 0 && isspace(line[redirection_size - 1])) {
                redirection_size--;
            }

            while (line[redirection_size] != '\0') { line[redirection_size] = '\0'; redirection_size++; }
            break;
        }
        pos++;
    }

    free(redirection_type);
    free(redirection_file);
    return result;

    error:
    free(redirection_type);
    free(redirection_file);
    return result;
}

/** Fonction permettant de changer la couleur du prompt. */
void color_prompt(char * prompt, int var) {
    size_t color_len = 0;
    size_t cyan_len = strlen(CYAN);
    size_t var_len = 3;
    if (var == 0) {
        color_len = strlen(GREEN);
        memmove(prompt + color_len, prompt, strlen(prompt));
        memmove(prompt, GREEN, color_len);
        memmove(prompt + color_len + var_len + cyan_len, prompt + color_len + var_len, strlen(prompt) - var_len);
        memmove(prompt + color_len + var_len, CYAN, cyan_len);
    } else if (var > 0) {
        color_len = strlen(RED);
        memmove(prompt + color_len, prompt, strlen(prompt));
        memmove(prompt, RED, color_len);
        memmove(prompt + color_len + var_len + cyan_len, prompt + color_len + var_len, strlen(prompt) - var_len);
        memmove(prompt + color_len + var_len, CYAN, cyan_len);
    } else {
        color_len = strlen(DEFAULT_COLOR);
        memmove(prompt + color_len, prompt, strlen(prompt));
        memmove(prompt, DEFAULT_COLOR, color_len);
        memmove(prompt + color_len + var_len + cyan_len, prompt + color_len + var_len, strlen(prompt) - var_len);
        memmove(prompt + color_len + var_len, CYAN, cyan_len);
    }
}

/** Adapte la taille du prompt et remplie le reste de '\0'. */
void make_prompt(char * prompt, int var) {
    size_t current_size = strlen(prompt);
    //Ne pas compter $_
    size_t size = PROMPT_SIZE - 2;
    char * return_value = calloc(BUFFER_SIZE, sizeof(char));

    if (var == 255) { 
        strcat(return_value, "[255]\0");

        //TODO: Remplacer par SIG une fois que c'est demandé.
        //strcat(return_value, "[SIG]");
    } else {
        strcat(return_value, "[");
        if (var == -1) {
            strcat(return_value, "0");
        } else {
            sprintf(return_value + 1, "%d", var);
        }
        strcat(return_value, "]");
        strcat(return_value, "\0");
    }

    size_t return_length = strlen(return_value);
    if (current_size > PROMPT_SIZE) {
        memmove(prompt, prompt+(current_size - size), size);
        prompt[size] = '\0';
        memmove(prompt, return_value, return_length);

        for (size_t i = return_length; i < return_length + 3; i++) {
            prompt[i] = '.';
        } 

        for (size_t i = strlen(prompt); i < BUFFER_SIZE; i++) {
            prompt[i] = '\0';
        }

    } else {
        memmove(prompt + return_length, prompt, size);
        memmove(prompt, return_value, return_length);
    }

    color_prompt(prompt, var);
    strcat(prompt, DEFAULT_COLOR);
    strcat(prompt, "$ \0");
    free(return_value);
}

int display_prompt(int var) {
    char * line = NULL;
    char * prompt = calloc(BUFFER_SIZE, sizeof(char));
    char * command = calloc(BUFFER_SIZE, sizeof(char));
    
    if (prompt == NULL || command == NULL) {
        last_return = 1;
        goto cleanup;
    }

    if (getcwd(prompt, BUFFER_SIZE) == NULL) {
        perror("display_prompt");
        last_return = 1;
        goto cleanup;
    }
    make_prompt(prompt, var);
    
    line = readline(prompt);
    if (line == NULL) {
        goto cleanup;
    } else if (strcmp(line, "") == 0) {
        free(line);
        free(prompt);
        return display_prompt(last_return);
    }
    add_history(line);
    memmove(command, line, strlen(line) + 1);

    free(line);
    free(prompt);

    last_return = handle_line(command);
    return display_prompt(last_return);

    cleanup:
    free(line);
    free(prompt);
    free(command);
    return last_return;
}

int main() {
    rl_outstream = stderr;
    return display_prompt(last_return);
}