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
#include <thread>
#include <cstring>
#include <mutex>
#include "anysort.h"
#include "network.h"


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

void benchmark()
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
        //m.topk(buf,UNIBUFFERSIZE);
        m.stacktopk(buf,UNIBUFFERSIZE);
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

    m.printTopk(20);
    printf("this is signal thread work out\n");

    //m.benchmark(buf,UNIBUFFERSIZE);
}
template<typename T>
void debugout(T *p,size_t n)
{
    for(int i=0;i<n;i++)
    {
        printf("%x ",*(p+i));
    }
    printf("\n");
}

std::vector<unsigned int> top10(10,0);
std::mutex mtx;
void common(int fd)
{
    ReadableFile a(fd);
   unsigned int buf[UNIBUFFERSIZE];
   timebench t;
   size_t cc=0;
   anysort<unsigned int> m(UNIBUFFERSIZE);
   t.begin();
   while(a.readFile((unsigned char*)buf,UNIBUFFERSIZE*4)>0)
    {
        cc++;
        //m.topk(buf,UNIBUFFERSIZE);
        m.stacktopk(buf,UNIBUFFERSIZE);
        //break;
        /*
        for(int i=0;i<UNIBUFFERSIZE;i++)
        {
            test.insert(buf[i]);
        }
        memset(buf,0,UNIBUFFERSIZE*4);
        */

    }

    //m.resulttopk();
    
mtx.lock();
m.resultTopk(10,top10);
t.end();
mtx.unlock();

printf("cc=%ld\n",cc);
}

void readtest(int fd)
{
    int cc=6;
    unsigned int buf[cc];

    int re=read(fd,buf,cc*4);
    if(re<=0)
    {
        printf("thread1 read failed!\n");
    }
    debugout(buf,cc);
}

void foo(int fd,int ith)
{
    /*
        int fd1 = ::open("../testdata/class.dat", O_RDONLY );
        printf("1 fd1:%d\n",fd1);
        ::close(fd1);
    */

   common(fd);


   printf("this end is the %d thread!\n",ith);
   
}

void bar(int fd,int ith)
{
int fd2 = ::open("../testdata/class.dat", O_RDONLY );
printf("1 fd2:%d\n",fd2);

    int cc=6;
    unsigned int buf[cc];
    int re=read(fd,buf,cc*4);
    if(re<=0)
    {
        printf("thread2 read failed!\n");
    }
    debugout(buf,cc);
    printf("this bar is the %d thread!\n",ith);

    ::close(fd2);

}

void multithread(int n)
{
    printf("we will start %d thread to work\n",n);
     timebench t;
     t.begin();
    int fd = ::open("../testdata/class.dat", O_RDONLY );
    std::vector<std::thread> threads;
    for (int i=1; i<=n; ++i)
    {
        threads.push_back(std::thread(foo,fd,i));
    }
    for (auto& th : threads) th.join();

    for(int i=0;i<10;i++) printf("lastresutl %dth : %x\n",i,top10[i]);
    printf("last total time:\n");
    t.end();
    ::close(fd);
}
void tfrev(netServer *f)
{

    while(1)
    {
        //sleep(2);
        
        f->recv();
    }
}

int main()
{  
    //priquesorttest();
    //benchmark();
    //multithread(3);

    printf("%d\n",((size_t)-1)); //-1

    char ip[]="127.0.0.1";
    int port=30256;
    int fk=0;
    netServer ns(port,ip,20);
    std::thread thr(tfrev,&ns);
    thr.detach();

    printf("pid %d\n",getpid());
        
    ns.serverstart();
    ns.serveraccept();


/*
    netclient nc(port,ip);
    nc.toconnect(); */

    return 0;
}