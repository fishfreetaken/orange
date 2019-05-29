
#include "util.h"

#define TMPBUFFERSIZE 64

int transfertwo(int _in_fd,int _out_fd)
{
    char buf[TMPBUFFERSIZE]={0};
    //char wbuf[TMPBUFFERSIZE]={0};
    memset(buf,0,TMPBUFFERSIZE);
    int rlen= read(_in_fd,buf,TMPBUFFERSIZE);
    if (rlen<0)
    {
        if ((errno == EINTR)||(errno== EAGAIN))
        {
            return UTILNET_SUCCESS;
        }
        LOG::record(UTILLOGLEVEL1,"%d read:%s",errno,strerror(errno));
        return UTILNET_ERROR;
    }
    else{
        if(strcmp(buf,"over")==0)
        {
            printf("skip the while\n");
            return UTILNET_ERROR;
        }
        printf("read from _in_fd:%d rlen:%d :%s\n",rlen,_in_fd,buf);

        //memset(wbuf,0,TMPBUFFERSIZE);
        //snprintf(wbuf,TMPBUFFERSIZE,"reback:%s, count:%d",buf,communit);

        rlen=write(_out_fd,buf,strlen(buf));
        if (rlen<0)
        {
            LOG::record(UTILLOGLEVEL1,"read:%s",strerror(errno));
            return UTILNET_ERROR;
        }else{
            printf("write to _out_fd:%d len:%d\n",_out_fd,rlen);
        }
    }
    return UTILNET_SUCCESS;
}

void epollreadcallback(int fd)
{
    char buf[TMPBUFFERSIZE]={0};
    memset(buf,0,TMPBUFFERSIZE);
    int rlen= read(fd,buf,TMPBUFFERSIZE);
    do{
        if (rlen<0)
        {
            LOG::record(UTILLOGLEVEL1,"%s %d read:%s",__FUNCTION__,errno,strerror(errno));
            break;
        }
    }while(0);

    rlen=write(fd,buf,strlen(buf));
    do{
        if (rlen<0)
        {
            LOG::record(UTILLOGLEVEL1,"%s %d write:%s",__FUNCTION__,errno,strerror(errno));
            break;
        }
    }while(0);
}

int main()
{
    int port =8888;

    int fd=tcpGenericServer("10.8.49.62",port);
    if(fd < 0)
    {
        LOG::record(UTILLOGLEVEL1, "createlistst __LINE__ : %s", strerror(errno));
        return -1;
    }

    printf("success create server fd: %d\n",fd);

    int ep_fd=epoll_create(1024);
    if (ep_fd<0)
    {
        LOG::record(1,"epcreate error:%d %s\n",errno,strerror(errno));
    }

    //char buf[TMPBUFFERSIZE]={0};
    //char wbuf[TMPBUFFERSIZE]={0};
    std::vector<int> vecclient;

    struct sockaddr m;
    socklen_t flag=1;
    int count=0;
    while(1)
    {
        int tfd=accept(fd,&m,&flag);
        if(tfd==-1)
        {
            if ((errno == EINTR)||(errno == EAGAIN))
            {
                continue;
            }else{
                LOG::record(UTILNET_ERROR,"%d accept:%s \n",errno,strerror(errno));
                return -1;
            }
        }
        LOG::record(1,"accept a connect:%d\n",tfd);
        
        //close(tfd);
        count++;
        LOG::record(UTILNET_ERROR,"__LINE__ accept count:%d\n",count);

        setNonBlock(tfd);

        vecclient.push_back(tfd);
        struct epoll_event ee = {0,0};
        ee.events |=  EPOLLIN | EPOLLET;
        ee.data.fd=tfd;
        //ee.data.ptr=(void*)epollreadcallback;
        if(epoll_ctl(ep_fd,EPOLL_CTL_ADD,tfd,&ee)==-1)
        {
            LOG::record(UTILNET_ERROR,"epoll create %d:%s\n",errno,strerror(errno));
        }
        if(count==1)
        {
            break;
        }
    }

    int numReady=0;
    printf("go into transfer!\n");

    while(1)
    {
        struct epoll_event ee[4];
        numReady=epoll_wait(ep_fd,ee,1024,10000);
        if(numReady==-1)
        {
            LOG::record(UTILNET_ERROR,"epoll create %d:%s\n",errno,strerror(errno));
        }
        if((numReady==0)||(numReady>4))
        {
            printf("exceed the time num is zero!%d\n",numReady);
        }

        for(int i=0;i<numReady;i++)
        {
            printf("occure info fd:%d ",ee[i].data.fd);
            if(ee[i].events & EPOLLIN )
            {
                epollreadcallback(ee[i].data.fd);
            }
        }
    }

    //printf("rev:%s\n",buf);//如果不加\n符号，是不会从stdout中打印出来的！

    printf("go into transfer2!\n");
    int rlen =0;
    int communit=0;
    while(1)
    {
        if(transfertwo(vecclient[0],vecclient[1])<0)
        {
            break;
        }
        if(transfertwo(vecclient[1],vecclient[0])<0)
        {
            break;
        }
    }

    LOG::record(UTILNET_ERROR,"dialog over! break count:%d",count);
    for (auto i :vecclient)
    {
        close(i);
    }

    close(fd);
    
    return 0;
}
