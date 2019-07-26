#include "util.h"
#include "protocal.h"
#include "epollserverhandle.h"
#include  "openssl.h"
#include "cryptmsg.h"

int WhetherServer=0;

int main()
{
    util_init();
    
#if 0
    LogInfo("Hello %d",1);
    LogWarning("hello %d",2);
    LogError("hello %d",3);
    LogRecord("hello %d",4);
    return 1;
#endif

    #if 0
        cryptmsg tmp;
        tmp.GenerateKeyFiles("./rsapub.pem","./rsapriv.pem");
//client:
        transfOnPer send[3];
        std::memset(&send,0,STRUCTONPERLEN*3);
        send[0].id=4;
        send[0].uid=2156787956;
        send[0].to=3;
        sprintf(send[0].buf,"Hello world !1234567!Hello world cycle!123456789");
        send[0].size=strlen(send[0].buf);
        genCrcPayload(send[0]);

        send[1].id=895;
        send[1].uid=234;
        send[1].to=7;
        sprintf(send[1].buf,"send buf 1...1235");
        send[1].size=strlen(send[1].buf);
        genCrcPayload(send[1]);

        unsigned char buf[RSA_KEY_LENGTH]={0};
        unsigned char aeskey[RSA_KEY_LENGTH]={0};

        printTransfOnPer(send,"client send");

        cryptmsg cli("./rsapub.pem");

    #if 0
        int ret=cli.RSAEncrypt((unsigned char*)&send,STRUCTONPERLEN,buf,RSA_KEY_LENGTH);

        printf("ret:%d enlen:%d\n",ret,i);

//server :
        cryptmsg s((char*)aeskey,RSA_KEY_LENGTH);

        transfOnPer recv;
        std::memset(&recv,0,STRUCTONPERLEN);

        ret=s.RSADecrypt(buf,ret,(unsigned char*)&recv,RSA_KEY_LENGTH);
        printf("recvlen: %d\n",ret);
        #endif


        cli.AESGenEnCryptKey(aeskey,0);
        
        cli.AESEncrypt((unsigned char*)&send[0],STRUCTONPERLEN,(unsigned char*)&send[0],STRUCTONPERLEN);

        cli.AESEncrypt((unsigned char*)&send[1],STRUCTONPERLEN,(unsigned char*)&send[1],STRUCTONPERLEN);


        cryptmsg s((char*)aeskey,RSA_KEY_LENGTH);

        transfOnPer recv;
        std::memset(&recv,0,STRUCTONPERLEN);

        s.AESDecrypt((unsigned char*)&send[1],RSA_KEY_LENGTH,(unsigned char*)&recv,STRUCTONPERLEN);

        //hexprint(aeskey,strlen((char*)aeskey));

        printTransfOnPer(&recv,"server manage receive");

        return 1;
    #endif

    #if 0
        unsigned char data[DATATESTLEN]={0};
        GenRandomKey grk;
        grk.GenStrEnLetter(1024,(char*)data);

        generalCheckout(data,128);

        return 1;
    #endif

    /*在堆上分配比较好，不会占用栈上有限的内存空间 */
    std::shared_ptr<epollserverhandle> m = std::make_shared<epollserverhandle>();

    m->ServerStart(SERVERLISTENIP,SERVERLISTENPORT);

    //m->ServerStart(SERVERLISTENIP,SERVERLISTENPORT);

    return 2;

}
