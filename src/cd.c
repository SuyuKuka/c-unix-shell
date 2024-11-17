#include "cd.h"


int cd(char ** argv) {
    static char last_dir[PATH_MAX];
    char cwd[MAX_PATH_LENGTH];
    const char * path = argv[1];
    
    if (path == NULL) {
        if (chdir(getenv("SRC")) == 0) {
            strncpy(last_dir, cwd, PATH_MAX);
            printf("Cas sans argument reconnu.");
            return EXIT_SUCCESS;
        }
        printf("Erreur: path src ?  ");
        return EXIT_FAILURE;  
    } else if (strcmp(path, "-") == 0) {
        if (strlen(last_dir) != 0 && chdir(last_dir) == 0) {  // Si il y a déjà last_dir enregistré.
            strncpy(last_dir, cwd, PATH_MAX); // Le courant devient le précèdent pour le prochain appel.
            printf("Cas '-' reconnu.");
            return EXIT_SUCCESS;
        }
        printf("Erreur: path - ?  ");
        return EXIT_FAILURE;
    }

    if (chdir(path) == 0) {               // Change le répertoire courant.
        strncpy(last_dir, cwd, PATH_MAX);  // Le courant devient le précèdent pour le prochain appel.
        printf("Cas path reconnu. ");
        return EXIT_SUCCESS;
    }
    printf("Erreur: problème path ? ");
    return EXIT_FAILURE;
}


/*
int change_dir(const char *path) {
    if (chdir(path) != 0) {
        perror("(cd) Impossible de trouver le chemin d'accès ");
        printf("1"); // Malgré conditions, cd echoué
        return 1;
    }
    chdir(path);
    printf("0"); // Dans conditions, cd effectué
    return 0;
}
int cd(char ** argv) {
    static char last_dir[PATH_MAX];
    char cwd[MAX_PATH_LENGTH];
    const char * path = argv[1];

    if (path == NULL) { // Sans argument ou '.'
     if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("Erreur lors de la récupération du répertoire courant");
            return 1;
        }
        printf("Cas dir courant : %s\n", cwd);
        return 0;
    }
    if (strcmp(path, "-") == 0) {    // Cas "-", erreur à chaque fois tant que last_dir -> not def
        if (strlen(last_dir) != 0 && chdir(last_dir) == 0) {
            // Le répertoire courant devient alors le répertoire précèdent pour le prochain appel.
            strncpy(last_dir, cwd, PATH_MAX);
            return EXIT_SUCCESS;
        }
        if (getcwd(cwd, sizeof(cwd)) != NULL) { // useless ?
            strcpy(last_dir, cwd);
        }
        printf("Cas '-'\n");
        return change_dir(last_dir);
    }
    // Cas de ref
    strcpy(last_dir, cwd);
    printf("Cas ref\n");
    return change_dir(path); // chdir('SRC' + '\' + argv[1]) ?
}
*/