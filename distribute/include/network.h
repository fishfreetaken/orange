#ifndef NET_WORK_MAIN_
#define NET_WORK_MAIN_
#include "debug.h"
#include "syslog.h"

#define REDIS_ERR -1
#define REDIS_OK 0

#define ANET_OK 0
#define ANET_ERR -1
#define ANET_ERR_LEN 256
class netServer{
public:
    netServer(const int &port,const std::string&addr,const size_t &back_log):
    port_(port),
    addr_(addr),
    backlog_(back_log)
    {
    }
    ~netServer()
    {
       int i=  ::close(socketfp_);
       if(i!=0)
       {
           printf("close file %d\n",i);
       }
    }

    void serverstart()
    {
        if(addr_.size()==0)
        {
            printf("you creat port=%d addr=%s is no valid\n",port_,addr_.c_str());
            return ;
        }
        char err[255]={'\0'};
        socketfp_= _anetTcpServer(err,port_,addr_.c_str(),AF_INET,backlog_);
        
        if(strlen(err)&&(strlen(err)<255))
        {
            printf("%s\n",err);
        }else{
            serverLog(LL_VERBOSE,"success create a server port=%d ip=%s",port_,addr_.c_str());
        }
    }

    void recv()
    {
        char buf[256];
        int ret=0;
        int count=0;
        static int cc=0;
        
        for(auto i : aaccept_)
        {
            //printf("begin to read %d\n",i);
            ret=::read(i,buf,256);
            if(ret>0)
            {
                count++;
                printf("i=%d rev heart %s\n",i,buf);
            }
            else{
                printf("i=%d %s\n",i,strerror(errno));
            }
            memset(buf,0,256);
        }
        cc++;
       // printf("begin to collect  aaccept_=%zu send NO=%d total=%d\n",aaccept_.size(),count,cc);
    }

    void clear()
    {
        static int count=0;
        if((aaccept_.size()-count)>10)
        {
            count=aaccept_.size();
            printf("the %zu connect\n",aaccept_.size());
        }else if (aaccept_.size()>1000)
        {
            printf("the %zu connect\n",aaccept_.size());
        }

        return;

        if(aaccept_.size()<4)
        {
            return;
        }
        printf("will clear the timeout client connect %zu\n",aaccept_.size());
        int ret=0;
        for(auto i:aaccept_)
        {
            ret=::close(i);
            if(ret<0)
            {
                printf("closed failed %d\n",ret);
            }
        }
        aaccept_.clear();
    }
    void serveraccept()
    {
        if(socketfp_<0)
        {
            serverLog(LL_VERBOSE,"Error bad file descriptor %s %d: %s",__FILE__,__LINE__);
            return ;
        }

        int cport, cfd;
        int max = 1000;
        char cip[46];
        //clusterLink *link;
        char error[255]={'\0'};
        ///UNUSED(el);
        //UNUSED(mask);
        //UNUSED(privdata);

        /* If the server is starting up, don't accept cluster connections:
        * UPDATE messages may interact with the database content. */

        while(max--) {
            cfd = anetTcpAccept(error, socketfp_, cip, sizeof(cip), &cport);
            if (cfd == ANET_ERR) {
                if (errno != EWOULDBLOCK)
                    serverLog(LL_VERBOSE,
                        "Error accepting cluster node: %s %d",strerror(errno),__LINE__);
                return;
            }
            //anetNonBlock(NULL,cfd);
            anetEnableTcpNoDelay(NULL,cfd);

            /* Use non-blocking I/O for cluster messages. */
            serverLog(LL_VERBOSE,"Accepted node %s:%d", cip, cport);
            
            clear();
            aaccept_.push_back(cfd);

            /* Create a link object we use to handle the connection.
            * It gets passed to the readable handler when data is available.
            * Initiallly the link->node pointer is set to NULL as we don't know
            * which node is, but the right node is references once we know the
            * node identity. */
            /*
            link = createClusterLink(NULL);
            link->fd = cfd;*/
            //aeCreateFileEvent(server.el,cfd,AE_READABLE,clusterReadHandler,link);
        }
        
    }
    
private:
    int port_;
    size_t backlog_;
    std::string addr_;
    std::vector<int> aaccept_;

    int socketfp_;
private:

