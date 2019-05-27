#include "util.h"

int main()
{
    int port =8889;
    int fd=tcpGenericConnect(NULL,port,"localhost",8888);
    if(fd < 0)
    {
        LOG::record(UTILLOGLEVEL, "tcpGenericConnect : %s", strerror(errno));
        return UTILNET_ERROR;
    }

    char buf[7]="hello\0";
    int r=write(fd,buf,6);
    if(r < 0)
    {
        LOG::record(UTILLOGLEVEL, "write %d : %s\n", __LINE__,strerror(errno));
        return UTILNET_ERROR;
    }

    close(fd);
    return 0;
}
