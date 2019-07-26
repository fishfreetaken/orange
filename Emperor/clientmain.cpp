#include "util.h"
#include "clientuser.h"
#include <sys/wait.h>
//#include "openssl.h"
int WhetherServer=0;

int main(int argc, char*argv[])  
{
    util_init();

    int f=-1;
    int ret=0;
    printf("argc:%d\n",argc);
    if(argc==2)
    {
        f=atoi(argv[1]);
        if(f>0)
        {
            std::shared_ptr<epollclienthandle> m = std::make_shared<epollclienthandle>(f);
            
            while(ret<5)
            {
                if(m->StartConnect(SERVERLISTENIP,SERVERLISTENPORT)==0)
                {
                    break;
                }
                GENERRORPRINT("StartConnect failed",ret,0);
                sleep(3);
                ret++;
            }
            return 1;
        }
    }
    
    int num=0;

    int fd=0;
    int i=0;
    if(argc==3)
    {
        num=atoi(argv[2]);
    }else{
        i=1;
    }
    
    printf("fork num is %d\n",num);

    #if 1
        
        for(i=1;i<num;i++)
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
