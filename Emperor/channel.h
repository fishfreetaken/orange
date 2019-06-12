
#define USERSUCCESS         0
#define USERFDREADFAIL      1
#define USERBUFDECRYPTFAIL  2
#define USERNEWFAIL         3
 

class channel;
class user{
public:
    //user(size_t uid,int fd);
    //user(size_t uid);
    user(int fd);

    int ParsePacket(transfOnPer *info);

    void GetOnlineFri(); // 从channel上获取当前online的朋友

    /*
        消息投递，1 对方在线；2 对方不在线，投递到一个消息队列中，检测等待上线；
    */
    int SendTo(transfOnPer *info);
    size_t SendToAll(transfOnPer *info);
    int SendMyself(transfOnPer *info);

    /*
        上下线管理，设置自身上下线状态，通知好友更改状态；
    */
    void SetOnLine();
    void setOffLine();

    bool WetherOnline();

private:
    int SendBuf(const char* buf,size_t len);

    int AddPartner(size_t uid); 
    int DelPartner(size_t uid);
private :
    size_t uid_; //使用者的uid
    std::unoredered_map<size_t,user*> partnermap; //将所有的使用一个map进行管理起来,没上线的nullptr

    char * buffer_;


    //channel *chmt; //用来进行当前所有在线用户的管理；单例模式
    int fd_;
};

/*
管理所有的成员，进行线程池管理，如果线程池满了，加入一个等待处理队列中去；
*/
class channel{
public:
    channel();

    static channel* GetInstance();

    /*传递给user去处理fd事件协议 */
    int UserReadProtocl(int fd);

    /*
        从文件，或者mysql加载已经注册的有效用户
    */
    //int LoadPartnerMap(); //没有必要所有的都进行加载，进行动态的增加用户和释放

    /*
        如果有一个partner登录进入服务器，进行验证加入群组；
        返回值：
    */
    user* UserOnlineReg(size_t &uid,int fd);
    void UserOfflineReg(size_t &uid);

private:
    int IncOneUser(user*p);
    int DelOneUser(int fd);

private: //能支持1千万的并发量就已经很不错了！
    std::unordered_map<int,user*> fdmapuser_;  //该chennl下所有的partner,由客户端文件描述符进行索引 fd->id->fd进行转发消息；

    std::unordered_map<size_t,user*> idmapuser_; //管理当前所有的user，由user id 进行索引；

/*通过参数配置的方式进行线程池的配置初始化 */
    std::thread *threadpool_;

};
