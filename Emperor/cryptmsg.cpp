

#include "cryptmsg.h"
#include "util.h"

#include <openssl/rand.h>

/*低版本不支持一些特性 */
//#define OPENSSLOLDVERSION

//#define PASSWORDKEY "12345678"
#define PASSWORDKEY "donggeshidalao"

int Shahash::Encrypt(const unsigned char* data,size_t datalen,unsigned char *hash_data)
{
    unsigned int val=0;
    EVP_MD_CTX *ctx=nullptr;
    if(hash_data==nullptr)
    {
        LogError("hash data is null");
        return Status::mInvalidArgument;
    }
    ctx=EVP_MD_CTX_create();
    
    if(EVP_DigestInit_ex(ctx, EVP_sha256(), NULL)<=0)
    {
        goto error;
    }

    if(EVP_DigestUpdate(ctx,data,datalen)<0)
    {
        goto error;
    }

    if(EVP_DigestFinal_ex(ctx,hash_data,&val))
    {
        goto error;
    }

#ifdef OPENSSLOLDVERSION
//在新的openssl版本上不是用阿！
    EVP_MD_CTX_cleanup(ctx);
    EVP_MD_CTX_destroy(ctx);
#else
    EVP_MD_CTX_free(ctx);
#endif

    if(val!=32)
    {
        LogError("not valid hash len %d val:%d",datalen,val);
        goto error;
    }

    return Status::mOk;

error:
#ifdef OPENSSLOLDVERSION
//在新的openssl版本上不是用阿！
    EVP_MD_CTX_cleanup(ctx);
    EVP_MD_CTX_destroy(ctx);
#else
    EVP_MD_CTX_free(ctx);
#endif

    return Status::mCorruption;
}

Aescrypt::Aescrypt(char *c,uint32_t len)
{
    aeskey_.assign(c,len);
    std::memset(iv_,0,MAX_BLOCK_SIZE);
    sprintf((char*)iv_,"jofjwoiiewi23");
}

void Aescrypt::AesKeyGen(PersonalInfo &f)
{
    
}

int Aescrypt::Encrypt(const unsigned char* data,size_t datalen,unsigned char *out )
{
    if(aeskey_.size()==0)
    {
        LogError("Aescrypt::Encrypt failed");
        return Status::mInvalidArgument;
    }
    if((out==nullptr)||(data==nullptr))
    {
        LogError("out or data is nullptr");
        return Status::mInvalidArgument;
    }

    AES_KEY aes_ks3;

    AES_set_encrypt_key((unsigned char*)aeskey_.c_str(), aeskey_.size(), &aes_ks3);
    size_t cutup= datalen/AES_BLOCK_SIZE;
    size_t minus= datalen%AES_BLOCK_SIZE;
    size_t i =0;
    for(i=0;i < cutup; i++)
    {
        AES_cbc_encrypt(data+i*AES_BLOCK_SIZE,out+i*AES_BLOCK_SIZE,AES_BLOCK_SIZE,&aes_ks3,iv_,1);
    }
    if(minus!=0)
    {
        AES_cbc_encrypt(data+i*AES_BLOCK_SIZE,out+i*AES_BLOCK_SIZE,minus,&aes_ks3,iv_,1);
    }
    
    return Status::mOk;
}

int Aescrypt::Decrypt(const unsigned char* encryptData, size_t encryptlen,unsigned char * decryptData)
{
    if(aeskey_.size()==0)
    {
        LogError("Aescrypt::Decrypt failed");
        return Status::mInvalidArgument;
    }
    if((encryptData==nullptr)||(decryptData==nullptr))
    {
        LogError("encryptData or decryptData is nullptr");
        return Status::mInvalidArgument;
    }
    AES_KEY aes_ksd3;
    AES_set_decrypt_key((unsigned char*)aeskey_.c_str(), aeskey_.size(), &aes_ksd3);

    size_t cutup= encryptlen/AES_BLOCK_SIZE;
    size_t minus= encryptlen%AES_BLOCK_SIZE;
    size_t i =0;
    for(i=0;i < cutup; i++)
    {
        AES_cbc_encrypt(encryptData+i*AES_BLOCK_SIZE,decryptData+i*AES_BLOCK_SIZE,AES_BLOCK_SIZE,&aes_ksd3,iv_,0);
    }
    if(minus!=0)
    {
        AES_cbc_encrypt(encryptData+i*AES_BLOCK_SIZE,decryptData+i*AES_BLOCK_SIZE,minus,&aes_ksd3,iv_,0);
    }

    return Status::mOk;
}

