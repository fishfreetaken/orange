#include <sys/types.h>
#include <sys/socket.h>

#include <errno.h>
#include <string.h>

#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include "log.h"
#define UTILLOGLEVEL 1

int createlisten(int port);