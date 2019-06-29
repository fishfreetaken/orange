
#include "util.h"
#include <memory>
#include <fstream>

int setResusedConfig(int socket_fd)
{
    int yes;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        LOG::record( UTILLOGLEVELERROR,"setsockopt SO_REUSEADDR: %s", strerror(errno));
        return -1;
    }
    return 0;
}

void setNonBlock(int socket_fd)
{
    fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFL) | O_NONBLOCK);
}

int tcpGenericServer(const char *source_addr,int port)
{
    char _port[6];
    struct addrinfo hints,*servinfo;
    snprintf(_port,6,"%d",port);
    memset(&hints,0,sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(source_addr,_port,&hints,&servinfo) <0 ) {
        LOG::record(1, "%s", strerror(errno));
        return -1;
    }

    int socket_fd=socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol);
    if(socket_fd ==-1 )
    {
        LOG::record(UTILLOGLEVELERROR,"socket create : %s",strerror(errno));
        goto error;
    }

    if (setResusedConfig(socket_fd)<0)
    {
        goto error;
    }
    
    //setNonBlock(socket_fd);

    if(bind(socket_fd,servinfo->ai_addr,servinfo->ai_addrlen)==-1)
    {
        LOG::record( UTILLOGLEVELERROR,"bind : %s", strerror(errno));
        goto error;
    }
    
    if (listen(socket_fd, 15) == -1) {
        LOG::record(UTILLOGLEVELERROR, "listen: %s", strerror(errno));
        goto error;
    }

    goto end;
error:
    if (socket_fd != -1) 
    {
        close(socket_fd);
        socket_fd=-1;
    }
    
end:
    freeaddrinfo(servinfo);
    return socket_fd;
}

int tcpGenericConnect(const char *source_addr,int port,const char *dest_ip,int dest_port)
{
    char portstr[6];  /* strlen("65535") + 1; */
    struct addrinfo hints, *servinfo, *bservinfo, *p, *b;

    snprintf(portstr,sizeof(portstr),"%d",dest_port);
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(dest_ip,portstr,&hints,&servinfo) != 0) {
        LOG::record(UTILLOGLEVELERROR, "getaddrinfo: %s", strerror(errno));
        return UTILNET_ERROR;
    }
    int socket_fd;
    if ((socket_fd = socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol)) == -1)
    {
        LOG::record(UTILLOGLEVELERROR, "clinet socket: %s", strerror(errno));
        goto error;
    }

    if (setResusedConfig(socket_fd)<0)
    {
        goto error;
    }

    if(source_addr)
    {
        
        char *srcport=NULL;
        if (port)
        {
            memset(&portstr,0,6);
            snprintf(portstr,sizeof(portstr),"%d",port);
            srcport=portstr;
        }
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
        if (getaddrinfo(source_addr, srcport, &hints, &bservinfo) != 0)
        {
            LOG::record(UTILLOGLEVELERROR, "source_addr getaddrinfo: %s", strerror(errno));
            goto error;
        }
        if(bind(socket_fd,bservinfo->ai_addr,bservinfo->ai_addrlen)!=-1){
            LOG::record(UTILLOGLEVELERROR, "source_addr bind: %s", strerror(errno));
        }
    }

    if(connect(socket_fd,servinfo->ai_addr,servinfo->ai_addrlen)==-1)
    {
        if (errno == EINPROGRESS )
        {
            goto end;
        }else{
            goto error;
        }
        LOG::record(UTILLOGLEVELERROR, "connect: %s", strerror(errno));
    }
    goto end;
error:
    if (socket_fd != -1) 
    {
        close(socket_fd);
        socket_fd=-1;
    }
    
end:
    freeaddrinfo(servinfo);
    return socket_fd;
}

int writeGenericSend(int fd,const char * buf,int len)
{
    if(buf==nullptr)
    {
        LOG::record(UTILLOGLEVELERROR, "%s buf is nullptr",__FUNCTION__);
        return UTIL_POINTER_NULL;
    }
    int re=write(fd,buf,len);
    if (re<=0)
    {
        LOG::record(UTILLOGLEVELERROR, "fd:%d %s error : %s",fd,__FUNCTION__, strerror(errno));
        return UTILNET_ERROR;
    }
    return re;
}


int readGenericReceive(int fd, char* buf,int len)
{
    if(buf==nullptr)
    {
        LOG::record(UTILLOGLEVELERROR, "%s buf is nullptr",__FUNCTION__);
        return UTIL_POINTER_NULL;
    }
    std::memset(buf,0,len);

    int ret=0;
    do{
        ret=read(fd,buf,len);
        if(ret==-1)
        {
            if ((errno == EINTR)||(errno == EAGAIN))
            {
                LOG::record(UTILNET_ERROR,"%s LINE:%d readfail:%s \n",__FUNCTION__,__LINE__,strerror(errno));
                return ret;
            }else{
                LOG::record(UTILNET_ERROR,"%s LINE:%d readfail:%s \n",__FUNCTION__,__LINE__,strerror(errno));
                return UTILLOGLEVELERROR; 
            }
        }
        break;
    }while(1);

    return ret;
}


void printTransfOnPer(transfOnPer *m,const char* from)
{
    /* 这边可以将心跳包进行过滤*/
    if(m->id==MSGHEART)
    {
        return;
    }
    printf("=====transfOnPer===from:=====%s=====\n",from);

    printf("id=%lu\n",m->id);
    printf("size=%lu\n",m->size);
    printf("uid=%zu\n",m->uid);
    printf("to=%zu\n",m->to);
    printf("buf: %s\n",m->buf);
    printf("crc:%zu\n",m->crc32); /*8个字节，直接强制输出一个整数 */
    printf("==============end=================\n");
}


void printfPartner(transfPartner *m,const char *from)
{
    printf("=======printfPartner===from:%s=======\n",from);
    printf("uid=%zu\n",m->uid);
    printf("state=%x\n",m->state);
    printf("name=%s\n",m->name);
    printf("signarue=%s\n",m->signature);
    printf("======printfPartner=====end===========\n");
}