void Aescrypt::AESGenEnCryptKey(unsigned char *out,int ctl)
{
    int ret=RAND_bytes(out, ctl);
    aeskey_.assign((char*)out,ctl);
    LogError("rand generate aes failed %d %d",ret,ctl);
    assert(ret<=0);
}



cryptmsg::cryptmsg()
{
    std::memset(iv_,0,MAX_BLOCK_SIZE);
    std::memset(passwd_,0,MAX_BLOCK_SIZE);

    int ret=0;
    ret=sprintf((char*)iv_,"jofjwoiiewi23");
    ret=sprintf((char*)passwd_,PASSWORDKEY);
}

cryptmsg::cryptmsg(const char*s):
//rsakeyfile_("./rsapub.pem"), //客户端读取public的key
rsakeyfile_(s),
evprsakey_(nullptr),
aeskeylen_(256),
md_ctx_(nullptr)
{

    std::memset(iv_,0,MAX_BLOCK_SIZE);
    std::memset(passwd_,0,MAX_BLOCK_SIZE);

    int ret=0;
    ret=sprintf((char*)iv_,"jofjwoiiewi23");
    ret=sprintf((char*)passwd_,PASSWORDKEY);

    //LOG::record(UTILLOGLEVELRECORD,"client cryptmsg create \n");
    AcquireRsaPubKey();
}

cryptmsg::cryptmsg(const char* s,size_t len):  /*server 初始化一个aes秘钥 */
rsakeyfile_("./rsapriv.pem"), /*private key */
evprsakey_(nullptr), 
aeskeylen_(256),
md_ctx_(nullptr)
{
    aeskey_.assign(s,aeskeylen_);
    //LOG::record(UTILLOGLEVELRECORD,"server cryptmsg create \n");
    std::memset(iv_,0,MAX_BLOCK_SIZE);
    std::memset(passwd_,0,MAX_BLOCK_SIZE);

    sprintf((char*)iv_,"jofjwoiiewi23");
    sprintf((char*)passwd_,PASSWORDKEY);

    AcquireRsaSecKey();
}

cryptmsg::~cryptmsg()
{
    EVP_PKEY_free(evprsakey_);

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

    RSA *rsa = nullptr;
    OpenSSL_add_all_algorithms(); 
    BIO *bio_pub = BIO_new(BIO_s_file());; 

    BIO_read_filename(bio_pub, rsakeyfile_.c_str()); 
    if (NULL == bio_pub) {
        printf("open_public_key bio file new error!\n"); 
        goto erro;
    } 
    rsa = PEM_read_bio_RSAPublicKey(bio_pub, NULL, NULL, NULL); 
    
    if (rsa == nullptr) {
        printf("open_public_key failed to PEM_read_bio_RSAPublicKey!\n");
        goto erro;
    }

    evprsakey_ = EVP_PKEY_new(); 
    if (nullptr == evprsakey_) { 
        printf("open_public_key EVP_PKEY_new failed\n"); 
        goto erro;
    }

    EVP_PKEY_assign_RSA(evprsakey_, rsa); 
    BIO_free(bio_pub);
    return 0;
erro:
    BIO_free(bio_pub); 
    RSA_free(rsa); /*这点是个坑，提前释放了这个rsa，导致运行直接core dump掉 */

    return 1;
}

int cryptmsg::AcquireRsaSecKey()
{
    //EVP_PKEY* key = NULL; 
    RSA *rsa = RSA_new(); 
    OpenSSL_add_all_algorithms(); 
    BIO* bio_priv = NULL; 
    bio_priv = BIO_new_file(rsakeyfile_.c_str(), "rb"); 
    if (NULL == bio_priv) { 
        printf("open_private_key bio file new error!\n"); 
        goto error;
    }
    
    rsa = PEM_read_bio_RSAPrivateKey(bio_priv, &rsa, NULL, (void *)passwd_); 
    if (rsa == NULL) { 
        printf("open_private_key failed to PEM_read_bio_RSAPrivateKey!\n"); 
        
        goto error;
    }
    evprsakey_ = EVP_PKEY_new(); 
    if (NULL == evprsakey_) {
        printf("open_private_key EVP_PKEY_new failed\n"); 
        goto error;
    }

    EVP_PKEY_assign_RSA(evprsakey_, rsa);
    BIO_free(bio_priv); 
    return 0;

error:
    RSA_free(rsa);
    BIO_free(bio_priv); 
    return 1;
}

