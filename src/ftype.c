#include <sys/stat.h>
#include <unistd.h>
#include "cmd.h"

int ftype(cmd_info * cmd) {
    struct stat s;
    char * result;

    if (lstat(cmd -> args, &s) == 0) {
        if (S_ISDIR(s.st_mode)) {
            result = "directory\n";
        } else if(S_ISREG(s.st_mode)) {
            result = "regular file\n";
        } else if (S_ISLNK(s.st_mode)) {
            result = "symbolic link\n";
        } else if (S_ISFIFO(s.st_mode)) {
            result = "named pipe\n";
        } else {
            result = "other\n";
        }
        write(STDOUT_FILENO, result, strlen(result));
    } else {
        perror("ftype");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
