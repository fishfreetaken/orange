#include <sys/types.h>
#include <sys/socket.h>

#include <errno.h>
#include <string.h>

#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include "log.h"

#include <vector>

#define UTILLOGLEVEL 1

#define UTILNET_ERROR  -1

int tcpGenericServer(int port);
int tcpGenericConnect(const char *source_addr,int port,const char *dest_ip,int dest_port);
