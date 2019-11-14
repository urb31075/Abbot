#ifndef LOG_H
#define LOG_H
#include <string>
using namespace std;
class simple_loger
{
private:
    int  fd_;
public:
    void log_write(const char *text);
    void log_write(string text){log_write(text.c_str()); }
    void log_write_msg(const string text){ auto writedata = "MSG: " + text; log_write(writedata.c_str()); }
    void log_write_err(const string text){ auto writedata = "ERR: " + text; log_write(writedata.c_str()); }
    
    void log_close(void);
    simple_loger(const char *filename);
    ~simple_loger(void);
};
#endif /* LOG_H */