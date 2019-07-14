


#include "epollevent.h"
#include <memory>
class channel;
class  epollserverhandle:public epollhandlebase{
public:
    epollserverhandle(int timeout=500):
    listen_fd_(-1),
    timeout_(timeout)
    {
    }

    ~epollserverhandle();
    int ServerStart(const char* listenip,const int port);

public:
    void ReadEvent(int tfd);
    void WriteEvent(int tfd);
    void AcceptEvent(int tfd);
    void DelEvent(int tfd);

    int WaitTimeOut();

private:
    size_t timeout_;
    //int tevtimeout_;
    int listen_fd_;

    std::shared_ptr<epollevent> evp_;

    std::shared_ptr<channel> chm_;

    std::shared_ptr<timeevent> tme_;

    size_t timeeventout_;

};
