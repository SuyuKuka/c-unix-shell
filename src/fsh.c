#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "pwd.h"
#include "cmd.h"
#define BUFFER_SIZE 512
#define PROMPT_SIZE 30
#define DEFAULT_COLOR "\033[00m"
#define GREEN "\033[32m"
#define RED "\033[91m"
#define CYAN "\033[36m"

//TODO exit();

void cmd_format(char * line, cmd_info * cmd) {
    size_t i = 0;
    if (line[0] == '-') {
        while(isspace(line[i]) == 0 && line[i] != '\0') {
            cmd -> opt[strlen(cmd -> opt)] = line[i];
            i++;
        }
        cmd -> opt[strlen(cmd -> opt)] = line[i];
        if (strlen(line) > i) {
            memmove(line, line + i + 1, strlen(line) - i + 2);
        } else {
            memmove(line, line + i, strlen(line) - i + 1);
        }
        cmd_format(line, cmd);
    } else if (line[0] != '\0') {
        while(line[i] != '\0') {
            cmd -> args[strlen(cmd -> args)] = line[i];
            i++;
        }
        memmove(line, line + i + 1, strlen(line) - i + 1);
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
   
    while(isspace(line[i]) == 0 && line[i] != '\0') {
        cmd -> cmd_name[i] = line[i];
        i++;
    }

    if (line[i] != '\0') {
        memmove(line, line + i + 1, strlen(line) - i + 1);
        cmd_format(line, cmd);
    }

    if (strcmp(cmd -> cmd_name,"pwd") == 0) {
        result = pwd(cmd);
    } else {
        result = 1;
    }
    
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
    if (var == 0) {
        color_len = strlen(GREEN);
        memmove(prompt + color_len, prompt, strlen(prompt));
        memmove(prompt, GREEN, color_len);
        memmove(prompt + color_len + var_len + cyan_len, prompt + color_len + var_len, strlen(prompt) - var_len);
        memmove(prompt + color_len + var_len, CYAN, cyan_len);
    } else if (var == 1) {
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
    return_value[1] = ((char) var) + '0';
    return_value[2] = ']';
    return_value[4] = '\0';
    if (current_size > size) {
        memmove(prompt, prompt+(current_size - size),size);
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
    color_prompt(prompt,var);
    strcat(prompt, DEFAULT_COLOR);
    strcat(prompt,"$ \0");
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

    if (strcmp(line, "exit") == 0) {
        result = 0;
        goto cleanup;
    }
    result = handle_line(command);

    free(line);
    free(prompt);
    free(command);
    return display_prompt(result);

    cleanup:
    free(line);
    free(prompt);
    free(command);
    return result;
}

int main(int argc, char** argv) {
    rl_outstream = stderr;
    display_prompt(0);
}