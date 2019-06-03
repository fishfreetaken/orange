

class user{
public:
    user(size_t uid,int fd);

    /*
        消息投递，1 对方在线；2 对方不在线，投递到一个消息队列中，检测等待上线；
    */
    int SendTo(transfOnPer *info);
    size_t SendToAll(transfOnPer *info);

    /*
        好友的管理
    */
    int AddPartner(size_t uid); 
    int DelPartner(size_t uid);


    int SendBuf(const char* buf,size_t len);
private :
    size_t uid_; //使用者的uid
    std::unoredered_map<size_t,user*> partermap; //将所有的使用一个map进行管理起来
    int fd_;
};

class channel{

public:
    channel(std::vector<int>&vi);

    Broadcast();

private:
    
};
