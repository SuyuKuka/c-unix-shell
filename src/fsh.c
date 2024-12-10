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

int exec_for_cmd(for_info * f) {
    int result = 0;
    char * path = NULL; 
    struct dirent * entry = NULL;
    struct stat s; 
    int file_arg = -1;
    char * copy_args = NULL;
    DIR * dirp = opendir(f -> directory); 
    if (dirp == NULL) {  
        perror("Référence invalide"); 
        result = 1;
        goto error;
    }
    path = malloc(BUFFER_SIZE * sizeof(char));
    if (path == NULL) {
        result = 1;
        goto error;
    }
    
    if (strstr(f -> command -> args, "$F") != NULL) {
        file_arg = 1;
    }
    
    if (f->directory[strlen(f -> directory) - 1] != '/') {
        strcat(f -> directory, "/");
    }
    memmove(path, f -> directory, strlen(f -> directory) + 1);
    //Copie de la ligne original de l'argument.
    if (file_arg == 1) {
        copy_args = calloc(strlen(f -> command -> args) + 1, sizeof(char));
        memmove(copy_args, f -> command -> args, strlen(f -> command -> args) + 1);
    }

    while((entry = readdir(dirp)) != NULL) {
        memmove(path + strlen(f -> directory), entry -> d_name, strlen(entry -> d_name) + 1);
        if (lstat(path, &s) == 0) {
            if(entry -> d_name[0] != '.' && S_ISREG(s.st_mode)) {
                if  (file_arg == 1) {
                    if (replace_var(f -> command, path) != 0) {goto error;}
                }
                int n = call_function(f -> command);
                if (file_arg == 1) {
                    memmove(f -> command -> args, copy_args, strlen(copy_args) + 1);
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
    int result = 0;
    cmd_info * cmd = NULL;
    for_info * f = NULL;
    if_info * iinfo = NULL;
    int redirection_result = 0;
    char * redirection_type = NULL;
    char * redirection_file = NULL;
    cmd = cmd_info_init();
    if (cmd == NULL) {
        result = 1;
        goto error;
    }
    
    size_t i = 0;
    while (isspace(line[i]) != 0) { i++; }
   
    while (isspace(line[i]) == 0 && line[i] != '\0') {
        cmd -> name[i] = line[i];
        i++;
    }

    size_t j = i;
    size_t k = 0;
    while (line[j] != '\0') {
        // Si une redirection est détectée
        if (line[j] == '>' || line[j] == '<' || line[j] == '2') {
            // On gere le type de redirection
            redirection_type = malloc(BUFFER_SIZE * sizeof(char));
            if (redirection_type == NULL) {
                result = 1;
                goto error;
            }
            k = 0;
            redirection_type[k++] = line[j++];

            while (!isspace(line[j]) && line[j] != '\0') {
                redirection_type[k++] = line[j++];
            }
            redirection_type[k] = '\0';

            // Ignorer les espaces avant le file
            while (isspace(line[j]) != 0) { j++; }

            // On gere le file de redirection
            redirection_file = malloc(BUFFER_SIZE * sizeof(char));
            if (redirection_file == NULL) {
                result = 1;
                goto error;
            }

            k = 0;
            while (line[j] != '\0' && line[j] != '>') {
                redirection_file[k++] = line[j++];
            }
            redirection_file[k] = '\0';

            // On applique la redirection
            redirection_result = redirection(redirection_type, redirection_file);
            if (redirection_result != 0) {
                result = 1;
                goto error;
            }

            size_t start = j - k - strlen(redirection_type);
            while (start > 0 && isspace(line[start - 1])) {
                start--;
            }

            size_t end = j;
            memmove(&line[start - 1], &line[end], strlen(line) - end + 1);
            break;
        }
        j++;
    }

    if (strcmp(cmd -> name, "") == 0) {
        result = 0;
    } else if (strcmp(cmd->name, "if") == 0) {  // Implémentation du handle_if_line ici ?
        iinfo = if_info_init();
        result = handle_if_line(line, iinfo);
    } else if (strcmp(cmd -> name, "for") != 0) {
        if (line[i] != '\0') {
            memmove(line, line + i + 1, strlen(line) - i + 1);
            cmd_format(line, cmd);
        }
        result = call_function(cmd);
    } else {
        if (line[i] != '\0') {
            memmove(line, line + i + 1, strlen(line) - i + 1);
        }
        f = handle_for_line(line);
        if (f == NULL) {
            result = 1;
            goto error;
        }
        result = exec_for_cmd(f);
    }

    close_redirection();
    free(line);
    destroy_cmd_info(cmd);
    destroy_if_info(iinfo); //
    free(iinfo);    //
    free(redirection_file);
    free(redirection_type);
    return result;
    
    error:
    free(line);
    destroy_cmd_info(cmd);
    free(redirection_file);
    free(redirection_type);
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
    } else if (var == 1 || var == 255) {
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
    size_t size = PROMPT_SIZE + 1;
    char * return_value = calloc(BUFFER_SIZE, sizeof(char));

    if (var == 255) { 
        return_value = "[SIG]";
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
        size = size - return_length;
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