
#include <mysql.h>
#include "util.h"
#include <sys/time.h>


void selectmytable(MYSQL &mysql,size_t n)
{
    GenRandomKey k;

    while(n--)
    {
        try{
            std::string sql="insert into first values(NULL,\""+k.GenStrEnLetter(17)+"\",now(),"+std::to_string(k.GenRealNum())+")";
            //std::cout<<sql<<std::endl;
            mysql_query(&mysql,sql.c_str());

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
     
}


int main()
{
    MYSQL mysql;
    mysql_init(&mysql);
    if(!mysql_real_connect(&mysql,"127.0.0.1","root","123456","chendong",3306,NULL,0))
    {
        std::cout<< std::string("cannot connect mysql!")<<std::endl;
        return -1;
    }

    struct timeval t1,t2;
    gettimeofday(&t1,NULL);
    
    selectmytable(mysql,100);
    //sleep(2);

    gettimeofday(&t2,NULL);
    double deltaT= double((t2.tv_sec-t1.tv_sec)*1000000+ t2.tv_usec-t1.tv_usec)/1000000;
    printf("deltat %f sec\n",deltaT);
    
    //clockend(st);

    int a=mysql_insert_id(&mysql);
    printf("mysql insert id: %d\n",a);
   
    mysql_close(&mysql);

    return 1;
}

