#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h> 
#include <openssl/evperr.h>
#include <openssl/evp.h>
#include <iostream> 
#include <cstring>

#define PUBLIC_KEY_FILE "rsapub.pem" 
#define PRIVATE_KEY_FILE "rsapriv.pem" 
#define RSA_KEY_LENGTH 1024 
#define RSA_PRIKEY_PSW "123" 

int generate_key_files(const char* pub_key_file, const char* priv_key_file, const unsigned char* passwd, int passwd_len)
{ 
    RSA *rsa = NULL; rsa = RSA_generate_key(RSA_KEY_LENGTH, RSA_F4, NULL, NULL); 
    if (rsa == NULL) { 
        printf("RSA_generate_key error!\n"); 
        return -1; 
    } 
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

EVP_PKEY* open_public_key(const char* pub_key_file)
{ 
    EVP_PKEY* key = NULL; 
    RSA *rsa = NULL; OpenSSL_add_all_algorithms(); 
    BIO *bio_pub = BIO_new(BIO_s_file());; 
    BIO_read_filename(bio_pub, pub_key_file); 
    if (NULL == bio_pub) {
            printf("open_public_key bio file new error!\n"); 
    return NULL; } 
    rsa = PEM_read_bio_RSAPublicKey(bio_pub, NULL, NULL, NULL); 
    if (rsa == NULL) { 
        printf("open_public_key failed to PEM_read_bio_RSAPublicKey!\n"); 
        BIO_free(bio_pub); RSA_free(rsa); return NULL; } 
        printf("open_public_key success to PEM_read_bio_RSAPublicKey!\n"); 
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
    OpenSSL_add_all_ciphers(); 
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
    if (EVP_PKEY_encrypt(ctx, encrypt_data, &encrypt_data_len, data, len) <= 0) { 
        printf("ras_pubkey_encryptfailed to EVP_PKEY_encrypt.\n"); 
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

int rsa_test(const unsigned char* data, size_t& len, char* encrypt_data, size_t& encrypt_data_len, char* decrypt_data, size_t& decrypt_data_len)
{ 
    generate_key_files(PUBLIC_KEY_FILE, PRIVATE_KEY_FILE,(const unsigned char *)RSA_PRIKEY_PSW, strlen(RSA_PRIKEY_PSW)); 
    EVP_PKEY *pub_key = open_public_key(PUBLIC_KEY_FILE); 
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
