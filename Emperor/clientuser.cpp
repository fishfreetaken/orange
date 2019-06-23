
#include "clientuser.h"
#include "util.h"


epollclienthandle::epollclienthandle(size_t uid):
fd_(0),
uid_(uid),
curdialog_(0),
send_pos_begin_(-1),
send_pos_end_(-1)
{
    try{
        readbuffer_= new char[CLIENTREADBUFFERLEN];
        serverbuffer_ = new char[CLIENTWRITEBUFFERLEN];
    }catch(...)
    {
        throw strerror(errno);
    }
    sendbuffer_.resize(VECTORBUFFERLEN);
    timerollback_.resize(VECTORBUFFERLEN);
    //recvbuffer_.resize(VECTORBUFFERLEN);
}

epollclienthandle::~epollclienthandle()
{
    delete [] readbuffer_;
    delete [] serverbuffer_;

    if(fd_>0)
    {
        ::close(fd_);
    }
}

int epollclienthandle::StartConnect(const char* listenip,int port)
{
    if(uid_==0)
    {
        LOG::record(UTILLOGLEVELERROR, "not valid uid zero");
        return UTILNET_ERROR; 
    }
    
    fd_=tcpGenericConnect(NULL,0,listenip,port);
    if(fd_<0)
    {
        LOG::record(UTILLOGLEVELERROR, "tcpGenericConnect : %s %d", strerror(errno),fd_);
        return UTILNET_ERROR;
    }

    LOG::record(UTILLOGLEVELWORNNING,"tcpGenericConnect server fd: %d", fd_);
    try
    {
        evp_=std::make_shared<epollevent>(this,fd_);
        tme_=std::make_shared<timeevent>();
    }catch (...){
        throw  strerror(errno);
    }
    tme_->TimeEventUpdate(1500,fd_);

    /*标准输入输出 */
    setNonBlock(0);
    struct epoll_event ee = {0,0};
    ee.events |=  EPOLLIN | EPOLLET ;
    ee.data.fd=0; //注册用户标准输入stdio 0事件；

    evp_->EpollEventAdd(ee);

    int ret=0;
    while(1)
    {
        UserSendMsgPoll(); //先将个人信息写入发送至服务端获取个人信息；
        ret=evp_->EpollEventWaite();//程序主入口，设置的轮训时间是1s，跳出来之后进行发送心跳包
        
        ret=tme_->TimeEventProc(timerollback_);
        if(ret>0)
        {
            FormMsgAddToBuffer(MSGHEART,nullptr,0); //1s发送一个心跳包
        }
    }
}

/*与server端建立连接，通信！ */


/*从server端收到的数据 */
void epollclienthandle::AcceptEvent(int fd)
{
    int ret=0;
    while(1)
    {
        std::memset(serverbuffer_,0,CLIENTREADBUFFERLEN);
        ret=readGenericReceive(fd_,serverbuffer_,STRUCTONPERLEN);
        if(ret<=0)
        {
            break;
        }
        #if 0
        ret=Decrypt() ; //使用密码进行解压；
        if(ret!=0)
        {
            LOG::record(UTILLOGLEVELERROR, "decrypt failed! disregard msg ");
        }
        #endif
        transfOnPer *p=(transfOnPer *)serverbuffer_;
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
                LOG::record(UTILLOGLEVELWORNNING, "not recoginsed tid: %d", p->id);
                return;
                break;
        }
    }
    return ;
}

void epollclienthandle::DelEvent(int tfd)
{
    evp_->EpollDelEvent(tfd);
    ::close(tfd);
    LOG::record(UTILLOGLEVELRECORD,"DelEvent close fd: %d\n",tfd);
}

void epollclienthandle::WriteEvent(int tfd)
{
    if(tfd==fd_)
    {
        UserSendMsgPoll();
    }
    LOG::record(UTILLOGLEVELERROR, "reservered %d", tfd);
    return ;
}

void epollclienthandle::ReadEvent(int tfd)
{
    switch(tfd)
    {
        case 0: //从标准输入设备中读入数据；
            MsgSendFromStd0();
            break;
        default:
            LOG::record(UTILLOGLEVELERROR, "not recognisied fd %d", tfd);
            return;
            break;
    }
}

