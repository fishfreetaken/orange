
#include "clientuser.h"
#include "util.h"


epollclienthandle::epollclienthandle(size_t uid):
fd_(0),
uid_(uid),
curdialog_(0),
send_pos_begin_(-1),
send_pos_end_(-1),
recpackagecount_(0),
sendpackagecount_(0)
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

int epollclienthandle::WaitTimeOut()
{
    return 5000;
}

int epollclienthandle::StartConnect(const char* listenip,int port)
{
    if(uid_==0)
    {
        LOG::record(UTILLOGLEVELERROR, "not valid uid");
        return UTILNET_ERROR; 
    }

    fd_=tcpGenericConnect(NULL,0,listenip,port);
    if(fd_<0)
    {
        //LOG::record(UTILLOGLEVELERROR, "tcpGenericConnect : %s", strerror(errno));
        return UTILNET_ERROR;
    }
    setNonBlock(fd_);
    LOG::record(UTILLOGLEVELWORNNING, "tcpGenericConnect to server uid: %zu", uid_);

    try
    {
        evp_=std::make_shared<epollevent>(this,fd_);
        tme_=std::make_shared<timeevent>();
        crypt_=std::make_shared<cryptmsg>();
    }catch (...){
        throw  strerror(errno);
    }

    /*加密协议消息 */
    transfcrptykey key;
    std::memset(&key,0,sizeof(key));

    crypt_->AESGenEnCryptKey(key.key,LOADAESCRPTYKEYLEN);
    snprintf(key.secret,LOADPERSONCRTPYLEN,"dadonggeSecret");
    FormMsgAddToBuffer(MSGSECRET,(char*)&key,sizeof(key));

    tme_->TimeEventUpdate(1500,fd_);

    /*标准输入输出 */
    setNonBlock(0);
    struct epoll_event ee = {0,0};
    ee.events |=  EPOLLIN | EPOLLET;
    ee.data.fd=0; //注册用户标准输入stdio 0事件；

    evp_->EpollEventAdd(ee);

    int ret=0;
    while(1)
    {
        UserSendMsgPoll(); //先将个人信息写入发送至服务端获取个人信息；

        ret=evp_->EpollEventWaite();//程序主入口，设置的轮训时间是1s，跳出来之后进行发送心跳包
        if(ret<0)
        {
            LOG::record(UTILLOGLEVELWORNNING, "EpollEventWaite LINE=%d error ret=%d\n",__LINE__,ret);
            break;
        }

        ret=tme_->TimeEventProc(timerollback_);
        if(ret>0)
        {
            tme_->TimeEventUpdate(1500,fd_);
            FormMsgAddToBuffer(MSGHEART,nullptr,0); //1s发送一个心跳包
        }
    }
}

/*从server端收到的数据 */
void epollclienthandle::AcceptEvent(int fd)
{
    int ret=0;

    std::memset(serverbuffer_,0,CLIENTREADBUFFERLEN);
    ret=readGenericReceive(fd_,serverbuffer_,STRUCTONPERLEN); 
    if(ret<STRUCTONPERLEN)
    {
        /*这里应该将buf内容进行二进制的输出到log上 */
        LOG::record(UTILLOGLEVELWORNNING, "readGenericReceive %s LINE:%d  ret:%d < %d",__FUNCTION__,__LINE__,ret,STRUCTONPERLEN);
        return;
    }
    #if 0
    ret=Decrypt() ; //使用密码进行解压；
    if(ret!=0)
    {
        LOG::record(UTILLOGLEVELERROR, "decrypt failed! disregard msg ");
    }
    #endif
    transfOnPer *p=(transfOnPer *)serverbuffer_;
    recpackagecount_++;

    printTransfOnPer(p, "epollclienthandle AcceptEvent");

    /*crc校验，这样校验不过应该需要从server端断开连接的 */
    if(verifyCrcPayload(*p)!=UTILNET_SUCCESS)
    {
        LOG::record(UTILLOGLEVELRECORD,"epollclienthandle::AcceptEvent crc query failed");
        return ;
    }
    
    switch(p->id)
    {
        case MSGHEART:
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
    transfOnPer * p;
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
        LOG::record(UTILLOGLEVELERROR, "zero dest uid, invalid! to:%zu",p->to);
        return ;
    }
    transfPartner *cc= (transfPartner *)p->buf;
    printfPartner(cc,"InintialMyInfo");
    if(cc->uid==uid_)
    {
        myinfo_.state=1;
        std::strcpy(myinfo_.name,cc->name);
        std::strcpy(myinfo_.name,cc->signature);
    }else{
        myfriends_[cc->uid]=*cc ;
    }

}

void epollclienthandle::AddNewFriends(transfOnPer *p)
{
    /*1 添加新的朋友；2 更新在线状态 */
    transfPartner *s=(transfPartner *)p->buf;
    printfPartner(s,"AddNewFriends");
    if(s->uid==0)
    {
        LOG::record(UTILLOGLEVELERROR, "zero dest uid, invalid!");
    }
    myfriends_[s->uid]= *s;

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
    if(ret<0)
    {
        return ret;
    }
    
    FormMsgAddToBuffer(MSGFRIEND,readbuffer_,ret);
}

int epollclienthandle::FormMsgAddToBuffer(uint32_t msgid,const char*buf,int len)
{
    /* 
    if((msgid==1)&&(curdialog_==0))
    {
        LOG::record(UTILLOGLEVELERROR, "FormMsgAddToBuffer msgid:1 and curdialog_ not initilized! cannot send");
        return UIIL_NOTFOUND;
    }*/
    transfOnPer *p= SendBufferPush();
    if(p==nullptr)
    {
        LOG::record(UTILLOGLEVELERROR, "FormMsgAddToBuffer SendBufferPush full");
        return UTIL_POINTER_NULL;
    }
    sendpackagecount_++;
    p->id=msgid;
    p->uid=uid_;
    p->to=curdialog_;
    p->size=len;
    if(buf!=nullptr)
    {
        memcpy(p->buf,buf,len);
    }
    
    genCrcPayload(*p);

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
