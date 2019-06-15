
#include "clientuser.h"
#include "util.h"

epollclienthandle::epollclienthandle(size_t uid):
fd_(0),
uid_(uid),
evp_(nullptr),
curdialog_(0),
send_pos_begin_(-1),
send_pos_begin_(-1)
{
    try:{
        readbuffer_= new char[CLIENTREADBUFFERLEN];
        serverbuffer_ = new char[CLIENTWRITEBUFFERLEN];
    }catch(...)
    {
        throw strerror(errno);
    }
    sendbuffer_.resize(VECTORBUFFERLEN);
    recvbuffer_.resize(VECTORBUFFERLEN);
}

epollclienthandle::~epollclienthandle()
{
    delete [] readbuffer_;
    delete [] serverbuffer_;
}

int epollclienthandle::StartConnect(const char* listenip,int port)
{
    if(uid==0)
    {
        LOG::record(UTILLOGLEVEL1, "not valid uid");
        return UTILNET_ERROR; 
    }

    fd_=tcpGenericConnect(NULL,0,listenip,port);
    if(fd_!=0)
    {
        LOG::record(UTILLOGLEVEL1, "tcpGenericConnect : %s", strerror(errno));
        return UTILNET_ERROR;
    }

    try:
    {
        evp_=new epollevent(this,listen_fd_);
        tme_=new timeevent();
    }catch (...){
        throw  strerror(errno);
    }
    tme_.TimeEventUpdate(1000,fd_,this);

    /*标准输入输出 */
    setNonBlock(0);
    struct epoll_event ee = {0,0};
    ee.events |=  EPOLLIN | EPOLLET | EPOLLOUT;
    ee.data.fd=0; //注册用户标准输入stdio 0事件；

    evp_->EpollEventAdd(ee)

    while(1)
    {
        //UserSendMsgPoll(); //先将个人信息写入发送至服务端获取个人信息；
        evp_->EpollEventWaite();//程序主入口
        tme_->TimeEventProc();
    }
}

int epollclienthandle::TimeOutHandle()
{
    /*服务器的定时包 */
    FormMsgAddToBuffer(MSGHEART,nullptr,0);
    tme_.TimeEventUpdate(1000,fd_,this); //继续更新
}

/*从server端收到的数据 */
int epollclienthandle::AcceptEvent()
{
    int ret=0;
    while(1)
    {
        memeset(serverbuffer_,0,CLIENTREADBUFFERLEN);
        ret=readGenericReceive(fd_,serverbuffer_,STRUCTONPERLEN);
        if(ret<=0)
        {
            break;
        }
        ParsePacket();
    }
    return ret;
}

void epollclienthandle::DelEvent(int tfd)
{
    evp_->EpollDelEvent(tfd);
    ::close(tfd);
}

int epollclienthandle::WriteEvent(int tfd)
{
    if(tfd==fd_)
    {
        UserSendMsgPoll();
    }
    LOG::record(UTILLOGLEVEL1, "reservered %d", tfd);
    return 0;
}

void epollclienthandle::ReadEvent(int tfd)
{
    switch(tfd)
    {
        case 0: //从标准输入设备中读入数据；
            MsgSendFromStd0();
            break;
        default:
            LOG::record(UTILLOGLEVEL1, "not recognisied fd %d", tfd);
            return;
            break;
    }
}

void epollclienthandle::UserSendMsgPoll()
{
    int ret=0;
    transfOnPer * p;
    while((p=SendBufferPop())!=nullptr)
    {
        ret=writeGenericSend(fd_,(char*)p, STRUCTONPERLEN);
        if(ret<=0)
        {
            break;
        }
    }
    return ret;
}

