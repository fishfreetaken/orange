#include "util.h"
#include "clientuser.h"

//std::cin.getline(buf,TMPBUFFERLEN);

int main()
{

    epollclienthandle s(3);
    s.StartConnect(SERVERLISTENIP,SERVERLISTENPORT);

    return 0;
}
