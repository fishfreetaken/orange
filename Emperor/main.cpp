#include "util.h"
#include "protocal.h"
#include "epollserverhandle.h"

int main()
{
    epollserverhandle m;
    m.ServerStart(SERVERLISTENIP,SERVERLISTENPORT);

    return 2;

}
