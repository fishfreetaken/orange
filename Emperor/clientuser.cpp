
#include "clientuser.h"
#include "util.h"
#include <signal.h>

/* */
epollclienthandle::epollclienthandle(uint32_t uid,char *password):
fd_(-1),
curdialog_(0),
myinfo_.uiId(uid),
myinfo_.sPassword.assign(password),
epolltimeout_(-1),
uiHeartInterval_(1000) //1s的间隔
{
}

epollclienthandle::~epollclienthandle()
{
    if(fd_>0)
    {
        ::close(fd_);
    }
}

int epollclienthandle::WaitTimeOut()
{
    return epolltimeout_;
}


void SingalCallBack(int signale_number)
{
    printf("encounter SIGPIPE t=%d\n",signale_number);
}

int epollclienthandle::StartConnect(const char* listenip,int port)
{

    fd_=tcpGenericConnect(NULL,0,listenip,port);
    if(fd_<0) {return Status::mIOError;}
    recpacket_.SetFd(fd_);

    try
    {
        evp_=std::make_shared<epollevent>(this,fd_);
        tme_=std::make_shared<timeevent>();
        aescrypt_=std::make_shared<Aescrypt>();
        sendqueue_=std::make_shared<SerialBuffer>();
        recpacket_ = std::make_shared<Protocal>(fd_);
    }catch (...){
        throw  "clientuser new failed";
    }

    /*加密协议消息 */
    /*直接使用用户名密码进行替换，生成256字节的对称加密秘钥；sha(sha(随机串))+sha(id)+sha(密码)+sha(id+密码)+shasha(id)+shasha(密码)+shasha(id+密码)+sha(随即串) */

    /*生成完直接拷贝到这个结构体中，设置myinf_的字符串*/
    if(aescrypt_->AesKeyGen(myinfo_)!=Statue::mOk)
    {
        LogError("AesKeyGen generator failed %d",ret);
    }

    sendqueue_.push(Protoal(fd_,Protocaltype::mSecret,myinfo_));

    tme_->TimeEventAdd(fd_,uiHeartInterval_);/*server 心跳事件 */

    /*标准输入输出 */
    //setNonBlock(0);
    struct epoll_event ee = {0,0};
    ee.events |=  EPOLLIN | EPOLLET;
    ee.data.fd=0; //注册用户标准输入stdio 0事件；
    evp_->EpollEventAdd(ee);

    //signal(SIGPIPE,SingalCallBack);
    /*当主控断开后，会由于SIGPIPE信号导致进程中断，这个也是断线重连机制的基础 */
    signal(SIGPIPE,SIG_IGN);

    while(1)
    {
        //程序主入口，按照redis的方法，对于下次的轮训时间设置为最近快要过期的时间
        if(evp_->EpollEventWaite()<0)
        {
            LogError("EpollEventWaite error %s\n",strerror(errno));
        }
        /*执行定时事件 */
        epolltimeout_=tme_->TimeEventProc(this);

        /*发送消息 */
        sendqueue_.pop();
    }

    return Status::mOk;
}

/*从server端收到的数据 */
void epollclienthandle::AcceptEvent(int fd)
{
    int ret=0;

    while(recpacket_.ReceivePkg(aescrypt_)==Status::mOk)/*一次并不能读完 */
    {
        switch(p->id)
        {
            case MSGHEART:
                //printf("Server reback num %zu\n",*(p->buf));
                break;
            case MSGFRIEND:
                PushMsgToTerminal(p);
                break;
            case MSGSERVERINFO:
                InintialMyInfo(p);
                break;
            case MSGFRIENDINFO:
                AddNewFriends(p);
                break;
            default:
                LOG::record(UTILLOGLEVELWORNNING, "not recoginsed tid: %d", p->id);
                return;
                break;
        }
    }
}

void epollclienthandle::TimeoutEvent(int tfd, timeevent *t);
{
    /*服务器定时心跳事件 */
    if(tfd==fd_)
    {
        sendqueue_.push(Protoal(fd_,Protocaltype::mHeart,myinfo_));
        tme_->TimeEventAdd(fd_,uiHeartInterval_);/*server 心跳事件 */
    }
}

