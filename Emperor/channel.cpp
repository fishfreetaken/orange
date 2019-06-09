

#include "log.h"

/*epoll 创建的根据fd创建的一个user类*/
user::user(int fd):
fd_(fd),
uid_(0);
{
}

user::~user()
{
    if(fd_>2)
    {
        ::close(fd_);
        //epoll中取消注册是事件
    }
}

int user::ParsePacket(transfOnPer *info)
{
    if(info==nullptr)
    {
        LOG::record(UTILLOGLEVEL1,"user::ParsePacket info is NULL");
        return -1;
    }

    if(info->uid!=uid_)
    {
        LOG::record(UTILLOGLEVEL1,"user::sendto info uid is mismatch: s:%zu d:%zu",info->udi,uid_);
        return -1;
    }

    switch (info->fd)
    {
        case 0: //心跳包；应该需要应答

            break;
        case 1: //转发消息包；
            SendTo(info);
            break;
        case 2: //广播包
            break;

        default:
            LOG::record(UTILLOGLEVEL1,"not recognise id");
            break;
    }
    return 0;
}

int user::SendTo(transfOnPer *info)
{
    //int JudegeIfMyFriend();
    //int ret=GetInstance()->SendToOne(info);
    /*
    if(GetInstance()->find(info->to)==-1)
    {
        //对方不在线，将消息投递到一个消息队列中。
        return 0;
    }
    //将消息通过接口发送给对方；
    int ret=partermap[info->to]->SendMyself(info);*/

    if(partnermap.find(info->to)==partnermap.end())
    {
        //找不到；
        return -1;
    }

    if(partnermap[info->to]==nullptr)
    {
        //投递到消息队列，等待上线进行处理，同事写数据库；
        return 0;
    }

    partnermap[info->to].SendMyself(info);

    return ret;
}

int user::SendMyself(transfOnPer *info)
{
    if(info==NULL)
    {
        LOG::record(UTILLOGLEVEL1,"%s para is NULL",__FUNCTION__);
        return -1;
    }
    if(info->to!=uid_)
    {
        LOG::record(UTILLOGLEVEL1,"%s uid mismatch %zu",__FUNCTION__,info->to);
        return;
    }
    if(fd_<=2)
    {//文件描述符失效，发送失败，将消息发送到数据库中，或者发送到消息队列中；

        return;
    }
    SendBuf(info->buf,info->size);
}

int user::SendBuf(const char* buf,size_t len)
{
    int ret=0;
    ret = ::write(fd_,buf,len);
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

void user::InfoFriendOnline(size_t uid,int fd)
{
    if(uid==uid_)
    {
        
    }
    return ;
}

void user::SetOnLine(int&fd)
{
    if(fd_>=0)
    {
        ::close(fd_);
        /*
            这个不但要关闭连接，还需要关闭epoll的注册事件
        */
        LOG::record(UTILLOGLEVEL1,"%s close fd_: %d",__FUNCTION__,fd_);
    }
    fd_=fd;
}
void user::SetOffLine(int& fd)
{
    if(fd_<=2)
    {
        LOG::record(UTILLOGLEVEL1,"%s close fd_: %d",__FUNCTION__,fd_);
        return;
    }
    ::close(fd_);
    fd_=-1;
}

int channel::IncOnePartner(size_t uid,int fd)
{
    for(auto it=partermap.begin();it!=partermap.end();it++)
    {
        partermap[fd].InfoFriendOnline(uid,fd);
    }

    if(partermap.find(fd)!=partermap.end())
    {
        delete partermap[fd];
    }
    partermap[fd]= new user(uid,fd);

    return 0;
}

user* UserOnlineReg(size_t &uid,int fd)
{
    /*
        fd不合法
    */
    if(fd<=2)
    {
        LOG::record(UTILLOGLEVEL1,"%s fd is invalid %d",__FUNCTION__,fd);
        return nullptr;
    }

    if(partermap.find(uid)==partermap.end())
    {
        /*
            如果找不到的话应该需要从mysql或者配置文件去加载这个用户
        */
        //LOG::record(UTILLOGLEVEL1,"%s no found uid: %zu",__FUNCTION__,uid);
        return IncOnePartner(uid,fd);;
    }
    user*p= partermap[uid];
    p.SetOnline(int fd);

    return p;
}
