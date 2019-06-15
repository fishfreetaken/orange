
#include "util.h"
#include "log.h"
#include "channel.h"

int epollhandlebase::WaitTimeOut()
{
    //默认的轮训时间1.5s
    return -1;
}

epollevent::epollevent(epollhandlebase *p,int listenfd=-1):
listen_fd_(listenfd),
epollfd_(-1),
objhandle_(p)
{
    EpollInit();
}

epollevent::epollevent(epollhandlebase *p,int epollfd,int listenfd=-1):
listen_fd_(listenfd),
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
        ee.data.fd=listen_fd_;
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
int epollevent::EpollEventWaite()
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
            objhandle_->DelEvent(ee_[i].data.fd); //客户端主动断开连接
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




int timeevent::TimeEventUpdate(size_t milliseconds,int fd)
{
    /* 不管fd是否已经存在，都进行更新/添加新的时间事件*/

    struct timeval tv;

    gettimeofday(&tv, NULL);
    
    tv.tv_usec += milliseconds * 1000;
    tv.tv_sec += (milliseconds / 1000) + tv.tv_usec/1000000;
    tv.tv_usec = tv.tv_usec %1000000;
    ump_[fd]= tv;
    return 0;
}

int timeevent::TimeEventUpdate(size_t milliseconds,int fd,void* handle)
{
    /* 不管fd是否已经存在，都进行更新/添加新的时间事件*/
    struct timeval tv;

    gettimeofday(&tv, NULL);
    
    tv.tv_usec += milliseconds * 1000;
    tv.tv_sec += (milliseconds / 1000) + tv.tv_usec/1000000;
    tv.tv_usec = tv.tv_usec %1000000;
    ump_[fd]= tv;
    return 0;
}

std::vector<int> timeevent::TimeEventProc()
{
    std::vector<int> m;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    for(auto it=ump_.begin();it!=ump_.end();)
    {
        if(TimeExceedJudge(tv,it->second))
        {
            m.push_back(it->first);
            it= ump_.erase(it); //从队列中删除
        }else{
            it++;
        }
    }
    return m;
}

int timeevent::TimeExceedJudge(struct timeval &a,struct timeval &b)
{
    if(a.tv_sec > b.tv_sec)
    {
        return 0;
    }else if(a.tv_sec < b.tv_sec)
    {
        return 1;
    }else{
        if(a.tv_usec <= b.tv.usec)
        {
            return 1;
        }
    }
    return 0;
}


