#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "cmd.h"

void destroy_split(char ** arr, int * size) {
    if (arr != NULL) {
        for (size_t i = 0; i < *size; i++) {
            free(arr[i]);
        }
    }
    free(arr);
}

//Sépare la commande, les options et les arguments dans un char **
char ** split(cmd_info * cmd, int * size) {
    char ** cmd_split = NULL;
    size_t split_index = 0;
    int i = 0;
    size_t space = 0;
    size_t opt_len = strlen(cmd -> opt);
    size_t args_len = strlen(cmd -> args);
    
    //On compte le nombre d'options et d'arguments.
    if (strlen(cmd -> opt) > 0) {
        while(cmd -> opt[i] != '\0') {
            if(isspace(cmd -> opt[i]) != 0) {
                space++;
            }
            i++;
        }
        space++;
        i = 0;
    }

    if (strlen(cmd -> args) > 0) {
        while(cmd -> args[i] != '\0') {
            if(isspace(cmd -> args[i]) != 0) {
                space++;
            }
            i++;
        }
        space++;
        i = 0;
    }

    //Taille : le nombre de paramètres séparés d'espace plus le nom de la commande.  
    *size = space + 1;
    cmd_split = (char **) malloc((*size + 1) * sizeof(char *));
    if (cmd_split == NULL) {
        *size = 0;
        goto error;
    }

    //Ajout du nom de la commande.
    cmd_split[split_index] = calloc(strlen(cmd -> name) + 1, sizeof(char));
    if (cmd_split[split_index] == NULL) {
        goto error;
    }
    memcpy(cmd_split[split_index], cmd -> name, strlen(cmd -> name) + 1);
    split_index++;

    //TODO : Déléguer à une fonction la copie.
    // Séparation des options et des arguments dans le char **
    if (opt_len > 0) {
        char * tmp = NULL;
        size_t tmp_len = 0;
        tmp = calloc(strlen(cmd -> opt) + 1, sizeof(char));
        if (tmp == NULL) {goto error;}
        while(cmd -> opt[i] != '\0') {
            if (isspace(cmd -> opt[i]) == 0) {
                tmp[strlen(tmp)] = cmd -> opt[i];
            } else {
                cmd_split[split_index] = malloc(strlen(tmp) + 1 * sizeof(char));
                if (cmd_split[split_index] == NULL) {
                    *size = split_index;
                    goto error;
                }
                memmove(cmd_split[split_index], tmp, strlen(tmp) + 1);
                split_index++;
                tmp_len = strlen(tmp);
                for (size_t t = 0; t < tmp_len; t++) {
                    tmp[t] = '\0';
                }
            }
            i++;
        }
        cmd_split[split_index] = malloc(strlen(tmp) + 1 * sizeof(char));
        if (cmd_split[split_index] == NULL) {
            *size = split_index;
            goto error;
        }
        memmove(cmd_split[split_index], tmp, strlen(tmp) + 1);
        split_index++;
        free(tmp);
        tmp = NULL;
        i = 0;
    }

    if (args_len > 0) {
        char * tmp = NULL;
        size_t tmp_len = 0;
        tmp = calloc(strlen(cmd -> args) + 1, sizeof(char));
        if (tmp == NULL) {goto error;}
        while(cmd -> args[i] != '\0') {
            if (isspace(cmd -> args[i]) == 0) {
                tmp[strlen(tmp)] = cmd -> args[i];
            } else {
                cmd_split[split_index] = malloc(strlen(tmp) + 1 * sizeof(char));
                if (cmd_split[split_index] == NULL) {
                    *size = split_index;
                    goto error;
                }
                memmove(cmd_split[split_index], tmp, strlen(tmp) + 1);
                split_index++;
                tmp_len = strlen(tmp);
                for (size_t t = 0; t < tmp_len; t++) {
                    tmp[t] = '\0';
                }
            }
            i++;
        }
        cmd_split[split_index] = malloc(strlen(tmp) + 1 * sizeof(char));
        if (cmd_split[split_index] == NULL) {
            *size = split_index;
            goto error;
        }
        memmove(cmd_split[split_index], tmp, strlen(tmp) + 1);
        split_index++;
        free(tmp);
        tmp = NULL;
    }

    cmd_split[split_index] = NULL;
    return cmd_split;

    error:
    destroy_split(cmd_split, size);
    return NULL;
}

int externalcmd(cmd_info *cmd) { 
    pid_t pid = fork();
    if (pid == -1) {
        write(STDERR_FILENO, "Error with fork: problem creating a child process\n", 45);
        return 1;
    } else if (pid == 0) {
        char ** cmd_split = NULL;
        int * size = NULL;
        
        size = calloc(1, sizeof(int));
        if(size == NULL) {return 1;}        
        
        cmd_split = split(cmd, size);
        if  (cmd_split == NULL) {
            free(size);
            _exit(1);
        }

        if (execvp(cmd_split[0], cmd_split) == -1) {
            perror("execvp");
            destroy_split(cmd_split, size);
            free(size);
            _exit(1);
        }

        destroy_split(cmd_split, size);
        free(size);
    } else {
        int status;
        waitpid(pid, &status, 0);
        //Vérification du status à la sortie du processus fils.
        if (WIFSIGNALED(status)) {
            int signal = WTERMSIG(status);
            //Si le processus fils a quitté par réception de SIGINT on retourne 2.
            if (signal == 2) {
                return signal;
            }
            return 255;
        } else if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                return exit_status;
            }
        }
    }
    return 0;
}

