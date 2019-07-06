#include <cstdarg>
#include <stdio.h>

/*general error! */
#define GENERALESUCCESS 0
#define GENERALERROR -1
#define GENERALNOTFOUND -2


/*log level */
#define UTILLOGLEVELERROR       1
#define UTILLOGLEVELRECORD      2
#define UTILLOGLEVELWORNNING    3
#define UTILLOGLEVELDIALOG      4



class LOG{
public:
    static void record(int level,const char* c,...);
    static void ErrorGenic(int level,const char* c,...);
};

#define GENERICERRORPRINT LOG::ErrorGenic(UTILLOGLEVELERROR,"file:%s LINE:%d error:%d info:%s",__FILE__,__LINE__,errno,strerror(errno))
#define GENERRORPRINT(str,a,b) LOG::ErrorGenic(UTILLOGLEVELERROR,"file:%s LINE:%d --- %s %d %d",__FILE__,__LINE__,str,a,b)
