
#include "genrandom.h"
#include "log.h"
#include <limits.h>
 GenRandomKey::GenRandomKey(uint32_t low,uint32_t high):
 normal(0),
 realgen(nullptr),
 realunif(nullptr)
{
    std::random_device rd;
    gen =new std::mt19937_64(rd());

    unif=new std::uniform_int_distribution<uint32_t>(low,high);
}

GenRandomKey::GenRandomKey():
 normal(0),
 realgen(nullptr),
 realunif(nullptr)
{
    std::random_device rd;
    gen =new std::mt19937_64(rd());

    unif=new std::uniform_int_distribution<uint32_t>();
}

GenRandomKey::GenRandomKey(uint32_t t):
normal(t),
realgen(nullptr),
realunif(nullptr)
{
    std::random_device rd;
    gen =new std::mt19937_64(rd());

    unif=new std::uniform_int_distribution<uint32_t>();
}

GenRandomKey::~GenRandomKey()
{
    delete gen;
    delete unif;
    delete realgen;
    delete realunif;
}

void GenRandomKey::GenNDigitU32(int n,std::vector<uint32_t>&v)
{
    /*
    std::random_device rd;  // 将用于获得随机数引擎的种子
    std::mt19937_64 gen(rd());
    std::uniform_real_distribution<uint32_t> dis(low, high);
    for (int n = 0; n < 10; ++n) {
        // 用 dis 变换 gen 生成的随机 unsigned int 为 [1, 2) 中的 double
        std::cout << dis(gen) << ' '; // 每次调用 dis(gen) 都生成新的随机 double
    }
    */
   for(int i=0;i<n;i++)
   {
       v.push_back((*unif)(*gen));
   }
   return ;
}

int GenRandomKey::GenAIntDigit()
{
    if(normal)
    {
        return ((int)((*unif)(*gen)-UINT_MAX)%normal);
    }
    return (int)((*unif)(*gen)-UINT_MAX);
}

uint GenRandomKey::GenAUIntDigit(int f)
{
    if(f)
    {
        return (uint)(*unif)(*gen)%f;
    }
    return (uint)(*unif)(*gen);
}

int GenRandomKey::GenStrEnLetter(size_t n, char *in)
{
    std::string s;
    int ret=GenStrEnLetter(n,s);
    for(size_t i=0;i<s.size();i++)
    {
        *(in+i)=s[i];
    }
    printf("GenStrEnLetter create size:%d\n",s.size());
}

int GenRandomKey::GenStrEnLetter(size_t n, std::string &m)
{
    if(enst.size()!=26)
    {
        enst.clear();
        enst += "qwertyuiopasdfghjklzxcvbnm";
    }
    GenericStringGen(n,m,enst);
    return n;
}
//std::chrono
std::string GenRandomKey::GenADate()
{
    uint32_t year=1974+GenAUIntDigit(44);
    uint32_t month=GenAUIntDigit(13);
    if(month==0)
    {
        month++;
    }
    uint32_t day;
    if(month==2)
    {
        day=GenAUIntDigit(29);
    }else{
        switch(month){
            case 4:
            case 6:
            case 9:
            case 11:
            day=GenAUIntDigit(31);
                break;
            default:
                day=GenAUIntDigit(32);
                break;
        }
    }
    return std::to_string(year)+std::to_string(month)+std::to_string(day);
    
}

std::string GenRandomKey::GenStrEnLetter(size_t n)
{
    std::string m;
    if(enst.size()!=26)
    {
        enst.clear();
        enst += "qwertyuiopasdfghjklzxcvbnm";
    }
    GenericStringGen(n,m,enst);
    return m;
}

int GenRandomKey::GenStrDigit(size_t n, std::string &m)
{
    if(enst.size()!=10)
    {
        enst.clear();
        enst += "0123456789";
    }
    return GenericStringGen(n,m,enst);
}

double GenRandomKey::GenRealNum()
{
    if(realgen==nullptr)
    {
        std::random_device rd;
        realgen =new std::mt19937_64(rd());

        realunif=new std::uniform_real_distribution<double>(0,5);
    }
    return (*realunif)(*realgen);
}


int GenRandomKey::GenericStringGen(size_t n,std::string &m, std::string &mod)
{
    int len=mod.size();
    for(size_t i=0;i<n;i++)
    {
        m +=mod[((*unif)(*gen)%len)];
    }
    return n;
}
