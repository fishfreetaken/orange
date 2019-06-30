
#include "protocal.h"
#include <fstream>
#include <memory>
#include <map>
#include <vector>

#define FRIENDFILEPATH "./data/userinfo"

class filehandle {
public:
    /*根据文件名称，解析出来自己和朋友的，第一个肯定是自己的uid */
    filehandle(const char *filename);
    filehandle();
    ~filehandle();

    int GetResult(std::vector<transfPartner> &p,size_t &uid);
    int GetResult(transfPartner &p,size_t &uid);

private:
    void ParseFile();
    void LineProcess();
    int  LineFilterMao(char *src,char *dest);
    int  LineFilterSpace(char *src,char *dest,int len);
private:
    /*每次传入一个指针吧，进行*/
    std::vector<transfPartner> *vp_;
    char *cbuf_;
    std::shared_ptr<std::ifstream> pifm;

    std::map<size_t,transfPartner> mst_;
    std::map<size_t,std::vector<size_t>> msvs_;
};
