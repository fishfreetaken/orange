
//#include "protocal.h"

#include <string>
#define CRYPTTYPERSA 1

class cryptmsg{

public:
    /* 
        客户端和服务端都需要初始化对称加密和非对称加密的功能；
        初始化时：先初始化一个非对称加密功能；
    */
    /*服务端直接先不初始化公钥，解密的时候再初始化 */
    cryptmsg(); /*初始化rsa非对称加密秘钥，客户端初始化pubkey */
    cryptmsg(int a); /*初始化aes对称加密秘钥，输入一个解密的秘钥 */
    ~cryptmsg();

    int AESDecrypt(const char* in,int size, char * packet);
    int AESEncrypt(const char* packet,int size,char *out);

    /*从远程获取当前秘钥的私密，可以通过加密日期时间来区分秘钥 */
    int RSADecrypt(const char* in,int size, char * packet);/*返回int的长度 */
    int RSAEncrypt(const char* packet,int size,char *out );

   // int AESSetDecryptKey(std::string &in);  /*设置一个对阵加密的解密的秘钥 */
    int AESGenEnCryptKey(); /*返回对阵加密的秘钥*/
    
private:
    int AcquireRsaPubKey();  /*从第三方获取一个公共秘钥 */
    int AcquireRsaSecKey();  /*预先在客户端内部加入一个公钥，使用文件配置私钥，从指定路径获取私钥*/

    int AESCallEnCryptKey(std::string &m);   /*生成一个对称加密的秘钥 */
private:
    std::string aesdekey_; /*远端的 */
    std::string aesenkey_; /*本地的 */

    std::string rsapubkey_; /*有的只需要初始化一个aeskey就行 */
    std::string rsaseckey_;

};

