

#include "log.h"
//#define MAXIUMBUFLEN 256

/*epoll 创建的根据fd创建的一个user类*/
user::user(int fd):
fd_(fd),
uid_(0),
buffer_(nullptr),  //使用主备定义好的消息协议
key_(nullptr)
{

}

user::~user()
{
    InformPartnerOnOrOff(0);
    if(buffer_!=nullptr)
    {
        delete [] buffer_;
    }
}

int user::ReadBufferParse()
{
    /*如果有读取数据请求的时候再进行内存地址分配 */
    if(buffer_==nullptr)
    {
        buffer_= new char[STRUCTONPERLEN];
        if(buffer_==nullptr)
        {
            return USERNEWFAIL;
        }
    }
    memset(buffer_,0,STRUCTONPERLEN);
    int ret=0;
    do{
        
        ret=read(fd_,buffer_,STRUCTONPERLEN);
        if(len==-1)
        {
            if ((errno == EINTR)||(errno == EAGAIN))
            {
                LOG::record(UTILNET_ERROR,"%s readfail:%s \n",__FUNCTION__,strerror(errno));
                continue;
            }else{
                LOG::record(UTILNET_ERROR,"%s readfail:%s \n",__FUNCTION__,strerror(errno));
                return USERFDREADFAIL; 
            }
        }
        break;
    }while(1);
    if(ret<STRUCTONPERLEN)
    {
        LOG::record(UTILNET_ERROR,"%s read len small:%d \n",__FUNCTION__,ret);
        return USERFDREADFAIL;
    }

    /*解密并进行校验如果出错直接返回失败，将buf解密后重新写入更新 */
    //ret=BufferDecryptCrc(); 
    //if(ret <0)
    //{
    //    return ret;
    //}
    ret=ParsePacket();
    if(ret <0)
    {
        return ret;
    }

    return ret;
}

int user::ParsePacket()
{
    transfOnPer *info=(transfOnPer *) buffer_;
    int ret=0;

    if(uid_==0)
    {
        /*从文件或者数据库中进行加载uid信息 */
        ret= LoadUserInfoFromDb(info->uid);
        if( USERSUCCESS !=ret )
        {
            LOG::record(UTILLOGLEVEL1,"user::ParsePacket NotFoundUid uid:%zu",info->udi);
            return ret;
        }
    }

    if(info->uid!=uid_)
    {
        LOG::record(UTILLOGLEVEL1,"user::ParsePacket uid is mismatch: remote:%zu local:%zu",info->udi,uid_);
        return USERIDMISMATCH;
    }

    switch (info->fd)
    {
        case 0: //心跳包；应该需要应答
            break;
        case 1: //好友目标 转发消息包；
            SendTo();
            break;
        case 2: //群消息包；
            break;
        case 3: //公钥加密，协商对称秘钥
            break;
        default:
            LOG::record(UTILLOGLEVEL1,"ParsePacket not recognised msg id");
            return USERFAILED;
            break;
    }
    return 0;
}

int user::LoadUserInfoFromDb()
{
    
}

int user::SendTo()
{
    transfOnPer *info= (transfOnPer *)buffer_;

    if(partnermap.find(info->to)==partnermap.end())
    {
        //找不到；
        return USERNOTMYFIREND;
    }

    if(partnermap[info->to]==nullptr)
    {
        //投递到消息队列，等待上线进行处理，同时写数据库；
        return 0;
    }

    partnermap[info->to].SendMyself(info);

    //DbFileStore(); //数据库操作进行数据存储

    return ret;
}

int user::SendMyself(transfOnPer *info)
{
    if(info==NULL)
    {
        LOG::record(UTILLOGLEVEL1,"%s para is NULL",__FUNCTION__);
        return USERPOINTNULL;
    }
    if(info->to!=uid_)
    {
        LOG::record(UTILLOGLEVEL1,"%s uid mismatch %zu",__FUNCTION__,info->to);
        return USERIDMISMATCH;
    }
    //BufferCryptCrc();//测试的时候进行不处理加密功能，保留接口

    SendBuf(info->buf,info->size);
}

int user::SendBuf(size_t len)
{
    int ret=0;
    ret = ::write(fd_,buffer_,len);
    if(ret<0)
    {
        LOG::record(UTILLOGLEVEL1,"user::SendBuf :%d errnor:%s",errno,strerror(errno));
    }

    return ret;
}

size_t user::SendToAll(transfOnPer *info)
{
    int ret=0;
    size_t cc=0;
    for(auto i :partermap)
    {
        ret=i.second->SendMyself(info);
        if(ret==0)
        {
            cc++;
        }
    }
    return cc;
}

int user::InformPartnerOnOrOff(int ret)
{
    int cc=0;
    for(auto it=partnermap.begin();it!=partnermap.end();it++)
    {
        if(it->second!=nullptr)
        {
            if(ret)
            {
                it->second->InformPartnerOnline(this,uid_); //通知同伴自身已经上线
            }else{
                it->second->InformPartnerOnline(nullptr,uid_); 
            }
            cc++;
        }
    }
    return cc;
}

int user::InformPartnerOnline(user*p,size_t ret)
{   
    auto it=partnermap.find(ret);
    if(it==partnermap.end()){
        LOG::record(UTILLOGLEVEL1,"%s  not find partner :%zu",__FUNCTION__,p->ReturnUid());
        return USERNOTFOUNDID;
    }
    it->second=p;
    return  USERSUCCESS;
}

int user::GroupMyPartner()
{
    int ret=0;
    for(auto it=partnermap.begin();it!=partnermap.end();it++)
    {
        it->second=channel::GetInstance()->FindUid(it->first);
        if(it->second!=nullptr)
        {
            ret++;
        }
    }
    InformPartnerOnOrOff(1);
    return ret;
}

/*对协议进行一个解析，读入的数据进行 校验，解密{解密失败时候，判断为非法连接断开}，分析协议报头，处理动作： 登录-》消息转发-》跳转*/
int channel::UserReadProtocl(int tfd)
{
    /*通知对应user进行处理，并接收结果 */
    if(fdmapuser_.find(tfd)==fdmapuser_.end())
    {
        /*新加入连接的一个客户端，执行协议解析 */
        fdmapuser_[tfd]=new user(tfd);
        if(fdmapuser_[tfd]==nullptr)
        {
            return USERNEWFAIL;
        }
        fdmapuser_[tfd]->GroupMyPartner();
        //fdmapuser_[tfd].InformPartner(); //通知同伴上线
    }
    int ret = fdmapuser_[tfd].ParsePacket(int tfd); //解析协议包并应答
    if(ret<0)
    {
        UserRemove(tfd);
        /*协议解析失败，无效链接，关闭端口 */
    }
    return ret;
}

void channel::UserRemove(int tfd)
{
    auto it= fdmapuser_.find(tfd)
    if(it==fdmapuser_.end())
    {
        return;
    }
    
    delete it->second;
    fdmapuser_.earse(it);
}

user* channel::FindUid(size_t&uid)
{
    auto it= idmapuser_.find(uid);
    if(it==idmapuser_.end())
    {
        return nullptr;
    }
    return it->second;
}
