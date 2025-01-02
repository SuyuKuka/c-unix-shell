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

// Fonction qui exécute une commande en passant une copie de la ligne de commande du for.
int exec_for_cmd(for_info * f, char * path, int var) {
    char * copy = NULL;
    int result = 0;
    if  (var == 1) {
        copy = replace_var(f -> command, path, f -> for_var);
        if (copy == NULL) {
            return 1;
        }
        return handle_line(copy);
    } else {
        copy = malloc((strlen(f->command) + 1) * sizeof(char));
        if (copy == NULL) {
            return 1;
        }
        memmove(copy, f -> command, strlen(f -> command) +1);
        return handle_line(copy);
    }
}

// Valeurs pour la gestion du for -p
static int active_processes = 0;
int global_result = 0;

//Fonction qui vérifie les dépendances du for (options, type de fichiers, etc...)
int handle_for_dep(for_info * f, struct stat st, char * path, char * dir_name, int var) {
    int result = 0;
    int n = 0;
    for_info * copy = NULL;
    if (strcmp(dir_name, ".") == 0 || strcmp(dir_name, "..") == 0) {
        return 0;
    }

    // Vérification de l'option -p
    if (f->opt_p) {
        int status;
        while (active_processes >= f->p_max) {
            pid_t finished_pid = waitpid(-1, &status, WNOHANG);
            if (finished_pid > 0) {
                active_processes--;
                if (WIFEXITED(status)) {
                    int exit_code = WEXITSTATUS(status);
                    if (exit_code > global_result) {
                        global_result = exit_code;
                    }
                }
            } else if (finished_pid == 0) {
                usleep(1000);   // mini-pause
            } else {
                perror("Erreur waitpid");
                return EXIT_FAILURE;
            }
        }
        pid_t pid = fork();
        if (pid == -1) {
            perror("Erreur fork");
            return EXIT_FAILURE;
        } else if (pid == 0) {      // Processus enfant
            exit(exec_for_cmd(f, path, var));
        } else {                    // Processus parent
            active_processes++;
            return 0;
        }
    }

    //Vérification de l'option -r
    if ((f -> opt_a && f -> opt_r) && S_ISDIR(st.st_mode)
    || dir_name[0] != '.' && f -> opt_r && S_ISDIR(st.st_mode)) {
        copy = copy_for_info(f);
        if (copy != NULL) {
            memmove(copy -> directory, path, strlen(path) + 1);
            n =  handle_for(copy);
        } else {
            perror("Erreur copy -r");
            exit(1);
        }
    } 

    //Vérification de l'option -t TYPE
    if (f -> opt_type) {
        if ((f -> file_type == 'f' && !S_ISREG(st.st_mode))
        ||(f -> file_type == 'd' && !S_ISDIR(st.st_mode))
        ||(f -> file_type == 'l' && !S_ISLNK(st.st_mode))
        ||(f -> file_type == 'p' && !S_ISFIFO(st.st_mode))) {
            return 0;
        }
    }

    //Vérification de l'option -e EXT
    if (f -> opt_ext) {
        if (check_ext(path, f -> ext)) {
            remove_ext(path);
        } else {
            return 0;
        }
    }

    //Vérification de l'option -A
    if(!f -> opt_a && dir_name[0] != '.') {
        result =  exec_for_cmd(f, path, var);
    } else if (f -> opt_a) {
        result = exec_for_cmd(f, path, var);
    } 

    if (n > result) {
        result = n;
    }
    
    return result;
}

// À la fin de la boucle principale du for, on wait tous les processus restants pour le cas du -p.
void wait_for_remaining_processes() {
    int status;
    while (active_processes > 0) {
        pid_t pid = waitpid(-1, &status, 0);
        if (pid > 0) {
            active_processes--;
            if (WIFEXITED(status)) {  // Si le process s'est terminé normalement
                int exit_code = WEXITSTATUS(status);
                if (exit_code > global_result) {    // Agrège la val max de retour
                    global_result = exit_code;
                }
            }
        } else {
            perror("Erreur waitpid");
            break;
        }
    }
}

