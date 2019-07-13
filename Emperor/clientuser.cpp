
#include "clientuser.h"
#include "util.h"
#include <signal.h>


epollclienthandle::epollclienthandle(size_t uid):
fd_(0),
uid_(uid),
curdialog_(0),
send_pos_begin_(-1),
send_pos_end_(-1),
recpackagecount_(0),
sendpackagecount_(0),
sendfailedcount_(0),
heart_count_(0)
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

void SingalCallBack(int signale_number)
{
    printf("encounter SIGPIPE t=%d\n",signale_number);
}

int epollclienthandle::StartConnect(const char* listenip,int port)
{
    if(uid_==0)
    {
        LOG::record(UTILLOGLEVELERROR, "not valid uid");
        return UTILNET_ERROR; 
    }
    errno=0;

    fd_=tcpGenericConnect(NULL,0,listenip,port);
    if(fd_<0)
    {
        sendfailedcount_++;
        LOG::record(UTILLOGLEVELERROR, "tcpGenericConnect %d: %s",errno,strerror(errno));
        return sendfailedcount_;
    }
    sendfailedcount_=0;

    setNonBlock(fd_);
    LOG::record(UTILLOGLEVELWORNNING, "tcpGenericConnect server fd: %d uid:%zu", fd_,uid_);

    try
    {
        evp_=std::make_shared<epollevent>(this,fd_);
        tme_=std::make_shared<timeevent>();
        crypt_=std::make_shared<cryptmsg>("./rsapub.pem");
    }catch (...){
        throw  strerror(errno);
    }

    /*加密协议消息 */
    transfOnPer st;
    std::memset(&st,0,STRUCTONPERLEN);
    transfcrptykey *key=(transfcrptykey*)st.buf;
    std::memset(key,0,sizeof(transfcrptykey));

    crypt_->AESGenEnCryptKey((unsigned char*)key->key,0);

    snprintf(key->secret,LOADPERSONCRTPYLEN,"dadonggeSecret");/*这个地方后边要用md5 */
    st.id=MSGSECRET;
    st.uid=uid_;

    genCrcPayload(st);

    //printTransfOnPer(&st,"StartConnect");

    int ret=crypt_->RSAEncrypt((unsigned char*)&st,STRUCTONPERLEN,(unsigned char*)serverbuffer_,CLIENTWRITEBUFFERLEN);
    if(ret<RSAENCRYPTBUFLEN)
    {
        GENERRORPRINT("RSAEncrypt failed!",ret,RSAENCRYPTBUFLEN);
        return UTILNET_ERROR;
    }

    ret=writeGenericSend(fd_,serverbuffer_,ret);
    if(ret<=0)
    {
        GENERRORPRINT("writeGenericSend",ret,0);
        return UTILNET_ERROR;
    }

    tme_->TimeEventUpdate(1500,fd_);

    /*标准输入输出 */
    setNonBlock(0);
    struct epoll_event ee = {0,0};
    ee.events |=  EPOLLIN | EPOLLET;
    ee.data.fd=0; //注册用户标准输入stdio 0事件；

    evp_->EpollEventAdd(ee);

    //signal(SIGPIPE,SingalCallBack);
    /*当主控断开后，会由于SIGPIPE信号导致进程中断，这个也是断线重连机制的基础 */
    signal(SIGPIPE,SIG_IGN);

    ret=0;
    while(1)
    {
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
            //向server发送
            heart_count_++;
            printf("server send heart :%zu\n",heart_count_);
            FormMsgAddToBuffer(MSGHEART,(char*)&heart_count_,sizeof(size_t),0);//1s发送一个心跳包
        }

        //先将个人信息写入发送至服务端获取个人信息；
        if(UserSendMsgPoll()!=UTILNET_SUCCESS)
        {/*发送数据失败，断开连接 */
            return sendfailedcount_;
        }
    }
}

