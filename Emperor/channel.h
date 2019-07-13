

#include "protocal.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include "cryptmsg.h"
#include "filehandle.h"

#define USERSUCCESS         0
#define USERFDREADFAIL      -1 /*fd读取失败 */
#define USERBUFDECRYPTFAIL  -2 /*包解密失败 ,或者crc校验错误*/
#define USERNEWFAIL         -3 /*开辟内存失败 */
#define USERIDMISMATCH      -4 /*与本人id并不相等 */
#define USERNOTMYFIREND     -5 /*朋友中并没有找到这个人 */
#define USERPOINTNULL       -6 /*入参指针为空 */
#define USERNOTFOUNDID      -7  /*查无此人 */
#define USERFAILED          -8  /*daily failed */
#define USERNOTINITIALZE    -9  /* */
 
class channel;
class user{
public:
    user(int fd,size_t uid);
    ~user();

    int ParsePacket(int tfd);
    size_t GetUid() const;
    int GetFd() const;
    int InitialMyInfo(transfOnPer &m,transfPartner &s,const channel *p);

    int SendMyself(const transfOnPer *info);

    void CurrentStat();

private:
    /*
        消息投递，1 对方在线；2 对方不在线，投递到一个消息队列中，检测等待上线；
    */
    int SendTo();
    size_t SendToAll();
    
    void HeartBeat();

    int InformPartnerOnline(user *handle, const size_t state);

    int LoadUserInfoFromDb(size_t &uid, std::vector<transfPartner>& p);

    int GenericSend(uint32_t id, const size_t &dest,const char*buf,size_t size,size_t from);

    int GenericSend(const transfOnPer*info );
    
private:
    size_t uid_; //使用者的uid
    /*内部需要维护一个朋友队列吗? */
    std::unordered_map<size_t,std::shared_ptr<user> > partnermap_; //将所有的使用一个map进行管理起来,没上线的nullptr，不应该内部维护的一个朋友队列

    transfOnPer recvpacket_;
    transfOnPer sendpacket_;

    int fd_;

    transfPartner myinfo_;
    std::shared_ptr<cryptmsg> aescrpty_;
    char *crpty_;    /*用户登陆密码 */

    size_t recpackagecount_;
    size_t sendpackagecount_;
    std::vector<size_t> vecrecvpackcc_; 
};

/*
管理所有的成员，进行线程池管理，如果线程池满了，加入一个等待处理队列中去；
*/
class channel{
public:
    channel();
    ~channel();

    //static channel* GetInstance();

    /*传递给user去处理fd事件协议 */
    int UserReadProtocl(int fd);

    std::shared_ptr<user> FindByUid(size_t&uid) const;

    int CheckUidIsInDb(transfOnPer &m,transfPartner &s);
    /*
        从文件，或者mysql加载已经注册的有效用户
    */
    //int LoadPartnerMap(); //没有必要所有的都进行加载，进行动态的增加用户和释放
    void UserRemove(int &tfd); //清楚用户

    const std::shared_ptr<filehandle> GetFileHD() const;

    void ServerStatPrint();

private:

    transfOnPer sendpacket_;
    unsigned char *rsarecvbuf_; /*1024 bytes */


private: //能支持1bai万的并发量就已经很不错了！
    std::unordered_map<int,std::shared_ptr<user> > fdmapuser_;  //该chennl下所有的partner,由客户端文件描述符进行索引 fd->id->fd进行转发消息；

    std::unordered_map<size_t,std::shared_ptr<user> > idmapuser_; //管理当前所有的user，由user id 进行索引；

    std::shared_ptr<cryptmsg> rsacrpty_;

    std::shared_ptr<filehandle> filehd_;

};

