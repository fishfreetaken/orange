

#include "channel.h"
#include "util.h"


//#define MAXIUMBUFLEN 256
#define VECTRESIZEPACTSIZE 20

/*epoll 创建的根据fd创建的一个user类*/
user::user(int fd,size_t uid):
fd_(fd),
uid_(uid),
aescrpty_(nullptr),
recpackagecount_(0),
sendpackagecount_(0)
{
    LOG::record(UTILLOGLEVELRECORD,"user new create fd: %d uid_:%zu",fd_,uid_);
    try{
        crpty_ = new char[LOADPERSONCRTPYLEN];
    }catch(...)
    {
        throw std::string("user new failed fd:") + std::to_string(fd);
    }

    vecrecvpackcc_.resize(VECTRESIZEPACTSIZE,0);
}

user::~user()
{
    /* 
    for(auto it=partnermap_.begin();it!=partnermap_.end();it++)
    {
        if(it->second!=nullptr)
        {
            it->second->InformPartnerOnline(this,0); 
        }
    }*/
}

int user::SendTo()
{
    transfOnPer *info= &recvpacket_;
    printTransfOnPer(&recvpacket_,"SendToPartner");

    if(partnermap_.find(info->to)==partnermap_.end())
    {
        //找不到；
        LOG::record(UTILLOGLEVELRECORD,"SendTo info->to not found");
        return USERNOTMYFIREND;
    }

    if(partnermap_[info->to]==nullptr)
    {
        LOG::record(UTILLOGLEVELRECORD,"SendTo friend %zu not online send equape",info->to);
        //投递到消息队列，等待上线进行处理，同时写数据库；
        return 0;
    }

    partnermap_[info->to]->SendMyself(info);

    return 0;
}

int user::SendMyself(const transfOnPer *info)
{

    if(info==NULL)
    {
        LOG::record(UTILLOGLEVELERROR,"%s para is NULL",__FUNCTION__);
        return USERPOINTNULL;
    }

    if(info->to!=uid_)
    {
        LOG::record(UTILLOGLEVELERROR,"%s uid mismatch %zu",__FUNCTION__,info->to);
        return USERIDMISMATCH;
    }
    //SendMsgKeepDb(); //写数据库buffer
    GenericSend(info);
}

size_t user::SendToAll()
{
    //transfOnPer *info= &recvpacket_;
    printTransfOnPer(&recvpacket_,"SendToAll");
    int ret=0;
    size_t cc=0;
    for(auto i :partnermap_)
    {
        ret=i.second->SendMyself(&recvpacket_);
        if(ret==0)
        {
            cc++;
        }
    }
    return cc;
}

size_t user::GetUid() const
{
    return uid_;
}
int user::GetFd() const
{
    return fd_;
}

int user::InformPartnerOnline( user *handle, const size_t state)
{
    size_t tuid=handle->GetUid();

    auto it=partnermap_.find(tuid);
    if(it==partnermap_.end()){
        LOG::record(UTILLOGLEVELERROR,"%s  not find partner :%zu",__FUNCTION__,tuid);
        return USERNOTFOUNDID;
    }
    if(state)
    {
        it->second=std::shared_ptr<user>(handle);
    }else{
        it->second=nullptr;
    }
    transfPartner p;
    p.uid=handle->GetUid();
    p.state=state;
    GenericSend(MSGFRIENDINFO,handle->GetUid(),(char*)&p,STRUCTONFRILEN,0); /*服务消息 */
    return  USERSUCCESS;
}

