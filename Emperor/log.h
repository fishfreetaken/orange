#ifndef LOG_HEADER_
#define LOG_HEADER_

#include <cstdarg>
#include <stdio.h>
#include "util.h"

/*general error! */
#define GENERALESUCCESS 0
#define GENERALERROR -1
#define GENERALNOTFOUND -2


/*log level */
#define UTILLOGLEVELERROR       1
#define UTILLOGLEVELRECORD      2
#define UTILLOGLEVELWORNNING    3
#define UTILLOGLEVELDIALOG      4

#define WETHERTODEBUG 1

#define LogInfo(...)     LOG::GetInstance()->AddLog("INFO", __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LogWarning(...)  LOG::GetInstance()->AddLog("WARNING", __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LogError(...)    LOG::GetInstance()->AddLog("ERROR", __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LogRecord(...)    LOG::GetInstance()->AddLog("RECORD", __VA_ARGS__)

class Status;
class LOG{
public:
    LOG();
    ~LOG();
    static LOG* GetInstance();
    static void record(int level,const char* c,...);
    static void ErrorGenic(int level,const char* c,...);

    bool AddLog(const char *level, const char* pszFile, int lineNo, const char* pszFuncSig,const char* pszFmt, ...);

    bool AddLog(const char *level,const char* pszFmt, ...);
private:
    Status Init();
    void WriteFd(int fd);
    void WrteMmap();

private:
    int errorfd_;
    int accessfd_;
    time_t timezone_;
    int daylight_active_;
    pid_t pid_;

    //const int tmpbuflen_;

    char *buf_;

    static LOG*  instance_;

};

#define GENERICERRORPRINT LOG::ErrorGenic(UTILLOGLEVELERROR,"file:%s LINE:%d error:%d info:%s",__FILE__,__LINE__,errno,strerror(errno))
#define GENERRORPRINT(str,a,b) LOG::ErrorGenic(UTILLOGLEVELERROR,"file:%s LINE:%d --- %s %d %d",__FILE__,__LINE__,str,a,b)

#endif
