#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmd.h"

cmd_info * cmd_info_init() {
    cmd_info * c = malloc(sizeof(cmd_info));
    if (c == NULL) {
        goto error;
    }

    c -> name = calloc(CMD_SIZE, sizeof(char));
    if (c -> name == NULL) { 
        goto error;
    }

    c -> opt = calloc(OPTIONS_SIZE, sizeof(char));
    if (c -> opt == NULL) {
        goto error;
    }

    c -> args = calloc(ARG_SIZE, sizeof(char));
    if (c -> args == NULL) {
        goto error;
    }
    return c;

    error:
    printf("NULL");
    free(c -> name);
    free(c -> opt);
    free(c -> args);
    free(c);
    return NULL;
}

void destroy_cmd_info(cmd_info * cmd) {
    free(cmd -> name);
    free(cmd -> opt);
    free(cmd -> args);
    free(cmd);
}
