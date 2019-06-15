

#define EPOLLMAXEVENTS 1024
#define READBUFFLENMAX 512

class channel;
class epollhandlebase
{
public:
    virtual void ReadEvent(int tfd);
    virtual void WriteEvent(int tfd);
    virtual void AcceptEvent(int tfd);
    virtual void DelEvent(int tfd);

};

class epollevent{
public:
    epollevent(epollhandlebase *p,int listen_fd=-1);
    epollevent(epollhandlebase *p,int epollfd,int listen_fd=-1);

    int EpollEventWait();

    int EpollEventAdd(struct epoll_event &ee);

    int EpollDelEvent(int fd);

private:
    void EpollInit();

private:
    int listen_fd_;
    int epollfd_;

    struct epoll_event *ee_; //epoll wait event回调

    epollhandlebase *objhandle_;
};

class timeeventbase{
public:
    virtual int TimeOutHandle();
}

class timeevent {
    typedef  struct timeeventstruct{
        struct timeval tv;
        timeeventbase * handle;
    } tmestru;
public:
    //int TimeEventUpdate(size_t milliseconds,int fd); //设置每个fd的过期时间
    std::vector<int> TimeEventProc(); //返回处理的fd个数，将过期的fd返回通知，并从事件map中进行取消注册，不再进行轮训
    int TimeEventUpdate(size_t milliseconds,int fd); //对于每个fd如果没有在map中的话，添加，如果在的话进行更新

    int TimeEventUpdate(size_t milliseconds,int fd,timeeventbase* handle); //设置定时回调函数

private :
    int TimeExceedJudge(struct timeval &a,struct timeval &b);
private :
    std::map<int,tmestru> ump_;
};

