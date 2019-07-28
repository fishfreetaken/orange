

#include "protocal.h"
#include "epollevent.h"
#include "cryptmsg.h"
#include <memory>
#include <queue>

class epollclienthandle: public epollhandlebase{

public:
    epollclienthandle(uint32_t uid,char *password);
    ~epollclienthandle();
    void StartConnect(const char* listenip,int port);

public:
    void ReadEvent(int tfd);
    void AcceptEvent(int tfd);
    void DelEvent(int tfd);
    int  WaitTimeOut();
    void TimeoutEvent(int tfd,timeevent *t);

private:
    int FormMsgAddToBuffer(uint32_t msgid,const char*buf,int len,size_t to);
    int MsgSendFromStd0();

    int  PushMsgToTerminal(transfOnPer *p);
    void AddNewFriends(transfOnPer *p);
    void InintialMyInfo(transfOnPer *p);

    int UserSendMsgPoll();

    void InitAesKeySend();

private :
    int fd_;
    PersonalInfo myinfo_;
    const uint32_t uiHeartInterval_;

    std::shared_ptr<epollevent> evp_;
    std::shared_ptr<Aescrypt> aescrypt_;
    std::shared_ptr<timeevent> tme_;

    std::map <uint32_t, PersonalInfo > myfriends_;
    uint32_t curdialog_; /*当前对话的伙伴 */

    int epolltimeout_;
    /*发送消息的循环缓冲数组 */

    std::shared_ptr<Protocal> recpacket_;

    /*最后一个发送完的自动进行释放 */
    std::queue<std::shared_ptr<Protocal> > sendqueue_;
};


#if 0
//使用Std的删除函数
std::shared_ptr<int> sp(new int[10], std::default_delete<int[]>());

//使用lambda表达式
std::shared_ptr<int> sp(new int[10], [](int *p) { delete[] p; });
#endif

