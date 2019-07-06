#include <openssl/rsa.h>
#include <openssl/aes.h>
#include <openssl/pem.h>
#include <openssl/rand.h> 
#include <openssl/evp.h>
#include <openssl/err.h>

#include <iostream> 
#include <cstring>

extern void hexprint(unsigned char *str,int len);

#define PUBLIC_KEY_FILE "rsapub.pem" 
#define PRIVATE_KEY_FILE "rsapriv.pem" 
#define RSA_KEY_LENGTH 2048 
#define RSA_PRIKEY_PSW "12345678" 

#define DATATESTLEN 2048

unsigned char g_EncryptData[DATATESTLEN]={0};
unsigned char g_DencryptData[DATATESTLEN]={0};

int generate_key_files(const char* pub_key_file, const char* priv_key_file, const unsigned char* passwd, int passwd_len)
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

    if (PEM_write_bio_RSAPrivateKey(bio_priv, rsa, EVP_des_ede3_ofb(), (unsigned char *)passwd, passwd_len, NULL, NULL) != 1) 
    { 
        printf("PEM_write_bio_RSAPublicKey error!\n"); 
        return -1; 
    } 
    printf("Create private key ok!\n"); 
    BIO_free_all(bio_priv); RSA_free(rsa); 
    return 0; 
}


EVP_PKEY* open_public_key(const char* pub_key_file,const unsigned char *data,unsigned char *encrypt_data )
{ 
    EVP_PKEY* key = NULL; 
    RSA *rsa = NULL;
    OpenSSL_add_all_algorithms(); 
    BIO *bio_pub = BIO_new(BIO_s_file());; 

    BIO_read_filename(bio_pub, pub_key_file); 
    if (NULL == bio_pub) {
            printf("open_public_key bio file new error!\n"); 
    return NULL; } 
    rsa = PEM_read_bio_RSAPublicKey(bio_pub, NULL, NULL, NULL); 
    
    if (rsa == NULL) { 
        printf("open_public_key failed to PEM_read_bio_RSAPublicKey!\n"); 
        BIO_free(bio_pub); 
        RSA_free(rsa); 
        return NULL; 
    } 
    printf("open_public_key success to PEM_read_bio_RSAPublicKey!\n"); 
    
    #if 0
    int klen = RSA_size(rsa);
    int ret=0;

    if((ret=RSA_public_encrypt(strlen((char*)(data)),data,encrypt_data,rsa,RSA_PKCS1_OAEP_PADDING))<0)
    {
        int err = ERR_peek_error();
        ERR_print_errors_fp(stderr);
        printf("RSA_public_encrypt failed to ret=%d! err:%d\n",ret,err); 
    }else{
        printf("RSA_public_encrypt success to ret=%d!\n",ret);
        return 0;
    }
    #endif

    key = EVP_PKEY_new(); 
    if (NULL == key) { 
        printf("open_public_key EVP_PKEY_new failed\n"); 
        RSA_free(rsa); return NULL; 
    } 
    EVP_PKEY_assign_RSA(key, rsa); 
    return key; 
}


EVP_PKEY* open_private_key(const char* priv_key_file, const unsigned char *passwd)
{ 
    EVP_PKEY* key = NULL; 
    RSA *rsa = RSA_new(); 
    OpenSSL_add_all_algorithms(); 
    BIO* bio_priv = NULL; 
    bio_priv = BIO_new_file(priv_key_file, "rb"); 
    if (NULL == bio_priv) { 
        printf("open_private_key bio file new error!\n"); 
        return NULL; 
    } 
    rsa = PEM_read_bio_RSAPrivateKey(bio_priv, &rsa, NULL, (void *)passwd); 
    if (rsa == NULL) { 
        printf("open_private_key failed to PEM_read_bio_RSAPrivateKey!\n"); 
        BIO_free(bio_priv); 
        RSA_free(rsa); 
        ERR_print_errors_fp(stderr);
        return NULL; 
    }

    printf("open_private_key success to PEM_read_bio_RSAPrivateKey!\n"); 

    key = EVP_PKEY_new(); 
    if (NULL == key) { 
        printf("open_private_key EVP_PKEY_new failed\n"); 
        RSA_free(rsa); 
        return NULL; 
    } 
    EVP_PKEY_assign_RSA(key, rsa); 
    return key; 
} 

