

#include<sys/time.h>
#include <map>
#include <vector>
#define EPOLLMAXEVENTS 1024
#define READBUFFLENMAX 512

class epollhandlebase
{
public:
    virtual void ReadEvent(int tfd)=0;
    virtual void WriteEvent(int tfd)=0;
    virtual void AcceptEvent(int tfd)=0;
    virtual void DelEvent(int tfd)=0;

    virtual int WaitTimeOut()
    {
        return 1000;
    }

};

class epollevent{
public:
    epollevent(epollhandlebase *p,int listen_fd);
    epollevent(epollhandlebase *p,int epollfd,int listen_fd);
    ~epollevent();

    int EpollEventWaite();

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

    int TimeEventProc(std::vector<int> &m); //返回处理的fd个数，将过期的fd返回通知，并从事件map中进行取消注册，不再进行轮训

    int TimeEventUpdate(size_t milliseconds,int fd); //对于每个fd如果没有在map中的话，添加，如果在的话进行更新

private :
    int TimeExceedJudge(struct timeval &a,struct timeval &b);
    void TimeShow(struct timeval &tv);
private :
    std::map<int,struct timeval> ump_;

};

