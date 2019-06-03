#include <cstdarg>
#include <stdio.h>

#define UTILLOGLEVEL1 1

class LOG{
public:
    static void record(int level,const char* c,...);

};

