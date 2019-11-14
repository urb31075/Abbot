#include <cstring>
#include <sys/fcntl.h>
#include <unistd.h>
#include "Log.h"
using namespace std;

#define STR_DATETIME_DATE             1
#define STR_DATETIME_TIME_SEC_NO      2
#define STR_DATETIME_TIME_SEC_YES     3
#define STR_DATETIME_DATETIME_SEC_NO  4
#define STR_DATETIME_DATETIME_SEC_YES 5
#define STR_DATETIME_DATETIME_MYSQL   6


string strdatetime(struct tm *l, int type)
{
    char t[32];
 
    switch (type)
    {         
        case STR_DATETIME_DATE             :snprintf(t, sizeof(t), "%.2d-%.2d-%d", l->tm_mday, l->tm_mon + 1, l->tm_year + 1900); break;
        case STR_DATETIME_TIME_SEC_NO      :snprintf(t, sizeof(t), "%.2d:%.2d", l->tm_hour, l->tm_min); break;
        case STR_DATETIME_TIME_SEC_YES     :snprintf(t, sizeof(t), "%.2d:%.2d:%.2d", l->tm_hour, l->tm_min, l->tm_sec); break;
        case STR_DATETIME_DATETIME_SEC_NO  :snprintf(t, sizeof(t), "%.2d-%.2d-%d %.2d:%.2d", l->tm_mday, l->tm_mon + 1, l->tm_year + 1900, l->tm_hour, l->tm_min); break;
        case STR_DATETIME_DATETIME_SEC_YES :snprintf(t, sizeof(t), "%.2d-%.2d-%d %.2d:%.2d:%.2d", l->tm_mday, l->tm_mon + 1, l->tm_year + 1900, l->tm_hour, l->tm_min, l->tm_sec); break;
        case STR_DATETIME_DATETIME_MYSQL   :snprintf(t, sizeof(t), "'%d-%.2d-%.2d %.2d:%.2d:%.2d'", l->tm_year + 1900, l->tm_mon + 1, l->tm_mday, l->tm_hour, l->tm_min, l->tm_sec); break;
        default: t[0] = 0; break;
    }  
    return t;
}    

string strdatetime(time_t c, int type)
{
    const auto l  = localtime(&c);
    return strdatetime(l, type);
}

string strdatetimenow(const int type)
{
    auto c  = time(nullptr);
    const auto l  = localtime(&c);
    return strdatetime(l, type);
    }
    void simple_loger::log_write(const char *text)
    {
        auto sw = strdatetimenow(STR_DATETIME_DATETIME_SEC_YES) + " " + text + "\n";
        write(fd_, sw.c_str(), strlen(sw.c_str())); 
    }

void simple_loger::log_close() { if (fd_ > 0) close(fd_); fd_ = -1; }
simple_loger::~simple_loger(){ log_close(); }
simple_loger::simple_loger(const char *filename){ fd_ = open(filename, O_RDWR | O_APPEND | O_CREAT , S_IRUSR | S_IWUSR | S_IROTH); }
