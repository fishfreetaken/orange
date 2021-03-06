


#include <map>
#include <unordered_map>
#include <list>
#include <vector>
#include <utility>
#include <queue>
#define EPOLLMAXEVENTS 1024
#define READBUFFLENMAX 512

class timeevent;
class epollhandlebase
{
public:
    virtual void ReadEvent(int tfd)=0;
    virtual void WriteEvent(int tfd)=0;
    virtual void AcceptEvent(int tfd)=0;
    virtual void DelEvent(int tfd)=0;
    virtual void TimeoutEvent(int tfd,timeevent *t)=0;

    virtual int WaitTimeOut()
    {/*0立刻返回，-1一直阻塞 */
        return -1;
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
    int TimeEventUpdateLRU(size_t milliseconds,int fd);
    int TimeEventProc();

    int CurrentFdEventNum();
    int TimeEventRemove(int tfd);
    void TimeEventAdd(int fd,size_t milisecond );

    int TimeEventProc(epollhandlebase *base);

private :
   static  int TimeExceedJudge(struct timeval &a,struct timeval &b);
    void TimeShow(struct timeval &tv);
private :
    //std::map<int,struct timeval> ump_;
    /*以下是lru算法 */
    std::unordered_map<int,std::list<std::pair<int,struct timeval>>::iterator > fdtopos_;
    std::list<std::pair<int,struct timeval>>  lst_;

    struct basetimer {
        int fd;
        struct timeval v;
    };

    class basetimecmp{
    public:
        bool operator () (struct basetimer&a, struct basetimer&b)
        {
            if(timeevent::TimeExceedJudge(a.v,b.v))
            {
                return true;
            }
            return false;
        }
    };
    
    std::priority_queue <struct basetimer, std::vector<struct basetimer> , basetimecmp > pri_;
};

