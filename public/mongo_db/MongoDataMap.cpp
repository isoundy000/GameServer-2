/*
 * MongoDataMap.cpp
 *
 * Created on: 2013-07-19 14:13
 *     Author: lyz
 */

#include "MongoDataMap.h"
#include "MongoData.h"
#include "PoolMonitor.h"
#include <mongo/client/dbclient.h>

void MongoDataMap::reset(void)
{
    for (DataMap::iterator iter = this->data_map_.begin(); iter != this->data_map_.end(); ++iter)
    {
        this->push_data(iter->second);
    }
    this->data_map_.unbind_all();
    this->other_value_ = 0;
    this->check_role_ = 0;
}

MongoData *MongoDataMap::pop_data(void)
{
    return POOL_MONITOR->mongo_data_pool()->pop();
}

int MongoDataMap::push_data(MongoData *data)
{
    return POOL_MONITOR->mongo_data_pool()->push(data);
}

void MongoDataMap::recycle(void)
{
    this->reset();
    POOL_MONITOR->mongo_data_map_pool()->push(this);
}

int MongoDataMap::bind_data(MongoData *data)
{
    return this->data_map_.bind(data->table(), data);
}

int MongoDataMap::find_data(const std::string &table, MongoData *&data)
{
    return this->data_map_.find(table, data);
}

int MongoDataMap::unbind_data(MongoData *data)
{
    return this->data_map_.unbind(data->table());
}

void MongoDataMap::push_update(const std::string &table, BSONObj condition, BSONObj data, const bool is_insert)
{
	MongoData *mongo_data = this->pop_data();
	mongo_data->set_update(table, condition, data, is_insert);
	if (this->bind_data(mongo_data) != 0)
		this->push_data(mongo_data);
	return;
}

void MongoDataMap::push_find(const std::string &table, BSONObj condition)
{
	MongoData *mongo_data = this->pop_data();
	mongo_data->set_find(table, condition);
	if (this->bind_data(mongo_data) != 0)
		this->push_data(mongo_data);
	return;
}

void MongoDataMap::push_query(const std::string &table, BSONObj condition)
{
	MongoData *mongo_data = this->pop_data();
	mongo_data->set_query(table, condition);
	if (this->bind_data(mongo_data) != 0)
		this->push_data(mongo_data);
	return;
}

void MongoDataMap::push_remove(const std::string &table, BSONObj condition)
{
	MongoData *mongo_data = this->pop_data();
	mongo_data->set_remove(table, condition);
	if (this->bind_data(mongo_data) != 0)
		this->push_data(mongo_data);
	return;
}

void MongoDataMap::push_multithread_query(const std::string &table, BSONObj condition)
{
	MongoData *mongo_data = this->pop_data();
	mongo_data->set_multithread_query(table, condition);
	if (this->bind_data(mongo_data) != 0)
		this->push_data(mongo_data);
	return;
}

void MongoDataMap::push_multithread_query(const std::string &table, BSONObj condition, BSONObj sort_condition)
{
	MongoData *mongo_data = this->pop_data();
	mongo_data->set_multithread_query(table, condition, sort_condition);
	if (this->bind_data(mongo_data) != 0)
		this->push_data(mongo_data);
	return ;
}

void MongoDataMap::push_find(const std::string &table)
{
	this->push_find(table, BSONObj());
}

void MongoDataMap::push_query(const std::string &table)
{
	this->push_query(table, BSONObj());
}

void MongoDataMap::push_multithread_query(const std::string &table)
{
	this->push_multithread_query(table, BSONObj());
}

MongoDataMap::DataMap &MongoDataMap::data_map(void)
{
    return this->data_map_;
}


