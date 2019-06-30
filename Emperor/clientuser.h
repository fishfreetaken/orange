

#include "protocal.h"
#include "epollevent.h"
#include "cryptmsg.h"
#include <memory>

#define  CLIENTREADBUFFERLEN  512
#define  CLIENTWRITEBUFFERLEN 512

#define VECTORBUFFERLEN 50

#if 0
typedef struct friendsstruct{
    int  fd;             //文件fd
    int  uid;
    size_t state;
    std::string name;   //名称
    std::string signature;
} friends;
#endif

class epollclienthandle: public epollhandlebase{

public:
    epollclienthandle(size_t uid);
    ~epollclienthandle();
    int StartConnect(const char* listenip,int port);

public:
    void ReadEvent(int tfd);
    void WriteEvent(int tfd); //标准输出端口输出
    void AcceptEvent(int tfd);
    void DelEvent(int tfd);
    int WaitTimeOut();

private:
    int FormMsgAddToBuffer(uint32_t msgid,const char*buf,int len);
    int MsgSendFromStd0();


    int PushMsgToTerminal(transfOnPer *p);
    void AddNewFriends(transfOnPer *p);
    void InintialMyInfo(transfOnPer *p);

    void UserSendMsgPoll();

    transfOnPer * SendBufferPush();
    transfOnPer * SendBufferPop();

private :
    int fd_;
    size_t uid_;
    std::shared_ptr<epollevent> evp_;

    std::shared_ptr<cryptmsg> crypt_;

    //std::vector<transfOnPer> recvbuffer_;
    std::vector<transfOnPer> sendbuffer_;

    char *readbuffer_;
    char *serverbuffer_;

    //int recv_pos_begin_;
    //int recv_pos_end_;

    int send_pos_begin_;
    int send_pos_end_;

    std::shared_ptr<timeevent> tme_;
    std::vector<int> timerollback_;
    transfPartner myinfo_;

    std::map<size_t, transfPartner> myfriends_;
    size_t curdialog_; /*当前对话的伙伴 */

    size_t recpackagecount_;
    size_t sendpackagecount_;
};


#if 0
//使用Std的删除函数
std::shared_ptr<int> sp(new int[10], std::default_delete<int[]>());

//使用lambda表达式
std::shared_ptr<int> sp(new int[10], [](int *p) { delete[] p; });
#endif

