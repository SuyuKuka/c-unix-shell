#include "cd.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char last_dir[PATH_MAX] = ""; // Trouver moyen qu'il enregistre correctement une fois appelé dans fsh
// strcpy(last_dir, cwd); -> enregistre le répertoire courant pour un futur '-'


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

/* Gestion erreur
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("Erreur lors de la récupération du répertoire courant");
            return 1;
        }

    // Check le nb d'arguments dans argv (0 ou 1, sinon trop)
    if (argv[2] != NULL) {
        fprintf(stderr, "Erreur : trop d'arguments pour la commande cd\n");
        return 1;
    }
*/


// Cas à gérer: rien (pas d'argument ou .), parent (..), - (dernier répertoire en date) et cible (réf cible)
int cd(char ** argv) {
    char cwd[MAX_PATH_LENGTH];

    if (argv[1] == NULL || strcmp(argv[1], ".") == 0) { // Sans argument ou '.'
     if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("Erreur lors de la récupération du répertoire courant");
            return 1;
        }
        printf("Cas dir courant : %s\n", cwd);
        return 0;
    }
    if (strcmp(argv[1], "..") == 0) {       // Cas ".."
        strcpy(last_dir, cwd);
        printf("Cas '..'\n");
        return change_dir("..");
    }
    if (strcmp(argv[1], "-") == 0) {    // Cas "-", erreur à chaque fois tant que last_dir -> not def
        if (strlen(last_dir) == 0) {
            fprintf(stderr, "Erreur: last_dir pas encore def.\n");
            return 1;
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
    return change_dir(argv[1]);
}