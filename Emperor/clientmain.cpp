#include "util.h"
#include "clientuser.h"
#include <sys/wait.h>
//#include "openssl.h"
//std::cin.getline(buf,TMPBUFFERLEN);



int main(int argc, char*argv[])  
{
    int f=-1;
    printf("argc:%d\n",argc);
    if(argc>1)
    {
        f=*(argv[1])-'0';
    }
    int ret=0;
    if(f>0)
    {
        std::shared_ptr<epollclienthandle> m = std::make_shared<epollclienthandle>(f);
        
        while((ret<5)&&(ret>=0))
        {
            ret=m->StartConnect(SERVERLISTENIP,SERVERLISTENPORT);
            GENERRORPRINT("StartConnect failed",ret,0);
            sleep(3);
        }
        return 1;
    }

    #if 1
        int fd=0;
        int i=0;
        for(i=1;i<1000;i++)
        {
            fd=fork();
            if(fd==0)
            {
                break;
            }else
            {
                printf("create processor uid=%d -> fd=%d\n",i,fd);
            }
        }

        //printf("processor fd:%d i:%d\n",fd,i);
        std::shared_ptr<epollclienthandle> m = std::make_shared<epollclienthandle>(i);

        m->StartConnect(SERVERLISTENIP,SERVERLISTENPORT);

        int status=0;
        if(fd==0)
        {
            exit(1);
        }
        int pid=wait(&status);
        printf("client status:%d wait pid=%d\n",status,pid);
    #else

    
    for(int i=0;i<argc;i++)
    {
        printf("%s\n",argv[i]);
    }
    printf("%d\n",f);

    std::shared_ptr<epollclienthandle> m = std::make_shared<epollclienthandle>(f);

    m->StartConnect(SERVERLISTENIP,SERVERLISTENPORT);

    #endif

    return 0;
}
