#include "log.h"

/*32M*1024*1024 */
/*
三种类型的日志：
1 服务器客户端错误日志；
2 系统运行日志；
3 用户行为日志(用户对话)；
 */

/*
debug //调试进行终端输出

//主要是实现这个打印的检测
error.log:
{
error
info
warnning
}

chat.log
{
    protocal
}
*/

#define LOGBUFFERLEN 32*1024*1024
/*双buf写日志,一个日志写满了，切换另一个写，另一个起一个线程进行写文件操作； */
LOG::LOG(int belong)
belong_(belong)
{
    try{
        headbufA_= new char[LOGBUFFERLEN];
    }catch(...)
    {
        throw "new failed or open file failed"
    }

    //headbufB_= new  char[];
}

LOG::~LOG()
{
    delete [] headbufA_;
    //delete [] headbufB_;
}

void LOG::Init()
{
    open(logfilename,O_CREAT|O_APPEND|O_WRONLY)
    pid_=getpid();
    timezone_=

    pid_t pid = getpid();

    tzset(); /* Now 'timezome' global is populated. */
    time_t t = time(NULL);
    struct tm *aux = localtime(&t);
    int daylight_active = aux->tm_isdst;

    struct tm tm;
    char buf[1024];

    nolocks_localtime(&tm,t,timezone,daylight_active);

    off = strftime(buf,sizeof(buf),"%d_%b_%Y",&tm);
    snprintf(buf+strlen(buf),"_error.log");
}

void LOG::record(int level,const char* c,...)
{
    va_list pArgList;
    va_start(pArgList, c);
    //int nByteWrite =vfprintf(stdout,c,pArgList);
    vfprintf(stdout,c,pArgList);
    vfprintf(stdout,"\n",NULL);
    va_end(pArgList);
}

void LOG::ErrorGenic(int level,const char* c,...)
{
    va_list pArgList;
    va_start(pArgList, c);
    //int nByteWrite =vfprintf(stdout,c,pArgList);
    vfprintf(stdout,c,pArgList);
    vfprintf(stdout,"\n",NULL);
    va_end(pArgList);
}
