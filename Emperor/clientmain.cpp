#include "util.h"

int main()
{
    int port =8889;
    int fd=tcpGenericConnect("localhost",port,"localhost",8888);
    if(fd < 0)
    {
        LOG::record(UTILLOGLEVEL, "tcpGenericConnect : %s", strerror(errno));
        return UTILNET_ERROR;
    }

    char buf[37]="hello\0";
    int r=write(fd,buf,6);
    if(r < 0)
    {
        LOG::record(UTILLOGLEVEL, "write %d : %s\n", __LINE__,strerror(errno));
        return UTILNET_ERROR;
    }

    r=read(fd,buf,37);
    if(r < 0)
    {
        LOG::record(UTILLOGLEVEL, "read %d : %s\n", __LINE__,strerror(errno));
        return UTILNET_ERROR;
    }
    printf("r:%d this is received from %s\n",r,buf);


    while(1)
    {
        
    }

    close(fd);
    return 0;
}
