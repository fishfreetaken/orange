

#include <sys/types.h>
#include <sys/socket.h>

#include <limits.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <stddef.h>

#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <iostream>

#include "genrandom.h"
#include <vector>
#include <thread>

#include <sys/epoll.h>


#define UTILNET_SUCCESS  0
#define UTILNET_ERROR  -1

#define SERVERLISTENIP    "localhost"
#define SERVERLISTENPORT 8888

#define LOADCHARLEN 150

int tcpGenericServer(const char *source_addr,int port);
int tcpGenericConnect(const char *source_addr,int port,const char *dest_ip,int dest_port);
void setNonBlock(int socket_fd);
int writeGenericSend(int fd,const char * buf,int len);


//统一的一个包头结构
typedef struct transferOnlinePersion{
    uint id; //=2
    size_t uid;
    size_t to;
    size_t size;
    char info_[LOADCHARLEN];
} transfOnPer;

#define STRUCTONPERLEN  sizeof(transfOnPer)

