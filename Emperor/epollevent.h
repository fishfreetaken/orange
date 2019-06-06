

#define EPOLLMAXEVENTS 1024
#define READBUFFLENMAX 512

class user;
class channel;

class epollevent{
public:
    epollevent(int listen_fd);
    epollevent(int listen_fd,int time_out);

    EpollEventWaite();

private:
    void EpollInit(int listen_fd,int time_out);

    //入参，accept到的fd
    void EpollCreateUserEvents(int tfd);

    void EpollReadCallback(int tfd);

    void EpollAcceptCallback(int fd);

    void EpollDelEvent(int fd);

private:
    int listen_fd_;
    int epollfd_;
    int timeout_; //epoll wait 等待时间，毫秒；

    char *readbuf_ ; //开辟一个固定大小的内存，用来缓存读取的数据

    struct epoll_event *ee_; //epoll wait event回调
};
