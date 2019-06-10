

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

    /*epoll wait 等待轮训的时间 毫秒 */
    virtual int GetWaitTimeOut();
    virtual int GetListenFd();

    virtual int EventsWait();
};

class epollevent{
public:
    epollevent(epollhandlebase *p);
    epollevent(epollhandlebase *p,int epollfd);

    int EpollEventWait();

    int EpollEventAdd(struct epoll_event &ee);

    int EpollEventDel(int fd);

private:
    void EpollInit();

private:
    int listen_fd_;
    int epollfd_;

    struct epoll_event *ee_; //epoll wait event回调

    epollhandlebase *objhandle_;
};


class  epollserverhandle:public epollhandlebase{
public:
    epollserverhandle(int timeout=1500);

    ~epollserverhandle();
    epollevent* ServerStart(const char* listenip,const int port);

    void ReadEvent(int tfd);
    //void WriteEvent(int tfd);
    void AcceptEvent(int tfd);
    void DelEvent(int tfd);

    int GetListenFd();

    int EventsWait();

private:
    int timeout_;
    int listen_fd_;

    epollevent* evp_;

    channel *chm_;
};
