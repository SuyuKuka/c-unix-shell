#include "if.h"

if_info *if_info_init() {   // Intialisation d'une struct if_info
    if_info *info = malloc(sizeof(if_info));
    if (!info) {
        perror("malloc");
        return NULL;
    }
    info->test = NULL;  // Test du if
    info->cmd_true = calloc(CMD_SIZE, sizeof(char));  // Commande effectuée si test = true
    if(info->cmd_true == NULL) {
        destroy_if_info(info);
        return NULL;
    }
    info->cmd_false = calloc(CMD_SIZE, sizeof(char)); // Si 'else', commande effectuée si test = false
    if(info->cmd_false == NULL) {
        destroy_if_info(info);
        return NULL;
    }
    return info;
}


void destroy_if_info(if_info *info) {
    if (info) {
        if (info->test != NULL) {
            destroy_cmd_info(info->test);
            info->test = NULL;
        }
        if (info->cmd_true != NULL) {
            free(info->cmd_true);
            info->cmd_true = NULL;
        }
        if (info->cmd_false != NULL) {
            free(info->cmd_false);
            info->cmd_false = NULL;
        }
        free(info);
    }
}

char *strip_brackets(char *condition) {
    if (!condition) return NULL;

    size_t len = strlen(condition);
    size_t start = 0, end = len - 1;

    // Retirer les crochets et espaces autour
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
    return clean_condition;
}

