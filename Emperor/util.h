

#ifndef UTIL_HEADER_
#define UTIL_HEADER_


#include <sys/types.h>
#include <sys/socket.h>

#include<sys/time.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <stddef.h>

#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <iostream>

#include "genrandom.h"
#include <vector>
#include <thread>
#include <iostream>
#include <cstring>

#include <sys/epoll.h>
#include "protocal.h"
#include "log.h"
#include <assert.h>
#include <cassert>

#define UTILNET_SUCCESS       0
#define UTILNET_ERROR        -1
#define UTIL_POINTER_NULL    -2
#define UIIL_NOTFOUND        -3


#define SERVERLISTENIP    "192.168.1.105"
#define SERVERLISTENPORT  45821

#define NEEDAESCRYPT

int tcpGenericServer(const char *source_addr,int port);
int tcpGenericConnect(const char *source_addr,int port,const char *dest_ip,int dest_port);
void setNonBlock(int socket_fd);
int writeGenericSend(int fd,const char * buf,int len);
int readGenericReceive(int fd, char *buf,int len);

//void printTransfOnPer(transfOnPer *m)
void printTransfOnPer(transfOnPer *m,const char* from);
void printfPartner(transfPartner *m,const char *from);

int verifyCrcPayload(transfOnPer &m);
int genCrcPayload(transfOnPer &m);

uint64_t crc64(uint64_t crc, const unsigned char *s, uint64_t l);

void hexprint(unsigned char *str,int len);

/*return UTILNET_SUCCESS valid */
int checkMsgIdValid(uint32_t t);

void util_init();

void nolocks_localtime(struct tm *tmp, time_t t, time_t tz, int dst);

class Statustype{

public:
  static int Ok(){ return  static_cast<int>(mOk);}
  static int NotFound(){ return  static_cast<int>(mNotFound);}
  static int Corruption(){ return  static_cast<int>(mCorruption);}
  static int InvalidArgument(){ return  static_cast<int>(mInvalidArgument);}
  static int IOError(){ return  static_cast<int>(mIOError);}

private:
  enum enCode {
        mOk = 0,
        mNotFound = 1,
        mCorruption = 2,
        mNotSupported = 3,
        mInvalidArgument = 4,
        mIOError = 5
    };
};

class Status {
 public:
  // Create a success status.
  Status() noexcept : state_(nullptr) {}
  ~Status() { delete[] state_; }

  Status(const Status& rhs);
  Status& operator=(const Status& rhs);

  Status(Status&& rhs) noexcept : state_(rhs.state_) { rhs.state_ = nullptr; }
  Status& operator=(Status&& rhs) noexcept;

  // Return a success status.
  static Status OK() { return Status(); }

  // Return error status of an appropriate type.
  static Status NotFound(const char* msg, const char* msg2 ) {
    return Status(kNotFound, msg, msg2);
  }
  static Status Corruption(const char* msg, const char* msg2) {
    return Status(kCorruption, msg, msg2);
  }
  static Status NotSupported(const char* msg, const char* msg2) {
    return Status(kNotSupported, msg, msg2);
  }
  static Status InvalidArgument(const char* msg, const char* msg2) {
    return Status(kInvalidArgument, msg, msg2);
  }
  static Status IOError(const char* msg, const char* msg2) {
    return Status(kIOError, msg, msg2);
  }

  // Returns true iff the status indicates success.
  bool ok() const { return (state_ == nullptr); }

  // Returns true iff the status indicates a NotFound error.
  bool IsNotFound() const { return code() == kNotFound; }

  // Returns true iff the status indicates a Corruption error.
  bool IsCorruption() const { return code() == kCorruption; }

  // Returns true iff the status indicates an IOError.
  bool IsIOError() const { return code() == kIOError; }

  // Returns true iff the status indicates a NotSupportedError.
  bool IsNotSupportedError() const { return code() == kNotSupported; }

  // Returns true iff the status indicates an InvalidArgument.
  bool IsInvalidArgument() const { return code() == kInvalidArgument; }

  // Return a string representation of this status suitable for printing.
  // Returns the string "OK" for success.
  std::string ToString() const;

    enum enCode {
        mOk = 0,
        mNotFound = 1,
        mCorruption = 2,
        mNotSupported = 3,
        mInvalidArgument = 4,
        mIOError = 5
    };

 private:
    enum Code {
        kOk = 0,
        kNotFound = 1,
        kCorruption = 2,
        kNotSupported = 3,
        kInvalidArgument = 4,
        kIOError = 5
    };

  Code code() const {
    return (state_ == nullptr) ? kOk : static_cast<Code>(state_[4]);
  }

  Status(Code code, const char* msg, const char* msg2);
  static const char* CopyState(const char* s);

  // OK status has a null state_.  Otherwise, state_ is a new[] array
  // of the following form:
  //    state_[0..3] == length of message
  //    state_[4]    == code
  //    state_[5..]  == message
  const char* state_;
};

#endif

