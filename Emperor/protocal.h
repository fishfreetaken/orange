
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

#ifndef PAOROCAL_HEADER_
#define PAOROCAL_HEADER_

#include <stdatomic.h>
#include "util.h"
#include "cryptmsg.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#pragma once

#define MSGHEART        0 /*心跳 */
#define MSGFRIEND       1 /*朋友转发包 */
#define MSGGROUP        2 /*群发包 */
#define MSGSECRET       3 /*加密协议包 */
#define MSGSERVERINFO   4 /*server请求初始化信息，包括个人信息，朋友信息加载 */
#define MSGFRIENDINFO   5 /*朋友信息更新通知包 */


class Protocaltype{
public:
    static std::string toString(uint16_t &m)
    {
        switch(m)
        {
            case mHeart:
                return std::string("Heart msg");
            break;
            case mFriMsg:
                return ;
            break;
            case mGroMsg:
                return std::string("Group msg");
            break;
            case mSecret:
                return std::string("Secret msg");
            break;
            case mUpdateInfo:
                return std::string("mUpdateInfo msg");
            break;
            default:
                return std::string("NOT a valid msg id");
                break;
        }
    }

    static uint16_t Heart(){ return  static_cast<uint16_t>(mHeart);}
    static uint16_t FriendMsg(){ return  static_cast<uint16_t>(mFriMsg);}
    static uint16_t GroupMsg(){ return  static_cast<uint16_t>(mGroMsg);}
    static uint16_t Secret(){ return  static_cast<uint16_t>(mSecret);}
    static uint16_t UpdateInfo(){ return  static_cast<uint16_t>(mUpdateInfo);}

    virtual void MsgProc(uint16_t &m);

private:
    enum enType {
        mHeart=0,
        mFriMsg,
        mGroMsg ,
        mSecret ,
        mUpdateInfo
    };

    std::vector<std::function> vef_;
};


class PersonalInfo{
public:
    uint32_t uiId;
    uint8_t ucSex;
    uint8_t ucAge;
    uint8_t ucNation;
    uint8_t ucState;
    std::string sPassword; /*sha后的密码 */
    std::string sName;
    std::string sSignature;
public:
    PersonalInfo():
    uiId(0),
    ucSex(0),
    ucAge(0),
    ucNation(0),
    ucState(0)
    {
    }
};

#define PROTOCALVERSION     0x0001
/*包头的版本号 */
#define PROTOCALHEADERLEN   32

#define PROTOCALHEADERFLAG  0X02

#define PROTOCALHEARDBUF   0X15269CDF15
/*主协议体 */
class Protocal{
    struct Header
    {
        uint16_t  ucFlag;       //=2
        uint16_t  usVer;        //版本信息
        uint16_t  usType;       //包类型
        uint16_t  usLength;     //还是要计算长度的，对于消息体比较小的，直接使用一些填充字符进行扩展
        uint32_t  uiSender;     //发送者id号
        uint32_t  uiReceiver;   //接收者id号
        size_t    ulSeq;        //包序号
        size_t    ulTimestamp;  //时间戳
    };
public:
/*发送 */
    Protocal()
    {
    }

    Protocal(uint32_t &type,uint32_t to,uint32_t from):
    buf_(nullptr),
    buflen_(0),
    ulCrc32_(0)
    {
        header_.uiSender=from;
        header_.uiReceiver=to;
        header_.usType=type;
        header_.usVer=PROTOCALVERSION;
        header_.ucFlag=PROTOCALHEADERFLAG;
        header_.usLength=0;
        header_.ulSeq=0;
    }

    ~Protocal()
    {
        if(buf_!=nullptr)
        {
            delete [] buf_;
        }
    }

    Protocal(Protocal& m):
    header_(m.header_),
    buf_(m.buf_),
    buflen_(m.buflen_)
    {
        m.buf_=nullptr;
        m.buflen_=0;
    }

