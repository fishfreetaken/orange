
#include "util.h"

#define TMPBUFFERSIZE 64

int transfertwo(int _in_fd,int _out_fd)
{
    char buf[TMPBUFFERSIZE]={0};
    //char wbuf[TMPBUFFERSIZE]={0};
    memset(buf,0,TMPBUFFERSIZE);
    int rlen= read(_in_fd,buf,TMPBUFFERSIZE);
    if ((rlen<0)&&(errno!=11))
    {
        LOG::record(UTILLOGLEVEL1,"%d read:%s",errno,strerror(errno));
        return UTILNET_ERROR;
    }else if((errno == 11)||(errno == EINTR))
    {
        return UTILNET_SUCCESS;
    }
    else{
        if(strcmp(buf,"over")==0)
        {
            printf("skip the while\n");
            return UTILNET_ERROR;
        }
        printf("rlen:%d receive client_FD:%d buf:%s\n",rlen,_in_fd,buf);

        //memset(wbuf,0,TMPBUFFERSIZE);
        //snprintf(wbuf,TMPBUFFERSIZE,"reback:%s, count:%d",buf,communit);

        rlen=write(_out_fd,buf,strlen(buf));
        if (rlen<0)
        {
            LOG::record(UTILLOGLEVEL1,"read:%s",strerror(errno));
            return UTILNET_ERROR;
        }else{
            printf("write _out_fd:%d len:%d\n",_out_fd,rlen);
        }
    }
    return UTILNET_SUCCESS;
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
            if ((errno == EINTR)||(errno== 11))
            {
                continue;
            }else{
                LOG::record(UTILNET_ERROR,"%d accept:%s ",errno,strerror(errno));
                return -1;
            }
        }
        LOG::record(1,"accept a connect:%d",tfd);
        
        //close(tfd);
        count++;
        LOG::record(UTILNET_ERROR,"__LINE__ accept count:%d",count);

        setNonBlock(tfd);

        vecclient.push_back(tfd);
        if(count==2)
        {
            break;
        }
    }
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
