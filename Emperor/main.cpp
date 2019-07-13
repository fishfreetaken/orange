#include "util.h"
#include "protocal.h"
#include "epollserverhandle.h"
#include  "openssl.h"
#include "cryptmsg.h"

int main()
{
    #if 0
        cryptmsg tmp;
        tmp.GenerateKeyFiles("./rsapub.pem","./rsapriv.pem");
//client:
        transfOnPer send;
        std::memset(&send,0,STRUCTONPERLEN);
        send.id=4;
        send.uid=2156787956;
        send.to=3;
        sprintf(send.buf,"Hello world !1234567!Hello world cycle!123456789");
        send.size=strlen(send.buf);
        genCrcPayload(send);

        unsigned char buf[RSA_KEY_LENGTH]={0};
        unsigned char aeskey[RSA_KEY_LENGTH]={0};

        printTransfOnPer(&send,"client send");

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
        cli.AESEncrypt((unsigned char*)&send,STRUCTONPERLEN,buf,RSA_KEY_LENGTH);

        int cc=RSA_KEY_LENGTH-1;
        while(*(buf+cc)=='\0')
        {
            cc--;
        }
        printf("cc==%d\n",cc);

        cryptmsg s((char*)aeskey,RSA_KEY_LENGTH);
        transfOnPer recv;
        std::memset(&recv,0,STRUCTONPERLEN);

        s.AESDecrypt(buf,RSA_KEY_LENGTH,(unsigned char*)&recv,STRUCTONPERLEN);

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
