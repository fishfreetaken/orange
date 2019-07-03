#include "util.h"
#include "clientuser.h"

#include "openssl.h"
//std::cin.getline(buf,TMPBUFFERLEN);

#include<sys/wait.h>

size_t DATATESTLEN=128;

int main()
{
    #if 0

    unsigned char data[DATATESTLEN]="QWERTYUIOPASDF";
    char endata[DATATESTLEN]={0};
    char dedata[DATATESTLEN]={0};
    rsa_test(data,DATATESTLEN,endata,DATATESTLEN,dedata,DATATESTLEN);

    return 1;
    #endif
    epollclienthandle m(1);
    m.StartConnect(SERVERLISTENIP,SERVERLISTENPORT);

    return 1;

    int status;

    size_t i=1;
    int t=0;
    int ret=0;
    for(;i<11;i++)
    {
        t=fork();
        if(t<0)
        {
            printf("Fork %d failed t=%d ,error:%s\n",i,t,strerror(errno));
            break;
        }else{
            ret=i;
            printf("Fork child t=%d\n",t);
        }
    }
    
    epollclienthandle s(ret);
    s.StartConnect(SERVERLISTENIP,SERVERLISTENPORT);

    ret= wait(&status);
    if(ret<0)
    {
        printf("wait failed ret=%d\n",ret);
    }

    exit(0);

}
