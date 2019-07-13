#include "util.h"
#include "clientuser.h"
#include<sys/wait.h>

//#include "openssl.h"
//std::cin.getline(buf,TMPBUFFERLEN);

int main()  
{
    

    //epollclienthandle s(3);
    //s.StartConnect(SERVERLISTENIP,SERVERLISTENPORT);
    int i=1;
    int fd=0;
    for(;i<8;i++)
    {
        fd=fork();
        if(fd==0)
        {
            
            break;
        }
    }
    printf("fork id=%d fd=%d\n",i,fd);
    std::shared_ptr<epollclienthandle> m = std::make_shared<epollclienthandle>(i);

    m->StartConnect(SERVERLISTENIP,SERVERLISTENPORT);
    int status;
    wait(&status);
    return 0;
}
