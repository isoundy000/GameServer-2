/*
 * LogicCustomer.cpp
 *
 *  Created on: Aug 11, 2014
 *      Author: louis
 */

#include "LogicCustomer.h"
#include "PubStruct.h"
#include "PoolMonitor.h"
#include "ProtoDefine.h"
#include "LogicMonitor.h"
#include "Transaction.h"
#include "MongoData.h"
#include "MongoDataMap.h"
#include "LogicPlayer.h"

#include "TQueryCursor.h"
#include "GameField.h"
#include "BackField.h"
#include "BackBrocast.h"
#include <mongo/client/dbclient.h>

LogicCustomer::LogicCustomer() {
	// TODO Auto-generated constructor stub
	this->contact_way_="";
}

LogicCustomer::~LogicCustomer() {
	// TODO Auto-generated destructor stub
}

int LogicCustomer::request_open_customer_service_pannel(Message* msg)
{
//	MSG_DYNAMIC_CAST_RETURN(Proto10100901*, request, RETURN_REQUEST_OPEN_CUSTOMER_SVC_PANNEL);
	Proto50100901 respond;
	respond.set_last_record_type(this->customer_service_detial_.__last_summit_type);
	respond.set_replay_amount(this->calc_unread_replay_record_amount());
	respond.set_title(this->customer_service_detial_.__title);
	respond.set_content(this->customer_service_detial_.__content);
	respond.set_contact(this->contact_way_);
	respond.set_is_official(0);

	//每次只取最新的官方反馈
	int size = (int)CONFIG_INSTANCE->opinion_record().size();
	int opinion_index = CONFIG_INSTANCE->opinion_record(size)["record_id"].asInt();

	const Json::Value &record_json = CONFIG_INSTANCE->opinion_record(size);
	respond.set_title(record_json["record_title"].asString());
	respond.set_last_record_type(record_json["record_type"].asInt());
	respond.set_reward_id(record_json["reward_id"].asInt());
	respond.set_official_id(opinion_index);

	if (this->customer_service_detial_.__opinion_reward[opinion_index] == 0)
	{
		respond.set_is_official(1);
	}

	FINER_PROCESS_RETURN(RETURN_REQUEST_OPEN_CUSTOMER_SVC_PANNEL, &respond);
}

int LogicCustomer::request_open_replay_pannel(Message* msg)
{
	Proto50100902 respond;
	CustomerRecordVec::iterator it =
			this->customer_service_detial_.__customer_record_vec.begin();
	for(; it != this->customer_service_detial_.__customer_record_vec.end(); ++it)
	{
		CustomerServiceRecord* record = *it;
		JUDGE_CONTINUE(NULL != record);

		ProtoCustomerSVCRecord* proto_record = respond.add_record_list();
		record->serialize(proto_record);
	}
	FINER_PROCESS_RETURN(RETURN_REQUEST_OPEN_REPLAY_PANNEL, &respond);
}

int LogicCustomer::request_evaluate_customer_service_replay(Message*msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100906*, request, RETURN_REQUEST_EVALUATE_CUSTOMER_REPLAY);
//	MSG_DEBUG(%s, request->Utf8DebugString().c_str());

	int eva_level = request->evaluate_level();
	Int64 record_id = request->record_id();

	CustomerServiceDetail& detail = this->customer_service_detial_;
	CONDITION_NOTIFY_RETURN(detail.__customer_record_map.count(record_id) > 0,
			RETURN_REQUEST_EVALUATE_CUSTOMER_REPLAY, ERROR_CLIENT_OPERATE);

	CustomerServiceRecord* record = detail.__customer_record_map[record_id];
	CONDITION_NOTIFY_RETURN(NULL != record &&
			record->__has_replay &&
			record->__evaluate_tick == 0,
			RETURN_REQUEST_EVALUATE_CUSTOMER_REPLAY, ERROR_SERVER_INNER);

	record->__evaluate_level = eva_level;
	record->__evaluate_tick  = Time_Value::gettimeofday().sec();

	//TODO: save in db
	this->sumit_customer_service_record_to_backstage(record);

	Proto50100906 respond;
	respond.set_record_id(record_id);
	respond.set_evaluate_level(eva_level);
	FINER_PROCESS_RETURN(RETURN_REQUEST_EVALUATE_CUSTOMER_REPLAY, &respond);
}