void epollclienthandle::DelEvent(int tfd)
{
    evp_->EpollDelEvent(tfd);
    ::close(tfd);
}

void epollclienthandle::WriteEvent(int tfd)
{
    if(tfd==fd_)
    {
        UserSendMsgPoll();
    }
    //LOG::record(UTILLOGLEVELERROR, "reservered %d", tfd);
    return ;
}

/*标准设备读入事件 */
void epollclienthandle::ReadEvent(int tfd)
{
    int ret;
    std::memset(readbuffer_,0,CLIENTWRITEBUFFERLEN);
    ret=readGenericReceive(0,readbuffer_,CLIENTREADBUFFERLEN);
    if(ret<0)
    {
        return ret;
    }
    auto it=myfriends_.begin();
    if(it==myfriends_.end())
    {
        GENERRORPRINT("I have no friends",0,0);
        return UTILNET_ERROR;
    }
    
    curdialog_=it->second.uid;
}

int epollclienthandle::UserSendMsgPoll()
{
    int ret=0;
    transfOnPer * p;
    while(SendBufferPop()==0){}
    /*表示对端主动断开连接 */
    if(errno==EPIPE)
    {
        errno=0;
        DelEvent(fd_);
        LogRecord("EPIPE error fd: %d, %s\n",fd_,sterrno(errno));
        return errno;
    }
    /*网线断了，或者buf满了 */
    #if 0
    if(errno==EWOULDBLOCK)
    {
        GENERRORPRINT("errno EWOULDBLOCK",errno,sendfailedcount_);
        if(sendfailedcount_>300)
        {/*检测如果断线了话，就进行重新连接 */
            sendfailedcount_=0;
            return errno;
        }else{
            sendfailedcount_++;
        }
    }
    #endif
    return UTILNET_SUCCESS;
}

void epollclienthandle::InintialMyInfo()
{
    transfPartner *cc= (transfPartner *)p->buf;
    //printfPartner(cc,"InintialMyInfo");
    if(cc->uid==uid_)
    {
        myinfo_.state=1;
        std::strcpy(myinfo_.name,cc->name);
        std::strcpy(myinfo_.signature,cc->signature);
    }else{
        myfriends_[cc->uid]=*cc ;
    }

    //printf("name:%s signature:%s friends:%lu\n",myinfo_.name,myinfo_.signature,myfriends_.size());
}

void epollclienthandle::AddNewFriends(transfOnPer *p)
{
    /*1 添加新的朋友；2 更新在线状态 */
    transfPartner *s=(transfPartner *)p->buf;
    printfPartner(s,"AddNewFriends");
    if(s->uid==0)
    {
        LOG::record(UTILLOGLEVELERROR, "AddNewFriends, invalid!");
    }
    myfriends_[s->uid]= *s;

    return;
}

void epollclienthandle::PushMsgToTerminal(transfOnPer *p)
{

    if(myfriends_.find(p->uid)==myfriends_.end())
    {
        LOG::record(UTILLOGLEVELERROR, "not my friend %zu",p->uid);
    }
    

    return PROTOCALSUCCESS;
}


int epollclienthandle::FormMsgAddToBuffer(uint32_t msgid,const char*buf,int len,size_t to)
{
    /*应该检测一下msgid是否有效 */
    if(len>LOADCHARLEN)
    {
        GENERRORPRINT("len not more",len,LOADCHARLEN);
        return UTILNET_ERROR;
    }
    transfOnPer *out= SendBufferPush();

    if(out==nullptr)
    {
        LOG::record(UTILLOGLEVELERROR, "FormMsgAddToBuffer SendBufferPush full");
        return UTIL_POINTER_NULL;
    }
    std::memset(out,0,STRUCTONPERLEN);
    sendpackagecount_++;
    out->id=msgid;
    out->uid=uid_;
    out->to=to;
    out->size=len;

    if(buf!=nullptr)
    {
        std::memcpy(out->buf,buf,len);
    }

    genCrcPayload(*out);

    return UTILNET_SUCCESS;
}
