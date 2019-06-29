
/*
    专门声明协议的头文件
*/

//统一的一个包头结构
/*
id 意义
0 心跳
1 好友发送信息
2 群消息
3 秘钥协商
4 服务端获取消息:个人的uid
5 朋友个人的信息包；
*/
#include <string>
#ifndef PAOROCAL_HEADER_
#define PAOROCAL_HEADER_


#pragma once

#define MSGHEART        0 /*心跳 */
#define MSGFRIEND       1 /*朋友转发包 */
#define MSGGROUP        2 /*群发包 */
#define MSGSECRET       3 /*加密协议包 */
#define MSGSERVERINFO   4 /*server请求初始化信息，包括个人信息，朋友信息加载 */
#define MSGFRIENDINFO   5 /*朋友信息更新通知包 */

#define PROTOCALSUCCESS 0
#define PROTOCALUIDMATCH 1
#define PROTOCALNOTFOUND 2

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define LOADCHARLEN 442

#define LOADPERSONLEN 44
#define LOADPERSONSIGNLEN 100

#define LOADAESCRPTYKEYLEN 120
#define LOADPERSONCRTPYLEN 40

typedef struct tarnsfercrptykeypacket{
    char secret[LOADPERSONCRTPYLEN];    /*个人密码应该不能超过30个数 */
    char key[LOADAESCRPTYKEYLEN];      /*对称加密的key */
} transfcrptykey;

typedef struct transferfriendspacket{
    size_t uid;
    size_t state; /*0下线，1上线 */
    char name[LOADPERSONLEN];
    char signature[LOADPERSONSIGNLEN];
} transfPartner;


typedef struct transferOnlinePersion{
    uint32_t id; //=2
    uint32_t size;
    size_t uid;  //db根据uid进行朋友索引
    size_t to;
    char buf[LOADCHARLEN];
    //char crc32[8];
    size_t crc32;
} transfOnPer;

#define STRUCTONPERLEN  sizeof(transfOnPer)
#define STRUCTONFRILEN  sizeof(transfPartner)

#endif
