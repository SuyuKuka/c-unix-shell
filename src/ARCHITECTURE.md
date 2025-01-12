- ARCHITECTURE DU PROJET -



Déroulement de l'écriture d'une ligne de commande dans le Shell :

Le prompt du shell est initialisé depuis le fichier 'fsh.c'. Lorsqu'une ligne est écrite dans le prompt puis validée, elle est récupérée par la fonction
'int handle_line(char * line)' qui va décortiquer son contenu puis appeler les commandes ou fonctions adéquates.



La structure du Shell est établie de la manière suivante (selon l'ordre des fichiers) :

- cd.c:
Fichier contenant la commande cd.

'int cd(cmd_info * cmd)' recupère les informations d'une commande 'cmd' puis exécute cd.



- cmd.c:
Une commande est définie par la structure suivante:
typedef struct cmd_info {
    char * name;
    char * opt;
    char * args;
} cmd_info;
Où on a name pour le nom de la commande, opt pour ses optins et args pour les arguments pris en compte.

'cmd_info * cmd_info_init()' et 'void destroy_cmd_info(cmd_info * cmd)' 
qui vont respectivement initialiser une structure de commande, et la détruire puis libérer sa mémoire.
'cmd_format(char * line, cmd_info * cmd)' permet de définir le format d'une commande 'cmd' par rapport à 'line' donnée en argument.



- cmdutils.c:
Regroupe un ensemble de fonctions utilitaires pour le traitement d'une ligne en commande. Notamment utilisé dans le fichier if.c.

'char *trim(char *str)' réduit la chaîne de caractères 'str' pour ne conserver que les caractères en ignorant les espaces superflus autour.
'char *strip_brackets(char *condition)' élimine les crochets entourant 'condition'.
'char *custom_strndup(const char *src, size_t n)' simule le comportement d'une fonction strndup.
'char *extract_block(const char *line, int *start)' extrait un bloc de texte entre des accolades imbriquées.



- externalcmd.c:
Fichier s'occupant de la gestion des commandes externes.

'void destroy_split(char ** arr, int * size)' détruit et libère le résultat d'un split.
'char ** split(cmd_info * cmd, int * size)' sépare 'cmd' ainsi que ses options et arguments selon une taille 'size',
et stocke les informations récupérées dans un char **.
'int externalcmd(cmd_info *cmd)' récupère une commande externe puis l'exécute.



- for.c:
Fichier contenant la commande for. Un for est défini selon la structure suivante:
typedef struct for_info {
    char * directory;
    char * command;
    char ** options;
    int opt_size;
    int opt_a;
    int opt_r;

    int opt_ext;
    char * ext;

    int opt_type;
    char file_type;

    int opt_p;
    int p_max;
    char for_var;
} for_info;

'for_info * for_info_init()' et 'void destroy_for_info(for_info * f)' qui vont respectivement
initialiser la structure d'une commande for, et la détruire puis libérer sa mémoire.
'int init_options(for_info * f)' initialise les champs de for_info en fonctions des options.
'for_info * handle_for_line(char * line)' analyse 'line' donnée pour construire les informations d'un for_info.
'char * replace_var(char * line, char * path, char var)' remplace les $F de 'line' par des paths.
'for_info * copy_for_info (for_info * f)' crée une copie de for_info avec la même valeur pour ses champs.
'int check_ext (char * file, const char * ext)' et 'void remove_ext(char * file)'
qui vont respectivement effectuer une vérification de l'extension de 'file', et effacer l'extension de 'file'.



- fsh.c:
Fichier principal regroupant l'ensembles des fonctions d'exécution, de gestion du prompt et de la ligne inscrite.
Une commande y est définie par la structure suivante:
typedef struct {
    char * cmd_line; 
    bool is_pipeline; 
    int input_fd; 
    int output_fd;
} command;

'int call_function (cmd_info * cmd)' gère l'exécution des commandes et les appels aux commandes de fsh si nécessaire.
Sinon, crée un processus et gère les commandes externes.

Les handlers (autres):
'int handle_for_dep(for_info * f, struct stat st, char * path, char * dir_name, int var)' vérifie et gère les dépendances d'un for.
'int handle_for(for_info * f)' s'occupe des for passés au prompt.
'int handle_redirection(char * line, size_t i)' s'occupe des redirections passées au prompt.

Les fonctions d'exécution:
'int exec_for_cmd(for_info * f, char * path, int var)'
'int execute_if(if_info *info)'
'int execute_pipeline(cmd_info * cmd_info, int in_fd, int * out_fd, command * cmd)'
-> exécutent respectivement le for, le if et les pipelines.

La fonction 'int handle_line(char * line)' s'occupe de 'line' donnée en argument.
'command ** handle_cmd(char * line)' traite la ligne de commande et la sépare en plusieurs commandes si on a une commande structurée ou une pipeline.

Les fonctions:
'void color_prompt(char * prompt, int var)'
'void make_prompt(char * prompt, int var)'
'int display_prompt(int var)'
-> s'occupent de la création, gestion et affichage du prompt du Shell.

Ici, 'int main()' est le main du programme. 'fsh.c' est le fichier principal.



- ftype.c:
Fichier contenant la commande ftype.

'int ftype(cmd_info * cmd)' récupère les informations d'une commande 'cmd' et exécute ftype.



- if.c:
Fichier contenant la commande if. Un if est défini selon la structure suivante:
typedef struct if_info {
    char *test;         // Commande/Condition TEST
    char *cmd_true;     // Commande CMD ou CMD_1
    char *cmd_false;    // Commande CMD_2 (si 'else')
} if_info;

'if_info * if_info_init()' et 'void destroy_if_info(if_info *info)'
qui vont respectivement initialiser la structure d'une commande if, et la détruire puis libérer sa mémoire.
'int evaluate_test(char *condition)' évalue 'condition' et renvoie sa valeur de retour.
'if_info * handle_if_line(char *line)' analyse 'line' donnée pour construire les informations d'un if_info.



- pwd.c:
Fichier contenant la commande pwd.

'int pwd(cmd_info * cmd)' recupère les informations d'une commande 'cmd' et exécute pwd.
'int redirect_pwd(int output)' redirige l'exécution de pwd selon 'output'.



- redirection.c:
Fichier contenant le code des redirections du shell.

'int redirection(char * redirection, const char * file)' applique 'redirection' vers 'file'.
'void close_redirection()' ferme toutes les redirections.