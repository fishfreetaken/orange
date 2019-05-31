
#include "util.h"

#define TMPBUFFERSIZE 64
#define MAXIUMEVENTS 3
#define SERVERLISTENPORT 8888


std::vector<int> vecclient;
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

void epollCreateEvents(int ep_fd,int tfd)
{   
    struct epoll_event ee = {0,0};
    ee.events |=  EPOLLIN | EPOLLET;
    ee.data.fd=tfd;

    LOG::record(UTILNET_ERROR,"epoll create %d:%d\n",ep_fd,tfd);
    if(epoll_ctl(ep_fd,EPOLL_CTL_ADD,tfd,&ee)==-1)
    {
        LOG::record(UTILNET_ERROR,"epoll create %d:%s\n",errno,strerror(errno));
    }
    vecclient.push_back(tfd);
}

void epollReadCallback(int fd)
{
    //printf("this is epoll read callback!\n");
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

void epollAcceptCallback(int ep_fd,int fd)
{
    int tfd=0;
    static int count=0;
    struct sockaddr m;
    socklen_t flag=1;
    do{
        tfd=accept(fd,&m,&flag);
        if(tfd==-1)
        {
            if ((errno == EINTR)||(errno == EAGAIN))
            {
                LOG::record(UTILNET_ERROR,"%d accept:%s \n",errno,strerror(errno));
                continue;
            }else{
                LOG::record(UTILNET_ERROR,"%d accept:%s \n",errno,strerror(errno));
                break; 
            }
        }
        break;
    }while(1);
    count++;
    printf("accept a connect:%d count:%d\n",tfd,count);
    
    setNonBlock(tfd);

    epollCreateEvents(ep_fd,tfd);

    transfOnPer reb;
    reb.id=2;
    reb.from=fd;
    reb.to=tfd;
    for(size_t i=1;i<vecclient.size();i++)
    {
        if(vecclient[i]==tfd)
        {
            continue;
        }
        reb.list_[i-1]=vecclient[i];
    }
    reb.size=vecclient.size()-2;
    printf("tfd:%d size:%d\n",reb.to,reb.size);
    writeGenericSend(tfd,(char *)&reb,STRUCTONPERLEN);

    return;
}

int main()
{

    int fd=tcpGenericServer(SERVERIP,SERVERLISTENPORT);
    if(fd < 0)
    {
        LOG::record(UTILLOGLEVEL1, "createlistst __LINE__ : %s", strerror(errno));
        return -1;
    }

    printf("success create server fd: %d\n",fd);

    int ep_fd=epoll_create(MAXIUMEVENTS);
    if (ep_fd < 0)
    {
        LOG::record(1,"epcreate error:%d %s\n",errno,strerror(errno));
    }

    epollCreateEvents(ep_fd,fd);

    int numReady=0;
    printf("go into transfer!\n");

    while(1)
    {
        struct epoll_event ee[MAXIUMEVENTS];
        numReady=epoll_wait(ep_fd,ee,MAXIUMEVENTS,15000);
        if(numReady==-1)
        {
            LOG::record(UTILNET_ERROR,"epoll create %d:%s\n",errno,strerror(errno));
        }
        /*
        if(numReady==0)
        {
            //printf("exceed the time num is zero!%d\n",numReady);
        }*/

        for(int i=0;i<numReady;i++)
        {
            if(ee[i].data.fd==vecclient[0])
            {
                epollAcceptCallback(ep_fd,fd);
                continue;
            }
            if(ee[i].events & EPOLLIN )
            {
                epollReadCallback(ee[i].data.fd);
            }
        }
    }

    //printf("rev:%s\n",buf);//如果不加\n符号，是不会从stdout中打印出来的！
/*
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
    */

    LOG::record(UTILNET_ERROR,"dialog over! break \n");
    for (auto i :vecclient)
    {
        close(i);
    }

    close(fd);
    
    return 0;
}
