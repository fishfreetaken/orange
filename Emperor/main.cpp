#include "util.h"
#include "protocal.h"
#include "epollserverhandle.h"

#define READLEN 2048

int main()
{

    epollserverhandle m;
    m.ServerStart(SERVERLISTENIP,SERVERLISTENPORT);

    return 2;

}