int LogicCustomer::summit_customer_service_record(Message*msg)
{
	JUDGE_RETURN(this->validate_operate_tick() == true, 0);
	MSG_DYNAMIC_CAST_RETURN(Proto10100903*, request, RETURN_SUMMIT_CUSTOMER_SERVICE_RECORD);

	int is_summit = request->is_summit();
	int record_type = request->record_type();
	this->customer_service_detial_.__last_summit_type = record_type;

	if (is_summit) // summit
	{
		int size = CONFIG_INSTANCE->opinion_record().size();
		int opinion_index = CONFIG_INSTANCE->opinion_record(size)["record_id"].asInt();
		CONDITION_NOTIFY_RETURN(opinion_index > 0, RETURN_SUMMIT_CUSTOMER_SERVICE_RECORD,
				ERROR_CLIENT_OPERATE);

		if (this->customer_service_detial_.__opinion_reward[opinion_index] == 0)
		{
			//每次只取最新的官方反馈
			this->request_add_reward(CONFIG_INSTANCE->opinion_record
					(size)["reward_id"].asInt(), ADD_FROM_OPINION_RECORD);
			this->customer_service_detial_.__opinion_reward[opinion_index] = 1;
		}

		this->sumit_customer_service_record(request->title(), request->content(),
				record_type, opinion_index, request->evaluate_star());

		this->customer_service_detial_.__title.clear();
		this->customer_service_detial_.__content.clear();
	}
	else // save setting
	{
		this->customer_service_detial_.__title = request->title();
		this->customer_service_detial_.__content = request->content();
	}

	Proto50100903 respond;
	respond.set_is_summit(is_summit);
	FINER_PROCESS_RETURN(RETURN_SUMMIT_CUSTOMER_SERVICE_RECORD, &respond);
}

int LogicCustomer::request_read_customer_service_record(Message*msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100904*, request, RETURN_REQUEST_READ_CUSTOMER_SVC_RECORD);
	Int64 record_id = request->record_id();

	CustomerServiceDetail& detail = this->customer_service_detial();
	CONDITION_NOTIFY_RETURN(detail.__customer_record_map.count(record_id) > 0,
			RETURN_REQUEST_READ_CUSTOMER_SVC_RECORD, ERROR_SERVER_INNER);

	CustomerServiceRecord* record = detail.__customer_record_map[record_id];
	CONDITION_NOTIFY_RETURN(NULL != record,
			RETURN_REQUEST_READ_CUSTOMER_SVC_RECORD, ERROR_SERVER_INNER);

	JUDGE_RETURN(record->__has_replay == 1 && record->__has_read == 0, ERROR_CLIENT_OPERATE);

	record->__has_read = 1;
	this->sort_customer_service_replay_record();
	this->sumit_customer_service_record_to_backstage(record);

	FINER_PROCESS_NOTIFY(RETURN_REQUEST_READ_CUSTOMER_SVC_RECORD);
}

int LogicCustomer::request_delete_customer_service_record(Message*msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100905*, request, RETURN_REQUEST_DEG_CUSTOMER_SVC_RECORD);
	Int64 record_id = request->record_id();

	CustomerServiceDetail& detail = this->customer_service_detial();
	CONDITION_NOTIFY_RETURN(detail.__customer_record_map.count(record_id) > 0,
			RETURN_REQUEST_DEG_CUSTOMER_SVC_RECORD, ERROR_SERVER_INNER);

	CustomerServiceRecord* record = detail.__customer_record_map[record_id];
	CONDITION_NOTIFY_RETURN(NULL != record,
			RETURN_REQUEST_DEG_CUSTOMER_SVC_RECORD, ERROR_SERVER_INNER);

	this->delete_customer_service_record(record_id);
	//TODO:save in db
	this->delete_customer_service_record_to_backstage(record);
	this->sort_customer_service_replay_record();
	MSG_DEBUG(%s-%ld, record->__title.c_str(), record->__record_id);
//	Proto50100905 respond;
//	respond.set_record_id(record_id);
	FINER_PROCESS_NOTIFY(RETURN_REQUEST_DEG_CUSTOMER_SVC_RECORD);
}

int LogicCustomer::notify_update_replay_pannel()
{
	Proto80100901 respond;
	respond.set_unread_amount(this->calc_unread_replay_record_amount());

	CustomerRecordVec::iterator it =
			this->customer_service_detial_.__customer_record_vec.begin();
	for(; it != this->customer_service_detial_.__customer_record_vec.end(); ++it)
	{
		CustomerServiceRecord* record = *it;
		JUDGE_CONTINUE(NULL != record);

		ProtoCustomerSVCRecord* proto_record = respond.add_replay_set();
		record->serialize(proto_record);
	}
//	MSG_DEBUG(%s, respond.Utf8DebugString().c_str());
	FINER_PROCESS_RETURN(ACTIVE_CUSTOMER_SERVICE_RECORD_UPDATE, &respond);
}

