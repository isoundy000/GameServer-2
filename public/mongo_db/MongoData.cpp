/*
 * MongoData.cpp
 *
 * Created on: 2013-07-18 20:52
 *     Author: lyz
 */

#include "MongoData.h"
#include "TQueryCursor.h"
#include "GameHeader.h"
#include "ProtoInnerPublic.pb.h"
#include <mongo/client/dbclient.h>

MongoData::MongoData(void) : 
    op_type_(0), cond_bson_(0), sort_cond_bson_(0), is_insert_(false), data_type_(0),
    data_bson_(0), data_cursor_(0), t_data_cursor_(0)
{
	this->cond_bson_ = new BSONObj();
	this->sort_cond_bson_ = new BSONObj();
	this->data_bson_ = new BSONObj();
	this->data_cursor_ = new auto_ptr<DBClientCursor>();
    this->t_data_cursor_ = new auto_ptr<TQueryCursor>();
}

MongoData::~MongoData(void)
{
	if (this->cond_bson_ != 0)
		delete this->cond_bson_;
	if (this->sort_cond_bson_ != 0)
		delete this->sort_cond_bson_;
	if (this->data_bson_ != 0)
		delete this->data_bson_;
	if (this->data_cursor_ != 0)
		delete this->data_cursor_;
    if (this->t_data_cursor_ != 0)
        delete this->t_data_cursor_;
	this->cond_bson_ = 0;
	this->data_bson_ = 0;
	this->data_cursor_ = 0;
    this->t_data_cursor_ = 0;
}

MongoData &MongoData::operator = (MongoData &data)
{
    this->op_type_ = data.op_type_;
    this->table_name_ = data.table_name_;
    *(this->cond_bson_) = *(data.cond_bson_);
    *(this->sort_cond_bson_) = *(data.sort_cond_bson_);
    this->is_insert_ = data.is_insert_;
    this->data_type_ = data.data_type_;
    *(this->data_bson_) = *(data.data_bson_);
    *(this->data_cursor_) = *(data.data_cursor_);
    *(this->t_data_cursor_) = *(data.t_data_cursor_);
    return *this;
}

void MongoData::reset(void)
{
    this->op_type_ = 0;
    this->is_insert_ = false;
    this->table_name_.clear();
    *(this->cond_bson_) = BSONObj();
    *(this->sort_cond_bson_) = BSONObj();
    this->data_type_ = 0;
    *(this->data_bson_) = BSONObj();
    this->t_data_cursor_->reset();
    this->data_cursor_->reset();
}

void MongoData::set_update(const std::string &table, BSONObj condition, const bool is_insert)
{
    this->op_type_ = MONGO_OP_UPDATE;
    this->is_insert_ = is_insert;
    this->table_name_ = table;
    *(this->cond_bson_) = condition;
    this->data_type_ = 0;
}

void MongoData::set_update(const std::string &table, BSONObj condition, BSONObj data, const bool is_insert)
{
    this->set_update(table, condition, is_insert);
    this->set_data_bson(data);
}

void MongoData::set_find(const std::string &table, BSONObj condition)
{
    this->op_type_ = MONGO_OP_FIND;
    this->table_name_ = table;
    *(this->cond_bson_) = condition;
    this->data_type_ = MONGO_DATA_BSON;
}

void MongoData::set_query(const std::string &table, BSONObj condition)
{
    this->op_type_ = MONGO_OP_QUERY;
    this->table_name_ = table;
    *(this->cond_bson_) = condition;
    this->data_type_ = MONGO_DATA_CURSOR;
}

void MongoData::set_remove(const std::string &table, BSONObj condition)
{
    this->op_type_ = MONGO_OP_REMOVE;
    this->table_name_ = table;
    *(this->cond_bson_) = condition;
    this->data_type_ = 0;
}

