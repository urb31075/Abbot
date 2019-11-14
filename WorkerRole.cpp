#include <sys/types.h> 
#include <sys/socket.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <mysql/mysql.h>
#include "WorkerRole.h"
#include "Log.h"


extern pthread_rwlock_t rwlock; // Синхронизация: мало пишут - много читают

void print_to_client_deprecated(const worker_role_parameters_t* worker_role_parameters, const char* msg)
{
    char buff[256]; 
    snprintf(buff, sizeof(buff), "%s\r\n", msg);
    write(worker_role_parameters->connfd, buff, strlen(buff));      
}

void print_to_log(const worker_role_parameters_t* worker_role_parameters, const char* msg)
{
    char buff[1024];
    simple_loger loger("/home/urb/abbot.log");
    sprintf(buff, "DBG:%i|%s", worker_role_parameters->number, msg);
    loger.log_write(buff);
    loger.log_close();
}

worker_command_t get_worker_command(const worker_role_parameters_t* worker_role_parameters, char *commandbody)
{
    char inbuff[MAX_MESSAGE_LEN];    
    char message[MAX_MESSAGE_LEN];
    inbuff[0] = 0; message[0] = 0;  
    
    struct timespec start_time{};
    clock_gettime(CLOCK_REALTIME, &start_time);

    while (true)
    {
        struct timespec current_time{};
        clock_gettime(CLOCK_REALTIME, &current_time);
        if (current_time.tv_sec - start_time.tv_sec > 5*60)
            return exit_time_out;

        const int byte_read = recv(worker_role_parameters->connfd, inbuff, sizeof(inbuff) - 1, 0);        
        if (byte_read <= 0) 
            continue;

        inbuff[byte_read] = 0;
        
        int message_len = strlen(message);
        if ((message_len + byte_read) >= MAX_MESSAGE_LEN)
        {
            print_to_log(worker_role_parameters, "Max command lenght exteed!");
            return unknown;
        }
        
        strcpy(message + strlen(message), inbuff);
        message_len = strlen(message);
        if (message_len > 1 && 
            (message[message_len - 2] == '\r' && message[message_len - 1] == '\n') || 
             message[message_len - 2] == 'x'  && (message[message_len - 1] == 'y')) // Для отладки с SIM900 в AT-команду терминале жутко не удобно набирать с \r \n
        {
            message[message_len - 2] = 0;
            print_to_log(worker_role_parameters, message);
            
            if (!strcmp(message, HELP_CMD))      return help;
            if (!strcmp(message, DEBUG_CMD))     return debug;
            if  (strstr(message, LOG_CMD) == message) 
            {
                strcpy(commandbody, message + strlen(LOG_CMD) + 1);   
                return log;
            }

            return unknown;
        }
    } 
}

void add_db_log_item(const worker_role_parameters_t* worker_role_parameters, const char *msg, const time_t timestamp)
{
    const auto con = mysql_init(nullptr); 
    if (con == nullptr) 
    {
        print_to_log(worker_role_parameters, "error mysql init!");                             
        return;
    }
    
    mysql_set_character_set(con,"utf8");
    if(mysql_real_connect(con, LOGDB, "abbot", "dupel", "abbotdb", 0, nullptr, 0) == nullptr)
    {
        mysql_close(con);            
        print_to_log(worker_role_parameters, "error mysql connect!");                     
        return;
    }
    
    char timestampbuff[64];
    strftime(timestampbuff, sizeof(timestampbuff), "%Y-%m-%d %H:%M:%S", localtime(&timestamp));
    
    char sql[512]; 
    const auto pos = strchr(msg, ' ') - msg + 1;
    char cod_str[16]; memset(cod_str, 0, 16);
    strncpy(cod_str, msg, pos);
    const int cod = std::atoi(cod_str);

    snprintf(sql, sizeof(sql), "INSERT INTO log (msg, cod, date) VALUES('%s', %d, '%s' )", msg + pos, cod, timestampbuff);
    if (mysql_query(con, sql))
    {
        print_to_log(worker_role_parameters, "error mysql log writing!");             
    }
    
    mysql_close(con);            
    }

    void add_file_log_item(const worker_role_parameters_t* worker_role_parameters, const char *msg, const time_t timestamp)
    {
        char timestampbuff[64];
        strftime(timestampbuff, sizeof(timestampbuff), "%Y-%m-%d %H:%M:%S", localtime(&timestamp));
    
        char logbuff[512];
        int cod = 0;
        snprintf(logbuff, sizeof(logbuff), "%d | %s | %s\r\n", cod, timestampbuff, msg);
    
        FILE * file = fopen(LOGFILENAME, "ae");
        if (file) // если есть доступ к файлу,
        {
            const auto result = fputs(logbuff, file); // и записываем ее в файл
            if (!result) // если запись произошла успешно
            {
                print_to_log(worker_role_parameters, "error file log writing!");     
            }
        
            fclose(file);        
        }    
    }

    void execute_log(const worker_role_parameters_t* worker_role_parameters, const char *commandbody)
    {
        const auto now = time(nullptr);    
        add_file_log_item(worker_role_parameters, commandbody, now);
        add_db_log_item(worker_role_parameters, commandbody, now);
    }

    void *worker_role_thread_func(void *arg)
    { 
        worker_role_parameters_t worker_role_parameters;
        memcpy(&worker_role_parameters, arg, sizeof(worker_role_parameters));
        print_to_log(&worker_role_parameters, "New Connect");    

        auto read_command_loop = true;    
        char message_body[MAX_MESSAGE_LEN];
        memset(message_body, 0, sizeof(message_body));

        while(read_command_loop)
        {
            const auto wc = get_worker_command(&worker_role_parameters, message_body);
            switch (wc)
            {
                case log:          
                    print_to_log(&worker_role_parameters, "log  command");                                        
                    execute_log(&worker_role_parameters, message_body);      
                    read_command_loop = false; 
                    break;
                case exit_time_out:  
                    print_to_log(&worker_role_parameters, "close by timeOut"); 
                    read_command_loop = false; 
                    break;
                case unknown:      
                    print_to_log(&worker_role_parameters, "unknown command"); 
                    break;
                default: 
                    print_to_log(&worker_role_parameters, "default");                        
                    break;
            }
        }
        //sleep(60);
        close(worker_role_parameters.connfd);
        pthread_exit(nullptr);
        return nullptr;
    }