#include "util.h"
#include "log.h"
#include "clientuser.h"
#define TMPBUFFERLEN 300

void threadRead(int fd,int i)
{
    char buf[TMPBUFFERLEN]={0};
    snprintf(buf,TMPBUFFERLEN,"%d",i);
    int ret=0;
    printf("Hello thread read!\n");
    int count=10000;
    while(count)
    {
        //memset(buf,0,TMPBUFFERLEN);
        //ret=readGenericReceive(1, buf,TMPBUFFERLEN);
        do{
            ret=write(fd,buf,strlen(buf));
            if (ret<0)
            {
                if ((errno == EINTR)||(errno== EAGAIN))
                {
                    LOG::record(UTILLOGLEVELERROR,"%d read:%s continue",errno,strerror(errno));
                    continue;
                }
                LOG::record(UTILLOGLEVELERROR,"%d read:%s",errno,strerror(errno));
                return ;
            }
            break;
        }while(1);
        
        if(strcmp(buf,"over")==0)
        {
            printf("skip the while\n");
            break;
        }
        count--;
        //printf("len:%d local to other:\n%s\n",ret,buf);
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
   //epollclienthandle s(1234567);
   //s.StartConnect(SERVERLISTENIP,SERVERLISTENPORT);

   #if 1
    char buf[TMPBUFFERLEN];

    int port =8889;
    int fd=tcpGenericConnect(NULL,port,SERVERLISTENIP,SERVERLISTENPORT);
    printf("connect server %d\n",fd);
    if(fd < 0)
    {
        LOG::record(UTILLOGLEVELERROR, "tcpGenericConnect : %s", strerror(errno));
        return UTILNET_ERROR;
    }
    std::thread alineread1(threadRead,fd,1);
    std::thread alineread2(threadRead,fd,2);
    std::thread alineread3(threadRead,fd,3);
    std::thread alineread4(threadRead,fd,4);
    std::thread alineread5(threadRead,fd,5);
    std::thread alineread6(threadRead,fd,6);
    alineread1.detach();
    alineread2.detach();
    alineread3.detach();
    alineread4.detach();
    alineread5.detach();
    alineread6.detach();

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