void MongoData::set_multithread_query(const std::string &table, BSONObj condition)
{
    this->op_type_ = MONGO_OP_THREAD_QUERY;
    this->table_name_ = table;
    *(this->cond_bson_) = condition;
    this->data_type_ = 0;
}

void MongoData::set_multithread_query(const std::string &table, BSONObj condition, BSONObj sort_condition)
{
	this->op_type_ = MONGO_OP_THREAD_QUERY_WITH_SORT;
	this->table_name_ = table;
	*(this->cond_bson_) = condition;
	*(this->sort_cond_bson_) = sort_condition;
	this->data_type_ = 0;
}

void MongoData::set_data_bson(BSONObj bson)
{
    this->data_type_ = MONGO_DATA_BSON;
    *(this->data_bson_) = bson;
}

void MongoData::set_data_cursor(auto_ptr<DBClientCursor> cursor)
{
    this->data_type_ = MONGO_DATA_CURSOR;
    *(this->data_cursor_) = cursor;
}

void MongoData::set_multithread_cursor(auto_ptr<DBClientCursor> cursor)
{
    this->data_type_ = MONGO_DATA_TCURSOR;
    
    TQueryCursor *t_cursor = new TQueryCursor();
    t_cursor->set_data(cursor);
    this->t_data_cursor_->reset(t_cursor);
}

int MongoData::op_type(void)
{
    return this->op_type_;
}

bool MongoData::is_update(void)
{
    return (this->op_type_ == MONGO_OP_UPDATE);
}

bool MongoData::is_auto_insert(void)
{
    return this->is_insert_;
}

bool MongoData::is_find(void)
{
    return (this->op_type_ == MONGO_OP_FIND);
}

bool MongoData::is_query(void)
{
    return (this->op_type_ == MONGO_OP_QUERY);
}

bool MongoData::is_multithread_query(void)
{
	return (this->op_type_ == MONGO_OP_THREAD_QUERY);
}

bool MongoData::is_multithread_query_with_sort(void)
{
	return (this->op_type_ == MONGO_OP_THREAD_QUERY_WITH_SORT);
}

bool MongoData::is_remove(void)
{
    return (this->op_type_ == MONGO_OP_REMOVE);
}

std::string &MongoData::table(void)
{
    return this->table_name_;
}

BSONObj &MongoData::condition(void)
{
    return *(this->cond_bson_);
}

BSONObj &MongoData::sort_condition(void)
{
	return *(this->sort_cond_bson_);
}

BSONObj &MongoData::data_bson(void)
{
    return *(this->data_bson_);
}

auto_ptr<DBClientCursor> MongoData::data_cursor(void)
{
    return *(this->data_cursor_);
}

auto_ptr<TQueryCursor> MongoData::multithread_cursor(void)
{
    return *(this->t_data_cursor_);
}

void MongoData::serialize(ProtoMongoData *proto_mongo_data)
{
    proto_mongo_data->set_op_type(this->op_type_);
    proto_mongo_data->set_table_name(this->table_name_);
    proto_mongo_data->set_is_insert(0);
    
    BSONObj cond_obj = this->condition();
    proto_mongo_data->set_cond_bson(cond_obj.jsonString());
    *(this->cond_bson_) = cond_obj;

    proto_mongo_data->set_data_type(this->data_type_);
    BSONObj data_obj = this->data_bson();
    proto_mongo_data->set_data_bson(data_obj.jsonString());
    this->set_data_bson(data_obj);
}

void MongoData::unserialize(const ProtoMongoData &proto_mongo_data)
{
	this->op_type_ = proto_mongo_data.op_type();
	this->table_name_ = proto_mongo_data.table_name();
	this->is_insert_ = proto_mongo_data.is_insert();
	*(this->cond_bson_) = fromjson(proto_mongo_data.cond_bson().c_str());

	BSONObj data_obj = fromjson(proto_mongo_data.data_bson().c_str());
	this->set_data_bson(data_obj);
}

