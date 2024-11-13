#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "cmd.h"

int ftype(cmd_info * cmd) {
    struct stat s;

    if (stat(cmd -> args, &s) == 0) {
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
        write(STDERR_FILENO, cmd -> args, (unsigned) strlen(cmd -> args));
        write(STDERR_FILENO, ": invalid path\n", 15);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
