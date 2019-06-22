#include "util.h"
#include "clientuser.h"
#define TMPBUFFERLEN 300

void threadRead(int fd)
{
    char buf[TMPBUFFERLEN];
    int r=0;
    printf("Hello thread read!\n");
    while(1)
    {
        memset(buf,0,TMPBUFFERLEN);
        //printf("Please input you want to send to server:");
        std::cin.getline(buf,TMPBUFFERLEN);
        //scanf("%s",buf);
        do{
             r=write(fd,buf,strlen(buf));
            if(r < 0)
            {
                LOG::record(UTILLOGLEVELERROR, "write %d : %s\n", __LINE__,strerror(errno));
            }
        }while(0);
        
        if(strcmp(buf,"over")==0)
        {
            printf("skip the while\n");
            break;
        }
        printf("len:%d local to other:\n%s\n",r,buf);
    }
    printf("threadRead over!");
}


int main()
{
    /*
    for(int i=0;i<3;i++)
    {
        int fk_fd=fork(); //2的n次方个进程
    }
    */
   epollclienthandle s(1234567);
   s.StartConnect(SERVERLISTENIP,SERVERLISTENPORT);

   #if 0
    char buf[TMPBUFFERLEN];

    int port =8889;
    int fd=tcpGenericConnect(NULL,port,SERVERLISTENIP,SERVERLISTENPORT);
    if(fd < 0)
    {
        LOG::record(UTILLOGLEVELERROR, "tcpGenericConnect : %s", strerror(errno));
        return UTILNET_ERROR;
    }
    std::thread alineread(threadRead,fd);
    alineread.detach();

    int diagcount=0;
    int r=0;
    transfOnPer *data=new transfOnPer;
    while(1)
    {
        memset(data,0,STRUCTONPERLEN);
        //memset(buf,0,TMPBUFFERLEN);
        r=read(fd,data,STRUCTONPERLEN);
        if(r < 0)
        {
            if ((errno == EINTR)||(errno == EAGAIN))
            {
                continue;
            }else{
                LOG::record(UTILLOGLEVELERROR, "read %d : %s\n", __LINE__,strerror(errno));
                break ;
            }
        }
        if(r==0)
        {
            printf("server is closed! break;\n");
            break;
        }
        diagcount++;
        printf("count:%d len:%d  other to local:\n%s\n",r,diagcount,buf);

        //printf("parse receive:id:%d from:%d to:%d size:%d\n",data->id,data->from,data->to,data->size);
    }
    close(fd);
    #endif

    
    return 0;
}
