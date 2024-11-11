#ifndef CMD
#define CMD
#define CMD_SIZE 64
#define OPTIONS_SIZE 64
#define ARG_SIZE 512

typedef struct cmd_info {
    char * cmd_name;
    char * opt;
    char * args;
} cmd_info;

cmd_info * cmd_info_init();
void destroy_cmd_info(cmd_info *);
char * get_cmd_name(cmd_info *);
char * get_opt(cmd_info *);
char * get_args(cmd_info *);
#endif 