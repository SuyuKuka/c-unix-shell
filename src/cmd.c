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
    destroy_cmd_info(c);
    return NULL;
}

void destroy_cmd_info(cmd_info * cmd) {
    if (cmd != NULL) {
        free(cmd -> name);
        cmd -> name = NULL;
        free(cmd -> opt);
        cmd -> opt = NULL;
        free(cmd -> args);
        cmd -> args = NULL;
    }
    free(cmd);
}

void cmd_format(char * line, cmd_info * cmd) {
    size_t i = 0;
    while(isspace(line[i]) != 0) { i++; }
    if (line[i] == '-' && isspace(line[i + 1]) == 0) {
        while(isspace(line[i]) == 0 && line[i] != '\0') {
            cmd -> opt[strlen(cmd -> opt)] = line[i];
            i++;
        }
        while(isspace(line[i]) != 0) {i++;}
        if (line[i] != '\0' && line[i] == '-') {
            strcat(cmd -> opt, " ");
            memmove(line, line + i, strlen(line) - i + 1);
            cmd_format(line, cmd);
        } else if (line[i] != '\0') {
            memmove(line, line + i, strlen(line) - i + 1);
            cmd_format(line, cmd);
        }
    } else if (line[i] != '\0') {
        while (isspace(line[i]) == 0 && line[i] != '\0') {
            cmd -> args[strlen(cmd -> args)] = line[i];
            i++;
        }
        while (isspace(line[i]) != 0) { i++; }
        if (line[i] != '\0' || (line[i] == '-' && isspace(line[i + 1]) == 0)) {
            strcat(cmd -> args, " ");
            memmove(line, line + i, strlen(line) - i + 1);
            cmd_format(line, cmd);
        } else if (line[i] != '\0') {
            memmove(line, line + i, strlen(line) - i + 1);
            cmd_format(line, cmd);
        }
    }
}
