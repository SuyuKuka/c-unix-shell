#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "cmd.h"

int split(const char *str, char **argv) {
    int argc = 0;
    const char *start = str;
    int len = strlen(str);
    char *arg = malloc(len + 1);

    if (!arg) {
        perror("malloc");
        return -1;
    }

    int arg_len = 0;
    for (int i = 0; i <= len; i++) {
        if (isspace(str[i]) || str[i] == '\0') {
            if (arg_len > 0) {
                arg[arg_len] = '\0';
                argv[argc] = malloc(arg_len + 1);
                if (argv[argc] == NULL) {
                    free(arg);
                    perror("malloc");
                    return -1;
                }
                
                memcpy(argv[argc], arg, arg_len + 1);
                argc++;
                arg_len = 0; 
            }
        } else {
            arg[arg_len++] = str[i];
        }
    }

    free(arg);
    return argc;
}

int externalcmd(cmd_info *cmd) { 
    pid_t pid = fork();
    if (pid == -1) {
        write(STDERR_FILENO, "Error with fork: problem creating a child process\n", 45);
        return EXIT_FAILURE;
    } else if (pid == 0) {
        int argc = 0;
        char **argv = malloc(4 * sizeof(char**));
        if (!argv) {
            perror("malloc");
            _exit(EXIT_FAILURE);
        }

        argv[argc++] = cmd -> name;
        if (strcmp(cmd -> opt, "") != 0) {
            int nb_opt = split(cmd -> opt, argv + argc);
            if (nb_opt < 0) {
                free(argv);
                perror("split opt");
                return EXIT_FAILURE;
            }
            argc += nb_opt;
        }

        if (strcmp(cmd -> args, "") != 0) {
            int nb_args = split(cmd -> args, argv + argc);
            if (nb_args < 0) {
                free(argv);
                perror("split arg");
                return EXIT_FAILURE;
            }
            argc += nb_args;
        }

        argv[argc] = NULL;

        if (execvp(argv[0], argv) == -1) {
            free(argv);
            perror("execvp");
            _exit(EXIT_FAILURE);
        }
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                return exit_status;
            }
        }
    }
    return EXIT_SUCCESS;
}

