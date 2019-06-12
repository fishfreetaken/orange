#include <random>

/*
 使用c++11的随机生成器
*/
class GenRandomKey{
    
public:
    GenRandomKey();
    GenRandomKey(uint32_t t);//指定生成的normal进行取余
    GenRandomKey(uint32_t low,uint32_t hi);

    ~GenRandomKey();

    double GenRealNum();


    int GenStrDigit(size_t n,std::string &m);
    int GenStrFloatDi(size_t n, std::string &m);
    int GenStrEnLetter(size_t n, std::string &m);
    std::string GenStrEnLetter(size_t n);

    int GenStrEnLeDigit(size_t n,std::string *m);
    //uint64_t GenDigitU64();
    void GenNDigitU32(int n,std::vector<uint32_t>&v);
    void GenUNDigitU32(int n,std::vector<uint32_t>&v); //生成一列不重复的随即数字S

    int GenAIntDigit();
    uint GenAUIntDigit(int f);

private:
    int GenericStringGen(size_t n,std::string &m, std::string &mod);

private :

    std::mt19937_64 *gen;
    std::uniform_int_distribution<uint32_t> *unif;

    uint32_t normal;
    std::string enst; //字典

    std::mt19937_64 *realgen;
    std::uniform_real_distribution<double> *realunif;

};