int user::ParsePacket(int fd)
{
    int ret=0;
while(1)
{

    ret=readGenericReceive(fd,(char *)&recvpacket_,STRUCTONPERLEN);
    //printf("ParsePacket while 1 ret=%d\n",ret);
    if((ret<STRUCTONPERLEN)||(ret<0))
    {
        if ((errno == EINTR)||(errno == EAGAIN))
        {
            //LOG::record(UTILNET_ERROR,"%s LINE:%d readfail:%s \n",__FUNCTION__,__LINE__,strerror(errno));
            return 0;
        }
        LOG::record(UTILLOGLEVELERROR,"user::ParsePacket ret: %d fd=%d uid=%zu",ret,fd_,uid_);
        return USERFDREADFAIL;
    }

    //LOG::record(UTILLOGLEVELERROR,"user::ParsePacket2 ret: %d fd=%d uid=%zu",ret,fd_,uid_);
#ifdef NEEDAESCRYPT
    ret=aescrpty_->AESDecrypt((unsigned char *)&recvpacket_,STRUCTONPERLEN,(unsigned char *)&recvpacket_,STRUCTONPERLEN);
#endif
    if(verifyCrcPayload(recvpacket_)!=USERSUCCESS)
    {
        LOG::record(UTILLOGLEVELRECORD,"readGenericReceive ParsePacket crc query failed");
        printTransfOnPer(&recvpacket_,"verifyCrcPayload ParsePacket");
        return ret;
    }

    if((recvpacket_.uid!=uid_)||(recvpacket_.uid==0))
    {
        LOG::record(UTILLOGLEVELERROR,"user::ParsePacket uid is mismatch: remote:%zu uid:%zu",recvpacket_.uid,uid_);
        printTransfOnPer(&recvpacket_,"decrypt after");
        return USERIDMISMATCH;
    }

    recpackagecount_++;

    switch (recvpacket_.id)
    {
        case MSGHEART: //心跳包；应该需要应答
            HeartBeat();
            break;
        case MSGFRIEND: //好友目标 转发消息包；
            SendTo();
            break;
        case MSGGROUP: //群消息包；
            SendToAll();
            break;
        case MSGSECRET: //公钥加密，协商对称秘钥
            break;
        case MSGSERVERINFO: //个人以及好友信息的请求包
            break;
        default:
            LOG::record(UTILLOGLEVELERROR,"ParsePacket not recognised msg id");
            return USERFAILED;
            break;
    }
    vecrecvpackcc_[recvpacket_.id]++;
}
    return USERSUCCESS;
}

int user::GenericSend(const transfOnPer*info )
{
    if(info==nullptr)
    {
        GENERRORPRINT("info is null",0,0);
        return USERPOINTNULL;
    }
    int ret=0;
    std::memcpy(&sendpacket_,info,STRUCTONPERLEN);
#ifdef NEEDAESCRYPT
    ret=aescrpty_->AESEncrypt((unsigned char*)&sendpacket_,STRUCTONPERLEN,(unsigned char*)&sendpacket_,STRUCTONPERLEN);
    if(ret!=0)
    {
        LOG::record(UTILLOGLEVELRECORD,"%s AESEncrypt failed ret=%d",__FUNCTION__,ret);
        return ret;
    }
#endif
    ret=writeGenericSend(fd_,(char*)&sendpacket_,STRUCTONPERLEN);
    if(ret==STRUCTONPERLEN)
    {
        sendpackagecount_++;
    }else{
        GENERRORPRINT("send failed",ret,uid_);
    }
    return ret;
}

int user::GenericSend(const uint32_t id,const size_t &dest,const char*buf,size_t size,size_t from)
{

    std::memset(&sendpacket_,0,STRUCTONPERLEN);

    sendpacket_.id=id;
    sendpacket_.size=size;
    sendpacket_.uid=from;
    sendpacket_.to=dest;

    std::memcpy(sendpacket_.buf,buf,size);

    genCrcPayload(sendpacket_);

    //printTransfOnPer(&sendpacket_,"GenericSend");

    int ret=0;

    if(aescrpty_==nullptr)
    {
        LOG::record(UTILLOGLEVELERROR,"%s aescrpty_ is null",__FUNCTION__,ret);
        return USERPOINTNULL;
    }
#ifdef NEEDAESCRYPT
    /*输入 */
    ret=aescrpty_->AESEncrypt((unsigned char*)&sendpacket_,STRUCTONPERLEN,(unsigned char*)&sendpacket_,STRUCTONPERLEN);
    if(ret!=0)
    {
        LOG::record(UTILLOGLEVELRECORD,"%s AESEncrypt failed ret=%d",__FUNCTION__,ret);
        return ret;
    }
#endif
    ret=writeGenericSend(fd_,(char*)&sendpacket_,STRUCTONPERLEN);
    if(ret==STRUCTONPERLEN)
    {
        sendpackagecount_++;
    }else{
        GENERRORPRINT("send failed",ret,uid_);
    }
    return ret;
}

void user::CurrentStat()
{
    printf("fd=%d uid:%zu recpackagecount_ :%zu  sendpackagecount_%zu\n",fd_,uid_,recpackagecount_,sendpackagecount_);

    for(size_t i=0;i<VECTRESIZEPACTSIZE;i++)
    {
        if(vecrecvpackcc_[i]==0)
        {
            continue;
        }
        //printf("%zu --> %zu\n",i,vecrecvpackcc_[i]);
    }
}

