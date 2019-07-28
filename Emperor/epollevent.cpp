
#include "util.h"
#include "epollevent.h"

epollevent::epollevent(epollhandlebase *p,int listenfd):
listen_fd_(listenfd),
epollfd_(-1),
objhandle_(p)
{
    EpollInit();
}

epollevent::epollevent(epollhandlebase *p,int epollfd,int listenfd):
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
        if (epollfd_ < 0)
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
        LOG::record(UTILLOGLEVELERROR,"%s create events Failed\n",__FUNCTION__);
        return -1;
    }
    /*
    struct epoll_event ee = {0,0};
    ee.events |=  EPOLLIN | EPOLLET |EPOLLRDHUP;
    ee.data.fd=tfd;
    */
    //printf("epoll create %d\n",ee.data.fd);
    if(epoll_ctl(epollfd_,EPOLL_CTL_ADD,ee.data.fd,&ee)==-1)
    {
        LOG::record(UTILLOGLEVELERROR,"epoll create %d:%s\n",errno,strerror(errno));
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
    int numReady=epoll_wait(epollfd_,ee_,EPOLLMAXEVENTS,objhandle_->WaitTimeOut());
    //LOG::record(UTILLOGLEVELRECORD,"this epoll wait events numReady:%d\n",numReady);
    if(numReady<0)
    {
        LOG::record(UTILLOGLEVELERROR,"epoll_wait %d:%s\n",errno,strerror(errno));
        return numReady;
    }

    for(int i=0;i<numReady;i++)
    {
        if((ee_[i].data.fd==listen_fd_) && (ee_[i].events & EPOLLIN )) //添加事件
        {
            objhandle_->AcceptEvent(listen_fd_);
        }else if(ee_[i].events & EPOLLRDHUP )
        {
            objhandle_->DelEvent(ee_[i].data.fd); //客户端主动断开连接
        }else if(ee_[i].events & EPOLLIN )
        {
            objhandle_->ReadEvent(ee_[i].data.fd);
        }else if(ee_[i].events & EPOLLOUT)
        {
            objhandle_->WriteEvent(ee_[i].data.fd);
        }
        else{
            LOG::record(UTILLOGLEVELERROR,"epoll_wait fd:%d not known events %u\n",ee_[i].data.fd,ee_[i].events);
        }
    }
    return numReady;
}

int epollevent::EpollDelEvent(int fd)
{
    struct epoll_event ee ; /* avoid valgrind warning */
    std::memset(&ee,0,sizeof(struct epoll_event));
    ee.events = 0;
    ee.data.fd = fd;
    int ret=epoll_ctl(epollfd_,EPOLL_CTL_DEL,fd,&ee);
    if(ret<0)
    {
        LOG::record(UTILLOGLEVELERROR,"epoll_ctl %d:%s\n",errno,strerror(errno));
        return ret;
    }
    return ret;
}

void timeevent::TimeShow( struct timeval &tv)
{
    printf("======TimeShow==start=====\n");
    printf("usec:%ld\n",tv.tv_usec);
    printf("sec:%ld\n",tv.tv_sec);
    printf("======TimeShow==end=====\n");
}


int timeevent::TimeEventRemove(int tfd)
{
    auto it=fdtopos_.find(tfd);
    if(it==fdtopos_.end())
    {
        return 0;
    }
    lst_.erase(it->second);
    fdtopos_.erase(it);
    return 1;
}

int timeevent::TimeEventUpdateLRU(size_t milliseconds,int fd)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    //TimeShow(tv);

    tv.tv_usec += milliseconds * 1000;
    tv.tv_sec += tv.tv_usec/1000000;
    tv.tv_usec = tv.tv_usec %1000000;

    auto it=fdtopos_.find(fd);
    if(it!=fdtopos_.end())
    {
        lst_.erase(it->second);
    }

    lst_.push_front(std::make_pair(fd,tv));
    fdtopos_[fd]=lst_.begin();
    return 0;
}

/*一次只返回一个fd，进行处理 */
int timeevent::TimeEventProc()
{
    //std::map<int,list<pair<int,<struct timeval>>::iterator> fdtopos_;
    //std::list<pair<int,<struct timeval>> lst_;
    if(0==lst_.size())
    {
        if(fdtopos_.size()!=0)
        {
            GENERRORPRINT("not null!",fdtopos_.size(),lst_.size());
        }
        return -1;
    }

    struct timeval tv;

    auto it=lst_.back();

    gettimeofday(&tv, NULL);
    if(TimeExceedJudge(tv,it.second))
    {/*超时处理 */
        int fd=it.first;
        fdtopos_.erase(fd);
        lst_.pop_back();
        
        return fd;
    }
    return -1;
}



void timeevent::TimeEventAdd(int fd,size_t milisecond )
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct basetimer t;
    t.fd=fd;
    t.v=tv;
    pri_.push(t);
}

int timeevent::TimeEventProc(epollhandlebase *base)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    for(;;)
    {
        struct basetimer t=pri_.top();
        if(TimeExceedJudge(tv,t.v))
        {/*添加新的时间事件 */
            base->TimeoutEvent(t.fd,this);
            pri_.pop();
        }else{
            size_t dif=tv.tv_sec-t.v.tv_sec;
            if(tv.tv_usec-t.v.tv_usec)
            {
                dif++;
            }
            return dif;
        }
    }
    /*一直阻塞等待事件发生*/
    return -1;
}

int timeevent::CurrentFdEventNum()
{
    if(fdtopos_.size()!=lst_.size())
    {
        LogWarning("mismatch! %d %d",fdtopos_.size(),lst_.size());
    }
    return fdtopos_.size();
}


int timeevent::TimeExceedJudge(struct timeval &a,struct timeval &b)
{
    if(a.tv_sec > b.tv_sec)
    {
        return 1;
    }else if(a.tv_sec < b.tv_sec)
    {
        return 0;
    }else{
        if(a.tv_usec >= b.tv_usec)
        {
            return 0;
        }
    }
    return 1;
}

