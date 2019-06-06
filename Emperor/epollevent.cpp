
#include "util.h"

epollevent::epollevent(int listen_fd):
listen_fd_(listen_fd),
timeout_(1000),  //一秒
readbuf_(nullptr)
{
    EpollInit();
}

epollevent::~epollevent()
{
    delete [] readbuf_;
    delete []  ee_;
}

void epollevent::EpollInit()
{   
    epollfd_=epoll_create(EPOLLMAXEVENTS);
    if (ep_fd < 0)
    {
        LOG::record(1,"epcreate error:%d %s\n",errno,strerror(errno));
        return;
    }
    epollCreateEvents(listen_fd_);

    readbuf_ = new char[STRUCTONPERLEN];
    memset(readbuf_,0,STRUCTONPERLEN);

    ee_ = new struct epoll_event[EPOLLMAXEVENTS];
    memset(ee_,0,sizeof(struct epoll_event)*EPOLLMAXEVENTS);
}

void epollevent::EpollCreateEvents(int tfd)
{
    struct epoll_event ee = {0,0};
    ee.events |=  EPOLLIN | EPOLLET |EPOLLRDHUP;
    ee.data.fd=tfd;
    
    printf("epoll create %d:%d\n",ep_fd,tfd);
    if(epoll_ctl(epollfd_,EPOLL_CTL_ADD,tfd,&ee)==-1)
    {
        LOG::record(UTILLOGLEVEL1,"epoll create %d:%s\n",errno,strerror(errno));
        return;
    }

    if(tfd!=listen_fd_)
    {
        channel::GetInstance()->AddNewUser(tfd);
    }
}

void epollevent::EpollReadCallback(int fd)
{
    memset(readbuf_,0,STRUCTONPERLEN);
    int rlen=0;
    do{
        rlen = read(fd,readbuf_,STRUCTONPERLEN);
        if (rlen<0)
        {
            if (errno == EAGAIN) || (errno == EINTR)) {
                continue;
                /* Try again later */
            }
            LOG::record(UTILLOGLEVEL1,"%s %d read:%s",__FUNCTION__,errno,strerror(errno));
            break;
        }else if(rlen==0)
        {//断开连接的直接删除该事件
            EpollDelEvent(fd);
            LOG::record(UTILLOGLEVEL1,"client diconnect %d",fd);
            return ;
        }
        break;
    }while(1);
    channel::GetInstance()->UserReadProtocl(buf); //进行协议解析处理
}

void epollevent::EpollAcceptCallback(int fd)
{
    int tfd=0;
    static int count=0;
    struct sockaddr m;
    socklen_t flag=1;
    do{
        tfd=accept(listen_fd_,&m,&flag);
        if(tfd==-1)
        {
            if ((errno == EINTR)||(errno == EAGAIN))
            {
                continue;
            }else{
                LOG::record(UTILLOGLEVEL1,"%s accept:%s \n",__FUNCTION__,strerror(errno));
                break; 
            }
        }
        break;
    }while(1);

    setNonBlock(tfd);

    EpollCreateEvents(tfd);
}

/*
wait 之后需要定期查看心跳是否
*/
int epollevent::EpollEventWaite(int fd)
{
    memset(ee_,0,sizeof(struct epoll_event)*EPOLLMAXEVENTS);
    int numReady=epoll_wait(epollfd_,ee_,MAXIUMEVENTS,timeout_);
    if(numReady<0)
    {
        LOG::record(UTILLOGLEVEL1,"epoll_wait %d:%s\n",errno,strerror(errno));
        return numReady;
    }
    
    for(int i=0;i<numReady;i++)
    {
        if(ee_[i].data.fd==listen_fd_)
        {
            EpollAcceptCallback(ee_[i].data.fd);
        }else if(ee_[i].events & EPOLLIN )
        {
            EpollReadCallback(ee_[i].data.fd);
        }else if(ee_[i].events & EPOLLRDHUP )
        {
            EpollDelEvent(ee_[i].data.fd);
        }
    }
    return 0;
}

void epollevent::EpollDelEvent(int fd)
{
    printf("disconnect %d\n",fd);
    aeApiState *state = eventLoop->apidata;
    struct epoll_event ee = {0}; /* avoid valgrind warning */
    ee.events = 0;
    ee.data.fd = fd;
    int ret=epoll_ctl(epollfd_,EPOLL_CTL_DEL,fd,&ee);
    if(ret<0)
    {
        LOG::record(UTILLOGLEVEL1,"epoll_ctl %d:%s\n",errno,strerror(errno));
        return;
    }
    channel::GetInstance()->UserRemove(fd); //从队列中删除；
}
