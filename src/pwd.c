#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "pwd.h"

int pwd(cmd_info * cmd) {
    if (strlen(cmd -> opt) == 0 && strlen(cmd -> args) == 0) {
        return redirect_pwd(STDOUT_FILENO);
    }

    char * error = "Too many options or arguments.\n";
    write(STDERR_FILENO, error, (unsigned) strlen(error));
    return EXIT_FAILURE;
}

int redirect_pwd(int output) {
    char * pwd = malloc(PWD_SIZE);
    if (pwd == NULL) {
        goto error;
    }

    getcwd(pwd, PWD_SIZE);
    strcat(pwd, "\n");
    write(output, pwd, (unsigned) strlen(pwd));
    free(pwd);
    return EXIT_SUCCESS;

    error:
    free(pwd);
    return EXIT_FAILURE;
}