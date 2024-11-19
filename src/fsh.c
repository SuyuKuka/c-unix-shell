#include "fsh.h"

int fsh_exit(cmd_info * cmd) {
    int flag = 1;
    size_t args_length = strlen(cmd -> args);
    for (size_t i = 0; i < args_length; i++) {
        if (!isdigit(cmd -> args[i])) {
            flag = 0;
        }
    }
    if (strlen(cmd -> opt) > 0 || flag == 0) {
        char * error = "Too many arguments or wrong value.\n";
        write(STDERR_FILENO, error, strlen(error));
        return 1;
    } else if (strlen(cmd -> args) > 0) {
        int n  = atoi(cmd -> args);
        destroy_cmd_info(cmd);
        exit(n);
    } else {
        destroy_cmd_info(cmd);
        exit(0);
    }
}

void cmd_format(char * line, cmd_info * cmd) {
    size_t i = 0;
    while(isspace(line[i]) != 0) {
        i++;
    }
    if (line[i] == '-') {
        while(isspace(line[i]) == 0 && line[i] != '\0') {
            cmd -> opt[strlen(cmd -> opt)] = line[i];
            i++;
        }

        cmd -> opt[strlen(cmd -> opt)] = line[i];
        if (strlen(line) > 0) {
            memmove(line, line + i, strlen(line) - i + 1);
            cmd_format(line, cmd);
        }
    } else if (line[i] != '\0') {
        while(isspace(line[i]) == 0 && line[i] != '\0') {
            cmd -> args[strlen(cmd -> args)] = line[i];
            i++;
        }
        cmd -> args[strlen(cmd -> args)] = line[i];
        if (strlen(line) > 0) {
            memmove(line, line + i, strlen(line) - i + 1);
            cmd_format(line, cmd);
        }
    }
}

int call_function (cmd_info * cmd) {
    if (strcmp("pwd", cmd -> name) == 0) {
        return pwd(cmd);
    } else if (strcmp("exit", cmd -> name) == 0) {
        return fsh_exit(cmd);
    } else if (strcmp(cmd -> name, "ftype") == 0) {
        return ftype(cmd);
    } else {
        return externalcmd(cmd);
    }
}

/** handle_line permet de s'occuper de la ligne donnée en argument.*/
int handle_line(char * line) {
    int result = 0;
    cmd_info * cmd = NULL;
    cmd = cmd_info_init();
    if (cmd == NULL) {
        result = 1;
        goto error;
    }
    
    size_t i = 0;

    while(isspace(line[i]) != 0) {
        i++;
    }
   
    while(isspace(line[i]) == 0 && line[i] != '\0') {
        cmd -> name[i] = line[i];
        i++;
    }

    if (line[i] != '\0') {
        memmove(line, line + i + 1, strlen(line) - i + 1);
        cmd_format(line, cmd);
    }
    
    free(line);

    result = call_function(cmd);
    
    destroy_cmd_info(cmd);
    return result;
    
    error:
    destroy_cmd_info(cmd);
    return result;
}

//TODO: Faire en sorte que la couleur par défaut soit pris en compte.
/** Fonction permettant de changer la couleur du prompt. */
void color_prompt(char * prompt, int var) {
    size_t color_len = 0;
    size_t cyan_len = strlen(CYAN);
    size_t var_len = 3;
    if (var == -1) {
        color_len = strlen(DEFAULT_COLOR);
        memmove(prompt + color_len, prompt, strlen(prompt));
        memmove(prompt, DEFAULT_COLOR, color_len);
        memmove(prompt + color_len + var_len + cyan_len, prompt + color_len + var_len, strlen(prompt) - var_len);
        memmove(prompt + color_len + var_len, CYAN, cyan_len);
    } else if (var == 0) {
        color_len = strlen(GREEN);
        memmove(prompt + color_len, prompt, strlen(prompt));
        memmove(prompt, GREEN, color_len);
        memmove(prompt + color_len + var_len + cyan_len, prompt + color_len + var_len, strlen(prompt) - var_len);
        memmove(prompt + color_len + var_len, CYAN, cyan_len);
    } else {
        color_len = strlen(RED);
        memmove(prompt + color_len, prompt, strlen(prompt));
        memmove(prompt, RED, color_len);
        memmove(prompt + color_len + var_len + cyan_len, prompt + color_len + var_len, strlen(prompt) - var_len);
        memmove(prompt + color_len + var_len, CYAN, cyan_len);
    }
}

/** Adapte la taille du prompt et remplie le reste de '\0'. */
void make_prompt(char * prompt, int var) {
    size_t current_size = strlen(prompt);
    size_t size = PROMPT_SIZE - 3;
    char return_value[4] = "\0";
    return_value[0] = '[';
    if (var == -1) { 
        return_value[1] = '0';
    } else {
        return_value[1] = ((char) var) + '0';
    }
    return_value[2] = ']';
    return_value[4] = '\0';
    if (current_size > size) {
        memmove(prompt, prompt+(current_size - size), size);
        prompt[size] = '\0';
        memmove(prompt, return_value, strlen(return_value));
        for (size_t i = 3; i < 6; i++) {
            prompt[i] = '.';
        } 

        for (size_t i = strlen(prompt); i < BUFFER_SIZE; i++) {
            prompt[i] = '\0';
        }

    } else {
        memmove(prompt+3, prompt, size);
        memmove(prompt, return_value, strlen(return_value));
    }

    color_prompt(prompt, var);
    strcat(prompt, DEFAULT_COLOR);
    strcat(prompt, "$ \0");
}

int display_prompt(int var) {
    int result = 0;
    char * prompt = NULL;
    char * command = NULL;
    char * line = NULL;
    prompt = malloc(BUFFER_SIZE * sizeof(char));
    command = malloc(BUFFER_SIZE * sizeof(char));
    
    if (prompt == NULL || command == NULL) {
        result = 1;
        goto cleanup;
    }

    getcwd(prompt, BUFFER_SIZE);
    make_prompt(prompt, var);
    
    line = readline(prompt);
    add_history(line);
    memmove(command, line, strlen(line) + 1);

    free(line);
    free(prompt);

    result = handle_line(command);

    return display_prompt(result);

    cleanup:
    free(line);
    free(prompt);
    free(command);
    return result;
}

int main() {
    rl_outstream = stderr;
    display_prompt(-1);
}