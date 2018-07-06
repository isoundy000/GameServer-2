/*
 * MysqlConnector.h
 *
 * Created on: 2013-01-15 12:00
 *     Author: glendy
 */

#ifndef _MYSQLCONNECTOR_H_
#define _MYSQLCONNECTOR_H_

#include <string>

namespace sql
{
    class Driver;
    class Connection;
}

class MysqlConnector
{
public:
    int init(const std::string &ip, const int port, const std::string &usr, const std::string &passwd, const bool is_connect = false);

    int process_sql(const std::string &sql_cmd);
    bool is_connected(void);

protected:
    int connect(void);
    int process_mysql_errcode(const int err_code);

protected:
    bool is_connect_;
    std::string address_;
    int port_;
    std::string usr_;
    std::string passwd_;

    sql::Driver *driver_;
    sql::Connection *connector_;
};

#endif //_MYSQLCONNECTOR_H_
