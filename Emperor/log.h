#include <cstdarg>
#include <stdio.h>

#define UTILLOGLEVELERROR       1
#define UTILLOGLEVELRECORD      2
#define UTILLOGLEVELWORNNING    3
#define UTILLOGLEVELDIALOG      4

class LOG{
public:
    static void record(int level,const char* c,...);

};