/*从server端收到的数据 */
void epollclienthandle::AcceptEvent(int fd)
{
    int ret=0;

while(1)/*一次并不能读完 */
{
    std::memset(serverbuffer_,0,CLIENTREADBUFFERLEN);
    std::memset(&recvpacketbuf_,0,STRUCTONPERLEN);
    ret=readGenericReceive(fd_,serverbuffer_,STRUCTONPERLEN);
    /*ret是int类型带符号，STRUCTONPERLEN是无符号类型，当ret<0时候，ret会大于STRUCTONPERLEN，这样比较会有问题 */
    if((ret< STRUCTONPERLEN )||(ret<0))
    {
        /*ret=0表示对端已经断开 */
        /*这里应该将buf内容进行二进制的输出到log上 */
        //LOG::record(UTILLOGLEVELWORNNING, "readGenericReceive %s LINE:%d  ret:%d errno=%d", strerror(errno),__LINE__,ret,errno);
        return;
    }

    transfOnPer *p=&recvpacketbuf_;
    #ifdef NEEDAESCRYPT
        ret=crypt_->AESDecrypt((unsigned char*)serverbuffer_,ret,(unsigned char*)p,STRUCTONPERLEN);
        if(ret!=0)
        {
            GENERRORPRINT("AESDecrypt failed",ret,0);
            return ;
        }
    #else 
        std::memcpy(&recvpacketbuf_,serverbuffer_,STRUCTONPERLEN);
    #endif

    recpackagecount_++;

    ShowPacketInfo();

    /*crc校验，这样校验不过应该需要从server端断开连接的 */
    if(verifyCrcPayload(*p)!=UTILNET_SUCCESS)
    {
        LOG::record(UTILLOGLEVELRECORD,"epollclienthandle::AcceptEvent crc query failed uid:%zu",uid_);
        printTransfOnPer(p, "epollclienthandle AcceptEvent");
        return ;
    }

    if(p->to!=uid_)
    {
        printTransfOnPer(p, "epollclienthandle AcceptEvent");
        LOG::record(UTILLOGLEVELERROR, "zero dest uid, invalid! to:%zu",p->to);
        return ;
    }

    switch(p->id)
    {
        case MSGHEART:
            printf("Server reback num %zu\n",*(p->buf));
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

void epollclienthandle::ShowPacketInfo()
{
    printf("uid:%zu recpackagecount_ :%zu  sendpackagecount_ :%zu \n",uid_,recpackagecount_,sendpackagecount_);
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

/*标准设备读入事件 */
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

int epollclienthandle::UserSendMsgPoll()
{
    int ret=0;
    transfOnPer * p;
    while(SendBufferPop()==0){}
    /*表示对端主动断开连接 */
    if(errno==EPIPE)
    {
        GENERRORPRINT("errno EPIPE",errno,0);
        DelEvent(fd_);
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

void epollclienthandle::InintialMyInfo(transfOnPer *p)
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

    printf("name:%s signature:%s friends:%d:\n",myinfo_.name,myinfo_.signature,myfriends_.size());
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

int epollclienthandle::PushMsgToTerminal(transfOnPer *p)
{

    if(myfriends_.find(p->uid)==myfriends_.end())
    {
        LOG::record(UTILLOGLEVELERROR, "not my friend %zu",p->uid);
        return PROTOCALNOTFOUND;
    }
    printTransfOnPer(p,"PushMsgToTerminal");

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
    auto it=myfriends_.begin();
    if(it==myfriends_.end())
    {
        GENERRORPRINT("I have no friends",0,0);
        return UTILNET_ERROR;
    }
    
    curdialog_=it->second.uid;
    FormMsgAddToBuffer(MSGFRIEND,readbuffer_,ret,curdialog_);
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

    //std::memcpy(out,&p,STRUCTONPERLEN);
    //printTransfOnPer(&p,"FormMsgAddToBuffer");

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

#if 0
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
#else
int epollclienthandle::SendBufferPop()
{
    int len=sendbuffer_.size();
    int ret=0;
 
    if(send_pos_begin_==send_pos_end_) //buffer空的
    {
        return -1;
    }

    transfOnPer p;

    int tmp=send_pos_end_;
    tmp++;
    if(tmp==len)
    {
        tmp=0;
    }

    std::memcpy(&p,&sendbuffer_[tmp],STRUCTONPERLEN); 

    /*到发送的时候再进行加密 */
    #ifdef NEEDAESCRYPT
        ret=crypt_->AESEncrypt((unsigned char*)&p,STRUCTONPERLEN,(unsigned char*)&p,STRUCTONPERLEN);
        if(ret!=0)
        {
            GENERRORPRINT("RSAEncrypt error",ret,0);
            return UTILNET_ERROR;
        }
    #endif

    ret=writeGenericSend(fd_,(char*)&p, STRUCTONPERLEN);
    if(ret<0)
    {
        return ret;
    }

    /*发送成功了才进一步 */
    send_pos_end_=tmp;

    return UTILNET_SUCCESS;
}
#endif
