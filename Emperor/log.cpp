#include "log.h"

void LOG::record(int level,const char* c,...)
{
    va_list pArgList;
    va_start(pArgList, c);
    //int nByteWrite =vfprintf(stdout,c,pArgList);
    vfprintf(stdout,c,pArgList);
    vfprintf(stdout,"\n",NULL);
    va_end(pArgList);
}
