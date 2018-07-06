/*
 * LogServerUnit.cpp
 *
 * Created on: 2013-01-09 14:44
 *     Author: glendy
 */

#include "LogServerUnit.h"
#include "LogServerMonitor.h"
#include "SqlTableCache.h"
#include "google/protobuf/message.h"
#include "google/protobuf/descriptor.h"
#include <fstream>

using std::ios;
using std::ofstream;

int LogUnit::type(void)
{
    return LOG_UNIT;
}

UnitMessage *LogUnit::pop_unit_message(void)
{
    return LOG_SERVER_MONITOR->unit_msg_pool()->pop();
}

int LogUnit::push_unit_message(UnitMessage *msg)
{
    return LOG_SERVER_MONITOR->unit_msg_pool()->push(msg);
}

int LogUnit::process_block(UnitMessage *unit_msg)
{
//    uint32_t len = unit_msg->__len;
    int32_t /*sid = unit_msg->__sid, */recogn = unit_msg->__msg_head.__recogn;

//    Block_Buffer *msg_buf = unit_msg->data_buff();
    Message *msg_proto = unit_msg->proto_msg();

    switch (recogn)
    {
        case INNER_LOG_WRITE:
            return LOG_SERVER_MONITOR->logging(msg_proto);
        default:
            return -1;
    }

    return 0;
}


UnitMessage *MysqlUnit::pop_unit_message(void)
{
    return LOG_SERVER_MONITOR->unit_msg_pool()->pop();
}

int MysqlUnit::push_unit_message(UnitMessage *msg)
{
    return LOG_SERVER_MONITOR->unit_msg_pool()->push(msg);
}

int MysqlUnit::process_block(UnitMessage *unit_msg)
{
//    uint32_t len = unit_msg->__len;
    int32_t /*sid = unit_msg->__sid, */recogn = unit_msg->__msg_head.__recogn;

//    Block_Buffer *msg_buf = unit_msg->data_buff();
    Message *msg_proto = unit_msg->proto_msg();

    switch (recogn)
    {
        case INNER_MYSQL_INSERT:
            return this->push_insert_op(msg_proto);
        case INNER_MYSQL_INSERT_WITH_TABLE_NAME:
        	return this->push_insert_op_with_table_name(msg_proto);
        default:
            // add log; TODO; can't reconigze message %d %d %d;
            return -1;
    }

    return 0;
}

int MysqlUnit::prev_loop_init(void)
{
    this->interval_amount_ = 0;

    return 0;
}

int MysqlUnit::interval_run(void)
{
    ++this->interval_amount_;
    if (this->interval_amount_ < (LOG_INTERVAL * 20))
        return 0;
    this->interval_amount_ = 0;

    Time_Value nowtime = Time_Value::gettimeofday();
    if (this->interval_tick_ > nowtime)
        return 0;
    this->interval_tick_ = nowtime + Time_Value(180);

    return this->process_sql_cache();
}

int MysqlUnit::push_insert_op(const ::google::protobuf::Message *message)
{
    if (message == 0)
        return -1;

    const std::string &tbl_name = message->GetDescriptor()->name();
    SqlTableCache *cache = 0;
    SqlTableCacheMap::iterator iter = this->sql_cache_map_.find(tbl_name);
    if (iter == this->sql_cache_map_.end())
    {
        if ((cache = LOG_SERVER_MONITOR->sql_cache_pool()->pop()) == NULL)
            return -1;

        cache->init(tbl_name);
        this->sql_cache_map_[tbl_name] = cache;
    }
    else
    {
        cache = iter->second;
    }

    return cache->push_insert_op(message);
}

int MysqlUnit::push_insert_op_with_table_name(const ::google::protobuf::Message *message)
{
    if (message == 0)
        return -1;

    ::google::protobuf::FieldDescriptor* tbl_name_field = NULL;
    const ::google::protobuf::Reflection *reflection = message->GetReflection();
    std::vector<const ::google::protobuf::FieldDescriptor*> fields;
    reflection->ListFields(*message, &fields);
	for (std::vector<const ::google::protobuf::FieldDescriptor*>::iterator iter = fields.begin();
			iter != fields.end(); ++iter)
	{
		const ::google::protobuf::FieldDescriptor* field = *iter;
//		MSG_DEBUG("sql field name: %s", field->name().c_str());
		if(field->name()=="table_name")
		{
			tbl_name_field = (::google::protobuf::FieldDescriptor*)field;
			break;
		}
	}

    if(NULL==tbl_name_field)
    {
    	MSG_USER("error:can't find table name");
    	return -1;
    }
    const std::string &tbl_name =reflection->GetString(*message, tbl_name_field);
    SqlTableCache *cache = 0;
    SqlTableCacheMap::iterator iter = this->sql_cache_map_.find(tbl_name);
    if (iter == this->sql_cache_map_.end())
    {
        if ((cache = LOG_SERVER_MONITOR->sql_cache_pool()->pop()) == NULL)
            return -1;

        cache->init(tbl_name);
        this->sql_cache_map_[tbl_name] = cache;
    }
    else
    {
        cache = iter->second;
    }

//    return cache->push_insert_op_with_name(tbl_name,message,fields);
    return cache->push_insert_op(message);
}

MysqlConnector &MysqlUnit::mysql_connector(void)
{
    return this->connector_;
}

int MysqlUnit::process_sql_cache(void)
{
    SqlTableCache *sql_cache = 0;
    std::string sql_cmd;
    for (SqlTableCacheMap::iterator iter = this->sql_cache_map_.begin();
            iter != this->sql_cache_map_.end(); ++iter)
    {
        sql_cache = iter->second;
        if (sql_cache->prepare_run(sql_cmd) != 0)
            continue;

        if (this->connector_.process_sql(sql_cmd) != 0)
        {
        	MSG_USER("process_sql() error");
			ofstream ofs;
			try{
				ofs.open("mysql_serial_failed.sql", ios::app);
				ofs << sql_cmd << ";" << std::endl;
			}catch(...){
				MSG_USER("erro writing to sql file, sql: %s", sql_cmd.c_str());
			}
			if(ofs.is_open())
				ofs.close();
        }
        sql_cache->reset_data_cache();
    }
    return 0;
}

int MysqlUnit::process_stop(void)
{
	MSG_DEBUG("MysqlUnit::process_sql_cache()");
    this->process_sql_cache();
	MSG_DEBUG("MysqlUnit::process_sql_cache() done");

    return BaseUnit::process_stop();
}

int MysqlUnit::type(void)
{
    return MYSQL_UNIT;
}

