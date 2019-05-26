#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <fcntl.h>
#include <cerrno> 
#include <cstddef>
#include <errno.h>
#include <string.h>
#include <random>
#include <functional>
#include <climits>
#include <utime.h>
#include <memory>
#include <chrono>
#include "randomkey.h"
#include "readsort.h"
#include <set>

constexpr const size_t kBufferSize =  1UL<<20;  //4M bytes 


template<typename T>
void writefile(T *buf,T &len)
{
    int fd = ::open("../testdata/num1.dat",O_TRUNC | O_WRONLY | O_CREAT, 0644);
    if(fd<0)
    {
        printf("Error opening file unexist.ent:%s\n",::strerror(errno));
    }

    //randomgen(buf);

    int kk= ::write(fd,buf,len);
    printf("write kk=%d\n",kk);
    ::close(fd);
}

template<typename T>
void randomgen(T *buf, T len)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<T> distribution(1,UINT_MAX);
    int dice_roll = distribution(generator);
    auto dice = std::bind ( distribution, generator );
    for(int i=0;i<len;i++)
    {
        //printf("%u ",dice());
        buf[i]=dice();
        #ifdef OPENPRINT
            printf("%x ",buf[i]);
        #endif
    }
    #ifdef OPENPRINT
        printf("\n");
    #endif
}

template<typename T>
void randomgen2(T *buf,T len,int it)
{

    srand(it);
    for(int i=0;i<len;i++)
    {
        //printf("%u ",dice());
        buf[i]=rand();
        #ifdef OPENPRINT
            printf("%x ",buf[i]);
        #endif
    }

    printf("\n");
}
void genramdomfile()
{
    unsigned  int len=kBufferSize* sizeof(unsigned int);
    //printf("%u, %ld\n",UINT_MAX,sizeof( unsigned int));
    unsigned int buf[kBufferSize];

    //writefile(buf,len);
    std::shared_ptr<env> p = std::make_shared<env>();
    WritableFile *result;
    
    p->NewAppendableFile("../testdata/class.dat",&result);


    std::shared_ptr<WritableFile> wp(result);

    timebench t;
    t.begin();
    for(int i=0;i<100;i++)
    {
        randomgen(buf,(unsigned int)kBufferSize);
        wp->WriteUnbuffered((char *)buf,len);
    }
    t.end();
    
    //wp->readFile(len,(unsigned char *)buf);
}
#define UNIBUFFERSIZE 4096
int main()
{
    ReadableFile a("../testdata/class.dat");

    unsigned int buf[UNIBUFFERSIZE];

    anysort<unsigned int> m(UNIBUFFERSIZE);
    size_t cc=0;
    timebench t;
    std::set<unsigned int> test;
    t.begin();
    while(a.readFile((unsigned char*)buf,UNIBUFFERSIZE*4))
    {
        cc++;
        m.topk(buf,UNIBUFFERSIZE);
        //break;
        /*
        for(int i=0;i<UNIBUFFERSIZE;i++)
        {
            test.insert(buf[i]);
        }
        memset(buf,0,UNIBUFFERSIZE*4);
        */

    }

    t.end();
    printf("cc=%ld\n",cc);
    /*
    std::set<unsigned int>::reverse_iterator it(test.rbegin());
    for(int i=0;i<10;i++)
    {
        printf("%x\n",*it++);
    }*/

    
    m.resulttopk();

    
    //m.benchmark(buf,UNIBUFFERSIZE);

    

    return 0;
}