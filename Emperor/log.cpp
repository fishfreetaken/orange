#include "log.h"
#include <sys/mman.h>

extern int WhetherServer;
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

#define tmpbuflen_ 512

/*双buf写日志,一个日志写满了，切换另一个写，另一个起一个线程进行写文件操作； 这样如果server dump掉了日志就丢了*/
LOG::LOG()
{
    try{
        buf_=new char[tmpbuflen_];
        memset(buf_,0,tmpbuflen_);
    }catch(...)
    {
        throw "new buf_ error!";
    }

    Status s= Init();
    assert(s.ok());
}

LOG::~LOG()
{
    //delete [] headbufB_;
    ::close(errorfd_);
    ::close(accessfd_);
    delete [] buf_;
}

LOG* LOG::instance_=new LOG;

LOG* LOG::GetInstance()
{
    return instance_;
}

Status LOG::Init()
{
     pid_ = getpid();

    //timezone_ = getTimeZone();
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    timezone_=tz.tz_minuteswest * 60UL;

    time_t t = time(NULL);
    struct tm *aux = localtime(&t);
    daylight_active_ = aux->tm_isdst;

    struct tm tm;
    
    char buf[tmpbuflen_]={0};
    std::memset(buf,0,tmpbuflen_);

    nolocks_localtime(&tm,t,timezone_,daylight_active_);
    strftime(buf,tmpbuflen_,"./log/%Y_%m_%d",&tm);
    strftime(buf_,tmpbuflen_,"./log/%Y_%m_%d",&tm);

    if(!WhetherServer)
    {
        sprintf(buf+strlen(buf),"_clienterror.log");
        sprintf(buf_+strlen(buf_),"_clientaccess.log");
    }else
    {
        sprintf(buf+strlen(buf),"_error.log");

        sprintf(buf_+strlen(buf_),"_access.log");
    }
    errorfd_= ::open(buf,O_CREAT|O_APPEND|O_WRONLY);
    if(errorfd_<0)
    {
        perror("Log error init failed");
        return Status::IOError("Log init failed",nullptr);
    }

    accessfd_= ::open(buf_,O_CREAT|O_APPEND|O_WRONLY);
    if(errorfd_<0)
    {
        perror("Log access init failed");
        return Status::IOError("Log init failed",nullptr);
    }

    //2019_06_05_error.log

    return Status();
}

bool LOG::AddLog(const char* pszLevel, const char* pszFile, int lineNo, const char* pszFuncSig,const char* pszFmt, ...)
{
    va_list pArgList;
    va_start(pArgList, pszFmt);

    //char buf[tmpbuflen_]={0};
    std::memset(buf_,0,tmpbuflen_);

    struct timeval tv;
    struct tm tm;

    gettimeofday(&tv,NULL);
    nolocks_localtime(&tm,tv.tv_sec,timezone_,daylight_active_);

    strftime(buf_,tmpbuflen_,"%H:%M:%S ",&tm);

    sprintf(buf_+strlen(buf_),"%d %s %s:%d %s: ",pid_,pszLevel,pszFile,lineNo,pszFuncSig);
    
    vsprintf(buf_+strlen(buf_),pszFmt,pArgList);
    sprintf(buf_+strlen(buf_),"\n");

    #ifdef WETHERTODEBUG
        fprintf(stderr,"%s",buf_);
    #endif

    va_end(pArgList);

#ifndef USEMMAPTOBUF
    WriteFd(errorfd_);
#else
    WrteMmap(errorfd_);
#endif
}

bool LOG::AddLog(const char *level,const char* pszFmt, ...)
{
    va_list pArgList;
    va_start(pArgList, pszFmt);

    //char buf[tmpbuflen_]={0};
    std::memset(buf_,0,tmpbuflen_);

    struct timeval tv;
    struct tm tm;

    gettimeofday(&tv,NULL);

    strftime(buf_,tmpbuflen_,"%H:%M:%S ",&tm);
    sprintf(buf_+strlen(buf_),"%d :",pid_);
    nolocks_localtime(&tm,tv.tv_sec,timezone_,daylight_active_);

    vsprintf(buf_+strlen(buf_),pszFmt,pArgList);
    sprintf(buf_+strlen(buf_),"\n");

    #ifdef WETHERTODEBUG
        fprintf(stdout,"%s",buf_);
    #endif

    va_end(pArgList);

#ifndef USEMMAPTOBUF
    WriteFd(accessfd_);
#else
    WrteMmap(accessfd_);
#endif
}

void LOG::WrteMmap()
{
}

void LOG::WriteFd(int fd)
{
    int ret=::write(fd,buf_,strlen(buf_));

    if(ret<(int)strlen(buf_))
    {
        printf("write fd ret %d len %d failed %s",ret,strlen(buf_),strerror(errno));
    }
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