    int anetSetBlock(char *err, int fd, int non_block) {
        int flags;

        /* Set the socket blocking (if non_block is zero) or non-blocking.
        * Note that fcntl(2) for F_GETFL and F_SETFL can't be
        * interrupted by a signal. */
        if ((flags = fcntl(fd, F_GETFL)) == -1) {
            anetSetError(err, "fcntl(F_GETFL): %s", strerror(errno));
            return ANET_ERR;
        }

        if (non_block)
            flags |= O_NONBLOCK;
        else
            flags &= ~O_NONBLOCK;

        if (fcntl(fd, F_SETFL, flags) == -1) {
            anetSetError(err, "fcntl(F_SETFL,O_NONBLOCK): %s", strerror(errno));
            return ANET_ERR;
        }
        return ANET_OK;
    }

    int anetSetTcpNoDelay(char *err, int fd, int val)
    {
        if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) == -1)
        {
            anetSetError(err, "setsockopt TCP_NODELAY: %s", strerror(errno));
            return ANET_ERR;
        }
        return ANET_OK;
    }

    int anetNonBlock(char *err, int fd) {
        return anetSetBlock(err,fd,1);
    }

    int anetEnableTcpNoDelay(char *err, int fd)
    {
        return anetSetTcpNoDelay(err, fd, 1);
    }


    int anetTcpAccept(char *err, int s, char *ip, size_t ip_len, int *port) {
        int fd;
        struct sockaddr_storage sa;
        socklen_t salen = sizeof(sa);
        if ((fd = anetGenericAccept(err,s,(struct sockaddr*)&sa,&salen)) == -1)
            return ANET_ERR;

        if (sa.ss_family == AF_INET) {
            struct sockaddr_in *s = (struct sockaddr_in *)&sa;
            if (ip) inet_ntop(AF_INET,(void*)&(s->sin_addr),ip,ip_len);
            if (port) *port = ntohs(s->sin_port);
        } else {
            struct sockaddr_in6 *s = (struct sockaddr_in6 *)&sa;
            if (ip) inet_ntop(AF_INET6,(void*)&(s->sin6_addr),ip,ip_len);
            if (port) *port = ntohs(s->sin6_port);
        }
        return fd;
    }

    int anetGenericAccept(char *err, int s, struct sockaddr *sa, socklen_t *len) {
        int fd;
        while(1) {
            fd = ::accept(s,sa,len);
            if (fd == -1) {
                if (errno == EINTR)
                    continue;
                else {
                    anetSetError(err, "accept: %s", strerror(errno));
                    return ANET_ERR;
                }
            }
            break;
        }
        return fd;
    }

    void anetSetError(char *err, const char *fmt, ...)
    {
        va_list ap;

        if (!err) return;
        va_start(ap, fmt);
        vsnprintf(err, ANET_ERR_LEN, fmt, ap);
        va_end(ap);
    }

    int _anetTcpServer(char *err, int port,const char *bindaddr, int af, int backlog)
    {
        int s = -1, rv;
        char _port[6];  /* strlen("65535") */
        struct addrinfo hints, *servinfo, *p;

        snprintf(_port,6,"%d",port);
        memset(&hints,0,sizeof(hints));
        hints.ai_family = af;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;    /* No effect if bindaddr != NULL */

        if ((rv = getaddrinfo(bindaddr,_port,&hints,&servinfo)) != 0) {
            anetSetError(err, "%s", gai_strerror(rv));
            return ANET_ERR;
        }
        for (p = servinfo; p != NULL; p = p->ai_next) {
            if ((s = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1)
                continue;

            //if (af == AF_INET6 && anetV6Only(err,s) == ANET_ERR) goto error;
            if (anetSetReuseAddr(err,s) == ANET_ERR) goto error;
            if (anetListen(err,s,p->ai_addr,p->ai_addrlen,backlog) == ANET_ERR) s = ANET_ERR;
            goto end;
        }
        if (p == NULL) {
            anetSetError(err, "unable to bind socket, errno: %d", errno);
            goto error;
        }

    error:
        if (s != -1) close(s);
        s = ANET_ERR;
    end:
        freeaddrinfo(servinfo);
        return s;
    }

    int anetSetReuseAddr(char *err, int fd) {
        int yes = 1;
        /* Make sure connection-intensive things like the redis benchmark
        * will be able to close/open sockets a zillion of times */
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            anetSetError(err, "setsockopt SO_REUSEADDR: %s", strerror(errno));
            return ANET_ERR;
        }
        return ANET_OK;
    }

    int anetListen(char *err, int s, struct sockaddr *sa, socklen_t len, int backlog) {
        if (bind(s,sa,len) == -1) {
            anetSetError(err, "bind: %s", strerror(errno));
            close(s);
            return ANET_ERR;
        }

        if (listen(s, backlog) == -1) {
                anetSetError(err, "listen: %s", strerror(errno));
                close(s);
                return ANET_ERR;
            }
            return ANET_OK;
    }

};

