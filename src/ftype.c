#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

int ftype(char ** argv) {
    struct stat s;

    if (stat(argv[0], &s) == 0) {
        if (S_ISDIR(s.st_mode)) {
            write(STDOUT_FILENO, "directory\n", 10);
        } else if(S_ISREG(s.st_mode)) {
            write(STDOUT_FILENO, "file\n", 5);
        } else if (S_ISLNK(s.st_mode)) {
            write(STDOUT_FILENO, "symbolic link\n", 14);
        } else if (S_ISFIFO(s.st_mode)) {
            write(STDOUT_FILENO, "named pipe\n", 11);
        } else {
            write(STDOUT_FILENO,"other\n", 6);
        }
    } else {
        write(STDOUT_FILENO, argv[1], strlen(argv[1]));
        write(STDOUT_FILENO, ": invalid path", 6);
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

int main(int argc, char ** argv) {
    if (argc < 2) {
        write(STDOUT_FILENO, "Not enough argument\n", 20);
        return EXIT_FAILURE;
    }

    return ftype(argv + 1);
}
