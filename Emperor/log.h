#include <cstdarg>
#include <stdio.h>

/*general error! */
#define GENERALESUCCESS 0
#define GENERALERROR -1
#define GENERALNOTFOUND -2


/*log level */
#define UTILLOGLEVELERROR       1
#define UTILLOGLEVELRECORD      2
#define UTILLOGLEVELWORNNING    3
#define UTILLOGLEVELDIALOG      4

class LOG{
public:
    static void record(int level,const char* c,...);

};

