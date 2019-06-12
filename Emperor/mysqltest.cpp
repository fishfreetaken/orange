
#include <mysql.h>
#include "util.h"
#include <sys/time.h>

std::string namelist[]={
    "xiaoniu",
    "chenxi",
    "NULL",
    "niuli",
    "xufujian",
    "kexiaolan"
};

void selectmytablesum(const char* tt,struct timeval t1)
{
    MYSQL mysql;
    mysql_init(&mysql);
    if(!mysql_real_connect(&mysql,"127.0.0.1","root","123456","chendong",3306,NULL,0))
    {
        std::cout<< std::string("cannot connect mysql!")<<std::endl;
        return ;
    }
    GenRandomKey k;
    int n=1000;
    std::string submit("insert into first values ");
    for(int i=0;i<n;i++)
    {
        std::string sql="(NULL,\""+namelist[k.GenAUIntDigit(5)]+"\",\""+k.GenStrEnLetter(17)+"\",now(),"+std::to_string(k.GenRealNum())+",\""+std::string(tt)+"\"), ";
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

void selectmytable(const char* tt,struct timeval t1)
{
    MYSQL mysql;
    struct timeval t2;
    double deltaT;

    mysql_init(&mysql);
    if(!mysql_real_connect(&mysql,"127.0.0.1","root","123456","chendong",3306,NULL,0))
    {
        std::cout<< std::string("cannot connect mysql!")<<std::endl;
        return ;
    }
    GenRandomKey k;
    size_t n=100;

    int ret=0;

    while(n--)
    {
        try{
            std::string sql="insert into first values(NULL,\""+namelist[k.GenAUIntDigit(5)]+"\",\""+k.GenStrEnLetter(17)+"\",now(),"+std::to_string(k.GenRealNum())+",\""+std::string(tt)+"\")";
            //std::cout<<sql<<std::endl;
           ret=mysql_query(&mysql,sql.c_str());
           if(ret!=0)
           {
               printf("mysql_query error thread:%s  error:%d\n",tt,ret);
               goto out;
           }

            continue;
            
            MYSQL_RES *result=mysql_store_result(&mysql);
            if(!result)
            {
                throw std::string("MySql not result!");
            }

            int num_fields=mysql_num_fields(result);
            if(0==num_fields)
            {
                throw std::string("MySQL fields number is 0!");
            }

            MYSQL_FIELD *fields= mysql_fetch_fields(result);
            if(!fields)
            {
                throw std::string("MYSQL fields fetch is error!");
            }
            for(int i=0;i<num_fields;i++)
            {
                printf("field i: %d  name is %s\n",i,fields[i].name);
            }
            printf("MYsql is OK!\n");
            MYSQL_ROW row;

            while((row = mysql_fetch_row(result)))
            {
                unsigned long *length;
                length = mysql_fetch_lengths(result);
                for(int i=0;i<num_fields;i++)
                {
                    printf("[%.*s] ",(int) length[i],row[i]?row[i]:"NULL");
                }
                printf("\n");
            }
        }
        catch(std::string &error_msg)
        {
            std::cout<<error_msg<<std::endl;
        }
        catch(...)
        {
            std::cout<<"MySql operation is error!" << std::endl;
        }
    }

    ret=mysql_insert_id(&mysql);
    printf("mysql insert id: %d\n",ret);

    
    gettimeofday(&t2,NULL);
    deltaT= double((t2.tv_sec-t1.tv_sec)*1000000+ t2.tv_usec-t1.tv_usec)/1000000;
    printf("thread %s: %f sec\n",tt,deltaT);

out:
    mysql_close(&mysql);
    
}


int main()
{ 
    

    struct timeval t1,t2;
    
    gettimeofday(&t1,NULL);

    std::thread p1(selectmytable,"t1",t1),p2(selectmytablesum,"t2",t1);
    
    //selectmytable(100);
    p1.join();
    p2.detach();
    //sleep(20);

    gettimeofday(&t2,NULL);
    double deltaT= double((t2.tv_sec-t1.tv_sec)*1000000+ t2.tv_usec-t1.tv_usec)/1000000;
    printf("deltat %f sec\n",deltaT);
    
    //clockend(st);


    return 1;
}

