#include <queue>
#include "protocal.h"

#define  CLIENTREADBUFFERLEN  512
#define  CLIENTWRITEBUFFERLEN 512

#define VECTORBUFFERLEN 50

class clientuser{

public:
    int SendMsg();
    int ReadParseMsg();

    int ReadSTD0();

private:
    int fd_;
};

class epollclienthandle:public epollhandlebase , timeeventbase{

typedef struct friendsstruct{
    int  fd;             //文件fd
    int  uid;
    size_t state;
    std::string name;   //名称
    std::string signature;
} friends;

public:
    epollclienthandle(size_t uid);
    void StartConnect(const char* listenip,int port);

public:
    void ReadEvent(int tfd);
    void WriteEvent(int tfd); //标准输出端口输出
    void AcceptEvent(int tfd);
    void DelEvent(int tfd);

public:
    TimeOutHandle();

private :
    int fd_;
    int uid_;
    std::shared_ptr<epollevent> evp_;

    std::vector<transfOnPer> recvbuffer_;
    std::vector<transfOnPer> sendbuffer_;

    char *readbuffer_;
    char *serverbuffer_;

    int recv_pos_begin_;
    int recv_pos_end_;

    int send_pos_begin_;
    int send_pos_end_;

    std::shared_ptr<timeevent> tme_;
    friends myinfo_;

    std::map<size_t,friends> myfriends_;
    size_t curdialog_; /*当前对话的伙伴 */
};


#if 0
//使用Std的删除函数
std::shared_ptr<int> sp(new int[10], std::default_delete<int[]>());

//使用lambda表达式
std::shared_ptr<int> sp(new int[10], [](int *p) { delete[] p; });
#endif

