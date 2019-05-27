
#include "util.h"

int main()
{
    int port =8888;

    int fd=tcpGenericServer(port);
    if(fd < 0)
    {
        LOG::record(UTILLOGLEVEL, "createlistst __LINE__ : %s", strerror(errno));
        return -1;
    }

    printf("success create server fd: %d\n",fd);
    char buf[32]={0};

    struct sockaddr m;
    socklen_t flag;
    int count=0;
    while(1)
    {
        int tfd=accept(fd,&m,&flag);
        if(tfd==-1)
        {
            if (errno == EINTR)
            {
                continue;
            }else{
                LOG::record(UTILNET_ERROR,"__LINE__ accept:%s",strerror(errno));
                return -1;
            }
        }
        LOG::record(1,"accept a connect:%d",tfd);
        int rlen = read(tfd,buf,32);
        if (rlen<0)
        {
            LOG::record(UTILNET_ERROR,"__LINE__ read:%s",strerror(errno));
        }
        printf("receive: %s",buf);
        memset(buf,0,32);
        close(tfd);
        count++;
        LOG::record(UTILNET_ERROR,"__LINE__ accept count:%d",count);
    }

    close(fd);
    
    return 0;
}
