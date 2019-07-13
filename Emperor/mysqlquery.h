
#ifndef MYQSQLQUERY_HEADER_
#define MYQSQLQUERY_HEADER_

#include <mysql.h>
class mysqlquery{
public:
    mysqlquery();/*默认连接 */
    mysqlquery(const char *s);/*指定连接某个数据库 */


void mysqlquery::MaxiumInsertUserInfo(int sum);

private:
    MYSQL mysql_;
};

#endif