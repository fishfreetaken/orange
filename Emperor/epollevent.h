

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

class timeevent {
public:
    //int TimeEventUpdate(size_t milliseconds,int fd); //设置每个fd的过期时间
    std::vector<int> TimeEventProc(); //返回处理的fd个数，将过期的fd返回通知，并从事件map中进行取消注册，不再进行轮训
    int TimeEventUpdate(size_t milliseconds,int fd); //对于每个fd如果没有在map中的话，添加，如果在的话进行更新

private :
    int TimeExceedJudge(struct timeval &a,struct timeval &b);
private :
    std::map<int,struct timeval> ump_;
};

class  epollserverhandle:public epollhandlebase{
public:
    epollserverhandle(int timeout=1500);

    ~epollserverhandle();
    void ServerStart(const char* listenip,const int port);

    void ReadEvent(int tfd);
    //void WriteEvent(int tfd);
    void AcceptEvent(int tfd);
    void DelEvent(int tfd);

    int GetListenFd();

private:
    size_t timeout_;
    //int tevtimeout_;
    int listen_fd_;

    epollevent* evp_;

    channel *chm_;

    timeevent *tme_;
};
