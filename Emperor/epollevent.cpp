
#include "util.h"
#include "log.h"

int epollhandlebase::WaitTimeOut()
{
    //默认的轮训时间1.5s
    return -1;
}

epollevent::epollevent(epollhandlebase *p):
listen_fd_(p->GetListenFd()),
epollfd_(-1),
objhandle_(p)
{
    EpollInit();
}

epollevent::epollevent(epollhandlebase *p,int epollfd):
listen_fd_(p->GetListenFd()),
epollfd_(epollfd),
objhandle_(p)
{
    EpollInit();
}

epollevent::~epollevent()
{
    if(ee_ != nullptr)
    {
        printf("~epollevent release ee_");
        delete []  ee_;
    }

    /* 单例模式进行析构
    if(objhandle_ !=nullptr)
    {
        pritnf("~epollevent release objhandle_ \n");
        delete objhandle_;
    }*/
}

void epollevent::EpollInit()
{
    if(objhandle_==nullptr)
    {
        throw std::string("objhandle_ not initialize!");
    }

    if(epollfd_<=2)
    {
        epollfd_=epoll_create(EPOLLMAXEVENTS);
        if (ep_fd < 0)
        {
            LOG::record(1,"epcreate error:%d %s\n",errno,strerror(errno));
            return;
        }
    }

    ee_ = new struct epoll_event[EPOLLMAXEVENTS];
    if(ee_==nullptr)
    {
        throw std::string("ee_ new failed!");
    }
    memset(ee_,0,sizeof(struct epoll_event)*EPOLLMAXEVENTS);

    if(listen_fd_ >=0)
    {
        struct epoll_event ee = {0,0};
        ee.events |=  EPOLLIN | EPOLLET |EPOLLRDHUP;
        ee.data.fd=tfd;
        EpollEventAdd(ee);
    }
}

int epollevent::EpollEventAdd(struct epoll_event &ee)
{
    if(ee.data.fd<0)
    {
        LOG::record(UTILLOGLEVEL1,"%s create events Failed\n",__FUNCTION__);
        return -1;
    }
    /*
    struct epoll_event ee = {0,0};
    ee.events |=  EPOLLIN | EPOLLET |EPOLLRDHUP;
    ee.data.fd=tfd;
    */
    printf("epoll create %d:%d\n",ep_fd,tfd);
    if(epoll_ctl(epollfd_,EPOLL_CTL_ADD,tfd,&ee)==-1)
    {
        LOG::record(UTILLOGLEVEL1,"epoll create %d:%s\n",errno,strerror(errno));
        return -1;
    }
    
    return 0;
}

/*
wait 之后需要定期查看心跳是否
*/
int epollevent::EpollEventWaite(int fd)
{

    memset(ee_,0,sizeof(struct epoll_event)*EPOLLMAXEVENTS);
    int numReady=epoll_wait(epollfd_,ee_,MAXIUMEVENTS,objhandle_->WaitTimeOut());
    if(numReady<0)
    {
        LOG::record(UTILLOGLEVEL1,"epoll_wait %d:%s\n",errno,strerror(errno));
        return numReady;
    }

    for(int i=0;i<numReady;i++)
    {
        if((ee_[i].data.fd==listen_fd_) && (ee_[i].events & EPOLLIN )) //添加事件
        {
            objhandle_->AcceptEvent();
        }else if(ee_[i].events & EPOLLIN )
        {
            objhandle_->ReadEvent(ee_[i].data.fd);
        }else if(ee_[i].events & EPOLLOUT)
        {
            objhandle_->WriteEvent(ee_[i].data.fd);
        }
        else if(ee_[i].events & EPOLLRDHUP )
        {
            EpollDelEvent(ee_[i].data.fd);
            objhandle_->DelEvent(ee_[i].data.fd); //通知断开客户定制的
        }else{
            LOG::record(UTILLOGLEVEL1,"epoll_wait fd:%d not known events %u\n",ee_[i].data.fd,ee_[i].events);
        }
    }
    return 0;
}

void epollevent::EpollDelEvent(int fd)
{
    printf("disconnect %d\n",fd);
    struct epoll_event ee = {0}; /* avoid valgrind warning */
    ee.events = 0;
    ee.data.fd = fd;
    int ret=epoll_ctl(epollfd_,EPOLL_CTL_DEL,fd,&ee);
    if(ret<0)
    {
        LOG::record(UTILLOGLEVEL1,"epoll_ctl %d:%s\n",errno,strerror(errno));
        return;
    }
    //channel::GetInstance()->UserRemove(fd); //从队列中删除；
}

/*要快速的处理 */
epollserverhandle::epollserverhandle(int timeout=-1): //-1就是阻塞wait
listen_fd_(-1),
timeout_(timeout)
{
}

epollserverhandle::~epollserverhandle()
{
    if(evp_!=nullptr)
    {
        delete evp_;
    }
}

epollevent* epollserverhandle::ServerStart(const char* listenip,const int port,int timeout=15000)
{
    listen_fd_=tcpGenericServer(SERVERLISTENIP,SERVERLISTENPORT);
    if(fd < 0)
    {
        LOG::record(UTILLOGLEVEL1, "ServerStart failed %s : %s",__FUNCTION__, strerror(errno));
        return nullptr;
    }
    evp_=new epollevent(this);
    if(evp_==nullptr)
    {
        LOG::record(UTILLOGLEVEL1, "ServerStart epollevent new failed %s : %s",__FUNCTION__, strerror(errno));
        return evp_
    }

    chm_= new channel();
    if(chm_==nullptr)
    {
        LOG::record(UTILLOGLEVEL1, "ServerStart channel new failed %s : %s",__FUNCTION__, strerror(errno));
    }

    return evp_;
}

void epollserverhandle::AcceptEvent()
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
                return; 
            }
        }
        break;
    }while(1);

    setNonBlock(tfd);
    struct epoll_event ee = {0,0};
    ee.events |=  EPOLLIN | EPOLLET |EPOLLRDHUP;
    ee.data.fd=tfd;

    if(evp_->EpollEventAdd(ee)>0)
    {
        if(tfd!=listen_fd_)
        {
            chm_->AddNewUser(tfd);
        }
    }
}

void epollserverhandle::ReadEvent(int tfd)
{//扩展以后就是分配一个线程池进行处理。
    /*
    memset(readbuf_,0,STRUCTONPERLEN);
    int rlen=0;
    do{
        rlen = read(fd,readbuf_,STRUCTONPERLEN);
        if (rlen<0)
        {
            if (errno == EAGAIN) || (errno == EINTR)) {
                continue; //Try again later
            }
            LOG::record(UTILLOGLEVEL1,"%s %d read:%s",__FUNCTION__,errno,strerror(errno));
            break;
        }else if(rlen==0)
        {//断开连接的直接删除该事件
            //EpollDelEvent(fd);
            LOG::record(UTILLOGLEVEL1,"client diconnect %d",fd);
            return ;
        }
        break;
    }while(1);*/
    /*将fd传递给下层的协议处理函数进行读取处理，还是说在上层进行一定的校验 */
    chm_->UserReadProtocl(tfd); //进行协议解析处理
}

void epollserverhandle::DelEvent(int tfd)
{
    chm_->UserRemove(fd); //从队列中删除；
    ::close(tfd);
}

