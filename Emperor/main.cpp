
#include "util.h"

int main()
{
    int port =8888;

    int fd=createlisten(port);

    printf("success create server fd: %d",fd);
    close(fd);
    return 0;

}