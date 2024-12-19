#include "redirection.h"

int saved_stdin = -1;
int saved_stdout = -1;
int saved_stderr = -1;

int redirection(char * redirection, const char * file) {
    int fd;

    if (saved_stdin == -1) saved_stdin = dup(STDIN_FILENO);
    if (saved_stdout == -1) saved_stdout = dup(STDOUT_FILENO);
    if (saved_stderr == -1) saved_stderr = dup(STDERR_FILENO);

    if (strcmp(redirection, "<") == 0) {
        fd = open(file, O_RDONLY);
        if (fd == -1) {
            perror("open");
            return EXIT_FAILURE;
        }

        if (dup2(fd, STDIN_FILENO) == -1) {
            perror("dup2");
            close(fd);
            return EXIT_FAILURE;
        }
        close(fd);
    } else if (strcmp(redirection, ">") == 0) {
        fd = open(file, O_CREAT | O_EXCL | O_WRONLY, 0666);
        if (fd == -1) {
            perror("open");
            return EXIT_FAILURE;
        }

        if (dup2(fd, STDOUT_FILENO) == -1) { 
            perror("dup2");
            close(fd);
            return EXIT_FAILURE;
        }
        close(fd);
    } else if (strcmp(redirection, ">|") == 0) {
        fd = open(file, O_TRUNC | O_WRONLY | O_CREAT, 0666);
        if (fd == -1) {
            perror("open");
            return EXIT_FAILURE;
        }

        if (dup2(fd, STDOUT_FILENO) == -1) { 
            perror("dup2");
            close(fd);
            return EXIT_FAILURE;
        }
        close(fd);
    } else if (strcmp(redirection, ">>") == 0) {
        fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd == -1) {
            perror("open");
            return EXIT_FAILURE;
        }

        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("dup2");
            close(fd);
            return EXIT_FAILURE;
        }
        close(fd);
    } else if (strcmp(redirection, "2>") == 0) {
        fd = open(file, O_CREAT | O_EXCL | O_WRONLY, 0666);
        if (fd == -1) {
            perror("open");
            return EXIT_FAILURE;
        }

        if (dup2(fd, STDERR_FILENO) == -1) {
            perror("dup2");
            close(fd);
            return EXIT_FAILURE;
        }
        close(fd);
    } else if (strcmp(redirection, "2>|") == 0) {
        fd = open(file, O_TRUNC | O_WRONLY | O_CREAT, 0666);
        if (fd == -1) {
            perror("open");
            return EXIT_FAILURE;
        }
        
        if (dup2(fd, STDERR_FILENO) == -1) {
            perror("dup2");
            close(fd);
            return EXIT_FAILURE;
        }
        close(fd);
    } else if (strcmp(redirection, "2>>") == 0) {
        fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd == -1) {
            perror("open");
            return EXIT_FAILURE;
        }
        
        if (dup2(fd, STDERR_FILENO) == -1) {
            perror("dup2");
            close(fd);
            return EXIT_FAILURE;
        }
        close(fd);
    } else {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void close_redirection() {
    if (saved_stdin != -1) {
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
        saved_stdin = -1;
    }
    if (saved_stdout != -1) {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
        saved_stdout = -1;
    }
    if (saved_stderr != -1) {
        dup2(saved_stderr, STDERR_FILENO);
        close(saved_stderr);
        saved_stderr = -1;
    }
}