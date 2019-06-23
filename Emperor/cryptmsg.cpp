

#include "cryptmsg.h"
#include "util.h"

cryptmsg::cryptmsg(int a)
{
    AcquireRsaPubKey();
    AESGenEnCryptKey();
}

cryptmsg::cryptmsg()  /*server端 */
{
    AcquireRsaSecKey();
    //AESGenEnCryptKey();
}
cryptmsg::~cryptmsg()
{

}

/*新建的aes的密码体系，使用解密进行初始化 */
/*
void cryptmsg::cryptmsg(std::string &aesdekey);
{
    aesdekey_.assign(aesdekey);
}
*/

int cryptmsg::AcquireRsaPubKey()
{
    rsapubkey_.assign("123456778");
}

int cryptmsg::AcquireRsaSecKey()
{
    rsaseckey_.assign("123456778");
}

int cryptmsg::AESDecrypt(const char* in,int size, char * packet)
{
    return 0;
}

int cryptmsg::AESEncrypt(const char* packet,int size,char *out )
{
    return 0;
}

int cryptmsg::RSADecrypt(const char* in,int size, char * packet)
{
    AcquireRsaSecKey();
    return 0;
}

int cryptmsg::RSAEncrypt(const char* packet,int size,char *out )
{
    return 0;
}

/* 
int cryptmsg::AESSetDecryptKey(std::string &m)
{
    aesdekey_.assign(m);
}
*/

int cryptmsg::AESCallEnCryptKey(std::string &m)
{
    //m.copy(aesenkey_);
}

int cryptmsg::AESGenEnCryptKey()
{
    aesenkey_.assign("16gr4eg");
}