/*
    ctl：1，随机设置一个aes秘钥；
    ctl：0，使用一个已知AESkey进行设置；
 */
int cryptmsg::AESGenEnCryptKey(unsigned char *out,int ctl)
{
    if(ctl==0)
    {
        int ret=RAND_bytes(out, aeskeylen_);
        if(ret<=0)
        {
            GENERRORPRINT("rand generate aes failed",ret,ctl);
            return -1;
        }
        aeskey_.assign((char*)out,aeskeylen_);
        //GENERRORPRINT("random gen aes init",aeskey_.size(),strlen((char*)out));
        return 0;
    }

    //ctl=256
    aeskey_.assign((char*)out,ctl);
    #if 0
    /* 公司的版本有点老啊，要不换一下？*/
    OPENSSL_CTX *ctx=nullptr;

    RAND_DRBG*rdb= RAND_DRBG_secure_new_ex(ctx,NID_aes_256_ctr,RAND_DRBG_FLAG_PRIVATE,);
    if(rdb==nullptr)
    {
        LOG::record(UTILLOGLEVELWORNNING,"RAND_DRBG_secure_new_ex new failed\n");
        return UTILNET_ERROR;
    }
    int ret=RAND_DRBG_bytes(rdb,out,len);
    if(ret<=0)
    {
        LOG::record(UTILLOGLEVELWORNNING,"RAND_DRBG_bytes create failed\n");
        return UTILNET_ERROR;
    }
    printf("generator random len:%d \n",strlen((char*)out));
    hexprint(out,len);

    aeskey_.assign((char*)out);
    #endif
    return aeskeylen_;
}

int cryptmsg::AESDecrypt(const unsigned char* encryptData,size_t encryptData_len,unsigned char * decryptData,size_t decryptData_len)
{
    if(aeskey_.size()==0)
    {
        LOG::record(UTILLOGLEVELWORNNING,"AESDecrypt aeskey_ not initlized\n");
        return -1;
    }
    if(0==decryptData_len)
    {
        GENERRORPRINT("encryptData_len>=decryptData_len",encryptData_len,decryptData_len);
        return -1;
    }
    AES_KEY aes_ksd3;
    AES_set_decrypt_key((unsigned char*)aeskey_.c_str(), aeskeylen_, &aes_ksd3);

    size_t cutup= encryptData_len/AES_BLOCK_SIZE;
    size_t minus= encryptData_len%AES_BLOCK_SIZE;
    size_t i =0;
    for(i=0;i < cutup; i++)
    {
        AES_cbc_encrypt(encryptData+i*AES_BLOCK_SIZE,decryptData+i*AES_BLOCK_SIZE,AES_BLOCK_SIZE,&aes_ksd3,iv_,0);
    }
    if(minus!=0)
    {
        AES_cbc_encrypt(encryptData+i*AES_BLOCK_SIZE,decryptData+i*AES_BLOCK_SIZE,minus,&aes_ksd3,iv_,0);
    }


    return 0;
}

size_t cryptmsg::AESBufLoadLen(size_t datalen)
{
    size_t tmp=datalen/AES_BLOCK_SIZE;
    if(datalen%AES_BLOCK_SIZE)
    {
        tmp++;
    }
    return tmp*AES_BLOCK_SIZE;
}

int cryptmsg::AESEncrypt(const unsigned char* data,size_t datalen,unsigned char *out, size_t outlen )
{
    if(aeskey_.size()==0)
    {
        LOG::record(UTILLOGLEVELWORNNING,"AESEncrypt aeskey_ not initlized\n");
        return -1;
    }
    if(datalen>outlen)
    {
        GENERRORPRINT("datalen>=outlen",datalen,outlen);
        return -1;
    }

    AES_KEY aes_ks3;

    AES_set_encrypt_key((unsigned char*)aeskey_.c_str(), aeskeylen_, &aes_ks3);
    size_t cutup= datalen/AES_BLOCK_SIZE;
    size_t minus= datalen%AES_BLOCK_SIZE;
    size_t i =0;
    for(i=0;i < cutup; i++)
    {
        AES_cbc_encrypt(data+i*AES_BLOCK_SIZE,out+i*AES_BLOCK_SIZE,AES_BLOCK_SIZE,&aes_ks3,iv_,1);
    }
    if(minus!=0)
    {
        AES_cbc_encrypt(data+i*AES_BLOCK_SIZE,out+i*AES_BLOCK_SIZE,minus,&aes_ks3,iv_,1);
    }
    
    return 0;
}

