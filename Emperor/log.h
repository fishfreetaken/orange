#include <cstdarg>
#include <stdio.h>

/*general error! */
#define GENERALESUCCESS 0
#define GENERALERROR -1
#define GENERALNOTFOUND -2

//都是按天创建吧
#define SERVERERRORLOG "./log/2019_07_19_error.log"
#define CLIENTERRORLOG "./log/2019_07_19_clienterror.log"
#define USERCHATCLIENTLOG  //这个根据时间创建；

/*log level */
#define UTILLOGLEVELERROR       1
#define UTILLOGLEVELRECORD      2
#define UTILLOGLEVELWORNNING    3
#define UTILLOGLEVELDIALOG      4

#define LogInfo(...)     Log::GetInstance().AddLog("INFO", __FILE__, __LINE__, __FUNCSIG__, __VA_ARGS__)
#define LogWarning(...)  Log::GetInstance().AddLog("WARNING", __FILE__, __LINE__, __FUNCSIG__, __VA_ARGS__)
#define LogError(...)    Log::GetInstance().AddLog("ERROR", __FILE__, __LINE__, __FUNCSIG__, __VA_ARGS__)
bool Log::AddLog(const char* pszLevel, const char* pszFile, int lineNo, const char* pszFuncSig, char* pszFmt, ...)

class LOG{
public:
    static void record(int level,const char* c,...);
    static void ErrorGenic(int level,const char* c,...);
};

#define GENERICERRORPRINT LOG::ErrorGenic(UTILLOGLEVELERROR,"file:%s LINE:%d error:%d info:%s",__FILE__,__LINE__,errno,strerror(errno))
#define GENERRORPRINT(str,a,b) LOG::ErrorGenic(UTILLOGLEVELERROR,"file:%s LINE:%d --- %s %d %d",__FILE__,__LINE__,str,a,b)
