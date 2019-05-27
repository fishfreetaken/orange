#include <cstdarg>
#include <stdio.h>
class LOG{
public:
    static void record(int level,const char* c,...);

};
