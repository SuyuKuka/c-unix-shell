#include "for.h"

for_info * for_info_init() {
    for_info * f = malloc(sizeof(for_info));
    if (f == NULL) { goto error; }

    f -> directory = calloc(DIR_SIZE , sizeof(char));
    if (f -> directory == NULL) { goto error; } 

    f -> command = malloc(CMD_SIZE);
    if (f -> command == NULL) { goto error; }

    f -> options = malloc((MAX_OPTIONS + 1) * sizeof(char *));
    if (f -> options == NULL) { goto error; }

    f -> options[MAX_OPTIONS] = NULL;

    f -> opt_size = 0;
    f -> opt_a = 0;
    f -> opt_r = 0;
    f -> opt_ext = 0;
    f -> opt_type = 0;
    f -> opt_p = 0;
    f -> ext = NULL;
    f -> file_type = '\0';
    f -> p_max = 0;

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

// Initialise les champs du for_info en fonctions des options.
int init_options(for_info * f) {
    size_t i = 0;
    while (i < f -> opt_size) {
        //Option -A
        if (strcmp(f -> options[i], "-A") == 0) {
            f -> opt_a = 1;
        //Option -r
        } else if (strcmp(f -> options[i], "-r") == 0) {
            f -> opt_r = 1; 
        //Option -e
        } else if (strcmp(f -> options[i], "-e") == 0) {
            if (i + 1 >= f -> opt_size) {
                write(STDERR_FILENO, INVALID_OPT, strlen(INVALID_OPT));
                return EXIT_FAILURE;
            }
            f -> opt_ext = 1;
            if (f -> options[i + 1][0] == '.') {
                memmove(f -> options[i + 1], f -> options[i +  1] + 1, strlen(f -> options[i + 1]) + 1);
            }
            f -> ext = f -> options[i + 1];
            i++;
        //Option -t
        } else if (strcmp(f -> options[i], "-t") == 0) {
            if (i + 1 >= f -> opt_size) {
                write(STDERR_FILENO, INVALID_OPT, strlen(INVALID_OPT));
                return EXIT_FAILURE;
            }
            char * type = f -> options[i+1];
            if (strcmp(type, "f") != 0 && strcmp(type, "d") != 0 && strcmp(type, "l") != 0 && strcmp(type, "p") != 0) {
                write(STDERR_FILENO, INVALID_TYPE, strlen(INVALID_TYPE));
                return EXIT_FAILURE;
            }
            type = NULL; 
            f -> opt_type = 1;
            f -> file_type = f -> options[i+1][0];
            i++;
        //Option -p
        } else if (strcmp(f -> options[i], "-p") == 0) {
            if (i + 1 >= f -> opt_size) {
                write(STDERR_FILENO, INVALID_OPT, strlen(INVALID_OPT));
                return EXIT_FAILURE;
            }
            size_t max_length = strlen(f -> options[i + 1]);
            for (size_t j = 0; j < max_length; j++) {
                if (!isdigit(f -> options[i+1][j])) {
                    write(STDERR_FILENO, INVALID_MAX, strlen(INVALID_MAX));
                    return EXIT_FAILURE;
                }   
            }
            f -> opt_p = 1;
            f -> p_max = atoi(f -> options[i+1]);
            i++;
        } else {
            write(STDERR_FILENO, INVALID_OPT,strlen(INVALID_OPT));
            return EXIT_FAILURE;
        }
        i++;
    }

    return EXIT_SUCCESS;
}

for_info * handle_for_line(char * line) {
    size_t i = 0;
    size_t end = strlen(line) - 1;
    for_info * f = NULL;
    char * opt_line = NULL;
    f = for_info_init();

    if (f == NULL) { goto error; } 
    while(isspace(line[i]) != 0) { i++; }
    if (line[i] == '\0') {
        write(STDERR_FILENO, SYNTAX_ERROR, strlen(SYNTAX_ERROR));
        goto error;
    }
    while (end > 0 && isspace(line[end]) != 0) { end--; }
    if (line[end] != '}' || line[end - 1] != ' ') {
        write(STDERR_FILENO, SYNTAX_ERROR, strlen(SYNTAX_ERROR));
        goto error;
    }
    line[end] = '\0';
    line[end - 1] = '\0';

    //Vérification de la syntaxe et si le for concerne les fichiers ou les répertoires.
    if (line[i+1] != ' ') {
        write(STDERR_FILENO, SYNTAX_ERROR, strlen(SYNTAX_ERROR));
        goto error;
    } else {
        f -> for_var = line[i];
    }
    i++;
    while(isspace(line[i]) != 0) { i++; }

    //On vérifie la syntaxe.
    if (line[i] != 'i' || line[i+1] != 'n') {
        write(STDERR_FILENO, SYNTAX_ERROR, strlen(SYNTAX_ERROR));
        goto error;
    } 
    i += 3;
    while(isspace(line[i]) != 0) { i++; }

    //Sauvegarde du répertoire.
    while(isspace(line[i]) == 0 && line[i] != '\0') {
        f -> directory[strlen(f -> directory)] = line[i];
        i++;
    }
    while(isspace(line[i]) != 0) { i++; }

    //Récupération des options.
    if (line[i] != '{' && line[i] != '\0') {
        f -> options[f -> opt_size] = calloc(OPTIONS_SIZE, sizeof(char)); 
        if (f -> options[f -> opt_size] == NULL) { goto error; }
        opt_line = f -> options[f -> opt_size];
    }
    while(line[i] != '{' && line[i] != '\0') {
        //Elimination des espaces, ajout du caractère '\0' à la fin de l'option actuelle et incrémentation de la taille.
        if (isspace(line[i]) != 0) {
            while(isspace(line[i]) != 0) { i++; };
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
    while(isspace(line[i]) != 0) { i++; }
    if (line[i] == '\0') {
        write(STDERR_FILENO, SYNTAX_ERROR, strlen(SYNTAX_ERROR));
        goto error;
    }

    //On réduit la ligne line à seulement la commande entre { };
    memmove(line, line + i, strlen(line) - i + 1);
    memmove(f -> command, line, strlen(line) + 1);
    if (init_options(f) != 0) {
        goto error;
    }
    return f;

    error:
    destroy_for_info(f);
    return NULL;
}

// Remplace les $F (fonctionne avec une variable étant une autre lettre que F) de line par path.
char * replace_var(char * line, char * path, char var) {
    char * replaced = calloc(CMD_SIZE, sizeof(char));

    if (replaced == NULL) {
        free(replaced);
        return NULL;
    }

    size_t i = 0;
    while (line[i] != '\0') {
        if (line[i] != '$') {
           replaced[strlen(replaced)] = line[i];
           i++;
        } else if (line[i + 1] != '\0' && line[i + 1] == var 
        && ((isupper(var) > 0 && isupper(line[i + 2]) <= 0) || (islower(var) > 0 && islower(line[i + 2]) <= 0))) {
            strcat(replaced, path);
            i = i + 2;
        } else {
            replaced[strlen(replaced)] = line[i];
            i++;
        }
    }
    return replaced;
}

// Fonction qui crée une copie de for_info avec la même valeur pour ses champs.
for_info * copy_for_info (for_info * f) {
    for_info * copy = NULL;
    copy = for_info_init();
    if (copy == NULL) { goto error; }

    memmove(copy -> directory, f -> directory, strlen(f -> directory) + 1); 
    memmove(copy -> command, f -> command, strlen(f -> command) + 1);

    copy -> opt_size = f -> opt_size;
    if (copy -> options == NULL) { goto error; }
    for (size_t i = 0; i < f -> opt_size; i++) {
        copy -> options[i] = calloc(strlen(f -> options[i]) + 1, sizeof(char));
        if (copy -> options[i] == NULL) { goto error; }
        memmove(copy -> options[i], f -> options[i], strlen(f -> options[i]) + 1);
    }

    if (init_options(copy) != 0) {
        goto error;
    }

    copy -> for_var = f -> for_var;
    return copy;

    error:
    destroy_for_info(copy);
    return NULL;
}

// Vérifie si l'extension correspond à celle attendue.
int check_ext (char * file, const char * ext) {
    size_t length = 0;
    length = strlen(file);

    size_t i = length;
    int result = 0;
    char * acc = NULL;

    while (i != 0 && file[i] != '.') {
        i--;
    }
    
    if (i == 0 || i == length - 1) {
        goto cleanup;
    }
    i++;

    acc = calloc(DIR_SIZE , sizeof(char));
    if (acc == NULL) {
        goto cleanup;
    } 

    while (i < length) {
        acc[strlen(acc)] = file[i];
        i++;
    }
    strcat(acc, "\0");

    if (strcmp(acc, ext) != 0) {
        goto cleanup;
    }
    
    free(acc);
    return EXIT_FAILURE;

    cleanup:
    free(acc);
    return result;
}

// Efface l'extension du nom d'un fichier.
void remove_ext(char * file) {
    size_t length = strlen(file);
    size_t i = 0;
    while (i < length && file[i] != '.') {
        i++;
    }
    while (i < length + 1) {
        file[i] = '\0';
        i++;
    }
}