#include "cmd.h"

cmd_info * cmd_info_init() {
    cmd_info * c = NULL; 
    c = malloc(3 * sizeof c);
    if (c == NULL) {goto error;}
    c -> cmd_name = calloc(CMD_SIZE, sizeof(char));
    if (c -> cmd_name == NULL) {goto error;}
    c -> opt = calloc(OPTIONS_SIZE, sizeof(char));
    if (c -> opt == NULL) {goto error;}
    c -> args = calloc(ARG_SIZE, sizeof(char));
    if (c -> args == NULL) {goto error;}
    return c;

    error:
    free(c -> cmd_name);
    free(c -> opt);
    free(c -> args);
    free(c);
    return NULL;
}

void destroy_cmd_info(cmd_info * cmd) {
    free(cmd -> cmd_name);
    free(cmd -> opt);
    free(cmd -> args);
    free(cmd);
}

char * get_cmd_name(cmd_info * cmd) {
    return cmd -> cmd_name;
}

char * get_opt(cmd_info * cmd) {
    return cmd -> opt;
}

char * get_args(cmd_info * cmd) {
    return cmd -> args;
}