#include "util.h"
#include "clientuser.h"

#include "openssl.h"
//std::cin.getline(buf,TMPBUFFERLEN);



size_t DATATESTLEN=128;

int main()
{

    unsigned char data[DATATESTLEN]="QWERTYUIOPASDF";
    char endata[DATATESTLEN]={0};
    char dedata[DATATESTLEN]={0};
    rsa_test(data,DATATESTLEN,endata,DATATESTLEN,dedata,DATATESTLEN);

    return 1;
    epollclienthandle s(3);
    s.StartConnect(SERVERLISTENIP,SERVERLISTENPORT);

    return 0;
}
