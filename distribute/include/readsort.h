#ifndef READ_ADN_SORT_
#define READ_ADN_SORT_

#include "debug.h"

class ReadableFile
{
public:
    ReadableFile(std::string filename):
    filename_(filename)
    {
        fd_ = ::open(filename.c_str(), O_RDONLY );
        if (fd_ < 0) {
            //*result = nullptr;
            printf("readable file failed! fd=%d\n",fd_);
            return;
        }
        struct   stat   t; 
        stat   (filename_.c_str(),&t); 
        printf("%s size=%ldByte\n",filename_.c_str(),t.st_size);
        size_ = t.st_size;

        lseek(fd_,0,SEEK_SET);
        
    }
    ReadableFile(int fd):fd_(fd)
    {
        if(fd<=0)
        {
            printf("ReadableFile init fail!fd=%d\n",fd);
        }
    }

    ~ReadableFile()
    {
        const int close_result = ::close(fd_);
        if (close_result < 0) {
            printf("close failed\n");
        }
        fd_ = -1;
    }
    int readFile(unsigned char* scratch,size_t n)
    {
        int status;
        int read_size;
        while (true) {
            read_size = ::read(fd_, scratch, n);
            if (read_size < 0) {  // Read error.
                if (errno == EINTR) {
                    printf("readFile EINTR!\n");
                    continue;  // Retry
                }
                printf("readFile error=%d!\n",read_size);
                break;
            }
            break;
        }
        size_ -= read_size;
        return read_size;
    }
    size_t available()
    {
        return size_;
    }
private:
    int fd_;
    std::string filename_;
    size_t size_; 
};


#endif