#include "cmdutils.h"

char *trim(char *str) {
    if (!str) return NULL;
    const char *start = str;
    while (*start == ' ') start++;
    const char *end = start + strlen(start);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    size_t len = end - start;
    char *trimmed = malloc(len + 1);    // Alloue une nouvelle chaîne "trimmed"
    if (!trimmed) {
        perror("malloc");
        return NULL;
    }
    memcpy(trimmed, start, len);
    trimmed[len] = '\0';
    return trimmed;  // Return pointeur vers trimmed
}

char *strip_brackets(char *condition) {
    if (condition == NULL) return NULL;

    size_t len = strlen(condition);
    size_t start = 0, end = len - 1;

    // Retire les crochets & espaces autour
    while (start < len && (condition[start] == '[' || isspace((unsigned char)condition[start]))) start++;
    while (end > start && (condition[end] == ']' || isspace((unsigned char)condition[end]))) end--;

    size_t new_len = end - start + 1;
    char *clean_condition = malloc(new_len + 1);
    if (clean_condition == NULL) {
        perror("malloc");
        return NULL;
    }

    memcpy(clean_condition, condition + start, new_len);
    clean_condition[new_len] = '\0';
    free(condition);
    return clean_condition;
}

char *custom_strndup(const char *src, size_t n) {
    if (src == NULL) return NULL;

    size_t len = strnlen(src, n); // Trouve la longueur de la chaîne jusqu'à n caractères, droit à strnlen ?
    char *dest = malloc(len + 1); // Alloue mémoire aussi pour '\0'
    if (dest == NULL) {
        goto error;
    }
    memcpy(dest, src, len);       // Copie les 'len' caractères
    dest[len] = '\0';             // Termine la chaîne par '\0'

    return dest;

    error:
    free(dest);
    return NULL;
}

/*
* Extrait un bloc de texte entre accolades imbriquées.
*/
char *extract_block(const char *line, int *start) {
    int i = *start;
    int len = 0;
    int brace_count = 0;
    char *block = NULL;

    while (line[i] != '\0' && line[i] != '{') { // Parcours jusqu'à 1ère accolade ouvrante '{'
        i++;
    }
    if (line[i] != '{') {
        write(STDERR_FILENO, SYNTAX_ERR, strlen(SYNTAX_ERR));
        goto error;
    }
    // Début du bloc après l'accolade ouvrante
    int block_start = i + 1; 
    brace_count++;
    // Parcours du texte pour trouver la fin du bloc
    for (i = block_start; line[i] != '\0'; i++) {
        if (line[i] == '{') {
            brace_count++; // Nouvelle accolade ouvrante
        } else if (line[i] == '}') {
            brace_count--; // Accolade fermante
            if (brace_count == 0) {
                break; // Fin du bloc trouvée
            }
        }
    }

    // Check si on a trouvé une accolade fermante
    if (brace_count != 0) {
        write(STDERR_FILENO, SYNTAX_ERR, strlen(SYNTAX_ERR));
        goto error;
    }
    // Calcule la longueur du bloc
    len = i - block_start;
    block = malloc(len + 1);    // Alloue la mémoire pour le bloc extrait
    if (block == NULL) {
        perror("malloc");
        goto error;
    }
    // Copie manuelle du contenu
    for (int j = 0; j < len; j++) {
        if (block_start < 0 || i < block_start || i > strlen(line)) {
            write(STDERR_FILENO, SYNTAX_ERR, strlen(SYNTAX_ERR));
            goto error;
        }
        block[j] = line[block_start + j];
    }
    block[len] = '\0';
    *start = i + 1; // Maj de la position de départ pour le prochain traitement + ignore accolade fermante
    return block;

    error:
    if (block != NULL) free(block);
    return NULL;
}