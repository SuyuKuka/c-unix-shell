#include "for.h"

for_info * for_info_init() {
    for_info * f = malloc(sizeof(for_info));
    if (f == NULL) {goto error;}

    f -> directory = calloc(DIR_SIZE , sizeof(char));
    if (f -> directory == NULL) {goto error;} 

    f -> command = malloc(CMD_SIZE);
    if (f -> command == NULL) {goto error;}

    return f;

    error:
    destroy_for_info(f);
    return NULL;
}

void destroy_for_info(for_info * f) {
    if (f != NULL) {
        free(f -> directory);
        free(f -> command);
    }
    free(f);
}

for_info * handle_for_line(char * line) {
    size_t i = 0;
    size_t end = strlen(line) - 1;
    for_info * f = NULL;
    f = for_info_init();
    if (f == NULL) {goto error;} 
    while(isspace(line[i]) != 0) {i++;}
    if (line[i] == '\0') {
        write(STDERR_FILENO, SYNTAX_ERROR, strlen(SYNTAX_ERROR));
        goto error;
    }
    while (end > 0 && isspace(line[end]) != 0) {end--;}
    if (line[end] != '}' || line[end - 1] != ' ') {
        write(STDERR_FILENO, SYNTAX_ERROR, strlen(SYNTAX_ERROR));
        goto error;
    }
    line[end] = '\0';
    line[end-1] = '\0';

    //On sauvegarde la référence du fichier.
    if(line[i] != 'F' && line[i+1] != ' ') {
        write(STDERR_FILENO, SYNTAX_ERROR, strlen(SYNTAX_ERROR));
        goto error;
    }
    i++;
    while(isspace(line[i]) != 0) {i++;}

    //On vérifie la syntaxe.
    if (line[i] != 'i' || line[i+1] != 'n') {
        write(STDERR_FILENO, SYNTAX_ERROR, strlen(SYNTAX_ERROR));
        goto error;
    } 
    i += 3;
    while(isspace(line[i]) != 0) {i++;}

    //Sauvegarde du répertoire.
    while(isspace(line[i]) == 0 && line[i] != '\0') {
        f -> directory[strlen(f -> directory)] = line[i];
        i++;
    }

    //On ignore pour le moment les options.
    while(line[i] != '{' && line[i] != '\0') {i++;}
    if (line[i] == '\0' || line[i+1] != ' ') {
        write(STDERR_FILENO, SYNTAX_ERROR, strlen(SYNTAX_ERROR));
        goto error;
    }
    
    //On supprime tous les espaces avant la ligne de commande.
    i++;
    while(isspace(line[i]) != 0) {i++;}
    if (line[i] == '\0') {
        write(STDERR_FILENO, SYNTAX_ERROR, strlen(SYNTAX_ERROR));
        goto error;
    }
    memmove(line, line + i, strlen(line) - i + 1);
    memmove(f -> command, line, strlen(line) + 1);
    return f;

    error:
    destroy_for_info(f);
    return NULL;
}

//Plus nécessaire après le jalon 1.
// void handle_for_cmd(for_info * f, char * line) {
//     size_t i = 0;
//     while(isspace(line[i]) != 0) {i++;}
//     while(isspace(line[i]) == 0 && line[i] != '\0') {
//         f -> command -> name[strlen(f -> command -> name)] = line[i];
//         i++;
//     }
//     if (line[i] != '\0') {
//         memmove(line, line + i + 1, strlen(line) - i + 1);
//         cmd_format(line, f -> command);
//     }
// }

int replace_var(char * line, char * path) {
    char * replaced = calloc(CMD_SIZE, sizeof(char));

    if (replaced == NULL) {
        goto error;
    }

    size_t i = 0;
    while (line[i] != '\0') {
        if (line[i] != '$') {
           replaced[strlen(replaced)] = line[i];
           i++;
        } else if (line[i+1] == 'F') {
            strcat(replaced, path);
            i = i + 2;
        }
    }
    memmove(line, replaced, strlen(replaced) + 1);
    free(replaced);
    return 0;

    error:
    puts("error");
    free(replaced);
    return 1;
}