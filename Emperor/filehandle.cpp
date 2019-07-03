#include "filehandle.h"

#include "util.h"
#include <cstring>

#define FILECBUFNUM 512
filehandle::filehandle(const char *filename)
{
    try{
        cbuf_=new char[FILECBUFNUM];
        pifm=std::make_shared<std::ifstream>(filename,std::ios::in);
    }catch(...)
    {
        throw std::string("filehandle init new cbuf error!");
    }

    ParseFile();
}

filehandle::filehandle()
{
    try{
        cbuf_=new char[FILECBUFNUM];
        pifm=std::make_shared<std::ifstream>(FRIENDFILEPATH,std::ios::in);
    }catch(...)
    {
        throw std::string("filehandle init new cbuf error!");
    }

    ParseFile();
}


filehandle::~filehandle()
{
    if(cbuf_!=nullptr)
    {
        delete [] cbuf_;
    }
    pifm->close();
}

void filehandle::ParseFile()
{

    memset(cbuf_,0,FILECBUFNUM);
    while(pifm->getline(cbuf_,FILECBUFNUM))
    {
        #ifdef FILEDEBUG
            printf("%s\n",cbuf_);
        #endif
        LineProcess();
        memset(cbuf_,0,FILECBUFNUM);
    }

    for(auto i : msvs_)
    {
        printf("uid %zu : ",i.first);
        for(auto j: i.second)
        {
            printf(" %zu ",j);
        }
        printf("\n");
    }
    printf("file load user :\n");
    for(auto it :mst_)
    {
        printf("uid %zu : \n",it.first);
        printfPartner(&it.second,"filehandle::ParseFile");
    }
    printf("============end============");
}

int filehandle::GetResult(std::vector<transfPartner> &p,size_t &uid)
{
    if((msvs_.find(uid)==msvs_.end())||(mst_.find(uid)==mst_.end()))
    {
        LOG::record(UTILLOGLEVELERROR,"func:%s not found %zu",__FUNCTION__,uid);
        return GENERALNOTFOUND;
    }
    int ret=0;
    for(auto i : msvs_[uid])
    {
        if(mst_.find(i)==mst_.end())
        {
            LOG::record(UTILLOGLEVELERROR,"GetResult mst_ fied uid: %d\n",i);
            continue;
        }
        p.emplace_back(mst_[i]);
        ret++;
    }
    return ret;
}

int filehandle::GetResult(transfPartner &p,size_t &uid)
{
    if(mst_.find(uid)==mst_.end())
    {
        return GENERALNOTFOUND;
    }

    p=mst_[uid];
    return GENERALESUCCESS;
}

int filehandle::LineFilterMao(char *src,char *dest)
{
    int ret=0;
    char *st=dest;
    std::memset(st,0,512);

    /*先过滤 */
    while((*src ==' ')&&(*src !='\0'))
    {
        ret++;
        src++;
    }

    if((*src =='\0')&&(*src!='"')&&(*src=='\r'))
    {
        LOG::record(UTILLOGLEVELERROR, "LineFilterMao %d  not match \" ret:%d \n", __LINE__,ret);
        return GENERALERROR;
    }

    src++; //默认这个是"符号
    ret++;

    while(( *src !='"' )&&(*src !='\0'))
    {
        *dest++=*src++;
        ret++;
    }

    if(*src !='\0')
    {
        src++;
        ret++;
    }
    #ifdef FILEDEBUG
        printf("ret=%d dest: %s\n",ret,st);
    #endif
    return ret;
}



int filehandle::LineFilterSpace(char *src,char *dest,int len)
{
    int ret=0;
    char *st=dest;
    std::memset(st,0,len);

    /*先过滤 */
    while((*src ==' ')&&(*src !='\0'))
    {
        ret++;
        src++;
    }

    if(*src =='\0')
    {
        return GENERALERROR;
    }

    while(( *src != ' ' )&&(*src !='\0'))
    {
        *dest++=*src++;
        ret++;
    }

    #ifdef FILEDEBUG
        printf("ret=%d dest: %s\n",ret,st);
    #endif

    return ret;
}

void filehandle::LineProcess()
{
    #ifdef FILEDEBUG
        printf("============LineProcess begin===========\n");
    #endif

    transfPartner p;
    std::memset(&p,0,sizeof(p));

    char ctmp[512];
    int cc=0;
    cc += LineFilterMao(cbuf_+cc,ctmp);
    if((cc<3)||(*ctmp<'0')||(*ctmp>'9'))
    {
        LOG::record(UTILLOGLEVELERROR, "LineProcess %d line not valid \n", __LINE__);
        return;
    }

    p.uid=std::stoull(std::string(ctmp));

    cc += LineFilterMao(cbuf_+cc,ctmp);
    std::memcpy(p.name,ctmp,strlen(ctmp));

    cc += LineFilterMao(cbuf_+cc,ctmp);
    std::memcpy(p.signature,ctmp,strlen(ctmp));

    cc += LineFilterMao(cbuf_+cc,ctmp);

    char sct[20];
    cc=0;
    int ret=0;
    
    while ((ret=LineFilterSpace(ctmp+cc,sct,20))>0)
    {
        msvs_[p.uid].emplace_back(std::stoll(sct));
        cc +=ret;
    }
    if(msvs_.find(p.uid)==msvs_.end())
    {
        msvs_[p.uid];
    }
    mst_[p.uid]=p;
}

