

#include <sys/types.h>
#include <sys/socket.h>

#include <errno.h>
#include <string.h>

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

int tcpGenericServer(const char *source_addr,int port);
int tcpGenericConnect(const char *source_addr,int port,const char *dest_ip,int dest_port);
void setNonBlock(int socket_fd);
