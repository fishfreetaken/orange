#include "util.h"
#include "clientuser.h"

//#include "openssl.h"
//std::cin.getline(buf,TMPBUFFERLEN);

int main()  
{
    

    //epollclienthandle s(3);
    //s.StartConnect(SERVERLISTENIP,SERVERLISTENPORT);
    std::shared_ptr<epollclienthandle> m = std::make_shared<epollclienthandle>(3);

    m->StartConnect(SERVERLISTENIP,SERVERLISTENPORT);
    

    return 0;
}
