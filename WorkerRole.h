#ifndef WORKERROLE_H
#define WORKERROLE_H

#define HELP_CMD      "help"
#define DEBUG_CMD     "debug"
#define LOG_CMD       "log"

#define MAX_MESSAGE_LEN 128

#define LOGFILENAME "/home/urb/abbot.dat"

#define LOGDB "185.104.248.240"
            
typedef enum { help = 0, unknown = 1, debug = 2, log = 3, exit_time_out = 4 } worker_command_t; 

typedef struct
{
    int number;
    int connfd;
    int mode;
} worker_role_parameters_t;

void *worker_role_thread_func(void *arg);
    
#endif /* WORKERROLE_H */

