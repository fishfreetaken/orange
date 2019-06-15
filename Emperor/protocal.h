
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

#define MSGHEART        0
#define MSGFRIEND       1
#define MSGGROUP        2
#define MSGSECRET       3
#define MSGSERVERINFO   4
#define MSGFRIENDINFO   5

#define PROTOCALSUCCESS 0
#define PROTOCALUIDMATCH 1
#define PROTOCALNOTFOUND 2

#define LOADCHARLEN 160

#define LOADPERSONLEN 50
#define LOADPERSONSIGNLEN 100

typedef struct transferfriendspacket{
    size_t uid;
    size_t state; /*0下线，1上线 */
    char name[LOADPERSONLEN];
    char signature[LOADPERSONLEN];
} transfPartner;

typedef struct transferOnlinePersion{
    uint id; //=2
    size_t uid;
    size_t to;
    size_t size;
    char buf[LOADCHARLEN];
    char crc32[4];
} transfOnPer;

#define STRUCTONPERLEN  sizeof(transfOnPer)

