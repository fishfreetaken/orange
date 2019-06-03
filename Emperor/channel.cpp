

#include "log.h"
user::user(size_t uid,int fd):
uid_(uid),
fd_(fd)
{

}

int user::SendTo(transfOnPer *info)
{
    if(info==NULL)
    {
        LOG::record(UTILLOGLEVEL1,"user::sendto info is NULL\n");
        return -1;
    }
    if(partermap.find(info->to)==partermap.end())
    {
        //对方不在线，将消息投递到一个消息队列中。
        return 0;
    }
    //将消息通过接口发送给对方；
    int ret=partermap[info->to].SendBuf(info->buf,info->size);

    return ret;
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
        ret=i.second->SendBuf(info->buf,info->size);
        if(ret==0)
        {
            cc++;
        }
    }
    return cc;

}