enum redisConnectionType {
    REDIS_CONN_TCP,
    REDIS_CONN_UNIX
};

 /* Context for a connection to Redis */
typedef struct redisContext {
    //int err; /* Error flags, 0 when there is no error */
    //char errstr[128]; /* String representation of error when applicable */
    int fd;
    int flags;
    char *obuf; /* Write buffer */
    //redisReader *reader; /* Protocol reader */

    enum redisConnectionType connection_type;
    struct timeval *timeout;

    struct {
        char *host;
        char *source_addr;
        int port;
    } tcp;

    struct {
        char *path;
    } unix_sock;

} redisContext;
/* Flag that is set when we should set SO_REUSEADDR before calling bind() */
#define REDIS_REUSEADDR 0x80
/* Connection type can be blocking or non-blocking and is set in the
 * least significant bit of the flags field in redisContext. */
#define REDIS_BLOCK 0x1

/* Connection may be disconnected before being free'd. The second bit
 * in the flags field is set when the context is connected. */
#define REDIS_CONNECTED 0x2

/* number of times we retry to connect in the case of EADDRNOTAVAIL and
 * SO_REUSEADDR is being used. */
#define REDIS_CONNECT_RETRIES  10

/* Event types that can be polled for.  These bits may be set in `events'
   to indicate the interesting event types; they will appear in `revents'
   to indicate the status of the file descriptor.  */
#define POLLIN		0x001		/* There is data to read.  */
#define POLLPRI		0x002		/* There is urgent data to read.  */
#define POLLOUT		0x004		/* Writing now will not block.  */

#define __MAX_MSEC (((LONG_MAX) - 999) / 1000)

class netclient
{

   
public:
    netclient(int port,char * addr):
    port_(port),
    source_addr_("127.0.0.1")
    {
        strcpy(addr_,addr);
        c= new redisContext;
        c->tcp.host = addr_;
        c->tcp.port = port_;
        c->tcp.source_addr= source_addr_;

        c->flags &= ~REDIS_BLOCK;
        c->flags |= REDIS_REUSEADDR;
    }
    ~netclient(){
        printf("total build connect %zu\n",client_.size());
        for(auto i : client_)
        {
            int re=::close(i);
            if(re <0)
            {
                clientLog(1,"%d close clinet fd failed : %s %d",i,strerror(errno),re);
            }
        }
        
        delete c;
    }

    void rec()
    {
        char buf[102]={'\0'};
        printf("begin read\n");
        int ret=::read(c->fd,buf,102);
        if(ret>=0)
        {
            printf("rec %s\n",buf);
        }else{
            printf("rec error %d  %s\n",ret,strerror(errno));
        }
    }

    void wri()
    {
        char buf[256]={'\0'};
        for(auto i : client_)
        {
            //sleep(1);
            sprintf(buf,"cli=%d heart message",i);
            ::write(i,buf,strlen(buf));
            memset(buf,0,256);
        }
    }

    int toconnect(){
        // _redisContextConnectTcp(c,c->tcp.host,c->tcp.port,c->timeout,c->tcp.source_addr);
        _redisContextConnectTcp(c,c->tcp.host,c->tcp.port,c->timeout,NULL);

        client_.push_back(c->fd);
        return c->fd;
    }

private:
    int port_;
    char addr_[50];
    char source_addr_[50];

    std::vector<int> client_;

