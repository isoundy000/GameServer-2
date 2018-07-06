/*
 * BackstageBrocastControl.cpp
 *
 *  Created on: Jun 7, 2014
 *      Author: louis
 */

#include "LogicStruct.h"
#include "BackstageBrocastControl.h"
#include "BackBrocast.h"
#include "LogicMonitor.h"
#include "Transaction.h"
#include "TransactionMonitor.h"
#include "MongoDataMap.h"
#include "BackField.h"
#include <mongo/client/dbclient.h>

BackstageBrocastControl::BackstageBrocastControl() {
	// TODO Auto-generated constructor stub

}

BackstageBrocastControl::~BackstageBrocastControl() {
	// TODO Auto-generated destructor stub
}

//int BackstageBrocastControl::BBCTimer::set_data(
//		BackstageBrocastControl* instance, BackstageBrocastRecord* record)
//{
//	this->instance_ = instance;
//	this->record_ = record;
//	return 0;
//}
//
//BackstageBrocastControl* BackstageBrocastControl::BBCTimer::instance()
//{
//	return this->instance_;
//}
//
//int BackstageBrocastControl::BBCTimer::type(void)
//{
//	return GTT_LOGIC_ONE_SEC;
//}
//
//int BackstageBrocastControl::BBCTimer::start_handlde(void)
//{
//	return 0;
//}
//
//int BackstageBrocastControl::BBCTimer::handle_timeout(const Time_Value &tv)
//{
//	if(NULL == this->record_)
//	{
//		this->cancel_timer();
//		return 0;
//	}
//
//	if(this->record_->__brocast_tick <= tv.sec())
//	{
//		//brocast
//		LOGIC_MONITOR->back_stage_push_system_announce(this->record_->__content);
//
//		this->record_->__brocast_tick = tv.sec() + this->record_->__interval_sec;
//		++this->record_->__brocast_times;
//		this->refresh_tick(tv, Time_Value(this->record_->__interval_sec));
//
//		//db
//		this->instance_->update_record(this->record_);
//
//		if(this->record_->__brocast_times >= this->record_->__max_repeat_times)
//			this->instance_->remove_record_from_backstage_brocast_timer(this->record_->__index);
//
//	}
//	else
//	{
//		Time_Value interval;
//		if(this->record_->__brocast_tick - tv.sec() > this->record_->__interval_sec)
//			interval = Time_Value(this->record_->__interval_sec);
//		else
//			interval = Time_Value(this->record_->__brocast_tick) - tv;
//
//		this->refresh_tick(tv, interval);
//	}
//	return 0;
//}

int BackstageBrocastControl::init(void)
{
	return 0;
}

int BackstageBrocastControl::start(void)
{
//	this->reinit_backstage_brocast_timer();
	return 0;
}

int BackstageBrocastControl::stop(void)
{
	for(BrocastRecordMap::iterator it = this->brocast_record_map_.begin();
			it != this->brocast_record_map_.end(); ++it)
	{
		BackstageBrocastRecord* record = it->second;
		JUDGE_CONTINUE(NULL != record);
		this->push_record(*record);
	}
	this->brocast_record_map_.clear();
	return 0;
}

int BackstageBrocastControl::fina(void)
{
	return 0;
}

int BackstageBrocastControl::request_load_data_from_db(void)
{
	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN( NULL != data_map, -1);

//	data_map->push_query(DBBackBroDetail::COLLECTION, BSON(DBBackBroDetail::DATA_CHANGE << 1));
	if(TRANSACTION_MONITOR->request_mongo_transaction(0,
			TRANS_CHECK_LOAD_BACK_BRO_CONTROL, data_map, LOGIC_MONITOR->logic_unit()) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
	}
	return 0;
}

int BackstageBrocastControl::after_load_data_from_db(Transaction* trans)
{
	JUDGE_RETURN(NULL != trans, -1);
	if(trans->detail().__error != 0)
	{
		trans->rollback();
		return trans->detail().__error;
	}

	MongoDataMap* data_map = trans->fetch_mongo_data_map();
	if(data_map == 0)
	{
		trans->rollback();
		return -1;
	}

	this->reinit_backstage_brocast_timer();
	trans->summit();
	return 0;
}


