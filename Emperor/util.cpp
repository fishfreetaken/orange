
#include "util.h"

int createlisten(int port)
{
    int yes=1;
    char _port[6];
    struct addrinfo hints,*servinfo;
    snprintf(_port,6,"%d",port);
    memset(&hints,0,sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo("localhost",_port,&hints,&servinfo) <0 ) {
        LOG::record(1, "%s", strerror(errno));
        return -1;
    }

    int socket_fd=socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol);
    if(socket_fd ==-1 )
    {
        LOG::record(UTILLOGLEVEL,"socket create : %s",strerror(errno));
        goto error;
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        LOG::record( UTILLOGLEVEL,"setsockopt SO_REUSEADDR: %s", strerror(errno));
        goto error;
    }
    //fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFL) | O_NONBLOCK);
    

    if(bind(socket_fd,servinfo->ai_addr,servinfo->ai_addrlen)==-1)
    {
        LOG::record( UTILLOGLEVEL,"bind : %s", strerror(errno));
        goto error;
    }
    
    if (listen(socket_fd, 15) == -1) {
        LOG::record(UTILLOGLEVEL, "listen: %s", strerror(errno));
        goto error;
    }
error:
    if (socket_fd != -1) close(socket_fd);
    yes = socket_fd;
end:
    freeaddrinfo(servinfo);
    return yes;
}