    static void DebugShow(Protocal &p)
    {
        printf("--------------------------------------------\n");
        printf("current send:%u\n",sendseq_.load());
        printf("current recv:%u\n",recvseq_.load());
        printf("Flag:0x%x Ver:0x%x\nType:%s\nSender:%u\nReceiver:%u\nSeq:%u\nLen:%u\n",\
        p.header_.ucFlag,p.header_.usVer,Protocaltype::toString(p.header_.usType).c_str(),p.header_.uiSender,\
        p.header_.uiReceiver,p.header_.ulSeq,p.header_.usLength);

        #ifdef NEEDRECORD
            LogRecord();
        #endif
    }

    /*默认载体都是字符串的类型 */
    void InitBuf( std::string &st, Basecrypt *crypt)
    {
        int len=st.size();
        if((len+8)%16!=0)
        {
            header_.usLength=len+16;
        }else{
            header_.usLength=len;
        }
        /*分配新的空间 */
        BufNew();

        GenTimeStamp();

        sendseq_++;
        header_.ulSeq=sendseq_.load();

        /*拷贝发送数据 */
        std::memcpy(buf_,st.c_str(),len);
        ulCrc32_=CrcGenCheck();
        std::memcpy(buf_+header_.usLength-8,&ulCrc32_,8);

        crypt->Encrypt(buf_,buflen_,buf_);
    }

    uint16_t GetPkgType(){return header_.usType;}
    uint32_t GetPkgSender(){return header_.uiSender;}
    uint32_t GetPkgReceiver(){return header_.uiReceiver;}
    uint32_t GetPkgSendSeq(){return sendseq_.load();}
    uint32_t GetPkgRecvSeq(){return recvseq_.load();}

    int SendPkg(int fd_){
        /*加密 */
        int ret=::write(fd_,(char*)&header_,PROTOCALHEADERLEN);
        if(ret<0)
        {
            return Statustype::IOError();
        }
        return write(fd_,buf_,header_.usLength);
    }

    int ReceivePkg(int fd_, Basecrypt  *crypt){
        std::memset(&header_,0,sizeof(struct Header));

        if(::read(fd_,&header_,PROTOCALHEADERLEN)!=Status::mOk) {
            if ((errno == EINTR)||(errno == EAGAIN))
            {
                return Statustype::IOError();
            }
            LogError("read header failed errno: %s",strerror(errno));
            return Statustype::IOError();
        }

        BufNew();

        if(::read(fd_,(char*)buf_,header_.usLength)!=Status::mOk) {
            LogError("read buf body failed errno: %s",strerror(errno));
            return Statustype::IOError();
        }

        if((header_.ucFlag!=PROTOCALHEADERFLAG)||(header_.usVer!=PROTOCALVERSION))
        {
            LogError("ucFlag:%x usVer:%x mismatch",header_.ucFlag,header_.usVer);
        }

        recvseq_++;

        crypt->Decrypt(buf_,header_.usLength,buf_);

        std::memcpy(buf_+header_.usLength-8,&ulCrc32_,8);

        return Statustype::Ok();
    }
    int CrcCheck()
    {
        size_t tmp=CrcGenCheck();
        return memcmp(&tmp,&ulCrc32_,8);
    }

private:
    void BufNew()
    {
        if(buflen_>=header_.usLength)
        {
            std::memset(buf_,0,buflen_);
            return ;
        }else{
            if(buf_!=nullptr)
            {
                delete [] buf_;
            }
        }
        try{
            buf_=new unsigned char[header_.usLength];
        }catch(...)
        {
            LogError("new buf failed len: %u\n",header_.usLength);
            throw "Protocal new char failed!";
        }
        buflen_=header_.usLength;

        std::memset(buf_,0,header_.usLength);
    }

    size_t CrcGenCheck()
    {
        size_t tmp=0;
        tmp=crc64(0,(unsigned char*)&header_,PROTOCALHEADERLEN);
        tmp=crc64(tmp,buf_,header_.usLength);
        //memcmp(&tmp,&ulCrc32_,8)
        return tmp;
    }

    void GenTimeStamp()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        header_.ulTimestamp=tv.tv_sec;
    }

private:
    struct Header header_;
    unsigned char*buf_; /*数据体 */
    uint16_t buflen_;
    size_t ulCrc32_;

    static atomic_uint sendseq_;
    static atomic_uint recvseq_;
};


#endif