void user::HeartBeat()
{
    //printf("uid:%zu recvpacket_ client HeartBeat num: %zu\n",recvpacket_.uid,*recvpacket_.buf);
    GenericSend(MSGHEART,uid_,recvpacket_.buf,sizeof(size_t),0);
}

int user::LoadUserInfoFromDb(size_t &uid, std::vector<transfPartner>& p)
{
    transfPartner s;
    std::memset(&s,0,sizeof(s));
    s.uid=uid;
    s.state=1;
    strncpy(s.name,"chendong",8);
    strncpy(s.signature,"qwertyui",8);
    p.push_back(s);
    printfPartner(&s,"LoadUserInfoFromDb");
    return 1; //没朋友
}

int user::InitialMyInfo(transfOnPer &m,transfPartner &s,const channel *p)
{
    uid_=m.uid;
    transfcrptykey *l= (transfcrptykey*)m.buf;
    std::memcpy(crpty_,l->secret,LOADPERSONCRTPYLEN);/*用户名密码匹配完就不要了也行 */
    //std::memcpy(key_,l->key,LOADAESCRPTYKEYLEN);
    memcpy(&myinfo_,&s,STRUCTONFRILEN); //拷贝自身的信息，可有可无；

    try{/*初始化秘钥句柄 */
         aescrpty_=std::make_shared<cryptmsg>(l->key,LOADAESCRPTYKEYLEN);
    }catch(...)
    {
        throw strerror(errno);
    }

    std::vector<transfPartner> fri;
    /*应该从db上捞取自己(应包含自己的信息)以及朋友的信息，并进行通知*/
    int ret = p->GetFileHD()->GetResult(fri,uid_);
    if(ret<0)
    {
        return USERNOTINITIALZE;
    }

    printf("InitialMyInfo Debug fd=%d uid=%zu friend:%lu\n",fd_,uid_,fri.size());

    GenericSend(MSGSERVERINFO,uid_,(char*)&s,STRUCTONFRILEN,0); /*返回数据给客户端，握手完成，如果客户端收不到再进行请求*/
    for(size_t i=0;i<fri.size();i++)
    {
        partnermap_[fri[i].uid]=p->FindByUid(fri[i].uid);
        if(partnermap_[fri[i].uid]!=nullptr)
        {
            printf("partnermap_ is not null!\n");
            //partnermap_[fri[i].uid]->InformPartnerOnline(this,1); //uid进行通知就行
        }
        GenericSend(MSGSERVERINFO,uid_,(char*)&fri[i],STRUCTONFRILEN,0);
    }
    return USERSUCCESS;
}

channel::channel()
{
    try{
        rsacrpty_=std::make_shared<cryptmsg>("asdf",1); /*new出来一个rsa加密的实体 */
        filehd_=std::make_shared<filehandle>(FRIENDFILEPATH);
        rsarecvbuf_ = new unsigned char[MAX_BUF_LEN];
    }catch(...)
    {
        throw "new channel error!";
    }
}
channel::~channel()
{
    delete [] rsarecvbuf_;
}

const std::shared_ptr<filehandle> channel::GetFileHD() const
{
    return filehd_;
}

void channel::ServerStatPrint(int timefds,int events)
{
    #if 0
    printf("------current state--------\n");
    for(auto i:idmapuser_)
    {
        if(i.second==nullptr)
        {
            printf("uid:%zu ",i.first);
        }else{
            i.second->CurrentStat();
        }
    }
    #endif
    //printf("------current state--------\n");
    static size_t lcount=0;
    printf("current %zu online num:%d tim:%d events:%d\n",lcount,idmapuser_.size(),timefds,events);
    lcount++;
    
}

int channel::CheckUidIsInDb(transfOnPer &m,transfPartner &s)
{
    /*目前假设m的check都是有效的*/
    #if 0
        dbm_->checkwetheror(m); //(两个信息 uid和uid对应的密码)捞取一个人的信息就行，匹配登录的用户名密码是否合法
        /*协商秘钥*/
    #endif

    return filehd_->GetResult(s,m.uid);

    char crpty[31]={0};
    memcpy(crpty,m.buf,LOADPERSONCRTPYLEN);
    LOG::record(UTILLOGLEVELRECORD,"line:%d uid: %zu crpty:%s",__LINE__,m.uid,crpty);

    s.uid=m.uid;
    s.state=1;
    strncpy(s.name,"chendong",8);
    strncpy(s.signature,"qwertyui",8);

    return USERSUCCESS;
}