int BackstageBrocastControl::reinit_backstage_brocast_timer(void)
{
	for(BIntSet::iterator it = this->remove_set_.begin();
			it != this->remove_set_.end(); ++it)
	{
		JUDGE_CONTINUE(this->brocast_record_map_.count(*it) > 0);
		this->push_record_by_id(*it);
		this->brocast_record_map_.erase(*it);
	}

	for(BIntSet::iterator it = this->modify_set_.begin();
			it != this->modify_set_.end(); ++it)
	{
		BackstageBrocastRecord* record = BBRecord_PACKAGE->find_object(*it);
		JUDGE_CONTINUE(record != NULL);
		this->brocast_record_map_[*it] = record;
	}

	this->remove_set_.clear();
	this->modify_set_.clear();
	return 0;
}

int BackstageBrocastControl::update_record(BackstageBrocastRecord* record)
{
	JUDGE_RETURN(NULL != record, -1);

	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, -1);

	BackBrocast::updata_data(record, data_map);
    if (TRANSACTION_MONITOR->request_mongo_transaction(0,
    		TRANS_UPDATE_BACK_BROCAST_INFO, data_map) != 0)
    {
        POOL_MONITOR->mongo_data_map_pool()->push(data_map);
        return -1;
    }
	return 0;
}


BackstageBrocastRecord* BackstageBrocastControl::pop_record(void)
{
	return BBRecord_PACKAGE->pop_object();
}

void BackstageBrocastControl::push_record(BackstageBrocastRecord& record)
{
	BBRecord_PACKAGE->unbind_and_push(record.__index, &record);
}

void BackstageBrocastControl::push_record_by_id(const int record_id)
{
	BBRecord_PACKAGE->unbind_and_push(record_id);
}

BIntSet& BackstageBrocastControl::modify_set()
{
	return this->modify_set_;
}

BIntSet& BackstageBrocastControl::remove_set()
{
	return this->remove_set_;
}

BackstageBrocastControl::BrocastRecordMap& BackstageBrocastControl::brocast_record_map()
{
	return this->brocast_record_map_;
}

int BackstageBrocastControl::backstage_brocast_handle_timeout(const Int64 now)
{
	IntVec remove_vec;
	for (BrocastRecordMap::iterator it = this->brocast_record_map_.begin();
			it != this->brocast_record_map_.end(); ++it)
	{
		BackstageBrocastRecord* record = it->second;
		JUDGE_CONTINUE(record != NULL);
		JUDGE_CONTINUE(record->__brocast_tick <= now);

		LOGIC_MONITOR->back_stage_push_system_announce(record->__content,
				record->__brocast_type);

		++record->__brocast_times;
		record->__brocast_tick = now + record->__interval_sec;

		//db
		this->update_record(record);
		JUDGE_CONTINUE(record->__brocast_times >= record->__max_repeat_times);

		remove_vec.push_back(record->__index);
	}

	for (IntVec::iterator iter = remove_vec.begin(); iter != remove_vec.end(); ++iter)
	{
		int record_id = *iter;
		this->push_record_by_id(record_id);
		this->brocast_record_map_.erase(record_id);
	}

	return 0;
}

//int BackstageBrocastControl::add_record_to_backstage_brocast_timer(const int record_id)
//{
//	BackstageBrocastRecord* record = BBRecord_PACKAGE->find_object(record_id);
//	JUDGE_RETURN(NULL != record, -1);
//
//	BBCTimer* cur_timer = new BBCTimer();
//	cur_timer->set_data(this, record);
//	cur_timer->schedule_timer(Time_Value(1));
//	this->bbc_timer_map_.insert(BBCTimerMap::value_type(record_id, cur_timer));
//	return 0;
//}
//
//int BackstageBrocastControl::remove_record_from_backstage_brocast_timer(const int record_id)
//{
//	this->push_record_by_id(record_id);
//	if(this->bbc_timer_map_.count(record_id) > 0)
//	{
//		BBCTimer* cur_timer = this->bbc_timer_map_[record_id];
//		JUDGE_RETURN(NULL != cur_timer, -1);
//
//		cur_timer->cancel_timer();
//
//		this->bbc_timer_map_.erase(record_id);
//	}
//
//	return 0;
//}
//
//int BackstageBrocastControl::modify_record_from_backstage_brocast_timer(const int record_id)
//{
//	BackstageBrocastRecord* record = BBRecord_PACKAGE->find_object(record_id);
//	JUDGE_RETURN(NULL != record, -1);
//
//	BBCTimer* cur_timer = this->bbc_timer_map_[record_id];
//	JUDGE_RETURN(NULL != cur_timer, -1);
//
//	cur_timer->cancel_timer();
//	cur_timer->set_data(this, record);
//	cur_timer->schedule_timer(Time_Value(1));
//	return 0;
//}