int rsa_key_encrypt(EVP_PKEY *key, const unsigned char* data, size_t len, unsigned char *encrypt_data, size_t &encrypt_data_len)
{ 
    EVP_PKEY_CTX *ctx = NULL; 
    ENGINE *eng;
    //OpenSSL_add_all_ciphers(); 
    ctx = EVP_PKEY_CTX_new(key, NULL); 
    if (NULL == ctx) { 
        printf("ras_pubkey_encryptfailed to open ctx.\n"); 
        EVP_PKEY_free(key); 
        return -1; 
    } 
    if (EVP_PKEY_encrypt_init(ctx) <= 0) { 
        printf("ras_pubkey_encryptfailed to EVP_PKEY_encrypt_init.\n"); 
        EVP_PKEY_free(key); 
        return -1; 
    }

    /*
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, EVP_PKEY_OP_ENCRYPT) <= 0)
    {
        printf("EVP_PKEY_CTX_set_rsa_padding failed to EVP_PKEY_encrypt.\n"); 
        return -1;
    }*/
    /*
    if (EVP_PKEY_CTX_ctrl(ctx, -1, EVP_PKEY_OP_ENCRYPT,
                          EVP_PKEY_CTRL_CMS_ENCRYPT, 0, ri) <= 0) {
        printf("error");
    }
    */

    int ret=0;

    if((ret=EVP_PKEY_encrypt(ctx, NULL, &encrypt_data_len, data, len)) <= 0)
    {
        printf("ras_pubkey_encrypt NULL failed to EVP_PKEY_encrypt. ret=%d\n",ret); 
        ERR_print_errors_fp(stderr);
         EVP_PKEY_CTX_free(ctx); EVP_PKEY_free(key); 
        return -1; //EVP_PKEY_OP_ENCRYPT
    }

    printf("encrypt_data %s size:%d encrypt_data_len:%d\n",encrypt_data,strlen((char*)encrypt_data),encrypt_data_len);

    //encrypt_data =(unsigned char*) OPENSSL_malloc(encrypt_data_len);

    if((ret=EVP_PKEY_encrypt(ctx, encrypt_data, &encrypt_data_len, data, strlen((char*)data))) <= 0) { 
        printf("ras_pubkey_encryptfailed to EVP_PKEY_encrypt. ret=%d\n",ret); 
        ERR_print_errors_fp(stderr);
        EVP_PKEY_CTX_free(ctx); EVP_PKEY_free(key); 
        return -1; 
    }
    EVP_PKEY_CTX_free(ctx); 
    EVP_PKEY_free(key); 
    return 0; 
} 
int rsa_key_decrypt(EVP_PKEY *key, const unsigned char *encrypt_data, size_t encrypt_data_len, unsigned char *decrypt_data, size_t &decrypt_data_len)
{ 
    EVP_PKEY_CTX *ctx = NULL;
    OpenSSL_add_all_ciphers(); 
    ctx = EVP_PKEY_CTX_new(key, NULL); 
    if (NULL == ctx) 
    { 
        printf("ras_prikey_decryptfailed to open ctx.\n"); 
        EVP_PKEY_free(key); 
        return -1; 
    } 
    if (EVP_PKEY_decrypt_init(ctx) <= 0) { 
        printf("ras_prikey_decryptfailed to EVP_PKEY_decrypt_init.\n"); 
        EVP_PKEY_free(key); 
        return -1; 
    } 
    if (EVP_PKEY_decrypt(ctx, decrypt_data, &decrypt_data_len, encrypt_data, encrypt_data_len) <= 0) { 
        printf("ras_prikey_decryptfailed to EVP_PKEY_decrypt.\n"); 
        EVP_PKEY_CTX_free(ctx); 
        EVP_PKEY_free(key); 
        return -1; 
    } 
    EVP_PKEY_CTX_free(ctx); 
    EVP_PKEY_free(key); 
    return 0; 
} 

int rsa_test(const unsigned char* data, size_t len, char* encrypt_data, size_t encrypt_data_len, char* decrypt_data, size_t decrypt_data_len)
{ 
    //generate_key_files(PUBLIC_KEY_FILE, PRIVATE_KEY_FILE,(const unsigned char *)RSA_PRIKEY_PSW, strlen(RSA_PRIKEY_PSW)); 

    EVP_PKEY *pub_key = open_public_key(PUBLIC_KEY_FILE,data,(unsigned char *)encrypt_data); 
    EVP_PKEY *pri_key = open_private_key(PRIVATE_KEY_FILE, (const unsigned char *)RSA_PRIKEY_PSW);

    int ret = rsa_key_encrypt(pub_key, data, len, (unsigned char *)encrypt_data, encrypt_data_len); 
    if (ret != 0) { 
        printf("rsa encrypt failed!\n"); return ret; 
    } else { 
        printf("rsa encrypt success!\n");
    } 
    ret = rsa_key_decrypt(pri_key, (const unsigned char *)encrypt_data, encrypt_data_len, (unsigned char *)decrypt_data, decrypt_data_len); 
    if (ret != 0) { 
        printf("rsa decrypt failed!\n"); 
    } else { 
        printf("rsa decrypt success!\n"); 
        printf("encrypt data:%s\n", encrypt_data); 
        printf("decrypt data:%s\n", decrypt_data); 
    } 
    return ret; 
}


