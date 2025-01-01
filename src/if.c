#include "if.h"
/*
* Intialisation d'une struct if_info
*/
if_info * if_info_init() {
    if_info *info = malloc(sizeof(if_info));
    if (info == NULL) {
        perror("malloc");
        return NULL;
    }
    info -> test = NULL;
    info -> cmd_true = calloc(CMD_SIZE, sizeof(char));  // Commande effectuée si test = true
    if (info -> cmd_true == NULL) {
        destroy_if_info(info);
        return NULL;
    }
    info -> cmd_false = calloc(CMD_SIZE, sizeof(char)); // Si 'else', commande effectuée si test = false
    if (info -> cmd_false == NULL) {
        destroy_if_info(info);
        return NULL;
    }
    return info;
}


void destroy_if_info(if_info *info) {
    if (info != NULL) {
        if (info -> test != NULL) {
            destroy_cmd_info(info -> test);
            info -> test = NULL;
        }
        if (info -> cmd_true != NULL) {
            free(info -> cmd_true);
            info -> cmd_true = NULL;
        }
        if (info -> cmd_false != NULL) {
            free(info -> cmd_false);
            info -> cmd_false = NULL;
        }
        free(info);
    }
}

char *strip_brackets(char *condition) {
    if (!condition) return NULL;

    size_t len = strlen(condition);
    size_t start = 0, end = len - 1;

    // Retire les crochets & espaces autour
    while (start < len && (condition[start] == '[' || isspace((unsigned char)condition[start]))) start++;
    while (end > start && (condition[end] == ']' || isspace((unsigned char)condition[end]))) end--;

    size_t new_len = end - start + 1;
    char *clean_condition = malloc(new_len + 1);
    if (!clean_condition) {
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
    dest[len] = '\0';             // Termine la chaîne par un '\0'

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
        return NULL;
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
        return NULL;
    }
    // Calcule la longueur du bloc
    len = i - block_start;
    block = malloc(len + 1);    // Alloue la mémoire pour le bloc extrait
    if (block == NULL) {
        perror("malloc");
        free(block);
        return NULL;
    }
    // Copier manuellement le contenu dans le tableau alloué
    for (int j = 0; j < len; j++) {
        block[j] = line[block_start + j];
    }
    block[len] = '\0'; // Terminer la chaîne par '\0'
    // Maj de la position de départ pour le prochain traitement
    *start = i + 1; // Ignorer l'accolade fermante
    return block;
}


/*
* Analyse une ligne de commande, vérifie si il s'agit d'un if. 
* Return 0 si succès, 1 en cas d'erreur de syntaxe.
*/
if_info * handle_if_line(char *line) {
    if (line == NULL) {
        write(STDERR_FILENO, SYNTAX_ERR, strlen(SYNTAX_ERR));
        return NULL;
    }
    if_info * info = if_info_init(); // Initialise la struct
    if (info == NULL) {
        perror("if_info_init");
        return NULL;
    }
    char *cursor = line;

    // 1. Vérification de la syntaxe : mot-clé "if", useless vu fonctionnement fsh ?
    while (*cursor != '\0' && isspace((unsigned char)*cursor)) cursor++; // Skip espaces
    if (strncmp(cursor, "if", 2) != 0 || !isspace(cursor[2])) {
        write(STDERR_FILENO, SYNTAX_ERR, strlen(SYNTAX_ERR));
        goto error;
    }
    cursor += 2; // Skip "if"

    // 2. Extraction de la condition (jusqu'à '{')
    while (isspace((unsigned char)*cursor)) cursor++; // Skip espaces
    char * condition_start = cursor;
    while (*cursor != '\0' && *cursor != '{') cursor++;
    if (*cursor != '{') {
        write(STDERR_FILENO, SYNTAX_ERR, strlen(SYNTAX_ERR));
        goto error;
    }
    size_t condition_len = cursor - condition_start;
    char * condition = custom_strndup(condition_start, condition_len);
    if (condition == NULL) {
        perror("strndup");
        goto error;
    }

    condition = strip_brackets(condition);
    cmd_info * new_cmd = malloc(sizeof(cmd_info));
    if (new_cmd == NULL) {
        perror("malloc");
        goto error;
    }
    char * new_name = malloc(strlen(condition) + 1);  // Alloue mémoire pour copie
    if (new_name == NULL) {
        perror("malloc");
        goto error;
    }
    memcpy(new_name, condition, strlen(condition) + 1);  // Copie de la condition
    new_cmd -> name = new_name;  // chaîne stockée dans 'name'
    new_cmd -> opt = NULL;
    new_cmd -> args = NULL;
    info -> test = new_cmd; // Transforme la condition en commande
    free(condition);

    // 3. Extraction de cmd_true (jusqu'à '}')
    while (isspace((unsigned char) * cursor)) cursor++;
    int start = cursor - line; // Position actuelle
    char * tmp = extract_block(line, &start); // Utilise extract_block avec pointeur sur {
    if (tmp == NULL) {
        goto error;
    }
    info->cmd_true = malloc(strlen(tmp) + 1);
    if (info->cmd_true == NULL) {
        free(tmp);
        goto error;
    }
    memcpy(info->cmd_true, tmp, strlen(tmp) + 1);
    free(tmp);
    cursor = line + start;
    /*
    memmove(info -> cmd_true, tmp, strlen(tmp) + 1);
    free(tmp);
    if (info -> cmd_true == NULL) {
        write(STDERR_FILENO, SYNTAX_ERR, strlen(SYNTAX_ERR));
        goto error;
    }
    cursor = line + start; // Maj curseur après le bloc cmd_true */

    // 4. Vérification & extraction de cmd_false si "else" présent
    while (isspace((unsigned char) * cursor)) cursor++;
    if (strncmp(cursor, "else", 4) == 0) {
        cursor += 4; // Skip "else"
        while (isspace((unsigned char)*cursor)) cursor++;
        if (*cursor != '{') {
            write(STDERR_FILENO, SYNTAX_ERR, strlen(SYNTAX_ERR));
            goto error;
        }

        start = cursor - line;
        char * tmp = extract_block(line, &start); // Utilise extract_block avec pointeur sur {
        if (tmp == NULL) {
            goto error;
        }

        info->cmd_false = malloc(strlen(tmp) + 1);
        if (info->cmd_false == NULL) {
            free(tmp);
            goto error;
        }
        memcpy(info->cmd_false, tmp, strlen(tmp) + 1);
        free(tmp);
        cursor = line + start;

        /*
        memmove(info -> cmd_false, tmp, strlen(tmp) + 1);
        free(tmp);
        if (info -> cmd_false == NULL) {
            write(STDERR_FILENO, SYNTAX_ERR, strlen(SYNTAX_ERR));
            goto error;
        }
        cursor = line + start; // Maj curseur après le bloc cmd_false */
    }
    return info;

    error:
    if (condition != NULL) free(condition);
    if (new_cmd != NULL) {
        if (new_cmd -> name != NULL) free(new_cmd -> name);
        free(new_cmd);
    }
    if (info != NULL) {
        destroy_if_info(info);  // Assurez-vous que destroy_if_info est robuste
        info = NULL;  // Important pour éviter des problèmes en aval
    }
    return NULL;
}