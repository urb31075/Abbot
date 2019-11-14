#include <cstdio> 
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <mysql/mysql.h>
#include "abbot.h"
#include "ServerRole.h"
#include "WorkerRole.h"
#include "Log.h"

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

void sigpipe_handler(const int unused)
{
     printf("Sigpipe_handler %i\n", unused);
}

void init_server_role()
{
    
}

void print_server_role_start_information(const abbot_params_t abbot_params)
{
    printf("Start Abbot!");
    printf("Configure on port:%i delay:%i mode:%i.\n", abbot_params.port, abbot_params.delay, abbot_params.mode);
    printf("MySql client version: %s\n", mysql_get_client_info());
}

void server_role_thread_func(const server_role_parameters server_role_parameters)
{
    auto connection_count = 0;
    char outbuff[256];
    simple_loger loger("/home/urb/abbot.log");    
    sprintf(outbuff, "Server thread pid=%d port=%d start", server_role_parameters.pid, server_role_parameters.abbot_params.port);
    loger.log_write(outbuff);

    const auto listenfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0); 
    struct timeval tv {};
    tv.tv_sec  = 10;          // 30 Secs Timeout
    tv.tv_usec = 0;          // Not init'ing this can cause strange errors
    setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(struct timeval)); // Таймаут для ожидания подключения к порту

    struct sockaddr_in serv_addr{};     
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serv_addr.sin_port = htons(server_role_parameters.abbot_params.port); 
    bind(listenfd, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)); 
    listen(listenfd, MAX_LISTENFD);     
    while(true) 
    {
        const auto connfd = accept4(listenfd, static_cast<struct sockaddr*>(nullptr), nullptr, SOCK_CLOEXEC);
        if (connfd > 0)
        {
            pthread_t thread;    
            pthread_attr_t thread_attr; 
            pthread_attr_init(&thread_attr); 
            pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);     
    
            worker_role_parameters_t worker_role_parameters;
            worker_role_parameters.number = ++connection_count;
            worker_role_parameters.connfd = connfd;
            worker_role_parameters.mode = server_role_parameters.abbot_params.mode;
            const pthread_t result = pthread_create(&thread, &thread_attr, worker_role_thread_func, static_cast<void*>(&worker_role_parameters));
            if (result != 0) {
                sprintf(outbuff, "Creating worker thread false!\r\n");
                loger.log_write(outbuff);
            }         
        }
    }
}
