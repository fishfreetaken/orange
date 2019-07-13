


#include "mysqlquery.h"

mysqlquery::mysqlquery()
{
    mysql_init(&mysql_);
    MYSQL mysql;
    mysql_init(&mysql);
    if(!mysql_real_connect(&mysql,"127.0.0.1","root","123456","chendong",3306,NULL,0))
    {
        std::cout<< std::string("cannot connect mysql!")<<std::endl;
        return ;
    }
}

void mysqlquery::MaxiumInsertUserInfo(int n)
{
    GenRandomKey k;

    std::string submit("insert into userinfo values ");
    for(int i=0;i<n;i++)
    {
        std::string sql="(NULL,\""+k.GenStrEnLetter(160)+"\",null,\""+k.GenStrEnLetter(7)+"\",\""+k.GenStrEnLetter(17)+"\",\""+std::to_string(k.GenAUIntDigit(1))+"\"\
        
        ), ";
        submit += sql;
    }
    std::string sql="(NULL,\""+namelist[k.GenAUIntDigit(5)]+"\",\""+k.GenStrEnLetter(17)+"\",now(),"+std::to_string(k.GenRealNum())+",\""+std::string(tt)+"\")";
    submit += sql;

    printf("submit size: %ld\n",submit.size());

    struct timeval t2;
    double deltaT;

    int ret=mysql_query(&mysql,submit.c_str());
    if(ret!=0)
    {
        printf("mysql_query error thread:%s  error:%d\n",tt,ret);
        goto out;
    }

    gettimeofday(&t2,NULL);
    deltaT= double((t2.tv_sec-t1.tv_sec)*1000000+ t2.tv_usec-t1.tv_usec)/1000000;
    printf("thread %s: %f sec\n",tt,deltaT);

out:
    mysql_close(&mysql);
}