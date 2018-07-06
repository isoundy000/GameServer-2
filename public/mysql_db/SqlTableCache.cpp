/*
 * SqlTableCache.cpp
 *
 * Created on: 2013-01-12 13:44
 *     Author: glendy
 */

#include "Log.h"
#include "SqlTableCache.h"
#include "google/protobuf/message.h"
#include "google/protobuf/descriptor.h"
#include <sstream>
#include "GameCommon.h"

using namespace ::google::protobuf;

SqlTableCache::SqlTableCache(void)
{ /*NULL*/ }

void SqlTableCache::reset(void)
{
	this->table_name_.clear();
	this->insert_prev_.clear();
	this->data_cached_.clear();
}

int SqlTableCache::init(const std::string &table_name)
{
    this->table_name_ = table_name;
    this->insert_prev_.clear();
    this->data_cached_.clear();
    return 0;
}

int SqlTableCache::push_insert_op(const Message *message)
{
    const Reflection *reflection = message->GetReflection();
    std::vector<const FieldDescriptor*> fields;
    reflection->ListFields(*message, &fields);

    if (this->insert_prev_.empty() == true)
    {
        this->init_insert_perv(fields);
    }

    std::stringstream stream;
    int i = 0;
    for (std::vector<const FieldDescriptor*>::iterator iter = fields.begin();
            iter != fields.end(); ++iter)
    {
    	const FieldDescriptor *field = *iter;
    	if(field->name() == "table_name" ){
    		continue;
    	}
    	if(i == 0)
    		stream << "(";
    	else if (i > 0)
    		stream << ",";

        if (field->type() == FieldDescriptor::TYPE_INT32)
            stream << reflection->GetInt32(*message, field);
        else if (field->type() == FieldDescriptor::TYPE_DOUBLE)
            stream << reflection->GetDouble(*message, field);
        else if (field->type() == FieldDescriptor::TYPE_INT64)
            stream << reflection->GetInt64(*message, field);
        else if (field->type() == FieldDescriptor::TYPE_STRING){
			stream 	<< "'"
					<< GameCommon::sql_escape(reflection->GetString(*message, field))
					<< "'";
        }
        else
        {
            MSG_USER("ERROR field type can't recognize name: %s, type: %s", field->name().c_str(), field->type());
            continue;
        }
        ++i;
    }
    stream << ")";
//    MSG_USER("SERIAL DATA[%s]: %s", this->table_name_.c_str(), stream.str().c_str());

    if (this->data_cached_.empty() == true)
        this->data_cached_ = stream.str();
    else
        this->data_cached_.append(",").append(stream.str());
    return 0;
}

//int SqlTableCache::push_insert_op_with_name(std::string table_name,const Message *message,
//		std::vector<const ::google::protobuf::FieldDescriptor*>& fields)
//{
//    const Reflection *reflection = message->GetReflection();
////    std::vector<const FieldDescriptor*> fields;
////    reflection->ListFields(*message, &fields);
//
//    if (this->insert_prev_.empty() == true)
//    {
//        this->init_insert_perv_without_table_name(table_name,fields);
//    }
//
//    std::stringstream stream;
//    stream << "(";
//    int i = 0;
//    for (std::vector<const FieldDescriptor*>::iterator iter = fields.begin();
//            iter != fields.end(); ++iter)
//    {
//    	const FieldDescriptor *field = *iter;
//    	MSG_DEBUG("FieldName: %s", field->name().c_str());
//    	if(field->name() == "table_name")
//		{
////    		MSG_DEBUG("FieldName: %s, skiped", field->name().c_str());
//			continue;
//		}
//
//        if (i > 0)
//            stream << ",";
//
//        if (field->type() == FieldDescriptor::TYPE_INT32)
//            stream << reflection->GetInt32(*message, field);
//        else if (field->type() == FieldDescriptor::TYPE_DOUBLE)
//            stream << reflection->GetDouble(*message, field);
//        else if (field->type() == FieldDescriptor::TYPE_INT64)
//            stream << reflection->GetInt64(*message, field);
//        else if (field->type() == FieldDescriptor::TYPE_STRING)
//        {
//        	 stream << "'" << reflection->GetString(*message, field) << "'";
//        }
//        else
//        {
//            MSG_USER("ERROR field type can't recognize %d", field->type());
//            continue;
//        }
//        ++i;
//    }
//    stream << ")";
//
//    MSG_USER("SERIAL DATA[%s]: %s", table_name.c_str(), stream.str().c_str());
//
//    if (this->data_cached_.empty() == true)
//        this->data_cached_ = stream.str();
//    else
//        this->data_cached_.append(",").append(stream.str());
//    return 0;
//}

int SqlTableCache::prepare_run(std::string &sql_cmd)
{
    if (this->data_cached_.empty() == true)
        return -1;
    sql_cmd = this->insert_prev_;
    sql_cmd.append(this->data_cached_);

    return 0;
}

void SqlTableCache::reset_data_cache(void)
{
    this->data_cached_.clear();
}

int SqlTableCache::init_insert_perv(std::vector<const ::google::protobuf::FieldDescriptor *> &fields)
{
    this->insert_prev_ = "INSERT INTO `";
    this->insert_prev_.append(this->table_name_).append("`");

    std::stringstream stream;
    int i = 0;
    for (std::vector<const FieldDescriptor*>::iterator iter = fields.begin();
            iter != fields.end(); ++iter)
    {
    	const FieldDescriptor *field = *iter;
        if(field->name() == "table_name")
        {
        	continue;
        }

        if (i == 0)
            this->insert_prev_.append("(");
        else
            this->insert_prev_.append(",");
        this->insert_prev_.append("`").append(field->name()).append("`");
        ++i;
    }
    this->insert_prev_.append(") VALUES ");
    return 0;
}

//int SqlTableCache::init_insert_perv_without_table_name(std::string table_name,
//		std::vector<const ::google::protobuf::FieldDescriptor *> &fields)
//{
//    this->insert_prev_ = "INSERT INTO " + table_name;
//
//    std::stringstream stream;
//    int i = 0;
//    for (std::vector<const FieldDescriptor*>::iterator iter = fields.begin();
//            iter != fields.end(); ++iter)
//    {
//        const FieldDescriptor *field = *iter;
//    	if(field->name()== "table_name" ){
//    		MSG_DEBUG("FieldName: %s, skiped", field->name().c_str());
//    		continue;
//    	}
//        if (i == 0)
//            this->insert_prev_.append("(");
//        else
//            this->insert_prev_.append(",");
//        this->insert_prev_.append(field->name());
//        ++i;
//    }
//    this->insert_prev_.append(") VALUES ");
//    return 0;
//}