int cryptmsg::RSADecrypt(const unsigned char* encrypt_data,size_t encrypt_data_len,unsigned char *decrypt_data,size_t decrypt_data_len)
{
    
    if((encrypt_data_len==0)||(decrypt_data_len==0))
    {
        GENERRORPRINT("encrypt_data_len <= decrypt_data_len ",encrypt_data_len,decrypt_data_len);
        return UTILNET_ERROR;
    }

    int ret=0;
    if(encrypt_data_len<MAX_BLOCK_SIZE)
    {
        return RSABaseDecrypt(encrypt_data,encrypt_data_len,decrypt_data);
    }
    size_t cutup= encrypt_data_len/MAX_BLOCK_SIZE;
    size_t minus= encrypt_data_len%MAX_BLOCK_SIZE;
    size_t i =0;
    int  tmp=0;
    for(i=0;i < cutup; i++)
    {
        tmp=RSABaseDecrypt(encrypt_data+i*MAX_BLOCK_SIZE,MAX_BLOCK_SIZE,decrypt_data+i*tmp);  /*固定最长的单帧密码有128，但是返回的长度就不一定了，这里要注意点，否则一直解不出来 */
        if(tmp==0)
        {
            GENERRORPRINT("RSABaseDecrypt tmp re zero",tmp,0);
            return UTILNET_ERROR;
        }
        ret += tmp;
    }

    if(minus!=0)
    {
        ret+=RSABaseDecrypt(encrypt_data+i*MAX_BLOCK_SIZE,minus,decrypt_data+i*tmp);
    }
    return ret;
}

int cryptmsg::RSABaseDecrypt(const unsigned char* encrypt_data,size_t encrypt_data_len,unsigned char * decrypt_data)
{
    EVP_PKEY_CTX *ctx = NULL;
    size_t decrypt_data_len=0;
    OpenSSL_add_all_ciphers(); 
    ctx = EVP_PKEY_CTX_new(evprsakey_, NULL); 
    if (NULL == ctx) 
    {
        printf("ras_prikey_decryptfailed to open ctx.\n"); 
        goto error;
    } 
    if (EVP_PKEY_decrypt_init(ctx) <= 0) { 
        printf("ras_prikey_decryptfailed to EVP_PKEY_decrypt_init.\n"); 
        goto error;
    }
    //printf("decrypt delen:%d  enlen:%d\n",decrypt_data_len,encrypt_data_len);
    if (EVP_PKEY_decrypt(ctx, nullptr, &decrypt_data_len, encrypt_data, encrypt_data_len) <= 0) { 
        printf("ras_prikey_decryptfailed to EVP_PKEY_decrypt null.\n"); 
        ERR_print_errors_fp(stderr);
        goto error;
    }
    //printf("decrypt delen:%d enlen:%d\n",decrypt_data_len,encrypt_data_len);

    //密文跟解密的文长度保持一致
    if (EVP_PKEY_decrypt(ctx, decrypt_data, &decrypt_data_len, encrypt_data, encrypt_data_len) <= 0) {
        printf("ras_prikey_decryptfailed to EVP_PKEY_decrypt.\n");
        goto error;
    }
    //printf("after decrypt delen:%d enlen:%d\n",decrypt_data_len,encrypt_data_len);
error:
    ERR_print_errors_fp(stderr);
    EVP_PKEY_CTX_free(ctx); 

    return decrypt_data_len;
}