/*对协议进行一个解析，读入的数据进行 校验，解密{解密失败时候，判断为非法连接断开}，分析协议报头，处理动作： 登录-》消息转发-》跳转*/
int channel::UserReadProtocl(int tfd)
{
    /*new a thread to decode c */
    /*通知对应user进行处理，并接收结果 */
    int ret= tfd;
   // LOG::record(UTILLOGLEVELRECORD,"UserReadProtocl parse %d",tfd);
    if(fdmapuser_.find(tfd)==fdmapuser_.end())
    {
        /*新加入连接的一个客户端，执行协议解析，先验证是一个合法的连接 */
        std::memset(rsarecvbuf_,0,MAX_BUF_LEN);
        int len=readGenericReceive(tfd,(char*)rsarecvbuf_,RSAENCRYPTBUFLEN);
        if(len<RSAENCRYPTBUFLEN)
        {
            LOG::record(UTILLOGLEVELRECORD,"readGenericReceive failed %d",len);
            return ret;
        }

        len=rsacrpty_->RSADecrypt(rsarecvbuf_,RSAENCRYPTBUFLEN,(unsigned char*)&sendpacket_,STRUCTONPERLEN);
        if(len!=STRUCTONPERLEN)
        {
            GENERRORPRINT("RSADecrypt failed",ret,tfd);
            return ret;
        }
        //printTransfOnPer(&sendpacket_,"UserReadProtocl");
        if(sendpacket_.id!=MSGSECRET)
        {/*非法连接消息，虽然可以正常解密，但是没有进行私钥的协商，还是不通 */
            LOG::record(UTILLOGLEVELRECORD,"readGenericReceive sendpacket_.id： %d not MSGSECRET",sendpacket_.id);
            return ret;
        }

        /*crc校验 */
        if(verifyCrcPayload(sendpacket_)!=USERSUCCESS)
        {
            LOG::record(UTILLOGLEVELRECORD,"readGenericReceive crc query failed");
            return ret;
        }

        transfPartner master; /*获取个人信息 */
        ret=CheckUidIsInDb(sendpacket_,master); /*从db中直接捞取数据，如果找到就创建一个新的user类，验证用户名密码,防止不合法连接浪费资源 */
        if(ret!=USERSUCCESS)
        {
            LOG::record(UTILLOGLEVELRECORD,"CheckUidIsInDb failed %d",ret);
            return ret;
        }

        if(idmapuser_.find(master.uid)!=idmapuser_.end())
        {/*这边涉及到如果同一个uid连入后的处理，鉴定权限？新连接的有效，将老的连接释放，只能存在一个有效连接 */
            
            ret= idmapuser_[master.uid]->GetFd();
            UserRemove(ret);
            GENERRORPRINT("idental uid",ret,master.uid);
        }

        try{
            fdmapuser_[tfd]=std::make_shared<user>(tfd,master.uid);
        }catch(...)
        {
            throw  strerror(errno);
        }

        idmapuser_[master.uid]=fdmapuser_[tfd];
        len=fdmapuser_[tfd]->InitialMyInfo(sendpacket_,master,this); /*sendpacket_ 包中需要包含AES秘钥，个人密码信息 */
        if(len!=USERSUCCESS)
        {
            GENERRORPRINT("InitialMyInfo failed",master.uid,tfd);
            return ret;
        }
    }else{
        /*user交给一个已存在user进行协议包处理 */
        ret = fdmapuser_[tfd]->ParsePacket(tfd); //解析协议包并应答
        if(ret<0)
        {
            /*设定一个定时时间，lru 超时的话进行删除 */
            return tfd;
            /*协议解析失败，无效链接，关闭端口 */
        }
    }

    return USERSUCCESS;
}

void channel::UserRemove(int &tfd)
{
    auto it= fdmapuser_.find(tfd);
    if(it==fdmapuser_.end())
    {
        return;
    }
    auto its=idmapuser_.find(it->second->GetUid());
    if(its==idmapuser_.end())
    {
        GENERRORPRINT("remove not valid",tfd,it->second->GetUid());
        return ;
    }
    static size_t count=0;
    count++;

    LOG::record(UTILLOGLEVELRECORD,"func:%s localcount=%zu fd=%d uid=%zu",__FUNCTION__,count,tfd,it->second->GetUid());

    idmapuser_.erase(its);
    fdmapuser_.erase(it);

}

std::shared_ptr<user> channel::FindByUid(size_t&uid) const
{
    auto it= idmapuser_.find(uid);
    if(it==idmapuser_.end())
    {
        return nullptr;
    }
    return it->second;
}


