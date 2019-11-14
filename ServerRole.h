#ifndef SERVERROLE_H
#define SERVERROLE_H
#include "abbot.h"

#define MAX_LISTENFD 100
typedef struct
{
    int pid;
    int param2;
    int param3;    
    abbot_params_t abbot_params;    
} server_role_parameters;

void init_server_role();
void print_server_role_start_information(abbot_params_t abbot_params);

void server_role_thread_func(server_role_parameters server_role_parameters);
#endif /* SERVERROLE_H */

