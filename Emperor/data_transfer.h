
#ifndef DATAFINTRANSFER
#define DATAFINTRANSFER
class DataTransfer{
public:
    void ParseProcol();

    void TwoTransfer(int _in_fd,int _out_fd);

    void CurrentOnlineUsers(int _req_fd);

private:
    
};

#endif