/*只对有效的ascii字符进行加密，不能出现'\0' */
int cryptmsg::RSAEncrypt(const unsigned char* data,size_t datalen,unsigned char *encrypt_data,size_t encrypt_data_len )
{
    if(datalen>=encrypt_data_len)
    {
        GENERRORPRINT("datalen <= encrypt_data_len",datalen,encrypt_data_len);
        return UTILNET_ERROR;
    }
    int ret=0;
    if(datalen <= MAX_ENCRYPT_SIZE)
    {
        ret= RSABaseEncrypt(data,datalen,encrypt_data);
        return ret;
    }

    //GENERRORPRINT("RSAEncrypt info",datalen,encrypt_data_len);
    size_t cutup= datalen/MAX_ENCRYPT_SIZE;
    size_t minus= datalen%MAX_ENCRYPT_SIZE;
    size_t i =0;
    for(i=0;i < cutup; i++)
    {
        ret+=RSABaseEncrypt(data+i*MAX_ENCRYPT_SIZE,MAX_ENCRYPT_SIZE,encrypt_data+i*MAX_BLOCK_SIZE);
    }

    if(minus!=0)
    {
        ret+=RSABaseEncrypt(data+i*MAX_ENCRYPT_SIZE,minus,encrypt_data+i*MAX_BLOCK_SIZE);
    }

    return ret;
}

/*return 密文的长度 */
int cryptmsg::RSABaseEncrypt(const unsigned char* data,size_t datalen,unsigned char *encrypt_data)
{
    EVP_PKEY_CTX *ctx = NULL; 
    size_t encrypt_data_len=0;
    int ret=0;
    //ENGINE *eng;
    OpenSSL_add_all_ciphers(); 

    if(evprsakey_==nullptr)
    {
        printf("RSAEncrypt evprsakey_ is null\n");

        AcquireRsaPubKey();
    }
    
    ctx = EVP_PKEY_CTX_new(evprsakey_, NULL); 
    if (NULL == ctx) { 
        printf("ras_pubkey_encryptfailed to open ctx.\n"); 
        goto error;
    } 
    if (EVP_PKEY_encrypt_init(ctx) <= 0) { 
        printf("ras_pubkey_encryptfailed to EVP_PKEY_encrypt_init.\n"); 
        goto error;
    }

    /*
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
    {
        printf("RSAEncrypt to EVP_PKEY_CTX_set_rsa_padding.\n"); 
        goto error;
    }*/

    #if 0
        //这种方式同样 RSA_size(evprsagen_) ==128 不能超过这个数；
        printf("data para:%d strlen:%d rsa_size:%d\n",datalen,strlen((char*)data),RSA_size(evprsagen_));
        if((ret=RSA_public_encrypt(datalen,data,encrypt_data,evprsagen_,RSA_PKCS1_OAEP_PADDING))<=0)
        {
            printf("ras_pubkey_encryptfailed to RSA_public_encrypt. ret=%d\n",ret);
            ERR_print_errors_fp(stderr);
            goto error;
        }
    #endif
    //这一步主要用来获取密文的最长长度
    if((ret=EVP_PKEY_encrypt(ctx, NULL, &encrypt_data_len, data,datalen)) <= 0)
    {
        printf("ras_pubkey_encrypt NULL failed to EVP_PKEY_encrypt. ret=%d\n",ret); 
        goto error;
    }

    //datalen=encrypt_data_len-2*21;/*可以加密的最大长度 */

    if((ret=EVP_PKEY_encrypt(ctx, encrypt_data, &encrypt_data_len, data, datalen)) <= 0) { 
        printf("ras_pubkey_encryptfailed to EVP_PKEY_encrypt. ret=%d\n",ret);
        ERR_print_errors_fp(stderr);
        goto error;
    }

    //printf("encrypt_data: %x, datalen:%d, encrypt_data_len:%d\n",encrypt_data,datalen,encrypt_data_len);
error:
    EVP_PKEY_CTX_free(ctx); 

    return encrypt_data_len; 
}

