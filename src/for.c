#include "for.h"

for_info * for_info_init() {
    for_info * f = malloc(sizeof(for_info));
    if (f == NULL) {goto error;}

    f -> directory = calloc(DIR_SIZE , sizeof(char));
    if (f -> directory == NULL) {goto error;} 

    f -> command = malloc(CMD_SIZE);
    if (f -> command == NULL) {goto error;}

    f -> options = malloc((MAX_OPTIONS + 1) * sizeof(char *));
    f -> options[MAX_OPTIONS + 1] = NULL;
    if (f -> options == NULL) {goto error;}

    f -> opt_size = 0;

    return f;

    error:
    destroy_for_info(f);
    return NULL;
}

void destroy_for_info(for_info * f) {
    if (f != NULL) {
        free(f -> directory);
        free(f -> command);
        for (size_t i = 0; i < f -> opt_size; i++) {
            free(f -> options[i]);
        }
        free(f -> options);
    }
    free(f);
}

for_info * handle_for_line(char * line) {
    size_t i = 0;
    size_t end = strlen(line) - 1;
    for_info * f = NULL;
    char * opt_line = NULL;
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

    //Vérification de la syntaxe et si le for concerne les fichiers ou les répertoires.
    if ((line[i] != 'F' && line[i] != 'D') && line[i+1] != ' ') {
        write(STDERR_FILENO, SYNTAX_ERROR, strlen(SYNTAX_ERROR));
        goto error;
    } else if (line[i] == 'F') {
        f -> for_type = FOR_FILE;
    } else {
        f -> for_type = FOR_DIRECTORY;
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
    while(isspace(line[i]) != 0) {i++;}

    //Récupération des options.
    f -> options[f -> opt_size] = calloc(OPTIONS_SIZE, sizeof(char)); 
    if (f -> options[f -> opt_size] == NULL) {goto error;}
    opt_line = f -> options[f -> opt_size];
    while(line[i] != '{' && line[i] != '\0') {
        //Elimination des espaces, ajout du caractère '\0' à la fin de l'option actuelle et incrémentation de la taille.
        if (isspace(line[i]) != 0) {
            while(isspace(line[i]) != 0) {i++;};
            opt_line[strlen(opt_line)] = '\0';
            f -> opt_size++;

            //Si on dépasse le nombre d'options maximales, le for n'est pas valide.
            if (f -> opt_size > MAX_OPTIONS) {
                write(STDERR_FILENO, TOO_MANY_OPT, strlen(TOO_MANY_OPT));
                goto error;
            }

            //Vérifier qu'on a pas atteint un delimiteur avant d'allouer la mémoire.
            if (line[i] != '{' && line[i] != '\0') {
                f -> options[f -> opt_size] = calloc(OPTIONS_SIZE, sizeof(char));
                if (f -> options[f -> opt_size] == NULL) { goto error; }
                opt_line = f -> options[f -> opt_size];
            }
        } else {
            //Ajout des caractères de l'option actuelle.
            opt_line[strlen(opt_line)] = line[i];
            i++;
        }
    }
    opt_line = NULL;

    //Si il manque un le caractère '{' suivi d'un espace (ou si on est à la fin) le for est invalide.
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

    //On réduit la ligne line à seulement la commande entre { };
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

//Remplace les $F de line par path.
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

for_info * copy_for_info (for_info * f) {
    for_info * copy = for_info_init();
    if (copy == NULL) { goto error; }
    memmove(copy -> directory, f -> directory, strlen(f -> directory) + 1); 
    memmove(copy -> command, f -> command, strlen(f -> command) + 1);

    for (size_t i = 0; i < f -> opt_size; i++) {
        copy -> opt_size = i;
        copy -> options[i] = malloc(strlen(f -> options[i]) + 1);
        if (copy == NULL) {
            goto error;
        }
        memmove(copy -> options[i], f -> options[i], strlen(f -> options[i]) + 1);
    }

    copy -> for_type = f -> for_type;

    return copy;

    error:
    destroy_for_info(copy);
    return NULL;
}