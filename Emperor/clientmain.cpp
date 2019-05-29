#include "util.h"
#define TMPBUFFERLEN 100

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
                LOG::record(UTILLOGLEVEL1, "write %d : %s\n", __LINE__,strerror(errno));
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
    int port =8889;
    int fd=tcpGenericConnect(NULL,port,"10.8.49.62",8888);
    if(fd < 0)
    {
        LOG::record(UTILLOGLEVEL1, "tcpGenericConnect : %s", strerror(errno));
        return UTILNET_ERROR;
    }
    std::thread alineread(threadRead,fd);
    alineread.detach();

    char buf[TMPBUFFERLEN];
    int diagcount=0;
    int r=0;

    while(1)
    {
        memset(buf,0,TMPBUFFERLEN);
        r=read(fd,buf,TMPBUFFERLEN);
        if(r < 0)
        {
            if ((errno == EINTR)||(errno == EAGAIN))
            {
                continue;
            }else{
                LOG::record(UTILLOGLEVEL1, "read %d : %s\n", __LINE__,strerror(errno));
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
    }

    close(fd);
    return 0;
}