int LogicCustomer::calc_unread_replay_record_amount(void)
{
	int cnt = 0;
	CustomerServiceDetail& detail = this->customer_service_detial();
	CustomerRecordVec::iterator it = detail.__customer_record_vec.begin();
	for(; it != detail.__customer_record_vec.end(); ++it)
	{
		CustomerServiceRecord* iter_record = *it;
		JUDGE_CONTINUE(NULL != iter_record);
		JUDGE_CONTINUE(iter_record->__has_read == 0 && iter_record->__has_replay == 1);

		++cnt;
	}
	return cnt;
}

CustomerServiceDetail& LogicCustomer::customer_service_detial()
{
	return this->customer_service_detial_;
}

void LogicCustomer::set_contact_way(std::string string)
{
	 this->contact_way_ = string;
}

void LogicCustomer::reset()
{
	this->customer_service_detial_.reset();
	this->contact_way_ = "";
}

int LogicCustomer::customer_svc_time_up(void)
{
	//check and delete
	this->auto_check_and_delete_expire_time_customer_svc_record();

	//load new record
	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, -1);

	BackCustomerSVC::request_load_customer_service_record(this->logic_player(), data_map);
	if(TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(),
			TRANS_LOAD_BACK_CUSTOMER_SVC_RECORD, data_map, LOGIC_MONITOR->logic_unit()) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}
	return 0;
}

int LogicCustomer::after_customer_svc_timeup_load_data(Transaction* trans)
{
	JUDGE_RETURN(trans != NULL, -1);
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

	MongoData* mongo_data = NULL;
	if(data_map->find_data(BackCustomerSVCRecord::COLLECTION, mongo_data) == 0)
	{
		CustomerRecordVec modify_set;
		auto_ptr<TQueryCursor> cursor = mongo_data->multithread_cursor();
		this->bson2customer_service_record(cursor, modify_set);

		//update db
		if((int)modify_set.size() > 0)
		{
			CustomerRecordVec::iterator it = modify_set.begin();
			for(; it != modify_set.end(); ++it)
			{
				CustomerServiceRecord* record = *it;
				JUDGE_CONTINUE(NULL != record);

				this->sumit_customer_service_record_to_backstage(record);
			}

			this->sort_customer_service_replay_record();
		}
	}

	trans->summit();
	return 0;
}

int LogicCustomer::auto_check_and_delete_expire_time_customer_svc_record(void)
{
	int need_sort = false;
	CustomerServiceDetail& detail = this->customer_service_detial();

	LongMap index_map;
	detail.fetch_record_map(index_map);

	for (LongMap::iterator iter = index_map.begin(); iter != index_map.end(); ++iter)
	{
		Int64 record_id = iter->first;

		CustomerServiceRecord* record = detail.__customer_record_map[record_id];
		JUDGE_CONTINUE(record != NULL);
		JUDGE_CONTINUE(LogicCustomer::need_delete(record) == true);

		//do delete opra
		this->delete_customer_service_record(record_id);
		//TODO:save in db
		this->delete_customer_service_record_to_backstage(record);

		need_sort = true;
	}

	if (need_sort)
	{
		this->sort_customer_service_replay_record();
	}

	return 0;
}

int LogicCustomer::sort_customer_service_replay_record(void)
{
//	CustomerServiceDetail& detail = this->customer_service_detial();
	CustomerRecordVec& record_vec = this->customer_service_detial().__customer_record_vec;
	JUDGE_RETURN((int)record_vec.size() > 0, -1);

	std::sort(record_vec.begin(), record_vec.end(), LogicCustomer::cmp_customer_svc_record);
	this->notify_update_replay_pannel();
	return 0;
}

int LogicCustomer::sumit_customer_service_record(const std::string& title, const std::string& content, const int record_type,
		const int opinion_index, const int evaluate_star)
{
	CustomerServiceRecord* record = POOL_MONITOR->customer_service_record_pool()->pop();
	JUDGE_RETURN(NULL != record, -1);

	record->__record_id = LogicCustomer::fetch_global_customer_svc_record_index();
	if(record->__record_id == -1)
		MSG_USER(ERROR_GEN_CUSTOMER_SVC_RECORD_ID--player_id:%ld--player_name:%s--record_index:%ld,
				this->role_id(), this->name(), record->__record_id);

	record->__record_type = record_type;
	record->__send_tick = Time_Value::gettimeofday().sec();
	record->__sender_id = this->role_id();
	record->__sender_level = this->role_level();
	record->__sender_name = this->role_detail().name();
	record->__server_code = CONFIG_INSTANCE->server_id();
	record->__agent = this->role_detail().__agent_code;
	record->__content = content;
	record->__title = title;

	record->__evaluate_star = evaluate_star;
	record->__opinion_index = opinion_index;

	record->__recharge_gold = this->role_detail().__recharge_total_gold;

	this->customer_service_detial_.__customer_record_vec.push_back(record);
	this->customer_service_detial_.__customer_record_map.insert(
			CustomerRecordMap::value_type(record->__record_id, record));

	this->sort_customer_service_replay_record();

	//TODO: save in db
	this->sumit_customer_service_record_to_backstage(record);
	return 0;
}