//Fonction qui s'occupe des for passés au prompt.
int handle_for(for_info * f) {
    int result = 0;
    int n = 0;
    char * path = NULL; 
    struct dirent * entry = NULL;
    struct stat st; 
    int var = -1;

    //Ouverture du fichier.
    DIR * dirp = opendir(f -> directory); 
    if (dirp == NULL) {  
        perror("open dir"); 
        goto error;
    }

    //malloc d'un char * pour sauvegarder la référence courante.
    path = malloc(BUFFER_SIZE * sizeof(char));
    if (path == NULL) {
        goto error;
    }
    
    //Vérification permettant de savoir si la variable F est présente.
    if (strstr(f -> command, "$") != NULL) {
        var = 1;
    }
    
    //Si le répertoire donné ne possède pas de '/' à la fin de la ligne on lui ajoute et on sauvegarde dans path.
    if (f -> directory[strlen(f -> directory) - 1] != '/') {
        strcat(f -> directory, "/");
    }
    memmove(path, f -> directory, strlen(f -> directory) + 1);

    //Boucle qui exécute le for.
    while((entry = readdir(dirp)) != NULL) {
        memmove(path + strlen(f -> directory), entry -> d_name, strlen(entry -> d_name) + 1);    
        if (lstat(path, &st) == 0) {
            n = handle_for_dep(f, st, path, entry -> d_name, var);
            if (result < n) {
                result = n;
            }
        } else {
            perror("error");
            closedir(dirp);
            goto error;
        }
    }

    wait_for_remaining_processes(); // Pour le for -p.
    if (global_result > result) {
        result = global_result;
    }

    free(path);
    closedir(dirp);
    destroy_for_info(f);
    return result;

    error:
    free(path);
    destroy_for_info(f);
    return EXIT_FAILURE;
}

// Exec commande if. Return 0 si succès, 1 si échec.
/* 
int execute_if(if_info *info) {
    if (!info || !info -> test || !info -> test -> name) {
        fprintf(stderr, "Erreur : if_info mal formé.\n");
        return 1;
    }
    /*
    char *original = strdup(info->test->name);
    if(original == NULL) {
        perror("strdup");
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
    char *name = info -> test -> name;
    // printf("execute_if | contenu de la chaîne condition name :%s\n", name); //contenu bien entier de la condition
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
            // printf("execute_if | contenu de test_args[%d] :%s\n", arg_index, test_args[arg_index]);
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
    /*printf("execute_if | contenu de la chaîne condition name post testargs :%s\n", name); //contenu bien entier de la condition
    printf("execute_if | contenu de original_name post test :%s\n", info->test->name); //contenu bien entier de la condition
    printf("execute_if | contenu de test_args[0] :%s\n", test_args[0]);
    printf("execute_if | contenu de test_args[1] :%s\n", test_args[1]);
    printf("execute_if | contenu de test_args[2] :%s\n", test_args[2]);
    printf("execute_if | contenu de test_args[3] :%s\n", test_args[3]);

    for (int i = 0; test_args[i] != NULL; i++) {
        if ((strcmp(test_args[i], ">") == 0 || strcmp(test_args[i], ">>") == 0 || strcmp(test_args[i], "<") == 0)) { //mettre les autres redirections 
            if (test_args[i + 1] == NULL) {
                fprintf(stderr, "Error file missing\n");
                free(test_args);
                return 1;
            }

            size_t length = strlen(test_args[i]) + strlen(test_args[i + 1]) + 2;
            char *redirection_str = malloc(length);
            if (!redirection_str) {
                perror("malloc");
                free(test_args);
                return 1;
            }
            strcpy(redirection_str, test_args[i]);
            strcat(redirection_str, " ");
            strcat(redirection_str, test_args[i + 1]);

            if (handle_redirection(redirection_str, 0) != 0) {
                close_redirection();
                free(redirection_str);
                free(test_args);
                return 1;
            }
            free(redirection_str);

            test_args[i] = test_args[i + 2]; 
            i--; 
        }
    }

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
    
    // Test réussi si le code de retour est 0
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    //free(original);
    return 1;
}
*/


