#ifndef RANDOM_KEY_GENERATOR_SINGLE_WORKER_
#define RANDOM_KEY_GENERATOR_SINGLE_WORKER_

#include "debug.h"


constexpr const size_t kWritableFileBufferSize = 65536;
class WritableFile
{
public:
    WritableFile(const std::string& filename,int fd):
    fd_(fd),
    pos_(0),
    filename_(filename)
    {

    }

    ~WritableFile()
    {
        #ifdef OPENPRINT
            printf("~WritableFile\n");
        #endif

        if (fd_ >= 0) {
            // Ignoring any potential errors
            Close();
        }
    }

    int Close() {
        int status = FlushBuffer();
        const int close_result = ::close(fd_);
        if (close_result < 0) {
            printf("close failed\n");
        }
        fd_ = -1;
        return status;
    }

    int Append(const char *data,size_t len) {
        size_t write_size = len;
        const char* write_data = data;

        if (write_size == 0) {
            return -1;
        }

        // Fit as much as possible into buffer.
        size_t copy_size = std::min(write_size, kWritableFileBufferSize - pos_);
        std::memcpy(buf_ + pos_, write_data, copy_size);
        write_data += copy_size;
        write_size -= copy_size;
        pos_ += copy_size;

        // Can't fit in buffer, so need to do at least one write.
        int status = FlushBuffer();
        if (!status) {
            return status;
        }

        // Small writes go to buffer, large writes are written directly.
        if (write_size < kWritableFileBufferSize) {
            std::memcpy(buf_, write_data, write_size);
            pos_ = write_size;
            return -2;
        }
        return WriteUnbuffered(write_data, write_size);
    }

    int WriteUnbuffered(const char* data, size_t size) {
        while (size > 0) {
            ssize_t write_result = ::write(fd_, data, size);
            
            if (write_result < 0) {
                if (errno == EINTR) {
                    #ifdef OPENPRINT
                        printf("system call too slow!\n");
                    #endif
                    
                    continue;  // Retry
                }
                return -1;
            }
            data += write_result;
            size -= write_result;
        }
        return 0;
    }

    int readFile(size_t n, unsigned char* scratch)
    {
        int result;
        result= ::fsync(fd_);
        if(result)
        {
            printf("sync error %d \n",result);
        }

        struct   stat   t; 
        stat(filename_.c_str(),&t); 
        printf("%s size=%ld\n",filename_.c_str(),t.st_size);

        lseek(fd_,0,SEEK_SET);

        int status;
        ::ssize_t read_size;
        while (true) {
            read_size = ::read(fd_, scratch, n);
            if (read_size < 0) {  // Read error.
                if (errno == EINTR) {
                    continue;  // Retry
                }
                break;
            }
            break;
        }
        
        for(int i=0;i<read_size;i++)
        {
            printf("%x ",scratch[i]);
        }
        printf("read_size=%ld\n",read_size);
        
    }
    int SyncFd(int fd) {
        
    }
private:
    int FlushBuffer() {
        int status = WriteUnbuffered(buf_, pos_);
        pos_ = 0;
        return status;
    }
    int fd_;
    size_t pos_;
    char buf_[kWritableFileBufferSize];
    std::string filename_;

};

class env
{
public:
    ~env()
    {
        
    }
    int NewWritableFile(const std::string& filename,WritableFile** result)
    {
        int fd = ::open(filename.c_str(), O_TRUNC | O_WRONLY | O_CREAT, 0644);
        if (fd < 0) {
            //*result = nullptr;
            return -1;//PosixError(filename, errno);
        }
        *result = new WritableFile(filename, fd);
        return 0;
    }
    int NewAppendableFile(const std::string& filename,
                           WritableFile** result) {
        int fd = ::open(filename.c_str(), O_RDWR | O_APPEND | O_CREAT, 0644);
        if (fd < 0) {
            *result = nullptr;
            return -1;
        }

        *result = new WritableFile(filename, fd);
        return 0;
    }
};

class timebench
{

public:
    timebench():
    begin_(0),
    end_(0)
    {

    }
    long long ustime(void) {
        struct timeval tv;
        long long ust;

        gettimeofday(&tv, NULL);
        ust = ((long)tv.tv_sec)*1000000;
        ust += tv.tv_usec;
        return ust;
    }
    
    void begin()
    {
        begin_=ustime();
        
    }
    void end()
    {
        end_=ustime();
        long long dev=end_-begin_;
        printf("elapse time:%lld.%llds\n",dev/1000000,dev%1000000);
        begin_=end_;
    }
private:
    long long begin_;
    long long end_;

};



#endif