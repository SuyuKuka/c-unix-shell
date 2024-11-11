#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

int externalcmd(char ** argv) {
    pid_t pid = fork();

    if (pid == -1) {
        write(STDOUT_FILENO, "Error fork: problem creating a child process\n", 45);
        return EXIT_FAILURE;
    } else if (pid == 0) {
        if (execvp(argv[0], argv) == -1) {
            write(STDOUT_FILENO, argv[0], strlen(argv[0]));
            write(STDOUT_FILENO, ": command not found\n", 20);
            _exit(EXIT_FAILURE);
        }
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int main(int argc, char ** argv) {
    if (argc < 2) {
        write(STDOUT_FILENO, "Not enough argument\n", 20);
        return 1;
    }

    return externalcmd(argv + 1);
}