//
/*
if (exec(test_cmd) == EXIT_SUCCESS) return exec(cmd1);
else if (has_else) return exec(cmd2);
*/
int execute_if(if_info *info) {
    if (!info || !info->test) {
        fprintf(stderr, "Erreur : if_info mal formé.\n");
        return 1;
    }
    //printf("execute_if avant | test :%s\n", info->test);
    int test_result = handle_line(info->test); // Exécute la condition via externalcmd (info->test->name ?), cause pb ?
    //printf("execute_if | test|test_result :%s|%d\n", info->test, test_result);
    if (test_result == 0) { // if (externalcmd(info->test) == EXIT_SUCCESS) { return handle_line(info->cmd_true); }
        // Condition réussie : exec cmd_true avec handle_line
        if (info->cmd_true != NULL) {
            printf("execute_if | contenu de cmd_true :%s\n", info->cmd_true);
            info->cmd_true = trim(info->cmd_true);
            printf("execute_if | contenu post trim cmd_true :%s\n", info->cmd_true);
            char *copy_true = malloc(strlen(info->cmd_true) + 1);
            if (copy_true == NULL) {
                return 1;
            }
            memcpy(copy_true, info->cmd_true, strlen(info->cmd_true) + 1);
            return handle_line(copy_true);
            /*
            if (handle_line(info->cmd_true) != 0) {
                fprintf(stderr, "Erreur lors de l'exécution de cmd_true.\n");
                return 1;
            }
            */
        }
    } else { // else if (info->cmd_false != NULL) { return handle_line(info->cmd_false); }
        // Condition échouée : exec cmd_false s'il existe avec handle_line
        if (info->cmd_false != NULL) {
            char *copy_false = malloc(strlen(info->cmd_false) + 1);
            if (copy_false == NULL) {
                return 1;
            }
            memcpy(copy_false, info->cmd_false, strlen(info->cmd_false) + 1);
            return handle_line(copy_false);
            //return handle_line(info->cmd_false);
            /*
             if (handle_line(info->cmd_false) != 0) {
                fprintf(stderr, "Erreur lors de l'exécution de cmd_false.\n");
                return 1;
            }
            */
        }
    }
    return 0;   // retour échec ? pour histoires de val retour
    /*
    // Test réussi si le code de retour est 0
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return 1;
    */
}
//

int execute_pipeline(cmd_info * cmd_info, int in_fd, int * out_fd, command * cmd) {
    int pipe_fds[2];
    pid_t pid;
    int status;

    if (cmd -> is_pipeline) {
        if (pipe(pipe_fds) < 0) {
            perror("pipe");
            return -1;
        }
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) { 
        if (in_fd != STDIN_FILENO) {
            if (dup2(in_fd, STDIN_FILENO) < 0) {
                perror("dup2 input");
                exit(EXIT_FAILURE);
            }
            close(in_fd);
        }

        if (cmd -> is_pipeline) {
            if (dup2(pipe_fds[1], STDOUT_FILENO) < 0) {
                perror("dup2 pipe");
                exit(EXIT_FAILURE);
            }
            close(pipe_fds[0]);
            close(pipe_fds[1]);
        } else if (cmd -> output_fd != STDOUT_FILENO) {
            if (dup2(cmd -> output_fd, STDOUT_FILENO) < 0) {
                perror("dup2 output_fd");
                exit(EXIT_FAILURE);
            }
        }

        if (call_function(cmd_info) < 0) {
            perror("call_function");
            exit(EXIT_FAILURE);
        }

        destroy_cmd_info(cmd_info);
        exit(EXIT_SUCCESS);
    } else { 
        if (in_fd != STDIN_FILENO) {
            close(in_fd);
        }

        if (cmd -> is_pipeline) {
            close(pipe_fds[1]); 
            *out_fd = pipe_fds[0]; 
        } else {
            *out_fd = -1; 
        }

        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return 1;
    }
}

