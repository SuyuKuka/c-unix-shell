#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int redirection(const char * redirection, char * file) {
    int fd;

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