void epollclienthandle::ParsePacket()
{
    int ret;
    #if 0
    ret=Decrypt() ; //使用密码进行解压；
    if(ret!=0)
    {
        LOG::record(UTILLOGLEVEL1, "decrypt failed! disregard msg "");
    }
    #endif
    transfOnPer *p=serverbuffer_;
    switch(p->id)
    {
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
            LOG::record(UTILLOGLEVEL1, "not recoginsed tid: %d", p->id);
            return;
            break;
    }
    return ;
}

void epollclienthandle::InintialMyInfo(transfOnPer *p)
{
    if(p->to!=uid_)
    {
        LOG::record(UTILLOGLEVEL1, "zero dest uid, invalid!");
        return ;
    }
    myinfo_.online=true;
    transfPartner *p= (transfPartner *)p->buf;
    myinfo_.name.assign(p->name);
    myinfo_.name.assign(p->signature);
    /*初始化一个本地写入的fd */
}

void epollclienthandle::AddNewFriends(transfOnPer *p)
{
    /*1 添加新的朋友；2 更新在线状态 */
    transfPartner *s=(transfPartner *)p->buf;
    if(s->uid==0)
    {
        LOG::record(UTILLOGLEVEL1, "zero dest uid, invalid!");
    }
    friends t;
    t->uid=s->uid;
    t->state=s->state;
    t->name.assign(s->name);
    t->signature.assign(s->signature);
    myfriends_[s->uid]=t;
}


int epollclienthandle::PushMsgToTerminal(transfOnPer *p)
{
    if((p->to!=uid_)&&(uid_!=0))
    {
        LOG::record(UTILLOGLEVEL1, "PushMsgToTerminal uid mismatch %zu %zu",uid_,p->uid);
        return PROTOCALUIDMATCH;
    }
    
    if(myfriends_.find(p->uid)==myfriends_.end())
    {
        LOG::record(UTILLOGLEVEL1, "not my friend %zu",p->uid);
        return PROTOCALNOTFOUND;
    }

    if(p->uid==curdialog_)
    {
        //LOG::record(UTILLOGLEVEL1, "PushMsgToTerminal uid mismatch %zu %zu",uid_,p->uid);
        printf("%uz to %uz say:\n%s\n\n",p->uid,p->to,p->buf);
    }

    //writetofile(p->uid);
    return PROTOCALSUCCESS;
}

/*从终端输入的信息发送至服务端 */
int epollclienthandle::MsgSendFromStd0()
{
    memeset(readbuffer_,0,CLIENTWRITEBUFFERLEN);
    ret=readGenericReceive(fd_,readbuffer_,CLIENTREADBUFFERLEN);
    if(ret<0)
    {
        return ret;
    }
    FormMsgAddToBuffer(MSGFRIEND,readbuffer_,ret);
}

int epollclienthandle::FormMsgAddToBuffer(int msgid,const char*buf,int len)
{
    if((msgid==1)&&(curdialog_==0))
    {
        LOG::record(UTILLOGLEVEL1, "FormMsgAddToBuffer msgid:1 and curdialog_ not initilized! cannot send");
        return UIIL_NOTFOUND;
    }
    transfOnPer *p= SendBufferPush();
    if(p==nullptr)
    {
        LOG::record(UTILLOGLEVEL1, "FormMsgAddToBuffer SendBufferPush full");
        return UTILPOINTER_NULL;
    }
    p->id=msgid;
    p->uid=uid_;
    p->to=curdialog_;
    p->size=ret;
    if(buf!=nullptr)
    {
        memcpy(p->buf,buf,ret);
    }
    
    CalculateCrc(p);
    return UTILNET_SUCCESS;
}

transfOnPer * epollclienthandle::SendBufferPush()
{
    int len=sendbuffer_.size();
    send_pos_begin_++;
    if(send_pos_begin_==len)
    {
        send_pos_begin_=0;
    }
    if(send_pos_begin_==send_pos_end_) //buffer满了
    {
        return nullptr;
    }

    transfOnPer * p= sendbuffer_[send_pos_begin_];

    return p;
}

transfOnPer * epollclienthandle::SendBufferPop()
{
    int len=sendbuffer_.size();

    if(send_pos_begin_==send_pos_end_) //buffer空的
    {
        return nullptr;
    }
    send_pos_end_++;
    if(send_pos_end_==len)
    {
        send_pos_end_=0;
    }

    transfOnPer * p= sendbuffer_[send_pos_end_];

    return p;
}

void epollclienthandle::CalculateCrc(transfOnPer *p)
{
    memecpy(p->crc32,0,4);
}