int LogicCustomer::sumit_customer_service_record_to_backstage(CustomerServiceRecord* record)
{
	JUDGE_RETURN(NULL != record, -1);

	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, -1);

	BackCustomerSVC::update_backstage_customer_service_record(record, data_map);
	if(TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(),
			TRANS_UPDATE_CUSTOMER_SVC_RECORD, data_map) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}
	return 0;
}


int LogicCustomer::delete_customer_service_record(const Int64 record_id)
{
	CustomerServiceDetail& detail = this->customer_service_detial();
	CustomerRecordVec::iterator it = detail.__customer_record_vec.begin();
	for(; it != detail.__customer_record_vec.end(); ++it)
	{
		CustomerServiceRecord* iter_record = *it;
		JUDGE_CONTINUE(NULL != iter_record);
		JUDGE_CONTINUE(iter_record->__record_id == record_id);

		break;
	}
	if(it != detail.__customer_record_vec.end())
		detail.__customer_record_vec.erase(it);

	detail.__customer_record_map.erase(record_id);

	return 0;
}

int LogicCustomer::delete_customer_service_record_to_backstage(CustomerServiceRecord* record)
{
	JUDGE_RETURN(NULL != record, -1);

	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, -1);

	BackCustomerSVC::request_remove_customer_service_record(record->__record_id, data_map);
	MSG_DEBUG(%s-%ld, record->__title.c_str(), record->__record_id);
	if(TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(),
			TRANS_UPDATE_CUSTOMER_SVC_RECORD, data_map) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}
	return 0;
}

int LogicCustomer::bson2customer_service_record(auto_ptr<TQueryCursor> cursor, CustomerRecordVec& modify_set)
{
	while(cursor->more())
	{
		BSONObj obj = cursor->next();
		Int64 record_id = obj[BackCustomerSVCRecord::ID].numberLong();
		JUDGE_CONTINUE(this->customer_service_detial_.__customer_record_map.count(record_id) > 0);

		CustomerServiceRecord* record = this->customer_service_detial_.__customer_record_map[record_id];
		JUDGE_CONTINUE(NULL != record);

		record->__replay_content = obj[BackCustomerSVCRecord::REPLAY_CONTENT].String();
		record->__has_replay = 1;

		MSG_DEBUG(%ld, record_id);

		modify_set.push_back(record);
	}
	return 0;
}

Int64 LogicCustomer::fetch_global_customer_svc_record_index(void)
{
	int64_t record_id = 0;
	JUDGE_RETURN(LOGIC_MONITOR->find_global_value(Global::CUSTOMER_SERVICE, record_id) == 0, -1);

	record_id += 1;

	MSG_USER(GEN_CUSTOMER_SVC_RECORD_ID--record_index:%ld,  record_id);
	//TODO: update global index
	BackCustomerSVC::save_global_customer_service_index(record_id);
	LOGIC_MONITOR->bind_global_value(Global::CUSTOMER_SERVICE, record_id);
	return record_id;
}

bool LogicCustomer::cmp_customer_svc_record(CustomerServiceRecord* lhs, CustomerServiceRecord* rhs)
{
	if(lhs->__has_replay == rhs->__has_replay)
	{
		if(lhs->__has_read == rhs->__has_read)
			return lhs->__send_tick < rhs->__send_tick;
		return lhs->__has_read > rhs->__has_read;
	}
	return lhs->__has_replay > rhs->__has_replay;
}

bool LogicCustomer::need_delete(CustomerServiceRecord* record)
{
	JUDGE_RETURN(NULL != record, false);
	JUDGE_RETURN(0 != record->__evaluate_tick, false);
	JUDGE_RETURN(record->__has_replay > 0, false);

	const Json::Value& cfg = CONFIG_INSTANCE->tiny("customer_record_disappear");
	JUDGE_RETURN(cfg != Json::Value::null, false);

	int disappear_hour = cfg[0u].asInt();
	Int64 now_tick = Time_Value::gettimeofday().sec();
	if(now_tick - record->__evaluate_tick >= disappear_hour * 3600)
		return true;

	return false;
}