#define RSA_KEY_LENGTH 1024 
int cryptmsg::GenerateKeyFiles(const char* pub_key_file, const char* priv_key_file)
{ 
    //BN_GENCB *cb = BN_GENCB_new();
    RSA *rsa = RSA_new();
    int ret = 0;
    BIGNUM *bne=BN_new();
    ret=BN_set_word(bne,RSA_F4);
    ret = RSA_generate_key_ex(rsa,RSA_KEY_LENGTH,bne,NULL);

    //生成公钥文件
    BIO* bio_pub = BIO_new(BIO_s_file()); 
    if (NULL == bio_pub) { 
        printf("generate_key bio file new error!\n"); 
        return -1; 
    } 
    if (BIO_write_filename(bio_pub, (void *)pub_key_file) <= 0) {
        printf("BIO_write_filename error!\n"); return -1; 
    } 
    if (PEM_write_bio_RSAPublicKey(bio_pub, rsa) != 1) { 
        printf("PEM_write_bio_RSAPublicKey error!\n"); 
        return -1; 
    } 
    printf("Create public key ok!\n"); 
    BIO_free_all(bio_pub); // 生成私钥文件 
    BIO* bio_priv = BIO_new_file(priv_key_file, "w+"); 
    if (NULL == bio_priv) {
        printf("generate_key bio file new error2!\n"); 
        return -1; 
    }
    printf("passwd_:%s  len=%d\n",passwd_,strlen((char*)passwd_));
    if (PEM_write_bio_RSAPrivateKey(bio_priv, rsa, EVP_des_ede3_ofb(), (unsigned char *)passwd_, strlen((char*)passwd_), NULL, NULL) != 1) 
    { 
        printf("PEM_write_bio_RSAPublicKey error!\n"); 
        return -1; 
    }
    printf("Create private key ok!\n"); 
    BIO_free_all(bio_priv); 
    RSA_free(rsa); 
    return 0; 
}
/*用于短码进行hash */
int cryptmsg::Sha256Hash(const unsigned char* data,size_t datalen,unsigned char *hash_data,size_t hash_datalen)
{
    unsigned int val=0;
    EVP_MD_CTX *ctx=nullptr;
    if(hash_datalen<32)
    {
        GENERRORPRINT("not valid hash len",datalen,hash_datalen);
        return UTILNET_ERROR;
    }
    ctx=EVP_MD_CTX_create();
    
    if(EVP_DigestInit_ex(ctx, EVP_sha256(), NULL)<=0)
    {
        goto error;
    }

    if(EVP_DigestUpdate(ctx,data,datalen)<0)
    {
        goto error;
    }

    if(EVP_DigestFinal_ex(ctx,hash_data,&val)<0)
    {
        goto error;
    }


#ifdef OPENSSLOLDVERSION
//在新的openssl版本上不是用阿！
    EVP_MD_CTX_cleanup(ctx);
    EVP_MD_CTX_destroy(ctx);
#else
    EVP_MD_CTX_free(ctx);
#endif
    if(val!=32)
    {
        GENERRORPRINT("not valid hash len",datalen,hash_datalen);
        goto error;
    }

    return 32;

error:
#ifdef OPENSSLOLDVERSION
//在新的openssl版本上不是用阿！
    EVP_MD_CTX_cleanup(ctx);
    EVP_MD_CTX_destroy(ctx);
#else
    EVP_MD_CTX_free(ctx);
#endif

    return UTILNET_ERROR;
}

/*update:1 update；update：0 getresult */
int cryptmsg::Sha256HashUpdate(const unsigned char* data,size_t datalen,unsigned char *hash_data)
{
    unsigned int val=0;

    if(md_ctx_==nullptr)
    {
        md_ctx_=EVP_MD_CTX_create();
        if(EVP_DigestInit_ex(md_ctx_, EVP_sha256(), NULL)<=0)
        {
            goto error;
        }
    }

    if(EVP_DigestUpdate(md_ctx_,data,datalen)<0)
    {
        goto error;
    }

    if(hash_data==nullptr)
    {
        return datalen;
    }

    if(EVP_DigestFinal_ex(md_ctx_,hash_data,&val))
    {
        goto error;
    }

    if(val!=32)
    {
        GENERRORPRINT("not valid hash len",datalen,val);
        goto error;
    }

#ifdef OPENSSLOLDVERSION
//在新的openssl版本上不是用阿！
    EVP_MD_CTX_cleanup(md_ctx_);
    EVP_MD_CTX_destroy(md_ctx_);
#else
    EVP_MD_CTX_free(md_ctx_);
#endif

    return val;

error:
#ifdef OPENSSLOLDVERSION
//在新的openssl版本上不是用阿！
    EVP_MD_CTX_cleanup(md_ctx_);
    EVP_MD_CTX_destroy(md_ctx_);
#else
    EVP_MD_CTX_free(md_ctx_);
#endif
    
    GENERRORPRINT("error hash sha256",0,0);
    return UTILNET_ERROR;
}


