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
    ~ReadableFile()
    {
        const int close_result = ::close(fd_);
        if (close_result < 0) {
            printf("close failed\n");
        }
        fd_ = -1;
    }
    size_t readFile(unsigned char* scratch,size_t n)
    {
        int status;
        ::ssize_t read_size;
        while (true) {
            read_size = ::read(fd_, scratch, n);
            if (read_size < 0) {  // Read error.
                if (errno == EINTR) {
                    printf("readFile EINTR!\n");
                    continue;  // Retry
                }
                printf("readFile error!\n");
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

template<typename T>
class anysort{
public:
    anysort():
    count_(0),
    buf_(nullptr)
    {

    }

    anysort(size_t n):
    count_(n)
    {
        buf_= (T*)malloc(sizeof(T)*n);
        memset(buf_,0,sizeof(T)*n);
    }
    ~anysort()
    {
        free(buf_);
        count_=0;
    }
    void insertsort(T *buf,size_t n)
    {
        if((n==0)||(buf==nullptr))
        {
            return ;
        }
        
        for(int i=1;i<n;i++)
        {
            int j=i-1;
            while((buf[j]>buf[j+1])&&(j>=0))
            {
                std::swap(buf[j],buf[j+1]);
                j--;
            }
        }
    }
    void quicksort(T *buf,size_t n)
    {
        if((n==0)||(buf==nullptr))
        {
            return ;
        }
        quicksub(buf,0,n-1);
    }

    void topk(T *buf,size_t n)
    {
        if((n==0)||(buf==nullptr))
        {
            return ;
        }

        quicksort(buf,n);
        /*
        for(int i=0;i<n;i++)
        {
            printf("%x\n",buf[i]);
        }*/
        
        if(buf[n-1]<=buf_[0])
        {
            return ;
        }
        if(buf[0]>=buf_[count_-1])
        {
            memcpy(buf_,buf,sizeof(T)*std::min(n,count_));
            return;
        }
        T tmp[count_];
        size_t a=count_-1;
        n--;
        for(int i=count_-1;i>=0;i--)
        {
            if(buf[n]>buf_[a])
            {
                tmp[i]=buf[n];
                n--;
            }else if(buf[n]<buf_[a]){
                tmp[i]=buf_[a];
                a--;
            }else{
                tmp[i]=buf_[a];
                a--;
                n--;
            }
        }
        memcpy(buf_,tmp,sizeof(T)*count_);
    }

    void resulttopk()
    {
        printf("topk :%ld\n",count_);
        for(int i=1;i<=10;i++)
        {
            printf("%x\n",buf_[count_-i]);
        }
    }

    void benchmark(T *tbuf,size_t n)
    {
        T buf[n];
        //T* buf= (T*)malloc(sizeof(T)*n);
        memcpy(buf,tbuf,sizeof(T)*n);

        timebench t;
        t.begin();

        insertsort(buf,n);
        t.end();
        
        quicksort(tbuf,n);
        t.end();

        int cc=0;
        for(int i=0;i<n;i++)
        {
            if(buf[i]==tbuf[i])
            {
                //printf("%x %x\n",buf[i],tbuf[i]);
                cc++;
            }
        }
        printf("total %ld: same:%d unsame:%ld\n",n,cc,n-cc);

        //free(buf);
    }
private:

    void  quicksub(T*buf,int begin,int end)
    {
        if((begin>=end)||(begin<0)||(end<0))
        {
            return;
        }

        T tmp = buf[end];
        int div=begin-1;
        for(int i=begin;i<end;i++)
        {
            if(buf[i]<=tmp)
            {
                div++;
                std::swap(buf[div],buf[i]);
            }
        }
        div++;
        std::swap(buf[div],buf[end]);

        quicksub(buf,begin,div-1);
        quicksub(buf,div+1,end);
    }

private:
size_t count_;
T *buf_;
};

#endif