char *custom_strndup(const char *src, size_t n) {
    if (!src) return NULL; // Vérifie que src n'est pas NULL

    size_t len = strnlen(src, n); // Trouve la longueur de la chaîne jusqu'à n caractères
    char *dest = malloc(len + 1); // Alloue suffisamment de mémoire (avec espace pour '\0')
    if (!dest) return NULL;       // Vérifie l'échec de malloc

    memcpy(dest, src, len);       // Copie les 'len' caractères
    dest[len] = '\0';             // Termine la chaîne par un '\0'

    return dest;
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
        fprintf(stderr, "Erreur : Accolade ouvrante non trouvée.\n");
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
        fprintf(stderr, "Erreur : Accolades non équilibrées.\n");
        return NULL;
    }
    // Calcule la longueur du bloc
    len = i - block_start;
    block = malloc(len + 1);    // Alloue la mémoire pour le bloc extrait
    if (!block) {
        perror("malloc");
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
if_info *handle_if_line(char *line) {
    if (!line) {
        fprintf(stderr, SYNTAX_ERR);
        return NULL;
    }
    if_info *info = if_info_init(); // Initialise la struct
    if (!info) {
        perror("if_info_init");
        return NULL;
    }
    char *cursor = line;

    // 1. Vérification de la syntaxe : mot-clé "if", useless vu fonctionnement fsh ?
    while (*cursor != '\0' && isspace((unsigned char)*cursor)) cursor++; // Skip espaces
    if (strncmp(cursor, "if", 2) != 0 || !isspace(cursor[2])) {
        fprintf(stderr, SYNTAX_ERR);
        destroy_if_info(info);
        return NULL;
    }
    cursor += 2; // Skip "if"

    // 2. Extraction de la condition (jusqu'à '{')
    while (isspace((unsigned char)*cursor)) cursor++; // Skip espaces
    char *condition_start = cursor;
    while (*cursor != '\0' && *cursor != '{') cursor++;
    if (*cursor != '{') {
        fprintf(stderr, "Erreur : '{' manquant après la condition.\n");
        destroy_if_info(info);
        return NULL;
    }
    size_t condition_len = cursor - condition_start;
    char *condition = custom_strndup(condition_start, condition_len);
    if (!condition) {
        perror("strndup");
        destroy_if_info(info);
        return NULL;
    }

    condition = strip_brackets(condition);
    cmd_info *new_cmd = malloc(sizeof(cmd_info));
    if (new_cmd == NULL) {
        perror("malloc");
        free(condition);
        return NULL;
    }
    char *new_name = malloc(strlen(condition) + 1);  // Alloue mémoire pour copie
    if (new_name == NULL) {
        perror("malloc");
        free(new_cmd);
        free(condition);
        return NULL;
    }
    memcpy(new_name, condition, strlen(condition) + 1);  // Copie de la condition
    new_cmd->name = new_name;  // chaîne stockée dans 'name'
    new_cmd->opt = NULL;
    new_cmd->args = NULL;
    info->test = new_cmd; // Transforme la condition en commande
    free(condition);

    // 3. Extraction de cmd_true (jusqu'à '}')
    while (isspace((unsigned char)*cursor)) cursor++;
    int start = cursor - line; // Position actuelle
    info->cmd_true = extract_block(line, &start); // Utilise extract_block avec pointeur sur {
    if (!info->cmd_true) {
        fprintf(stderr, "Erreur lors de l'extraction de cmd_true.\n");
        destroy_if_info(info);
        return NULL;
    }
    cursor = line + start; // Maj curseur après le bloc cmd_true

    // 4. Vérification & extraction de cmd_false si "else" présent
    while (isspace((unsigned char)*cursor)) cursor++;
    if (strncmp(cursor, "else", 4) == 0) {
        cursor += 4; // Skip "else"
        while (isspace((unsigned char)*cursor)) cursor++;
        if (*cursor != '{') {
            fprintf(stderr, "Erreur : '{' manquant après 'else'.\n");
            destroy_if_info(info);
            return NULL;
        }

        start = cursor - line;
        info->cmd_false = extract_block(line, &start); // Utilise extract_block avec pointeur sur {
        if (!info->cmd_false) {
            fprintf(stderr, "Erreur lors de l'extraction de cmd_false.\n");
            destroy_if_info(info);
            return NULL;
        }
        cursor = line + start; // Maj curseur après le bloc cmd_false
    }

    return info;
}




/*

//
    printf("handle_if_line | condition pre strip:%s\n", condition);
    condition = strip_brackets(condition);
    printf("handle_if_line | condition post strip:%s\n", condition);
    cmd_info *new_cmd = malloc(sizeof(cmd_info));
    if (new_cmd == NULL) {
        perror("malloc");
        free(condition);
        return NULL;
    }
    char *new_name = malloc(strlen(condition) + 1);  // Alloue mémoire pour la copie brute
    if (new_name == NULL) {
        perror("malloc");
        free(new_cmd);
        free(condition);
        return NULL;
    }
    memcpy(new_name, condition, strlen(condition) + 1);  // Copie brute de la condition
    new_cmd->name = new_name;  // chaîne stockée dans 'name'
    new_cmd->opt = NULL;
    new_cmd->args = NULL;
    info->test = new_cmd; // Transforme la condition en commande
    printf("handle_if_line | info->test->name :%s\n", info->test->name);
    free(condition);


/*
* Analyse une ligne de commande, vérifie si il s'agit d'un if. 
* Return 0 si succès, 1 en cas d'erreur de syntaxe.
*
if_info *handle_if_line(char *line) {
    if_info *info = NULL;
    info = if_info_init();
    if(!line) {
        fprintf(stderr, SYNTAX_ERR);
        return NULL;
    }
    char *cursor = line;
    while (*cursor != '\0' && *cursor != ' ') cursor++; // Avance jusqu'au 1er espace puis check si mot=='if'
    if (strncmp(line, "if", cursor - line) != 0) {
        fprintf(stderr, SYNTAX_ERR);
        return NULL;
    }
    cursor++; // on skip ' '
    
    // CONDITION - PREDICAT
    char *condition_start = cursor;
    while (*cursor != '\0' && *cursor != '{') cursor++;
    if (*cursor != '{') {
        fprintf(stderr, "Erreur : '{' manquant après la condition.\n");
        return NULL;
    }
    char *condition_end = cursor;
    size_t condition_len = condition_end - condition_start;

    char *condition = malloc(condition_len + 1);    // Extraction de la condition
    if (condition == NULL) {
        perror("malloc");
        return NULL;
    }
    memcpy(condition, condition_start, condition_len);
    condition[condition_len] = '\0';
    char *clean_condition = strip_brackets(condition);  // Nettoie crochets autour de la condition
    free(condition);
    if (clean_condition == NULL) {
        fprintf(stderr, "Erreur : nettoyage des crochets échoué.\n");
        return NULL;
    }

    // Parser la condition comme une commande
    cmd_info *new_cmd = malloc(sizeof(cmd_info));
    if (new_cmd == NULL) {
        perror("malloc");
        free(clean_condition);
        return NULL;
    }
    char *new_name = malloc(strlen(clean_condition) + 1);  // Alloue mémoire pour la copie brute
    if (new_name == NULL) {
        perror("malloc");
        free(new_cmd);
        free(clean_condition);
        return NULL;
    }
    memcpy(new_name, clean_condition, strlen(clean_condition) + 1);  // Copie brute de la condition
    new_cmd->name = new_name;  // chaîne stockée dans 'name'
    new_cmd->opt = NULL;
    new_cmd->args = NULL;
    info->test = new_cmd;  // Stocke directement dans info->test
    free(clean_condition);


    cursor++;
    while (isspace((unsigned char)*cursor)) cursor++;

    // CMD TRUE
    char *cmd_true_start = cursor;
    while (*cursor != '\0' && *cursor != '}') cursor++;
    if (*cursor == '\0') {
        fprintf(stderr, SYNTAX_ERR);
        return NULL;
    }

    char *cmd_true_end = cursor;
    cursor++;

    size_t cmd_true_len = cmd_true_end - cmd_true_start;
    char *cmd_true = malloc(cmd_true_len + 1);
    if (cmd_true == NULL) {
        perror("malloc");
        return NULL;
    }
    memcpy(cmd_true, cmd_true_start, cmd_true_len);
    cmd_true[cmd_true_len] = '\0';
    // Nettoyage des crochets et des espaces
    size_t tstart = 0;
    while (cmd_true[tstart] == '{' || isspace((unsigned char)cmd_true[tstart])) tstart++;
    size_t tend = cmd_true_len - 1;
    while (tend > tstart && (cmd_true[tend] == '}' || isspace((unsigned char)cmd_true[tend]))) tend--;

    size_t new_len = tend - tstart + 1;
    char *clean_cmd = malloc(new_len + 1);
    if (clean_cmd == NULL) {
        perror("malloc");
        free(cmd_true);
        return NULL;
    }
    memcpy(clean_cmd, cmd_true + tstart, new_len);
    clean_cmd[new_len] = '\0';
    free(cmd_true);

    char *trimmed_cmd = trim(clean_cmd);
    free(clean_cmd);
    if (!trimmed_cmd) {
        return NULL;
    }
    memmove(info->cmd_true, trimmed_cmd, strlen(trimmed_cmd)+1);
    free(trimmed_cmd);

    // CMD_FALSE
    while (*cursor == ' ') cursor++;
    if (strncmp(cursor, "else", 4) == 0) {
        cursor += 4;
        while (*cursor == ' ') cursor++;
        if (*cursor != '{') {
            fprintf(stderr, "Erreur : CMD_FALSE mal formée, '{' manquant.\n");
            return NULL;
        }
        cursor++;

        char *cmd_false_start = cursor;
        while (*cursor != '\0' && *cursor != '}') cursor++;
        if (*cursor == '\0') {
            fprintf(stderr, SYNTAX_ERR);
            return NULL;
        }
        char *cmd_false_end = cursor;
        size_t cmd_false_len = cmd_false_end - cmd_false_start;
        char *cmd_false = malloc(cmd_false_len + 1);
        if (!cmd_false) {
            perror("malloc");
            return NULL;
        }
        memcpy(cmd_false, cmd_false_start, cmd_false_len);
        cmd_false[cmd_false_len] = '\0';

        size_t estart = 0;
        while (cmd_false[estart] == '{' || isspace((unsigned char)cmd_false[estart])) estart++;
        size_t eend = cmd_false_len - 1;
        while (eend > estart && (cmd_false[eend] == '}' || isspace((unsigned char)cmd_false[eend]))) eend--;

        size_t new_len_false = eend - estart + 1;
        char *clean_false = malloc(new_len_false + 1);
        if (clean_false == NULL) {
            perror("malloc");
            free(cmd_false);
            return NULL;
        }
        memcpy(clean_false, cmd_false + estart, new_len_false);
        clean_false[new_len_false] = '\0';
        free(cmd_false);

        char *trimmed_false = trim(clean_false);
        free(clean_false);
        if (trimmed_false == NULL) {
            return NULL;
        }
        memmove(info->cmd_false, trimmed_false, strlen(trimmed_false)+1);
        free(trimmed_false);
    }
    return info;    // return le if info
}
*/