/** handle_line permet de s'occuper de la ligne donnée en argument.*/
int handle_line(char * line) { 
    command ** cmds = handle_cmd(line);
    if (cmds == NULL) {
        free(line);
        return EXIT_FAILURE;
    }

    int result = 0;
    int in_fd = STDIN_FILENO;
    int out_fd;
    cmd_info * cmd = NULL;
    for_info * f = NULL;
    if_info * iinfo = NULL;
    for (size_t all = 0; cmds[all] != NULL; all++) {
        char * line_cmd = cmds[all] -> cmd_line;
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

        if (cmds[all] -> is_pipeline) {
            if (line_cmd[pos] != '\0') {
                memmove(line_cmd, line_cmd + pos + 1, strlen(line_cmd) - pos + 1);
                cmd_format(line_cmd, cmd);
            }

            if (cmds[all] -> input_fd >= 0) {
                in_fd = cmds[all] -> input_fd;
            }

            if (cmds[all + 1] == NULL) {
                cmds[all] -> is_pipeline = false;
            }

            result = execute_pipeline(cmd, in_fd, &out_fd, cmds[all]);

            if (result != 0) {
                fprintf(stderr, "Erreur d'exécution pour la commande : %s\n", cmds[all]->cmd_line);
                break;
            }

            if (in_fd != STDIN_FILENO) {
                close(in_fd);
            }
            in_fd = out_fd;
        } else {
            if (strcmp(cmd -> name, "") == 0) {
                result = 0;
            } else if (strcmp(cmd -> name, "if") == 0) {
                iinfo = handle_if_line(line);
                if (iinfo == NULL) {
                    result = 1;
                    goto error;
                }
                //printf("handle_line | pre execute, iinfo :%s\n", iinfo->test->name);
                //printf("handle_line | pre execute, iinfo_true :%s\n", iinfo->cmd_true);
                result = execute_if(iinfo);
                if(iinfo->test != NULL) {
                    iinfo->test = NULL;
                }
                printf("handle_line | contenu de iinfo :%s\n", iinfo->test);
                printf("handle_line | contenu de iinfo_true :%s\n", iinfo->cmd_true);   // pour pb du free de cmd_true, pointeur de line ?
                /*if(iinfo->cmd_true != NULL) {
                    iinfo->cmd_true = NULL;
                }
                if(iinfo->cmd_false != NULL) {
                    iinfo->cmd_false = NULL;
                }*/
                //printf("handle_line | contenu de iinfo_true post bled :%s\n", iinfo->cmd_true);   // pour pb du free de cmd_true, pointeur de line ?
                //printf("handle_line | contenu de line :%s\n", line);
                /* Libération de la mémoire de if_info après exécution
                printf("handle_line | appel de destroyifinfo\n");
                destroy_if_info(iinfo);
                printf("handle_line | appel du destroy fini\n");
                iinfo = NULL; */

                //return result; // Retourner le résultat de l'exécution du if
                /*
                if (test_result == 0) {
                    if (iinfo -> cmd_true != NULL) {
                        if (handle_redirection(iinfo -> cmd_true, 0) != 0) {
                            result = 1;
                            goto error;
                        }
                        result = handle_line(iinfo -> cmd_true);
                        close_redirection();
                        destroy_cmd_info(iinfo -> test);
                        iinfo -> test = NULL;
                        free(iinfo -> cmd_false);
                        iinfo -> cmd_false = NULL;
                        free(iinfo);
                        iinfo = NULL;
                    } else {
                        result = 1;
                        goto error;
                    }
                } else {
                    if (iinfo -> cmd_false != NULL) {
                        if (handle_redirection(iinfo -> cmd_false, 0) != 0) {
                            result = 1;
                            goto error;
                        }
                        result = handle_line(iinfo -> cmd_false);
                        close_redirection();
                        destroy_cmd_info(iinfo -> test);
                        iinfo -> test = NULL;
                        free(iinfo -> cmd_true);
                        iinfo -> cmd_true = NULL;
                        free(iinfo);
                        iinfo = NULL;
                    } else {
                        result = 1;
                        goto error;
                    }
                } */
            } else if (strcmp(cmd -> name, "for") != 0) {
                if (handle_redirection(line_cmd, pos) != 0) {
                    result = 1;
                    goto error;
                }
                if (line_cmd[pos] != '\0') {
                    memmove(line_cmd, line_cmd + pos + 1, strlen(line_cmd) - pos + 1);
                    cmd_format(line_cmd, cmd);
                }
                result = call_function(cmd);
                close_redirection();
            } else {
                if (line_cmd[pos] != '\0') {
                    memmove(line_cmd, line_cmd + pos + 1, strlen(line_cmd) - pos + 1);
                }
                f = handle_for_line(line_cmd);
                if (f == NULL) {
                    result = 2;
                    goto error;
                }
                result = handle_for(f);
            }
            in_fd = STDIN_FILENO;
        }

        if (cmd != NULL) {
            destroy_cmd_info(cmd);
            cmd = NULL;
        }
    }

    if (iinfo != NULL) {
        //printf("vers fin du handle_line, clean iinfo\n");
        //printf("handle_line | appel de destroyifinfo\n");
        destroy_if_info(iinfo);
        //printf("handle_line | appel du destroy fini\n");
        iinfo = NULL;
    }

    close_redirection();
    
    if (in_fd != STDIN_FILENO) {
        close(in_fd);
    }

    if (cmds != NULL) {
        destroy_cmd(cmds);
        cmds = NULL;
    }
    free(line);
    //line = NULL;
    return result;

error:
    close_redirection();
    if (cmd != NULL) {
        destroy_cmd_info(cmd);
        cmd = NULL;
    }
    if (cmds != NULL) {
        destroy_cmd(cmds);
        cmds = NULL;
    }
    free(line);
    //line = NULL;
    if (iinfo != NULL) {
        destroy_if_info(iinfo);
        iinfo = NULL;
    }
    return result;
}

