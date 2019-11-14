#include <cstdio> 
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <csignal>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include "abbot.h"
#include "ServerRole.h"
#include <sys/fcntl.h>
#include <cstring>
#include "Log.h"

using namespace std;
#define LOOK_PID_FILE "/var/tmp/lookd.pid"

void log_write_err(const char *err){puts(err); }
int main(const int argc, char** argv) 
{
    //setlocale(0,""); // включаем кириллицу в консоли пока не нужна но пригодится
    abbot_params_t abbot_params;   // 
    if(parse_command_line(argc, argv, &abbot_params) == 0) return 0;
    print_server_role_start_information(abbot_params);    //Печатаем хелп

    char  buff[256];
    const pid_t pid = fork();
    if (pid == -1)return 1;
    else if (pid != 0) exit(EXIT_SUCCESS);
    if (setsid() == -1)   return 1; 
    if (chdir("/") == -1) return 1;
    for (auto i = 0; i < 3; i++) close(i);
    const int stdio = open("/dev/null", O_RDWR);
    if ((dup(stdio) != -1) && (dup(stdio) != -1))  
        setpgrp();
    else  
        printf("Error dup");

    simple_loger loger("/home/urb/abbot.log");
    sprintf(buff, "Abbot start");
    loger.log_write(buff);
    const auto f = open(LOOK_PID_FILE, O_RDWR | O_CREAT | O_TRUNC, 0200 + 0004);
    if (f != -1)
    {
        if (ftruncate(f, 0) != -1)
        {
            const int abbot_pid = getpid();
            snprintf(buff, sizeof(buff), "%d", abbot_pid);
            if (write(f, buff, strlen(buff)) != -1)
            {
                server_role_parameters server_role_parameters;    
                server_role_parameters.pid = abbot_pid; 
                server_role_parameters.param2 = 0;
                server_role_parameters.param3 = 0;    
                server_role_parameters.abbot_params = abbot_params;                
                server_role_thread_func(server_role_parameters);    //-----старт демона-------------
            }
            else log_write_err("Error write pid file");
        }
        else log_write_err("Error truncate pid file");
    }
    else log_write_err("Error open pid file");
    log_write_err("Error run Lexx"); 
    return 1;
}