#include "network.h"
int main()
{  
    //priquesorttest();
    //benchmark();
    //multithread(3);

    char ip[]="127.0.0.1";
    int port=30256;
    //int fk=0;

    printf("client pid:%d\n",getpid());
        
    netclient nc(port,ip);
    int ret=0;
    for(int i=0;i<1;i++)
    {
        ret=nc.toconnect();
        if(ret<0)
        {
            return 1;
        }
    }

    while(1)
    {
        sleep(4);
        nc.wri();
    }


    return 0;
}