
#include "channel.h"
#include "epollserverhandle.h"
#include "util.h"

/*要快速的处理 */
/*
epollserverhandle::epollserverhandle(int timeout=1500): //定时延长一个1.5s
listen_fd_(-1),
timeout_(timeout)
{
}
*/

epollserverhandle::~epollserverhandle()
{
}

int epollserverhandle::ServerStart(const char* listenip,const int port)
{
    listen_fd_=tcpGenericServer(SERVERLISTENIP,SERVERLISTENPORT);
    if(listen_fd_ < 0)
    {
        LOG::record(UTILLOGLEVELERROR, "ServerStart failed %s : %s",__FUNCTION__, strerror(errno));
        return -1;
    }
    try{
        evp_=std::make_shared<epollevent>(this,listen_fd_);
        chm_=std::make_shared<channel>();
        tme_=std::make_shared<timeevent>();
    }catch(...)
    {
        LOG::record(UTILLOGLEVELERROR, "ServerStart failed %s : %s",__FUNCTION__, strerror(errno));
        throw strerror(errno);
    }

    while(1)
    {
        evp_->EpollEventWaite();//程序主入口
        std::vector<int> t;
        int ret = tme_->TimeEventProc(t);
        for(auto i:t)
        {
            DelEvent(i);
        }
    }

    return 0;
}

void epollserverhandle::AcceptEvent(int listenfd)
{
    int tfd=0;
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
                LOG::record(UTILLOGLEVELERROR,"%s accept:%s \n",__FUNCTION__,strerror(errno));
                return; 
            }
        }
        break;
    }while(1);

    setNonBlock(tfd);
    struct epoll_event ee = {0,0};
    ee.events |=  EPOLLIN | EPOLLET |EPOLLRDHUP;
    ee.data.fd=tfd;

    if(evp_->EpollEventAdd(ee)==0)
    {
        /*添加定时事件，超时后将自动关闭该 端口*/
        //chm_->AddNewUser(tfd);
        tme_->TimeEventUpdate(timeout_,tfd);//注册时间事件，如果超时没有连接的话需要清楚断开该连接
    }
}

void epollserverhandle::ReadEvent(int tfd)
{
    /*将fd传递给下层的协议处理函数进行读取处理，还是说在上层进行一定的校验 */
    if(chm_->UserReadProtocl(tfd)>3)  //进行协议解析处理，考虑使用线程池进行处理，快速的进行切换,对于某些错误放在一个队列中进行集中处理
    { //如果协议被有效的解析后，可以设置心跳时间
        DelEvent(tfd);//如果解析处理后出错，直接断开删除该连接
       //tme_->TimeEventUpdate(timeout_,tfd);//更新注册时间，否则将进行断开，同时用于心跳检测！
    }
    //else{ //不合法的连接及时关闭连接
    //    
    //}
}

void epollserverhandle::DelEvent(int tfd)
{
    evp_->EpollDelEvent(tfd);
    chm_->UserRemove(tfd); //从通知channel从用户队列中删除；
    LOG::record(UTILLOGLEVELRECORD,"Del connection %d",tfd);
    ::close(tfd);
}

int epollserverhandle::WaitTimeOut()
{
    return timeout_;
}

void epollserverhandle::WriteEvent(int tfd)
{
    return ;
}