int rsatest()
{
    printf("\nRSA_generate_key_ex TESTING...\n\n");
    RSA *rsa = RSA_new();
    int ret = 0;
    BIGNUM *bne=BN_new();
    ret=BN_set_word(bne,RSA_F4);
    ret = RSA_generate_key_ex(rsa,512,bne,NULL);

    unsigned char plain[512]="Hello world!";
    unsigned char cipper[512]={0};
    unsigned char newplain[512]={0};
    size_t outl=512;
    size_t outl2;
    printf("%s\n", plain);
    for(size_t i =0;i<strlen((char*)plain);i++){
        printf("%02x ",plain[i]);
    }
    printf("\n---------------\n");
    outl=RSA_public_encrypt(strlen((char*)plain),plain,cipper,rsa,RSA_PKCS1_OAEP_PADDING);
    for(size_t i =0;i<outl;i++){
        printf("%02x ",cipper[i]);
        if((i+1)%10 ==0) printf("\n");
    }
    printf("\n");
    outl2=RSA_private_decrypt(outl,cipper,newplain,rsa,RSA_PKCS1_OAEP_PADDING);
    printf("-----------------\n%s\n", newplain);
    for(size_t i =0;i<outl2;i++) {
        printf("%02x ",newplain[i]);
    }
    printf("\n");
    return 0;
}
 

int myrsatest(unsigned char* data, size_t len, unsigned char* encrypt_data, size_t encrypt_data_len,unsigned char* decrypt_data, size_t decrypt_data_len)
{
    FILE *file;
    if((file=fopen("./openssllib/pub.key","r"))==NULL){
        perror("open key file error");
        return -1; 
    } 
    RSA *p_rsa;

    if((p_rsa=PEM_read_RSAPublicKey(file,NULL,NULL,NULL))==NULL){
        ERR_print_errors_fp(stderr);
        return -1;
    }

    int rsa_len,num;
    //rsa_len=RSA_size(p_rsa);
    //unsigned char *outstr=(unsigned char *)malloc(rsa_len+1);
    //memset(outstr,0,rsa_len+1);
     num = RSA_public_encrypt(strlen((char*)data),data,encrypt_data,p_rsa,RSA_PKCS1_OAEP_PADDING);
    if(num == -1)
    {
        ERR_print_errors_fp(stderr);
        printf("Got error on enc/dec!\n");
        return num;
    }
    return num;

}

/*后期随机生成一个 */
static const unsigned char key16[16] = {
    0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
    0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x12
};

static const unsigned char key32[32] = {
    0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
    0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x12,
    0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x12, 0x34,
    0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x12, 0x34, 0x56
};


#define MAX_BLOCK_SIZE 128

int myAesTestInt(unsigned char* data, size_t &len)
{
    AES_KEY aes_ks3;
    AES_KEY aes_ksd3;
    unsigned char iv[MAX_BLOCK_SIZE]="jofjwoiiewi23";
    unsigned char iv2[ MAX_BLOCK_SIZE]="jofjwoiiewi23";
    AES_set_encrypt_key(key32, 256, &aes_ks3);

    AES_cbc_encrypt(data,g_EncryptData,strlen((char*)data),&aes_ks3,iv,1);

    //AES_encrypt(data,g_EncryptData,&aes_ks3);

    AES_set_decrypt_key(key32, 256, &aes_ksd3);

    AES_cbc_encrypt(g_EncryptData,g_DencryptData,strlen((char*)g_EncryptData),&aes_ksd3,iv2,0);
    //AES_decrypt(g_EncryptData,g_DencryptData,&aes_ksd3);

    return 1;
}

int generalCheckout(unsigned char *data ,size_t len)
{
    int ret=0;

    printf("origin str: \n%s\n",data);

    ret=myAesTestInt((unsigned char*)data,len);

    hexprint(g_EncryptData,strlen((char *)g_EncryptData));
    printf("decrypt_data_len:%d--buf：\n%s\n",strlen((char *)g_DencryptData),g_DencryptData);


    ret=strncmp((char*)data,(char*)g_DencryptData,len);
    if(ret!=0)
    {
        printf("generalCheckout test failed!\n");
        ERR_print_errors_fp(stderr);
        return -1;
    }
    return ret;
}
