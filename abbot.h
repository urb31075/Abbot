#ifndef ABBOTHELP_H
#define ABBOTHELP_H

typedef struct  
    {
        int port;
        int mode;
        int delay;
    } abbot_params_t;

int parse_command_line(int argc, char** argv, abbot_params_t *abbot_params);    

#endif

