#ifndef  CRYPTMSG_HEADER_
#define CRYPTMSG_HEADER_

#include <openssl/rsa.h>
#include <openssl/aes.h>
#include <openssl/pem.h>
#include <openssl/rand.h> 
#include <openssl/evp.h>
#include <openssl/err.h>
 #include <openssl/engine.h>

#include <string>
#define CRYPTTYPERSA 1

#define MAX_BLOCK_SIZE 128
#define MAX_ENCRYPT_SIZE 86 //
#define MAX_BUF_LEN 1024
class cryptmsg{

public:
    /* 
        客户端和服务端都需要初始化对称加密和非对称加密的功能；
        初始化时：先初始化一个非对称加密功能；
    */
    /*服务端直接先不初始化公钥，解密的时候再初始化 */
    cryptmsg(const char* s,size_t len); /*初始化rsa非对称加密秘钥，客户端初始化pubkey */
    cryptmsg(); /*初始化aes对称加密秘钥，输入一个解密的秘钥 */
    cryptmsg(const char*s);
    ~cryptmsg();

    int AESDecrypt(const unsigned char* encryptData, size_t encryptlen,unsigned char * decryptData,size_t decryptlen);
    int AESEncrypt(const unsigned char* data,size_t datalen,unsigned char *out,size_t outlen );

    /*从远程获取当前秘钥的私密，可以通过加密日期时间来区分秘钥 */
    int RSADecrypt(const unsigned char* encrypt_data,size_t encrypt_data_len,unsigned char * decrypt_data, size_t decrypt_data_len);/*返回int的长度 */
    int RSAEncrypt(const unsigned char* data,size_t datalen,unsigned char *encrypt_data,size_t encrypt_data_len );

   // int AESSetDecryptKey(std::string &in);  /*设置一个对阵加密的解密的秘钥 */
    int AESGenEnCryptKey(unsigned char *s,int ctl); /*生成一个对称加密的秘钥*/

    int GenerateKeyFiles(const char* pub_key_file, const char* priv_key_file);
    
    int RSABaseDecrypt(const unsigned char* encrypt_data,size_t encrypt_data_len,unsigned char *decrypt_data);
    int RSABaseEncrypt(const unsigned char* data,size_t datalen,unsigned char *encrypt_data);

private:
    int AcquireRsaPubKey();  /*从第三方获取一个公共秘钥 */
    int AcquireRsaSecKey();  /*预先在客户端内部加入一个公钥，使用文件配置私钥，从指定路径获取私钥*/

private:

    size_t aeskeylen_;
    std::string aeskey_; /*aes就这么一个key */

    std::string rsapubkey_; /*有的只需要初始化一个aeskey就行 */
    std::string rsaseckey_;

    std::string rsakeyfile_;

    EVP_PKEY *evprsakey_;

    unsigned char iv_[MAX_BLOCK_SIZE];      //="jofjwoiiewi23";
    unsigned char passwd_[MAX_BLOCK_SIZE];  //="donggeshidalao";

};

#endif

