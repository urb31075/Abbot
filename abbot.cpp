#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include "abbot.h"

void print_help(void);

int parse_command_line(int argc, char** argv, abbot_params_t *abbot_params)
{
    abbot_params->port = 44001;
    abbot_params->mode = 1;
    abbot_params->delay = 1;

    if(argc == 1) // если запускаем без аргументов, выводим краткую справку
    { 
        printf("missing command-line options.\n");   
        printf("running with default parameters.\n"); //предупреждение что хорошо бы ввести явно параметры
        printf("use abbot -h for help.\n");        
        return 1; // запустить можно, поэтому выхолдим с 1
    } 
    
    int opt;        
    char opts[] = "p:m:d:o:h:"; // Доступные опции p-port, m-mode, d-delay
    struct option long_opt[] = 
    {
        {"port", 1, 0, 'p'},
        {"mode", 1, 0, 'm'},
        {"delay", 1, 0, 'd'},
        {"help", 0, 0, 'h'},
        {0,0,0,0}
    };
    
    int optIdx;
    while((opt = getopt_long(argc, argv, opts, long_opt, &optIdx)) != -1)
    {
        switch(opt) 
        {
            case 'p': // -p, заменяем порт по умолчанию
                abbot_params->port = atoi(optarg);
                break;
            case 'm': // -m устанавливаем режим (0 - тихий, 1 - отладка, 2 - итд)
                abbot_params->mode = atoi(optarg);
                break;
            case 'd': // -d задержка между посылками на telnet в секундах (0 - забить весь канал)
                abbot_params->delay = atoi(optarg);
                break;
            case 'h': // -h вывели справку
                print_help(); // После вывода справки запускать не стоит
                return 0;
        }
    }

    return 1;
}

void print_help(void)
{
    printf("abbot - ControlSP Dispatcher\n");
    printf("usage: abbot [--port] [--delay] [--mode] [--operator]\n");
    printf("\t port - port number\n");
    printf("\t delay - delay between packets in seconds\n");
    printf("\t mode - 0 - silent, 1 - debugging, 2 - logging\n");
    printf("\t operator - not used yet\n");    
    
    printf("example: abbot --port 34567 --delay 2 --mode 5 --operator xyz\n");
    printf("example: abbot -p 34567\n");    
}

