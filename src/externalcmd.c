#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "cmd.h"

int externalcmd(cmd_info *cmd) {
    pid_t pid = fork();
    if (pid == -1) {
        write(STDERR_FILENO, "Error with fork: problem creating a child process\n", 45);
        return EXIT_FAILURE;
    } else if (pid == 0) {
        int argc = 0;
        char ** argv = malloc(CMD_SIZE);
        argv[argc++] = cmd -> name;

        if (strcmp(cmd -> opt, "") != 0) {
            argv[argc++] = cmd -> opt;
        }

        if (strcmp(cmd -> args, "") != 0) {
            argv[argc++] = cmd -> args;
        }

        argv[argc] = NULL;
        for(int i = 0; i < argc + 1; i++) {
            printf("argv[%d] = %s\n", i, argv[i]);
        }

        if (execvp(cmd -> name, argv) == -1) {
            perror("execvp");
            _exit(EXIT_FAILURE);
        }
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            write(STDERR_FILENO, "Error in child process\n", 24);
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
