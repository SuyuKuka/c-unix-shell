#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "pwd.h"

int pwd() {
    char * pwd = malloc(PWD_SIZE);

    if (pwd == NULL) {
        goto error;
    }

    getcwd(pwd, PWD_SIZE);
    strcat(pwd, "\n");
    write(STDOUT_FILENO, pwd, strlen(pwd));

    free(pwd);
    return 0;

    error:
    free(pwd);
    return 1;
}