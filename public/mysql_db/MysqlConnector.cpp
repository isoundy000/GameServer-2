/*
 * MysqlConnector.cpp
 *
 * Created on: 2013-01-15 14:07
 *     Author: glendy
 */

#include "MysqlConnector.h"
#include "Log.h"
#include <sstream>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h> 
//#include <cppconn/prepared_statement.h>

int MysqlConnector::init(const std::string &ip, const int port, const std::string &usr, const std::string &passwd, const bool is_connect)
{
    this->is_connect_ = is_connect;
    this->address_ = ip;
    this->port_ = port;
    this->usr_ = usr;
    this->passwd_ = passwd;

    this->driver_ = 0;
    this->connector_ = 0;

    return 0;
}

int MysqlConnector::connect(void)
{
    if (this->connector_ != 0)
        delete this->connector_;
    this->connector_ = 0;

    std::stringstream stream;
    stream << this->port_;
    std::string url = "tcp://" + this->address_ + ":" + stream.str();
    try 
    {
    	if (this->driver_ == 0)
    		this->driver_ = get_driver_instance();
        this->connector_ = this->driver_->connect(url.c_str(), this->usr_.c_str(), this->passwd_.c_str());
        if (this->connector_->isClosed()) 
        {
            MSG_USER("connect mysql error, url = [%s], user = [%s], pw = [%s]",
                     url.c_str(), this->usr_.c_str(), this->passwd_.c_str());
            return -1;
        }   
    }
    catch (sql::SQLException &e) 
    {
        int err_code = e.getErrorCode();
        MSG_USER("SQLException, MySQL Error Code = %d, SQLState = [%s], [%s]", err_code, e.getSQLState().c_str(), e.what());
//        this->process_mysql_errcode(err_code);	// 会造成无限递归调用
        return -1;
    }   
    return 0;
}

int MysqlConnector::process_sql(const std::string &sql_cmd)
{
    if (this->is_connected() == false)
    {
        if (this->connect() != 0)
            return -1;
    }

    for(size_t i=0; i<3; i++)
    {
		try
		{
			this->connector_->setSchema("back_stream");
			this->connector_->setAutoCommit(false);

			sql::Statement *stmt = this->connector_->createStatement();
			if (stmt == 0)
			{
				MSG_USER("stmt == 0");
				return -1;
			}
			stmt->execute(sql_cmd);
			delete stmt;

			this->connector_->commit();
			this->connector_->setAutoCommit(true);
			return 0;
		}
		catch (sql::SQLException &e)
		{
			int err_code = e.getErrorCode();
			MSG_USER("execute SQL=[%s]", sql_cmd.c_str());
			MSG_USER("ERROR SQLException, MySQL Error Code = %d, SQLState = [%s], [%s]", err_code, e.getSQLState().c_str(), e.what());
			int ret = this->process_mysql_errcode(err_code);
			if(ret != 0) return ret;
		}
    }

    return -1;
}

bool MysqlConnector::is_connected(void)
{
    if (this->connector_ == 0)
        return false;
    if (this->connector_->isClosed())
        return false;

    return true;
}

int MysqlConnector::process_mysql_errcode(const int err_code)
{
    switch (err_code) 
    {
        case 2003:  // Can't connect to MySQL server
        {
            return this->connect();
            break;
        }
        default :
            break;  
    }
    return err_code;
}

