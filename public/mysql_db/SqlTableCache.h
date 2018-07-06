/*
 * SqlTableCache.h
 *
 * Created on: 2013-01-12 13:44
 *     Author: glendy
 */

#ifndef _SQLTABLECACHE_H_
#define _SQLTABLECACHE_H_

#include "GameHeader.h"

class SqlTableCache
{
public:
    SqlTableCache(void);

    void reset(void);

    int init(const std::string &table_name);
    int push_insert_op(const Message *message);
    int push_insert_op_with_name(std::string table_name,const Message *message,
    		std::vector<const ::google::protobuf::FieldDescriptor*>& fields);
    int prepare_run(std::string &sql_cmd);
    void reset_data_cache(void);

protected:
    int init_insert_perv(std::vector<const ::google::protobuf::FieldDescriptor *> &fields);
    int init_insert_perv_without_table_name(std::string table_name,std::vector<const ::google::protobuf::FieldDescriptor *> &fields);
private:
    std::string table_name_;
    std::string insert_prev_;
    std::string data_cached_;
};

#endif //_SQLTABLECACHE_H_