command ** handle_cmd(char * line) {
    size_t pos = 0, nb_cmd = 0, pos_cmd = 0, for_cmd = 0;
    command ** cmd = calloc(BUFFER_SIZE, sizeof(command *));
    if (cmd == NULL) {
        perror("calloc");
        return NULL;
    }

    bool is_pipeline = false;
    while (line[pos] != '\0') {
        if (cmd[nb_cmd] == NULL) {
            cmd[nb_cmd] = calloc(1, sizeof(command));
            if (cmd[nb_cmd] == NULL) {
                perror("calloc cmd");
                for (size_t i = 0; cmd[i] != NULL; i++) {
                    free(cmd[i] -> cmd_line);
                    free(cmd[i]);
                }
                free(cmd);
                return NULL;
            }

            cmd[nb_cmd] -> cmd_line = calloc(BUFFER_SIZE, sizeof(char));
            if (cmd[nb_cmd] -> cmd_line == NULL) {
                perror("calloc cmd_line");
                for (size_t i = 0; cmd[i] != NULL; i++) {
                    free(cmd[i] -> cmd_line);
                    free(cmd[i]);
                }
                free(cmd[nb_cmd]);
                free(cmd);
                return NULL;
            }

            cmd[nb_cmd] -> is_pipeline = is_pipeline;
            cmd[nb_cmd] -> input_fd = (nb_cmd == 0) ? STDIN_FILENO : -1; 
            cmd[nb_cmd] -> output_fd = -1; 
            is_pipeline = false; 
        }

        if (line[pos] == '{') {
            for_cmd++;
            cmd[nb_cmd] -> cmd_line[pos_cmd++] = line[pos++];
            continue;
        } else if (line[pos] == '}') {
            for_cmd--;
            cmd[nb_cmd] -> cmd_line[pos_cmd++] = line[pos++];
            continue;
        }

        if (strcmp(cmd[nb_cmd] -> cmd_line, "yes") == 0) {
            cmd[nb_cmd] -> cmd_line = strdup("timeout 0.1s yes");
            if (cmd[nb_cmd] -> cmd_line == NULL) {
                perror("strdup");
                for (size_t i = 0; cmd[i] != NULL; i++) {
                    free(cmd[i] -> cmd_line);
                    free(cmd[i]);
                }
                free(cmd);
                return NULL;
            }
            pos_cmd = pos_cmd + 13;
        }

        if ((line[pos] == ';' || (line[pos] == '|') && line[pos - 1] != '>') && for_cmd == 0) {
            if (line[pos] == '|') {
                cmd[nb_cmd] -> is_pipeline = true;
                is_pipeline = true;
            }
            cmd[nb_cmd] -> cmd_line[pos_cmd] = '\0';
            pos++;
            nb_cmd++;
            pos_cmd = 0;
            continue;
        }

        cmd[nb_cmd] -> cmd_line[pos_cmd++] = line[pos++];
    }

    if (cmd[nb_cmd] != NULL) {
        cmd[nb_cmd] -> output_fd = STDOUT_FILENO;
    }

    cmd[nb_cmd + 1] = NULL;
    return cmd;
}

void destroy_cmd(command ** cmd) {
    for (size_t i = 0; cmd[i] != NULL; i++) {
        free(cmd[i] -> cmd_line);
        free(cmd[i]);
    }
    free(cmd);
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

            for (size_t j = redirection_size; j < pos + strlen(redirection_file + strlen(redirection_type) + 1); j++) {
                line[j] = ' ';
            }
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