#include "util.h"
#define TMPBUFFERLEN 300

/*
void serialinput(char *buf,int len)
{
    memset(buf,0,len);
    char ch;    
    int i = 0;
    while (1)
    {
        ch = getc();
        if (ch == 13)
            break;
        if (ch == 8)
        {
            i--;
            //RePrint(i);
            continue;
        }
        std::cout<<ch;
        buf[i]=ch;
        i++;
        if(i==len)
        {
            LOG::record(UTILLOGLEVEL1, "serialinput exceed %d\n", len);
            buf[len-1]='\0';
            break;
        }
    }
    std::cout<<std::endl;
    return ;
}*/

void threadRead(int fd)
{
    char buf[TMPBUFFERLEN];
    int r=0;
    printf("Hello thread read!\n");
    while(1)
    {
        memset(buf,0,TMPBUFFERLEN);
        //printf("Please input you want to send to server:");
        std::cin.getline(buf,TMPBUFFERLEN);
        //scanf("%s",buf);
        do{
             r=write(fd,buf,strlen(buf));
            if(r < 0)
            {
                LOG::record(UTILLOGLEVEL1, "write %d : %s\n", __LINE__,strerror(errno));
            }
        }while(0);
        
        if(strcmp(buf,"over")==0)
        {
            printf("skip the while\n");
            break;
        }
        printf("len:%d local to other:\n%s\n",r,buf);
    }
    printf("threadRead over!");
}


int main()
{
    /*
    for(int i=0;i<3;i++)
    {
        int fk_fd=fork(); //2的n次方个进程
    }
    */
    char buf[TMPBUFFERLEN];

    int port =8889;
    int fd=tcpGenericConnect(NULL,port,"10.8.49.62",8888);
    if(fd < 0)
    {
        LOG::record(UTILLOGLEVEL1, "tcpGenericConnect : %s", strerror(errno));
        return UTILNET_ERROR;
    }
    std::thread alineread(threadRead,fd);
    alineread.detach();

    int diagcount=0;
    int r=0;

    while(1)
    {
        memset(buf,0,TMPBUFFERLEN);
        r=read(fd,buf,TMPBUFFERLEN);
        if(r < 0)
        {
            if ((errno == EINTR)||(errno == EAGAIN))
            {
                continue;
            }else{
                LOG::record(UTILLOGLEVEL1, "read %d : %s\n", __LINE__,strerror(errno));
                break ;
            }
        }
        if(r==0)
        {
            printf("server is closed! break;\n");
            break;
        }
        diagcount++;
        printf("count:%d len:%d  other to local:\n%s\n",r,diagcount,buf);
        transfOnPer rec;
        memcpy(&rec,buf,sizeof(transfOnPer));
        printf("id:%d to:%d size:%d\n",rec.id,rec.to,rec.size);
    }

    close(fd);
    return 0;
}