    redisContext *c;
private:
    int _redisContextConnectTcp(redisContext *c, const char *addr, int port,
                                    const struct timeval *timeout,
                                    const char *source_addr) {
        int s, rv, n;
        char _port[6];  /* strlen("65535"); */
        struct addrinfo hints, *servinfo, *bservinfo, *p, *b;
        int blocking = (c->flags & REDIS_BLOCK);
        int reuseaddr = (c->flags & REDIS_REUSEADDR);
        int reuses = 0;
        long timeout_msec = -1;

        servinfo = NULL;
        c->connection_type = REDIS_CONN_TCP;
        c->tcp.port = port;

        /* We need to take possession of the passed parameters
        * to make them reusable for a reconnect.
        * We also carefully check we don't free data we already own,
        * as in the case of the reconnect method.
        *
        * This is a bit ugly, but atleast it works and doesn't leak memory.
        **/
        if (c->tcp.host != addr) {
            if (c->tcp.host)
                free(c->tcp.host);

            c->tcp.host = strdup(addr);
        }

        if (timeout) {
            if (c->timeout != timeout) {
                if (c->timeout == NULL)
                    c->timeout = (struct timeval *)malloc(sizeof(struct timeval));

                memcpy(c->timeout, timeout, sizeof(struct timeval));
            }
        } else {
            if (c->timeout)
                free(c->timeout);
            c->timeout = NULL;
        }

        if (redisContextTimeoutMsec(c, &timeout_msec) != REDIS_OK) {
            //__redisSetError(c, REDIS_ERR_IO, "Invalid timeout specified");
            clientLog(1,"Invalid timeout specified");
            goto error;
        }

        if (source_addr == NULL) {
            //free(c->tcp.source_addr);
            c->tcp.source_addr = NULL;
        } else if (c->tcp.source_addr != source_addr) {
            free(c->tcp.source_addr);
            c->tcp.source_addr = strdup(source_addr);
        }

        snprintf(_port, 6, "%d", port);
        memset(&hints,0,sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        /* Try with IPv6 if no IPv4 address was found. We do it in this order since
        * in a Redis client you can't afford to test if you have IPv6 connectivity
        * as this would add latency to every connect. Otherwise a more sensible
        * route could be: Use IPv6 if both addresses are available and there is IPv6
        * connectivity. */
        if ((rv = getaddrinfo(c->tcp.host,_port,&hints,&servinfo)) != 0) {
           // hints.ai_family = AF_INET6;
           // if ((rv = getaddrinfo(addr,_port,&hints,&servinfo)) != 0) {
                //__redisSetError(c,REDIS_ERR_OTHER,);
                clientLog(1,"Invalid tgetaddrinfo: %s",gai_strerror(rv));
                return REDIS_ERR;
            //}
        }
        for (p = servinfo; p != NULL; p = p->ai_next) {
    addrretry:
            if ((s = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1)
                continue;

            c->fd = s;
            if (redisSetBlocking(c,1) != REDIS_OK)
                goto error;
            if (c->tcp.source_addr) {
                int bound = 0;
                /* Using getaddrinfo saves us from self-determining IPv4 vs IPv6 */
                if ((rv = getaddrinfo(c->tcp.source_addr, NULL, &hints, &bservinfo)) != 0) {
                    clientLog(1,"Can't get addr: %s",gai_strerror(rv));
                    //char buf[128];
                   // snprintf(buf,sizeof(buf),"Can't get addr: %s",gai_strerror(rv));
                   // __redisSetError(c,REDIS_ERR_OTHER,buf);
                    goto error;
                }

                if (reuseaddr) {
                    n = 1;
                    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*) &n,
                                sizeof(n)) < 0) {
                        goto error;
                    }
                }

                for (b = bservinfo; b != NULL; b = b->ai_next) {
                    if (bind(s,b->ai_addr,b->ai_addrlen) != -1) {
                        bound = 1;
                        break;
                    }
                }

                freeaddrinfo(bservinfo);
                if (!bound) {
                    clientLog(1,"%s %d: Can't bind socket: %s ",__FILE__,__LINE__,strerror(errno));
                    //char buf[128];
                   // snprintf(buf,sizeof(buf),"Can't bind socket: %s",strerror(errno));
                   // __redisSetError(c,REDIS_ERR_OTHER,buf);
                    goto error;
                }
            }
            if (connect(s,p->ai_addr,p->ai_addrlen) == -1) {
                if (errno == EHOSTUNREACH) {
                    redisContextCloseFd(c);
                    continue;
                } else if (errno == EINPROGRESS && !blocking) {
                    /* This is ok. */
                } else if (errno == EADDRNOTAVAIL && reuseaddr) {
                    if (++reuses >= REDIS_CONNECT_RETRIES) {
                        goto error;
                    } else {
                        redisContextCloseFd(c);
                        goto addrretry;
                    }
                } else {
                    if (redisContextWaitReady(c,timeout_msec) != REDIS_OK)
                        goto error;
                }
            }
            if (blocking && redisSetBlocking(c,1) != REDIS_OK)
                goto error;
            if (redisSetTcpNoDelay(c) != REDIS_OK)
                goto error;

            c->flags |= REDIS_CONNECTED;
            rv = REDIS_OK;
            goto end;
        }
        if (p == NULL) {

            clientLog(1,"Can't create socket: %s",strerror(errno));
            
            goto error;
        }