void epollclienthandle::UserSendMsgPoll()
{
    int ret=0;
    transfOnPer * p=nullptr;
    while((p=SendBufferPop())!=nullptr)
    {
        ret=writeGenericSend(fd_,(char*)p, STRUCTONPERLEN);
        if(ret<=0)
        {
            break;
        }
    }
    return ;
}

void epollclienthandle::InintialMyInfo(transfOnPer *p)
{
    if(p->to!=uid_)
    {
        LOG::record(UTILLOGLEVELERROR, "zero dest uid, invalid!");
        return ;
    }
    myinfo_.state=true;
    transfPartner *cc= (transfPartner *)p->buf;
    myinfo_.name.assign(cc->name);
    myinfo_.name.assign(cc->signature);
    /*初始化一个本地写入的fd */
}

void epollclienthandle::AddNewFriends(transfOnPer *p)
{
    /*1 添加新的朋友；2 更新在线状态 */
    transfPartner *s=(transfPartner *)p->buf;
    if(s->uid==0)
    {
        LOG::record(UTILLOGLEVELERROR, "zero dest uid, invalid!");
    }
    friends t;
    t.uid=s->uid;
    t.state=s->state;
    t.name.assign(s->name);
    t.signature.assign(s->signature);
    myfriends_[s->uid] = t;
    return;
}


int epollclienthandle::PushMsgToTerminal(transfOnPer *p)
{
    if((p->to!=uid_)&&(uid_!=0))
    {
        LOG::record(UTILLOGLEVELERROR, "PushMsgToTerminal uid mismatch %zu %zu",uid_,p->uid);
        return PROTOCALUIDMATCH;
    }
    
    if(myfriends_.find(p->uid)==myfriends_.end())
    {
        LOG::record(UTILLOGLEVELERROR, "not my friend %zu",p->uid);
        return PROTOCALNOTFOUND;
    }

    if(p->uid==curdialog_)
    {
        //LOG::record(UTILLOGLEVELERROR, "PushMsgToTerminal uid mismatch %zu %zu",uid_,p->uid);
        LOG::record(UTILLOGLEVELDIALOG,"%zU to %zU say:\n%s\n\n",p->uid,p->to,p->buf);
    }

    //writetofile(p->uid);
    return PROTOCALSUCCESS;
}

/*从终端输入的信息发送至服务端 */
int epollclienthandle::MsgSendFromStd0()
{
    int ret;
    std::memset(readbuffer_,0,CLIENTWRITEBUFFERLEN);
    ret=readGenericReceive(0,readbuffer_,CLIENTREADBUFFERLEN);
    if(ret<=0)
    {
        LOG::record(UTILLOGLEVELDIALOG,"read from 0 ret:%d",ret);
        return ret;
    }
     LOG::record(UTILLOGLEVELDIALOG,"read from 0 msg:%s",readbuffer_);
    FormMsgAddToBuffer(MSGFRIEND,readbuffer_,ret);
}

int epollclienthandle::FormMsgAddToBuffer(uint32_t msgid,const char*buf,int len)
{
    if((msgid==1)&&(curdialog_==0))
    {
        LOG::record(UTILLOGLEVELERROR, "FormMsgAddToBuffer msgid:1 and curdialog_ not initilized! cannot send");
        return UIIL_NOTFOUND;
    }
    LOG::record(UTILLOGLEVELDIALOG, "time out and send a heart  msgid:%d  buf:%s",msgid, buf);
    transfOnPer *p= SendBufferPush();
    if(p==nullptr)
    {
        LOG::record(UTILLOGLEVELERROR, "FormMsgAddToBuffer SendBufferPush full");
        return UTIL_POINTER_NULL;
    }
    p->id=msgid;
    p->uid=uid_;
    p->to=curdialog_;
    p->size=len;
    if(buf!=nullptr)
    {
        memcpy(p->buf,buf,len);
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

    transfOnPer * p= &sendbuffer_[send_pos_begin_];

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

    transfOnPer * p= &sendbuffer_[send_pos_end_];

    return p;
}

void epollclienthandle::CalculateCrc(transfOnPer *p)
{
    if(p==nullptr)
    {
        LOG::record(UTILLOGLEVELERROR,"CalculateCrc p is nullptr\n");
        return ;
    }
    char *s=p->crc32;
    if(s==nullptr)
    {   
        LOG::record(UTILLOGLEVELERROR,"CalculateCrc s is nullptr\n");
    }
    //std::memcpy(s,0,4);
}
