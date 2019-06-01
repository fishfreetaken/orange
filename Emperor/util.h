

#include <sys/types.h>
#include <sys/socket.h>

#include <errno.h>
#include <string.h>
#include <string>

#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <iostream>
#include "log.h"

#include <vector>
#include <thread>

#include <sys/epoll.h>

#define UTILLOGLEVEL1 1

#define UTILNET_SUCCESS  0
#define UTILNET_ERROR  -1

#define SERVERLISTENIP    "localhost"
#define SERVERLISTENPORT 8888

int tcpGenericServer(const char *source_addr,int port);
int tcpGenericConnect(const char *source_addr,int port,const char *dest_ip,int dest_port);
void setNonBlock(int socket_fd);
int writeGenericSend(int fd,const char * buf,int len);

typedef struct transferObj {
    uint id; //=1
    int from;
    int to;
    std::string buf_;
} transfObjInfo;

//统一的一个包头结构
typedef struct transferOnlinePersion{
    uint id; //=2
    int from;
    int to;
    int size;
    char list_[100];
} transfOnPer;

#define STRUCTONPERLEN  sizeof(transfOnPer)

