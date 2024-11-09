#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int externalcmd(char * argv[]) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("Erreur de fork");
        return EXIT_FAILURE;
    } else if (pid == 0) {
        if (execvp(argv[0], argv) == -1) {
            perror(argv[0]);
            perror(": command not found");
            return EXIT_FAILURE;
        }
    } else {
        int * status;
        waitpid(pid, status, 0);
    }

    return EXIT_SUCCESS;
}

int main() {
    char * test[] = {"ls", "-l", "../", NULL};

    if (externalcmd(test) == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}