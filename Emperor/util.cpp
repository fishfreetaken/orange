#include "util.h"
#define TMPBUFFERLEN 100
int main()
{
    int port =8889;
    int fd=tcpGenericConnect(NULL,port,"10.8.49.62",8888);
    if(fd < 0)
    {
        LOG::record(UTILLOGLEVEL1, "tcpGenericConnect : %s", strerror(errno));
        return UTILNET_ERROR;
    }

    char buf[TMPBUFFERLEN];
    int diagcount=0;
    while(1)
    {
        memset(buf,0,TMPBUFFERLEN);
        printf("Please input you want to send to server:");
        std::cin.getline(buf,TMPBUFFERLEN);

        printf("your input len:%d\n",strlen(buf));

        int r=write(fd,buf,strlen(buf));
        if(r < 0)
        {
            LOG::record(UTILLOGLEVEL1, "write %d : %s\n", __LINE__,strerror(errno));
            return UTILNET_ERROR;
        }
        if(strcmp(buf,"over")==0)
        {
            printf("skip the while\n");
            break;
        }

        printf("send over and begin to read!\n");
        memset(buf,0,TMPBUFFERLEN);
        r=read(fd,buf,TMPBUFFERLEN);
        if(r < 0)
        {
            LOG::record(UTILLOGLEVEL1, "read %d : %s\n", __LINE__,strerror(errno));
            return UTILNET_ERROR;
        }
        diagcount++;
        printf("r:%d dia:%d server message: %s\n",r,diagcount,buf);
    }
    
    
    close(fd);
    return 0;
}
