

//#ifdef UTIL_HEADER_
//#define UTIL_HEADER_


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
#include <iostream>
#include <cstring>

#include <sys/epoll.h>
#include "protocal.h"
#include "log.h"

#define UTILNET_SUCCESS       0
#define UTILNET_ERROR        -1
#define UTIL_POINTER_NULL    -2
#define UIIL_NOTFOUND        -3


#define SERVERLISTENIP    "192.168.1.105"
#define SERVERLISTENPORT 45821

int tcpGenericServer(const char *source_addr,int port);
int tcpGenericConnect(const char *source_addr,int port,const char *dest_ip,int dest_port);
void setNonBlock(int socket_fd);
int writeGenericSend(int fd,const char * buf,int len);
int readGenericReceive(int fd, char *buf,int len);

//void printTransfOnPer(transfOnPer *m)

//#endif

void printTransfOnPer(transfOnPer *m,const char* from);
void printfPartner(transfPartner *m,const char *from);

int verifyCrcPayload(transfOnPer &m);
int genCrcPayload(transfOnPer &m);