    error:
        rv = REDIS_ERR;
    end:
        freeaddrinfo(servinfo);
        return rv;  // Need to return REDIS_OK if alright
    }

    int redisContextTimeoutMsec(redisContext *c, long *result)
    {
        const struct timeval *timeout = c->timeout;
        long msec = -1;

        /* Only use timeout when not NULL. */
        if (timeout != NULL) {
            if (timeout->tv_usec > 1000000 || timeout->tv_sec > __MAX_MSEC) {
                *result = msec;
                return REDIS_ERR;
            }

            msec = (timeout->tv_sec * 1000) + ((timeout->tv_usec + 999) / 1000);

            if (msec < 0 || msec > INT_MAX) {
                msec = INT_MAX;
            }
        }

        *result = msec;
        return REDIS_OK;
    }
    int redisSetBlocking(redisContext *c, int blocking) {
        int flags;

        /* Set the socket nonblocking.
        * Note that fcntl(2) for F_GETFL and F_SETFL can't be
        * interrupted by a signal. */
        if ((flags = fcntl(c->fd, F_GETFL)) == -1) {
            //__redisSetErrorFromErrno(c,REDIS_ERR_IO,"fcntl(F_GETFL)");
            clientLog(1,"cntl(F_GETFL)");
            redisContextCloseFd(c);
            return REDIS_ERR;
        }

        if (blocking)
            flags &= ~O_NONBLOCK;
        else
            flags |= O_NONBLOCK;

        if (fcntl(c->fd, F_SETFL, flags) == -1) {
            //__redisSetErrorFromErrno(c,REDIS_ERR_IO,"fcntl(F_SETFL)");
            clientLog(1,"cntl(F_SETFL)");
            redisContextCloseFd(c);
            return REDIS_ERR;
        }
        return REDIS_OK;
    }
    void redisContextCloseFd(redisContext *c) {
        if (c && c->fd >= 0) {
            close(c->fd);
            c->fd = -1;
        }
    }

    int redisContextWaitReady(redisContext *c, long msec) {
        struct pollfd   wfd[1];

        wfd[0].fd     = c->fd;
        wfd[0].events = POLLOUT;

        if (errno == EINPROGRESS) {
            int res;

            if ((res = poll(wfd, 1, msec)) == -1) {
                clientLog(1,"poll(2)");
                //__redisSetErrorFromErrno(c, REDIS_ERR_IO, "poll(2)");
                redisContextCloseFd(c);
                return REDIS_ERR;
            } else if (res == 0) {
                errno = ETIMEDOUT;
                clientLog(1,"ETIMEDOUT");
               // __redisSetErrorFromErrno(c,REDIS_ERR_IO,NULL);
                redisContextCloseFd(c);
                return REDIS_ERR;
            }

            if (redisCheckSocketError(c) != REDIS_OK)
                return REDIS_ERR;

            return REDIS_OK;
        }
        clientLog(1,"REDIS_ERR_IO %d %s",__LINE__,strerror(errno));
        //__redisSetErrorFromErrno(c,REDIS_ERR_IO,NULL);
        redisContextCloseFd(c);
        return REDIS_ERR;
    }

    int redisCheckSocketError(redisContext *c) {
        int err = 0;
        socklen_t errlen = sizeof(err);

        if (getsockopt(c->fd, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1) {
            clientLog(1,"%s getsockopt(SO_ERROR)",__LINE__ );
          //  __redisSetErrorFromErrno(c,REDIS_ERR_IO,"getsockopt(SO_ERROR)");
            return REDIS_ERR;
        }

        if (err) {
            errno = err;
            clientLog(1,"%s getsockopt(REDIS_ERR_IO)",__LINE__);
            //__redisSetErrorFromErrno(c,REDIS_ERR_IO,NULL);
            return REDIS_ERR;
        }

        return REDIS_OK;
    }


    int redisSetTcpNoDelay(redisContext *c) {
        int yes = 1;
        if (setsockopt(c->fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes)) == -1) {
            //__redisSetErrorFromErrno(c,REDIS_ERR_IO,"setsockopt(TCP_NODELAY)");
            clientLog(1,"setsockopt(TCP_NODELAY)");
            redisContextCloseFd(c);
            return REDIS_ERR;
        }
        return REDIS_OK;
    }

};


#endif