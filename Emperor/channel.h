


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

private:
    int SendBuf(const char* buf,size_t len);

    int AddPartner(size_t uid); 
    int DelPartner(size_t uid);
private :
    size_t uid_; //使用者的uid
    std::unoredered_map<size_t,user*> partnermap; //将所有的使用一个map进行管理起来,没上线的nullptr
    
    //channel *chmt; //用来进行当前所有在线用户的管理；单例模式
    int fd_;
};

/*
管理所有的成员
*/
class channel{
public:
    channel();

    static channel* GetInstance();

    /*
        该频道所有用户发送消息；
        返回值：发送的用户数；
        入参：无；
    */
    size_t Broadcast();

    int SendToOne(size_t uid, const char * buf,size_t len);

    /*
        从文件，或者mysql加载已经注册的有效用户
    */
    int LoadPartnerMap();

    user *operator[](size_t &t);

    /*
        如果有一个partner登录进入服务器，进行验证加入群组；
        返回值：
    */

    user* UserOnlineReg(size_t &uid,int fd);
    void UserOfflineReg(size_t &uid);

private:
    int IncOnePartner(user*p);
    int DelOnePartner(int fd);

private:
    std::unoredered_map<size_t,user*> usermap;//该chennl下所有的partner,由文件描述符进行索引

};
