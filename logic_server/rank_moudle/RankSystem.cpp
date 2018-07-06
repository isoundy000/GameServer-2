/*
 * RankSystem.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: louis
 */

#include "RankSystem.h"
#include "MapStruct.h"
#include "TransactionMonitor.h"
#include "MongoData.h"
#include "MongoDataMap.h"
#include "Transaction.h"
#include "LogicPlayer.h"
#include "LogicMonitor.h"
#include "MMORankPannel.h"
#include "TQueryCursor.h"
#include "GameField.h"
#include "ProtoDefine.h"
#include <mongo/client/dbclient.h>
#include "SerialRecord.h"
#include "MMOGlobal.h"

RankSystem::RankSystem()
{
	// TODO Auto-generated constructor stub
	this->average_role_level_ = 1;
	this->rank_cache_timer_.rank_system_ = this;

	//init rank cache
	double time_inner[] = {0,
			MAP_LOGIC_RANK_ON_SECOND};
	int time_inner_size = ARRAY_LENGTH(time_inner);
	this->rank_cache_tick_.init(RankSystem::RANK_CACHE_TYPE_END, RankSystem::TIMEUP_END, time_inner, time_inner_size);
}

RankSystem::~RankSystem()
{
	// TODO Auto-generated destructor stub
}


int RankSystem::init(void)
{
	return 0;
}

int RankSystem::fina(void)
{
	return 0;
}

int RankSystem::start(void)
{
	this->init_rank_data();
	this->init_rank_manager();

	MSG_USER("RANK_SYS %d %d %d", this->rank_show_pannel_.size(), this->rank_pannel_.size(),
			this->rank_manager_.size());
	MMOGlobal::load_global_key_value(Global::AVERAGE_ROLE_LEVEL,
			this->average_role_level_);

    Time_Value nowtime = Time_Value::gettimeofday(), interval(1);
    for (int i = 0; i < TIMEUP_END; ++i)
    {
        this->rank_cache_tick_.update_timeup_tick(i, nowtime);
    }

	MSG_USER("RANK_SYS %d %d %d", this->rank_show_pannel_.size(), this->rank_pannel_.size(),
			this->rank_manager_.size());
    this->rank_cache_timer_.schedule_timer(interval);
	return 0;
}

int RankSystem::stop(void)
{
	this->rank_cache_timer_.cancel_timer();
	this->request_save_rank_pannel_data();
	this->save_rank_manager_last_refresh();
	this->reset();
	return 0;
}

int RankSystem::init_rank_data(void)
{
	MMORankPannel::load_rank_pannel_data();
	return 0;
}

RankRecordPool* RankSystem::rank_record_pool()
{
	return &(this->rank_record_pool_);
}

RankPannel& RankSystem::rank_pannel()
{
	return this->rank_pannel_;
}

RankShowPannel& RankSystem::rank_show_pannel()
{
	return this->rank_show_pannel_;
}

GameCache& RankSystem::cache_tick()
{
	return this->rank_cache_tick_;
}

PoolPackage<PlayerOfflineData, Int64>& RankSystem::offline_data_map()
{
	return this->offline_data_map_;
}

RankRecord* RankSystem::pop_rank_record()
{
	return this->rank_record_pool_.pop();
}

int RankSystem::push_rank_record(RankRecord* record)
{
	return this->rank_record_pool_.push(record);
}

void RankSystem::reset(void)
{
	RankPannel::iterator iter = this->rank_pannel_.begin();
	for(; iter != this->rank_pannel_.end(); ++iter)
	{
		RankRecordMap& record_map = iter->second;
		RankRecordMap::iterator iterator = record_map.begin();
		for(; iterator != record_map.end(); ++iterator)
		{
			this->rank_record_pool_.push(iterator->second);
		}
		record_map.clear();
	}
	this->rank_pannel_.clear();

	RankShowPannel::iterator it = this->rank_show_pannel_.begin();
	for(; it != this->rank_show_pannel_.end(); ++it)
	{
		it->second.clear();
	}
	this->rank_show_pannel_.clear();
}

int RankSystem::clear_rank_data_by_rank_type(const int rank_type)
{
	JUDGE_RETURN(this->rank_pannel_.count(rank_type) > 0, 0);
	JUDGE_RETURN(this->rank_pannel_[rank_type].size() > 0, 0);

	RankRecordMap& record_map = this->rank_pannel_[rank_type];

	if (this->rank_show_pannel_.count(rank_type) > 0)
	{
		RankRecordVec &rank_vec = this->rank_show_pannel_[rank_type];
		for (RankRecordVec::iterator iter = rank_vec.begin(); iter != rank_vec.end(); ++iter)
		{
			RankRecord *rec = *iter;
			record_map.erase(rec->__single_player_info.__role_id);
			this->rank_record_pool_.push(rec);
		}
		rank_vec.clear();
	}

	RankRecordMap::iterator it = record_map.begin();
	for(; it != record_map.end(); ++it)
	{
		this->rank_record_pool_.push(it->second);
	}
	record_map.clear();
	return 0;
}

int RankSystem::update_player_offline(Message* msg)
{
//	MSG_DYNAMIC_CAST_RETURN(Proto30100215*, request, -1);
//
//	PlayerOfflineData* offline_data = this->check_and_pop_player_offline(request->role_id());
//	JUDGE_RETURN(offline_data != NULL, -1);
//
//	switch (request->offline_type())
//	{
//	case GameEnum::OFFLINE_DT_BEAST_MOUNT:
//	{
//		offline_data->mount_beast_info_ = request->offline_data();
//		offline_data->mount_beast_tick_ = ::time(NULL);
//
//		MMORankPannel::save_offline_data(offline_data, GameEnum::OFFLINE_DT_BEAST_MOUNT);
//		break;
//	}
//
//	}

	return 0;
}

int RankSystem::fetch_player_offline(LogicPlayer* player, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400261*, request, -1);

	PlayerOfflineData* offline_data = this->find_player_offline(request->other_id());
	if (offline_data == NULL)
	{
		return player->request_load_other_master(request->other_id(), request->query_type());
	}

	switch (request->recogn())
	{
	case RETURN_OTHER_MASTER_INFO:
	{
		Proto50100031 master_info;
		master_info.ParseFromString(offline_data->mount_beast_info_);
		master_info.set_query_type(request->query_type());

		return LOGIC_MONITOR->dispatch_to_client(player, request->recogn(), master_info.SerializeAsString());
	}

	default:
	{
		break;
	}
	}

	return 0;
}

int RankSystem::map_fetch_rank_info(const ProtoHead& head, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100235*, request, -1);

	MSG_USER("RankSystem %d, %d, %d", request->second_type(),
			head.__src_scene_id, head.__src_line_id);

	Proto31400026 rank_info;
	rank_info.set_second_type(request->second_type());

	int rank_type;
	switch(request->second_type())
	{
	case RANK_FIGHT_LEVEL:
	{
		RankShowPannel& rank_pannel = RANK_SYS->rank_show_pannel();
		RankRecordVec& rank_vec = rank_pannel[RANK_FIGHT_LEVEL];
		int length = 20;
		length = length > int(rank_vec.size())? rank_vec.size() : length;
		if(length <= 0)
		{
			Proto30401602 respond;
			respond.set_ave_lvl(0);
			return LOGIC_MONITOR->dispatch_to_aim_sid(head, &respond);
		}
		int total_lvl = 0;
		for(int i = 0; i < length; ++i)
		{
			total_lvl += (rank_vec[i]->__single_player_info.__rank_value);
			MSG_USER("TOP 20 RANK_LEVEL-index:%d,role_lvl:%d",i,rank_vec[i]->__single_player_info.__rank_value);
		}
		int ave_lvl = total_lvl / length;
		MSG_USER("TOP 20 RANK_AVERAGE_LEVEL-total_lvl:%d,length:%d,ave_lvl:%d",total_lvl,length,ave_lvl);
		Proto30401602 respond;
		respond.set_ave_lvl(ave_lvl);
		return LOGIC_MONITOR->dispatch_to_aim_sid(head, &respond);
	}

	default:
	{
		rank_type = request->second_type();
		break;
	}
	}

	RankShowPannel& rank_pannel = RANK_SYS->rank_show_pannel();
	RankRecordVec& rank_vec = rank_pannel[rank_type];

	int msg_num = std::min<int>(100, rank_vec.size());
	for(int i = 0; i < msg_num; ++i)
	{
		ProtoRankRecord* proto_record = rank_info.add_rank_set();
		RankRecord* &cur_record = rank_vec[i];
		cur_record->serialize(proto_record);
	}

	return LOGIC_MONITOR->dispatch_to_aim_sid(head, &rank_info);
}

PlayerOfflineData* RankSystem::find_player_offline(Int64 role_id)
{
	PlayerOfflineData *offline_data = this->offline_data_map_.find_object(role_id);
	if (offline_data != NULL)
		offline_data->mount_beast_tick_ = ::time(NULL);
	return offline_data;
}

PlayerOfflineData* RankSystem::check_and_pop_player_offline(Int64 role_id)
{
	PlayerOfflineData* offline_data = this->find_player_offline(role_id);
	JUDGE_RETURN(offline_data == NULL, offline_data);

	offline_data = this->offline_data_map_.pop_object();
	offline_data->role_id_ = role_id;

	this->offline_data_map_.bind_object(role_id, offline_data);
	return offline_data;
}

int RankSystem::update_rank_data(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31401601*, request, -1);
    
    if (request->rank_type() > 10000)
        return this->update_realtime_rank_data(msg);

    if (request->rank_type() == RANK_SINGLE_SCRIPT_ZYFM)
	    return this->request_save_player_script_zyfm_date(request);

    return -1;
}

int RankSystem::request_save_player_script_zyfm_date(Proto31401601* pass_info)
{
	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();

	RankRecord record;
	record.__single_player_info.__role_id = pass_info->role_id();
	record.__single_player_info.__role_name = pass_info->role_name();
	record.__single_player_info.__league_name = pass_info->league_name();
	record.__single_player_info.__vip_type = pass_info->vip_type();
	record.__single_player_info.__rank_value = pass_info->rank_value();
	record.__single_player_info.__achive_tick = pass_info->achieve_tick();
//	record.__single_player_info.__is_worship = pass_info->is_worship();
//	record.__single_player_info.__worship_num = pass_info->worship_num();

	MMORankPannel::update_date_player_script_zyfm(&record, data_map);

	if(TRANSACTION_MONITOR->request_mongo_transaction(0,
			TRANS_SAVE_PLAYER_SCRIPT_ZYFM, data_map) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}

	return 0;
}

int RankSystem::make_up_rank_pannel_info(const int rank_type, RankRecordVec& rank_set)
{
	switch(rank_type)
	{
	case RANK_FIGHT_FORCE:
	case RANK_KILL_VALUE:
	case RANK_HERO:
	case RANK_FIGHT_LEVEL:
	case RANK_PET:
	case RANK_MOUNT:
	case RANK_SINGLE_SCRIPT_ZYFM:
    case RANK_SEND_FLOWER:
    case RANK_RECV_FLOWER:
	case RANK_FUN_MOUNT:
	case RANK_FUN_GOD_SOLIDER:
	case RANK_FUN_MAGIC_EQUIP:
	case RANK_FUN_XIAN_WING:
	case RANK_FUN_LING_BEAST:
	case RANK_FUN_BEAST_EQUIP:
	case RANK_FUN_BEAST_MOUNT:
	case RANK_FUN_BEAST_WING:
	case RANK_FUN_BEAST_MAO:
	case RANK_FUN_TIAN_GANG:
		this->make_up_rank_pannel(rank_type, rank_set);
		break;
	default:
		break;
	}
	return 0;
}

int RankSystem::sort_rank_data(const int rank_type, RankRecordVec& rank_set)
{
    std::sort(rank_set.begin(), rank_set.end(), rank_record_cmp);
	return 0;
}

int RankSystem::refresh_rank_data_at_hour(void)
{
//	const Json::Value &rank_json = CONFIG_INSTANCE->rank_json();
//	JUDGE_RETURN(rank_json != Json::Value::null &&
//			rank_json.isMember("default_refresh_rank_types") &&
//			rank_json["default_refresh_rank_types"].isArray(), -1);
//
//	for(int i = 0; i < (int)rank_json["default_refresh_rank_types"].size(); ++i)
//	{
//		int refresh_rank_type = rank_json["default_refresh_rank_types"][i].asInt();
//		const Json::Value &rank_type_json = CONFIG_INSTANCE->rank_pannel(refresh_rank_type);
//		JUDGE_CONTINUE(rank_type_json != Json::Value::null);
//		JUDGE_CONTINUE(rank_manager_.count(refresh_rank_type) > 0);
//
//
//		RankRefreshDetail& refresh_detail = rank_manager_[refresh_rank_type];
//
//		this->request_refresh_rank_data(refresh_detail.__rank_type);
//		this->modify_rank_refesh_tick(refresh_detail);
//	}

	return 0;
}

int RankSystem::request_refresh_rank_data(const int rank_type)
{
	int recogn = 0;
	BSONObj sort_condition;
	BSONObj query_conditon = BSONObj();
	switch(rank_type)
	{
	case RANK_FIGHT_FORCE:
		recogn = TRANS_LOAD_FIGHT_FORCE_RANK_DATA;
		sort_condition = BSON(DBRanker::FIGHT_FORCE << -1);
		break;
//	case RANK_KILL_VALUE:
//		recogn = TRANS_LOAD_KILL_VALUE_RANK_DATA;
//		sort_condition = BSON(DBRanker::WEEK_TICK << -1
//				<< DBRanker::KILL_NORMAL << -1
//				<< DBRanker::FIGHT_LEVEL << 1);
//		break;
//	case RANK_HERO:
//		recogn = TRANS_LOAD_HERO_RANK_DATA;
//		sort_condition = BSON(DBRanker::WEEK_TICK << -1
//			   << DBRanker::KILL_EVIL << -1
//			   << DBRanker::FIGHT_LEVEL << 1);
//		break;
	case RANK_FIGHT_LEVEL:
		recogn = TRANS_LOAD_FIGHT_LEVEL_RANK_DATA;
		sort_condition = BSON(DBRanker::FIGHT_LEVEL << -1
				<< DBRanker::EXPERIENCE << -1);
		break;
	case RANK_PET:
		recogn = TRANS_LOAD_PET_RANK_DATA;
		sort_condition = BSON(DBRanker::Pet::PET_FORCE << -1);
		break;
//	case RANK_MOUNT:
//		recogn = TRANS_LOAD_MOUNT_RANK_DATA;
//		sort_condition = BSON(DBRanker::MOUNT_GRADE << -1);
//		break;
	case RANK_FUN_MOUNT:
		recogn = TRANS_LOAD_RANK_FUN_INFO;
		sort_condition = BSON(DBRanker::FUN_MOUNT_GRADE << -1
				<< DBRanker::FUN_MOUNT_FORCE << -1);
		break;
	case RANK_FUN_GOD_SOLIDER:
		recogn = TRANS_LOAD_RANK_FUN_INFO;
		sort_condition = BSON(DBRanker::FUN_GOD_SOLIDER_GRADE << -1
				<< DBRanker::FUN_GOD_SOLIDER_FORCE << -1);
		break;
	case RANK_FUN_MAGIC_EQUIP:
		recogn = TRANS_LOAD_RANK_FUN_INFO;
		sort_condition = BSON(DBRanker::FUN_MAGIC_EQUIP_GRADE << -1
				<< DBRanker::FUN_MAGIC_EQUIP_FORCE << -1);
		break;
	case RANK_FUN_XIAN_WING:
		recogn = TRANS_LOAD_RANK_FUN_INFO;
		sort_condition = BSON(DBRanker::FUN_XIAN_WING_GRADE << -1
				<< DBRanker::FUN_XIAN_WING_FORCE << -1);
		break;
	case RANK_FUN_LING_BEAST:
		recogn = TRANS_LOAD_RANK_FUN_INFO;
		sort_condition = BSON(DBRanker::FUN_LING_BEAST_GRADE << -1
				<< DBRanker::FUN_LING_BEAST_FORCE << -1);
		break;
	case RANK_FUN_BEAST_EQUIP:
		recogn = TRANS_LOAD_RANK_FUN_INFO;
		sort_condition = BSON(DBRanker::FUN_BEAST_EQUIP_GRADE << -1
				<< DBRanker::FUN_BEAST_EQUIP_FORCE << -1);
		break;
	case RANK_FUN_BEAST_MOUNT:
		recogn = TRANS_LOAD_RANK_FUN_INFO;
		sort_condition = BSON(DBRanker::FUN_BEAST_MOUNT_GRADE << -1
				<< DBRanker::FUN_BEAST_MOUNT_FORCE << -1);
		break;

	case RANK_FUN_BEAST_WING:
		recogn = TRANS_LOAD_RANK_FUN_INFO;
		sort_condition = BSON(DBRanker::FUN_BEAST_WING_GRADE << -1
				<< DBRanker::FUN_BEAST_WING_FORCE << -1);
		break;

	case RANK_FUN_BEAST_MAO:
		recogn = TRANS_LOAD_RANK_FUN_INFO;
		sort_condition = BSON(DBRanker::FUN_BEAST_MAO_GRADE << -1
				<< DBRanker::FUN_BEAST_MAO_FORCE << -1);
		break;

	case RANK_FUN_TIAN_GANG:
		recogn = TRANS_LOAD_RANK_FUN_INFO;
		sort_condition = BSON(DBRanker::FUN_TIAN_GANG_GRADE << -1
				<< DBRanker::FUN_TIAN_GANG_FORCE << -1);
		break;

	case RANK_SINGLE_SCRIPT_ZYFM:
		recogn = TRANS_LOAD_SCRIPT_ZYFM_RANK_DATA;
		sort_condition = BSON(DBRanker::ZYFM_PASS_KEY << -1
				<< DBRanker::ZYFM_PASS_TICK << 1
				<< DBRanker::FIGHT_FORCE << -1);
		break;
    case RANK_SEND_FLOWER:
        recogn = TRANS_LOAD_SEND_FLOWER_RANK;
        sort_condition = BSON(DBRanker::SEND_FLOWER << -1);
        query_conditon = BSON(DBRanker::SEND_FLOWER << BSON("$gt" << 0));
        break;
    case RANK_RECV_FLOWER:
        recogn = TRANS_LOAD_RECV_FLOWER_RANK;
        sort_condition = BSON(DBRanker::RECV_FLOWER << -1);
        query_conditon = BSON(DBRanker::RECV_FLOWER << BSON("$gt" << 0));
        break;
    case RANK_ACT_SEND_FLOWER:
        recogn = TRANS_LOAD_ACT_SEND_FLOWER_RANK;
        sort_condition = BSON(DBRanker::ACT_SEND_FLOWER << -1);
        query_conditon = BSON(DBRanker::ACT_SEND_FLOWER << BSON("$gt" << 0));
        break;
    case RANK_ACT_RECV_FLOWER:
        recogn = TRANS_LOAD_ACT_RECV_FLOWER_RANK;
        sort_condition = BSON(DBRanker::ACT_RECV_FLOWER << -1);
        query_conditon = BSON(DBRanker::ACT_RECV_FLOWER << BSON("$gt" << 0));
        break;

	default:
        return 0;
		break;
	}

	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	data_map->other_value_ = rank_type;
	JUDGE_RETURN(data_map != NULL, -1);
	data_map->push_multithread_query(DBRanker::COLLECTION, query_conditon, sort_condition);
	if(	TRANSACTION_MONITOR->request_mongo_transaction(0, recogn, data_map, LOGIC_MONITOR->logic_unit()) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}
	return 0;
}

int RankSystem::after_db_refresh_rank_data(Transaction* transaction)
{
	JUDGE_RETURN(transaction != NULL, -1);
	if (transaction->detail().__error != 0)
	{
		transaction->rollback();
		return transaction->detail().__error;
	}

	MongoDataMap* data_map = transaction->fetch_mongo_data_map();
	if (data_map == 0)
	{
		transaction->rollback();
		return -1;
	}
	MongoData* mongo_data = NULL;

	int src_recogn = transaction->detail().__res_recogn;

	int rank_type = 0;
	switch(src_recogn)
	{
	case TRANS_LOAD_FIGHT_FORCE_RANK_DATA:
		rank_type = RANK_FIGHT_FORCE;
		break;
	case TRANS_LOAD_KILL_VALUE_RANK_DATA:
		rank_type = RANK_KILL_VALUE;
	    break;
	case TRANS_LOAD_HERO_RANK_DATA:
		rank_type = RANK_HERO;
		break;
	case TRANS_LOAD_FIGHT_LEVEL_RANK_DATA:
		rank_type = RANK_FIGHT_LEVEL;
		break;
	case TRANS_LOAD_PET_RANK_DATA:
		rank_type = RANK_PET;
		break;
	case TRANS_LOAD_MOUNT_RANK_DATA:
		rank_type = RANK_MOUNT;
		break;
	case TRANS_LOAD_SCRIPT_ZYFM_RANK_DATA:
		rank_type = RANK_SINGLE_SCRIPT_ZYFM;
		break;
    case TRANS_LOAD_SEND_FLOWER_RANK:
        rank_type = RANK_SEND_FLOWER;
        break;
    case TRANS_LOAD_RECV_FLOWER_RANK:
        rank_type = RANK_RECV_FLOWER;
        break;
    case TRANS_LOAD_ACT_SEND_FLOWER_RANK:
    	rank_type = RANK_ACT_SEND_FLOWER;
    	break;
    case TRANS_LOAD_ACT_RECV_FLOWER_RANK:
    	rank_type = RANK_ACT_RECV_FLOWER;
    	break;

	case TRANS_LOAD_RANK_FUN_INFO:
		rank_type = data_map->other_value_;
		break;
		/*
	case TRANS_LOAD_RANK_FUN_MOUNT_INFO:
		rank_type = RANK_FUN_MOUNT;
		break;
	case TRANS_LOAD_RANK_FUN_GOD_SOLIDER_INFO:
		rank_type = RANK_FUN_GOD_SOLIDER;
		break;
	case TRANS_LOAD_RANK_FUN_MAGIC_EQUIP_INFO:
		rank_type = RANK_FUN_MAGIC_EQUIP;
		break;
	case TRANS_LOAD_RANK_FUN_XIAN_WING_INFO:
		rank_type = RANK_FUN_XIAN_WING;
		break;
	case TRANS_LOAD_RANK_FUN_LING_BEAST_INFO:
		rank_type = RANK_FUN_LING_BEAST;
		break;
	case TRANS_LOAD_RANK_FUN_BEAST_EQUIP_INFO:
		rank_type = RANK_FUN_BEAST_EQUIP;
		break;
	case TRANS_LOAD_RANK_FUN_BEAST_MOUNT_INFO:
		rank_type = RANK_FUN_BEAST_MOUNT;
		break;
*/
	default:
		break;
	}
	if(data_map->find_data(DBRanker::COLLECTION, mongo_data) == 0)
	{
		auto_ptr<TQueryCursor> data_cursor = mongo_data->multithread_cursor();
		RankRecordVec db_rank_set;
		this->bson2rank_data(data_cursor, rank_type, db_rank_set);
		this->make_up_rank_pannel_info(rank_type, db_rank_set);
	}
	transaction->summit();
	return 0;
}

int RankSystem::request_save_rank_pannel_data(void)
{
	RankPannel::iterator iter = this->rank_pannel_.begin();
	for(; iter != this->rank_pannel_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second.size() > 0);

		int rank_type = iter->first;
		this->request_save_rank_data(rank_type);
	}
	return 0;
}

int RankSystem::request_save_rank_data(const int rank_type)
{
	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	MMORankPannel::update_data(this->rank_show_pannel_, data_map, rank_type);

	if(TRANSACTION_MONITOR->request_mongo_transaction(0,
			TRANS_SAVE_RANK_PANEL_DATA, data_map) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}

	//save rank record detail
	this->request_save_rank_record_detail(rank_type);
	return 0;
}

int RankSystem::request_save_rank_record_detail(const int rank_type)
{
//	if(rank_type == RANK_PET)
//	{
//		RankPannel& rank_pannel = this->rank_pannel_;
//		JUDGE_RETURN(rank_pannel.count(rank_type) > 0, -1);
//		JUDGE_RETURN(rank_pannel[rank_type].size() > 0, -1);
//
//		DBShopMode* shop_mode = GameCommon::pop_shop_mode(TRANS_DB_SAVE_PET_RANK_DETAIL);
//		JUDGE_RETURN(NULL != shop_mode, -1);
//
//		RankRecordMap& record_map = rank_pannel[rank_type];
//		RankRecordMap::iterator it = record_map.begin();
//		for(; it != record_map.end(); ++it)
//		{
//			JUDGE_CONTINUE(NULL != it->second);
//
//			Int64 player_id = it->second->__single_player_info.__role_id;
//			shop_mode->input_argv_.extra_long_vec_.push_back(player_id);
//		}
//
//		if(TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_LOAD_SHOP_MODE,
//		DB_SHOP_LOAD_MODE, shop_mode, POOL_MONITOR->shop_mode_pool()) != 0)
//		{
//			POOL_MONITOR->shop_mode_pool()->push(shop_mode);
//			return -1;
//		}
//		MSG_USER(pet_data_save_size:%d, (int)shop_mode->input_argv_.extra_long_vec_.size());
//	}
	return 0;
}

int RankSystem::rank_system_open_level(void)
{
	int rank_level = 10;
	const Json::Value& cfg = CONFIG_INSTANCE->tiny("level_limit");
	if(cfg != Json::Value::null && cfg["rank_pannel"].asInt() != 0)
		rank_level = cfg["rank_pannel"].asInt();

	return rank_level;
}

Int64 RankSystem::calc_rank_player_pet_id(const Int64 player_id)
{
	JUDGE_RETURN(this->rank_pannel_.count(RANK_PET) > 0, 0);
	JUDGE_RETURN((int)this->rank_pannel_[RANK_PET].size() > 0, 0);

	RankRecordMap& record_map = this->rank_pannel_[RANK_PET];
	JUDGE_RETURN(record_map.count(player_id) > 0, 0);

	RankRecord* &record = record_map[player_id];
	JUDGE_RETURN(NULL != record, 0);

	return record->__single_player_info.__additional_id;
}

RankRecord* RankSystem::fetch_rank_record(const int rank_type, const int rank_index)
{
	JUDGE_RETURN(this->rank_show_pannel_.count(rank_type) > 0, NULL);

	RankRecordVec& rank_vec = this->rank_show_pannel_[rank_type];
	JUDGE_RETURN(rank_index >= 0 && rank_index < (int)rank_vec.size(), NULL);

	return rank_vec[rank_index];
}

RankRecord* RankSystem::fetch_rank_record(const int rank_type, Int64 player_id)
{
	JUDGE_RETURN(this->rank_pannel_.count(rank_type) > 0, NULL);

	RankRecordMap& rank_map = this->rank_pannel_[rank_type];
	JUDGE_RETURN(rank_map.count(player_id) > 0, NULL);

	RankRecord* &rank_record = rank_map[player_id];
	return rank_record;
}

int RankSystem::fetch_player_rank(const int rank_type, Int64 player_id)
{
	JUDGE_RETURN(this->rank_pannel_.count(rank_type) > 0, -1);

	RankRecordMap& rank_map = this->rank_pannel_[rank_type];
	JUDGE_RETURN(rank_map.count(player_id) > 0, -1);

	RankRecord* &rank_record = rank_map[player_id];
	JUDGE_RETURN(rank_record != NULL, -1);

	return rank_record->__single_player_info.__cur_rank;
}

bool RankSystem::is_player_on_rank(const int rank_type, Int64 player_id)
{
	int rank = this->fetch_player_rank(rank_type, player_id);
	return rank > 0 && rank <= GameEnum::RANK_RECORD_DEFAULT_LIMIT_NUM;
}

bool RankSystem::is_better_single_script_record(RankRecord* target_record, int rank_value, Int64 achieve_tick)
{
//	if(target_record->__single_player_info.__rank_value == rank_value)
//		return target_record->__single_player_info.__achive_tick < achieve_tick;
	return target_record->__single_player_info.__rank_value < rank_value;
}

bool RankSystem::is_better_single_script_record(RankRecord* target_record, RankRecord* src_record)
{
	return target_record->__single_player_info.__rank_value
			> src_record->__single_player_info.__rank_value;
}

int RankSystem::push_new_rank_record_to_show_pannel(RankRecord* record)
{
	JUDGE_RETURN(NULL != record, -1);

	int rank_type = record->__single_player_info.__rank_type;
	JUDGE_RETURN(this->rank_show_pannel_.count(rank_type) > 0, -1);
	JUDGE_RETURN(this->hide_player_.find(record->__single_player_info.__role_id) == this->hide_player_.end(), -1);
	RankRecordVec& record_vec = this->rank_show_pannel_[rank_type];
	RankRecordMap& record_map = this->rank_pannel_[rank_type];

	//check show pannel is full or not
	if((int)record_vec.size() >= GameEnum::RANK_RECORD_DEFAULT_LIMIT_NUM)
	{
		MSG_USER(FULL_RANK);
		RankRecord* last_record = this->fetch_rank_record(rank_type,
				GameEnum::RANK_RECORD_DEFAULT_LIMIT_NUM - 1);
		JUDGE_RETURN(NULL != last_record, -1);

		if(this->is_better_single_script_record(record, last_record)) // need replace
		{
			record_vec.pop_back();
			record_vec.push_back(record);
			record_map.insert(RankRecordMap::value_type(record->__single_player_info.__role_id, record));

			this->recycle_single_rank_record(last_record);
			record_map.erase(last_record->__single_player_info.__role_id);
		}
		else // no need replace , recycle
		{
//			this->recycle_single_rank_record(record);
			return -1;
		}
	}
	else // not full
	{
		MSG_USER(NOT_FULL_RANK);
		record_vec.push_back(record);
		record_map.insert(RankRecordMap::value_type(record->__single_player_info.__role_id, record));
	}

	//resort
	this->sort_rank_data(rank_type, record_vec);

	//make_rank_index
	this->show_pannel_refresh_data(rank_type);
	this->cache_tick().update_cache(RANK_TYPE_SINGLE_SCRIPT_ZYFM);
	return 0;
}

int RankSystem::show_pannel_refresh_data(const int rank_type)
{
	JUDGE_RETURN(this->rank_show_pannel_.count(rank_type) > 0, 0);

	RankRecordVec& record_vec = this->rank_show_pannel_[rank_type];
	RankRecordMap& record_map = this->rank_pannel_[rank_type];

	for(int i = 0; i < (int)record_vec.size(); ++i)
	{
		RankRecord* db_record = record_vec[i];
		//calc last rank
		{
			RankRecordMap::iterator it = record_map.find(db_record->__single_player_info.__role_id);
			if(it != record_map.end())
				db_record->__single_player_info.__last_rank = it->second->__single_player_info.__cur_rank;
		}

		db_record->__single_player_info.__cur_rank = i + 1;
        db_record->__vec_index = i;
	}
	return 0;
}

void RankSystem::recycle_single_rank_record(RankRecord* record)
{
	this->rank_record_pool_.push(record);
}

void RankSystem::display_rank_show_pannel(const int rank_type)
{
	JUDGE_RETURN(this->rank_show_pannel_.count(rank_type) > 0, );

	RankRecordVec& record_vec = this->rank_show_pannel_[rank_type];
	for(int i = 0; i < (int)record_vec.size(); ++i)
	{
		RankRecord* record = record_vec[i];
		JUDGE_CONTINUE(NULL != record);

		std::stringstream ss;
		ss << "last_rank : " << record->__single_player_info.__last_rank << "\t"
				<< "cur_rank : " << record->__single_player_info.__cur_rank << "\t"
				<< "name : " << record->__single_player_info.__role_name.c_str() << "\t"
				<< "league_name : " << record->__single_player_info.__league_name.c_str() << "\t"
				<< "vip_type : " << record->__single_player_info.__vip_type << "\t"
				<< "role_id : " << record->__single_player_info.__role_id << "\t"
				<< "rank_value : " << record->__single_player_info.__rank_value << "\t"
				<< "achieve_tick : " << record->__single_player_info.__achive_tick  << "\t"
				<< "vip_type : " << record->__single_player_info.__vip_type << "\t"
				<< "worship_num : " << record->__single_player_info.__worship_num << "\t"
				<< "is_worship : " << record->__single_player_info.__is_worship << "\t"
				<< std::endl;
		MSG_USER(%s, ss.str().c_str());
	}
}

int RankSystem::make_up_rank_pannel(const int rank_type, RankRecordVec& db_record_vec)
{
	JUDGE_RETURN(db_record_vec.empty() == false, -1);
	this->sort_rank_data(rank_type, db_record_vec);

//	int rank_limit = GameCommon::fetch_rank_record_limit_form_config();
	int rank_limit = db_record_vec.size();
//	rank_limit = std::max<int>(db_record_vec.size(), rank_limit);
//	rank_limit = std::min<int>(db_record_vec.size(), rank_limit);
	RankRecordVec show_vec;
	show_vec.reserve(rank_limit);

	RankRecordMap& rank_data_map = this->rank_pannel_[rank_type];
	RankRecordMap tmp_rank_data_map;

	// 排行流水
	int serial_rank_type = 0;
	switch(rank_type)
	{
	case RANK_FIGHT_FORCE: serial_rank_type = SERIAL_RANK_TYPE_FIGHT_FORCE; break;
	case RANK_KILL_VALUE: serial_rank_type = SERIAL_RANK_TYPE_KILL_VALUE; break;
	case RANK_HERO: serial_rank_type = SERIAL_RANK_TYPE_HERO; break;
	case RANK_FIGHT_LEVEL: serial_rank_type = SERIAL_RANK_TYPE_FIGHT_LEVEL; break;
	case RANK_PET: serial_rank_type = SERIAL_RANK_TYPE_PET; break;
	case RANK_MOUNT: serial_rank_type = SERIAL_RANK_TYPE_MOUNT; break;
	case RANK_SINGLE_SCRIPT_ZYFM: serial_rank_type = SERIAL_RANK_TYPE_SCRIPT_ZYFM; break;
    case RANK_SEND_FLOWER: serial_rank_type = SERIAL_RANK_TYPE_SEND_FLOWER; break;
    case RANK_RECV_FLOWER: serial_rank_type = SERIAL_RANK_TYPE_RECV_FLOWER; break;

	case RANK_FUN_MOUNT: serial_rank_type = SERIAL_RANK_FUN_MOUNT; break;
	case RANK_FUN_GOD_SOLIDER: serial_rank_type = SERIAL_RANK_FUN_GOD_SOLIDER; break;
	case RANK_FUN_MAGIC_EQUIP: serial_rank_type = SERIAL_RANK_FUN_MAGIC_EQUIP; break;
	case RANK_FUN_XIAN_WING: serial_rank_type = SERIAL_RANK_FUN_XIAN_WING; break;
	case RANK_FUN_LING_BEAST: serial_rank_type = SERIAL_RANK_FUN_LING_BEAST; break;
	case RANK_FUN_BEAST_EQUIP: serial_rank_type = SERIAL_RANK_FUN_BEAST_EQUIP; break;
	case RANK_FUN_BEAST_MOUNT: serial_rank_type = SERIAL_RANK_FUN_BEAST_MOUNT; break;
	case RANK_FUN_BEAST_WING: serial_rank_type = SERIAL_RANK_FUN_BEAST_WING; break;
	case RANK_FUN_BEAST_MAO: serial_rank_type = SERIAL_RANK_FUN_BEAST_MAO; break;
	case RANK_FUN_TIAN_GANG: serial_rank_type = SERIAL_RANK_FUN_TIAN_GANG; break;
	}

	int total_value = 0;
	Int64 record_time = Time_Value::gettimeofday().sec();
	for(int i = 0; i < rank_limit; ++i)
	{
		RankRecord* db_record = db_record_vec[i];
		//排行榜过滤掉指定玩家id
		JUDGE_CONTINUE(this->hide_player_.count(db_record->__single_player_info.__role_id) == 0);

		//calc last rank
		RankRecordMap::iterator it = rank_data_map.find(db_record->__single_player_info.__role_id);
		if(it != rank_data_map.end())
		{
			db_record->__single_player_info.__last_rank = it->second->__single_player_info.__cur_rank;
		}

		db_record->__single_player_info.__cur_rank = i + 1;
        db_record->__vec_index = int(show_vec.size());

		tmp_rank_data_map.insert(RankRecordMap::value_type(db_record->__single_player_info.__role_id, db_record));
		show_vec.push_back(db_record);

		BaseRankInfo &rank_info = db_record->__single_player_info;
		switch(serial_rank_type)
		{
		default:
		{
			SERIAL_RECORD->record_rank(PairObj(rank_info.__role_id, rank_info.__vip_type),
					rank_info.__role_name, serial_rank_type, rank_info.__cur_rank, record_time,
					rank_info.__rank_value);
			break;
		}
		}

		if (rank_type == RANK_FIGHT_LEVEL)
		{
			total_value += rank_info.__rank_value;
		}

	}

	// 回收多餘數據的空間
	for(uint i = rank_limit; i < db_record_vec.size(); ++i)
	{
		this->rank_record_pool_.push(db_record_vec[i]);
	}

	this->clear_rank_data_by_rank_type(rank_type);
	this->rank_pannel_[rank_type] = tmp_rank_data_map;

	if (rank_type == RANK_FIGHT_LEVEL && total_value > 0)
	{
		this->average_role_level_ = std::max<int>(1, total_value / rank_limit);
		MMOGlobal::save_global_key_to_mongo_unit(Global::AVERAGE_ROLE_LEVEL,
				this->average_role_level_);

		Proto30400025 inner;
		inner.set_average_level(this->average_role_level_);
		LOGIC_MONITOR->dispatch_to_all_map(&inner);
	}

	//refresh rank show pannel
	this->clear_rank_show_data_by_rank_type(rank_type);
	this->rank_show_pannel_[rank_type] = show_vec;

	//save rank pannel to db
	this->request_save_rank_data(rank_type);
	return 0;
}

int RankSystem::bson2rank_data(auto_ptr<TQueryCursor> &cursor, const int rank_type, RankRecordVec& record_vec)
{
	if(cursor->more() == false)
		return 0;

    Time_Value week_tick,null_tick;
    null_tick.sec(0);
    Time_Value nowtime = Time_Value::gettimeofday();

	while(cursor->more())
	{
		BSONObj res = cursor->next();
		int role_level = res[DBRanker::FIGHT_LEVEL].numberInt();
		JUDGE_CONTINUE(role_level >= RankSystem::rank_system_open_level());

		if(rank_type == RANK_SINGLE_SCRIPT_ZYFM)
		{
			int zyfm_pass_key = res[DBRanker::ZYFM_PASS_KEY].numberInt();
			JUDGE_CONTINUE(zyfm_pass_key > 0);
		}
		Int64 role_id = res[DBRanker::ID].numberLong();
		JUDGE_CONTINUE(this->hide_player_.find(role_id) == this->hide_player_.end());
		RankRecord* record = this->rank_record_pool_.pop();
		JUDGE_CONTINUE(record != NULL);

		record->__single_player_info.__rank_type = rank_type;
		record->__single_player_info.__role_id = role_id;
		record->__single_player_info.__role_name = res[DBRanker::NAME].str();
		record->__single_player_info.__league_name = res[DBRanker::LEAGUE_NAME].str();
		record->__single_player_info.__vip_type = res[DBRanker::VIP_STATUS].numberInt();

		RankRecord* rank_player = RANK_SYS->fetch_rank_record(rank_type, role_id);
		if (rank_player != NULL)
		{
			record->__single_player_info.__worship_num = rank_player->__single_player_info.__worship_num;
			record->__single_player_info.__is_worship = rank_player->__single_player_info.__is_worship;
		}

		switch(rank_type)
		{
		case RANK_FIGHT_FORCE:
			record->__single_player_info.__rank_value = res[DBRanker::FIGHT_FORCE].numberInt();
			break;

		case RANK_KILL_VALUE:
            week_tick.sec(res[DBRanker::WEEK_TICK].numberLong());
            if(nowtime < week_tick || week_tick <= null_tick)
            {
            	record->__single_player_info.__rank_value = res[DBRanker::KILL_NORMAL].numberInt();
            	record->__single_player_info.__ext_value = res[DBRanker::FIGHT_LEVEL].numberInt();
            }
            else
            {
            	record->__single_player_info.__rank_value = 0;
            	record->__single_player_info.__ext_value = 0;
            }
			break;

		case RANK_HERO:
			week_tick.sec(res[DBRanker::WEEK_TICK].numberLong());
			if(nowtime < week_tick || week_tick <= null_tick)
			{
			      record->__single_player_info.__rank_value = res[DBRanker::KILL_EVIL].numberInt();
			      record->__single_player_info.__ext_value = res[DBRanker::FIGHT_LEVEL].numberInt();
		    }
			else
			{
			      record->__single_player_info.__rank_value = 0;
			      record->__single_player_info.__ext_value = 0;
			}
			break;

		case RANK_FIGHT_LEVEL:
			record->__single_player_info.__rank_value = res[DBRanker::FIGHT_LEVEL].numberInt();
			record->__single_player_info.__ext_value = res[DBRanker::EXPERIENCE].numberInt();
			break;

		case RANK_PET:
			if(res[DBRanker::Pet::PET_IS_REMOVE].numberLong() > 0)
			{
				this->rank_record_pool_.push(record);
				continue;
			}

			record->__single_player_info.__additional_id = res[DBRanker::Pet::PET_INDEX].numberLong();
			record->__single_player_info.__rank_value = res[DBRanker::Pet::PET_FORCE].numberInt();
			break;

		case RANK_MOUNT:
			record->__single_player_info.__rank_value = res[DBRanker::MOUNT_GRADE].numberInt();
			record->__single_player_info.__additional_id = res[DBRanker::MOUNT_SHAPE].numberLong();
			break;

		case RANK_FUN_MOUNT:
			record->__single_player_info.__rank_value = res[DBRanker::FUN_MOUNT_GRADE].numberInt();
			record->__single_player_info.__ext_value = res[DBRanker::FUN_MOUNT_FORCE].numberInt();
			break;

		case RANK_FUN_GOD_SOLIDER:
			record->__single_player_info.__rank_value = res[DBRanker::FUN_GOD_SOLIDER_GRADE].numberInt();
			record->__single_player_info.__ext_value = res[DBRanker::FUN_GOD_SOLIDER_FORCE].numberInt();
			break;

		case RANK_FUN_MAGIC_EQUIP:
			record->__single_player_info.__rank_value = res[DBRanker::FUN_MAGIC_EQUIP_GRADE].numberInt();
			record->__single_player_info.__ext_value = res[DBRanker::FUN_MAGIC_EQUIP_FORCE].numberInt();
			break;

		case RANK_FUN_XIAN_WING:
			record->__single_player_info.__rank_value = res[DBRanker::FUN_XIAN_WING_GRADE].numberInt();
			record->__single_player_info.__ext_value = res[DBRanker::FUN_XIAN_WING_FORCE].numberInt();
			break;

		case RANK_FUN_LING_BEAST:
			record->__single_player_info.__rank_value = res[DBRanker::FUN_LING_BEAST_GRADE].numberInt();
			record->__single_player_info.__ext_value = res[DBRanker::FUN_LING_BEAST_FORCE].numberInt();
			break;

		case RANK_FUN_BEAST_EQUIP:
			record->__single_player_info.__rank_value = res[DBRanker::FUN_BEAST_EQUIP_GRADE].numberInt();
			record->__single_player_info.__ext_value = res[DBRanker::FUN_BEAST_EQUIP_FORCE].numberInt();
			break;

		case RANK_FUN_BEAST_MOUNT:
			record->__single_player_info.__rank_value = res[DBRanker::FUN_BEAST_MOUNT_GRADE].numberInt();
			record->__single_player_info.__ext_value = res[DBRanker::FUN_BEAST_MOUNT_FORCE].numberInt();
			break;

		case RANK_FUN_BEAST_WING:
			record->__single_player_info.__rank_value = res[DBRanker::FUN_BEAST_WING_GRADE].numberInt();
			record->__single_player_info.__ext_value = res[DBRanker::FUN_BEAST_WING_FORCE].numberInt();
			break;

		case RANK_FUN_BEAST_MAO:
			record->__single_player_info.__rank_value = res[DBRanker::FUN_BEAST_MAO_GRADE].numberInt();
			record->__single_player_info.__ext_value = res[DBRanker::FUN_BEAST_MAO_FORCE].numberInt();
			break;

		case RANK_FUN_TIAN_GANG:
			record->__single_player_info.__rank_value = res[DBRanker::FUN_TIAN_GANG_GRADE].numberInt();
			record->__single_player_info.__ext_value = res[DBRanker::FUN_TIAN_GANG_FORCE].numberInt();
			break;

		case RANK_SINGLE_SCRIPT_ZYFM:
			record->__single_player_info.__rank_value = res[DBRanker::ZYFM_PASS_KEY].numberInt();
			record->__single_player_info.__achive_tick = res[DBRanker::ZYFM_PASS_TICK].numberInt();
			record->__single_player_info.__fight_force = res[DBRanker::FIGHT_FORCE].numberInt();
			break;
        case RANK_SEND_FLOWER:
            record->__single_player_info.__rank_value = res[DBRanker::SEND_FLOWER].numberInt();
            break;
        case RANK_RECV_FLOWER:
            record->__single_player_info.__rank_value = res[DBRanker::RECV_FLOWER].numberInt();
            break;
        case RANK_ACT_SEND_FLOWER:
            record->__single_player_info.__rank_value = res[DBRanker::ACT_SEND_FLOWER].numberInt();
            break;
        case RANK_ACT_RECV_FLOWER:
            record->__single_player_info.__rank_value = res[DBRanker::ACT_RECV_FLOWER].numberInt();
            break;
		default:
			break;
		}

		record_vec.push_back(record);
	}

	return 0;
}

int RankSystem::init_rank_manager(void)
{
	this->refresh_rank_manager();
	this->fix_rank_manager_next_refresh();

	return 0;
}

int RankSystem::refresh_rank_manager(void)
{
	int now_time  = Time_Value::gettimeofday().sec();
	const Json::Value& rank_json = CONFIG_INSTANCE->rank_json();
	for(int i = 0; i < (int)rank_json["specail_refresh_rank_types"].size(); ++i)
	{
		int rank_type = rank_json["specail_refresh_rank_types"][i].asInt();
		const Json::Value& rank_pannel = CONFIG_INSTANCE->rank_pannel(rank_type);
		JUDGE_CONTINUE(rank_pannel != Json::Value::null);

		RankRefreshDetail refresh_detail;
		refresh_detail.__rank_type = rank_type;
		refresh_detail.__refresh_type = rank_pannel["refresh"]["refresh_type"].asInt();
		refresh_detail.__next_refresh_tick = ::next_day(0, 0).sec();

		if(1 == refresh_detail.__refresh_type)
		{
			int hour = 0, min = 0;
			for(int j = 0; j < (int)rank_pannel["refresh"]["refresh_tick"].size(); ++j)
			{
				hour = rank_pannel["refresh"]["refresh_tick"][j][0u].asInt();
				min = rank_pannel["refresh"]["refresh_tick"][j][1u].asInt();

				int refresh_tick = ::current_day(hour, min).sec();
				refresh_detail.__refresh_tick_set.push_back(refresh_tick);
			}

			bool update_tick_flag = false;
			IntVec::iterator it = refresh_detail.__refresh_tick_set.begin();
			for(; it != refresh_detail.__refresh_tick_set.end(); ++it)
			{
				if(now_time <= *it)
				{
					refresh_detail.__next_refresh_tick = *it;
					update_tick_flag = true;
					break;
				}
			}

			if((int)refresh_detail.__refresh_tick_set.size() > 0 && !update_tick_flag)
			{
			    for(it = refresh_detail.__refresh_tick_set.begin(); it != refresh_detail.__refresh_tick_set.end(); ++it)
			    {
                    *it += 24 * 3600;
                }
				it = refresh_detail.__refresh_tick_set.begin();
				refresh_detail.__next_refresh_tick = *it;
			}
		}

		this->rank_manager_.insert(RankRefreshManager::value_type(rank_type, refresh_detail));
	}
	return 0;
}

int RankSystem::fix_rank_manager_next_refresh(void)
{
	return MMORankPannel::fix_rank_manager_next_refresh(this->rank_manager_);
}

int RankSystem::save_rank_manager_last_refresh(void)
{
	return MMORankPannel::save_rank_manager_last_refresh(this->rank_manager_);
}

int RankSystem::save_rank_manager_last_refresh(int rank_type, int refresh_tick)
{
	MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	MMORankPannel::update_rank_last_refresh(data_map, rank_type, refresh_tick);
	if (TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_SAVE_RANK_LAST_TICK, data_map) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}
	return 0;
}

int RankSystem::refresh_rank_manager_at_midnight(void)
{
	this->rank_manager_.clear();
	this->refresh_rank_manager();
	return 0;
}

int RankSystem::rank_manager_maintenance()
{
	RankRefreshManager::iterator it = this->rank_manager_.begin();
	for (; it != this->rank_manager_.end(); ++it)
	{
		RankRefreshDetail& refresh_detail = it->second;
		this->check_rank_refresh(refresh_detail);
	}

	return 0;
}

int RankSystem::check_rank_refresh(RankRefreshDetail& refresh_detail)
{
	if(Time_Value::gettimeofday().sec() >= refresh_detail.__next_refresh_tick)
	{
		this->request_refresh_rank_data(refresh_detail.__rank_type);
		this->modify_rank_refesh_tick(refresh_detail);
	}
	return 0;
}

int RankSystem::modify_rank_refesh_tick(RankRefreshDetail& refresh_detail)
{
	int now_time = Time_Value::gettimeofday().sec();
	refresh_detail.__last_refresh_tick = now_time;
//	this->save_rank_manager_last_refresh(refresh_detail.__rank_type, refresh_detail.__last_refresh_tick);

	if(refresh_detail.__refresh_tick_set.size() == 0)
	{
		refresh_detail.__next_refresh_tick = ::next_day(0,0).sec();
		return 0;
	}

	int update_tick_flag = false;
	IntVec::iterator it = refresh_detail.__refresh_tick_set.begin();
	for(; it != refresh_detail.__refresh_tick_set.end(); ++it)
	{
		if( now_time < *it)
		{
			refresh_detail.__next_refresh_tick = *it;
			update_tick_flag = true;
			break;
		}
	}

	if(!update_tick_flag)
	{
		for(it = refresh_detail.__refresh_tick_set.begin(); it != refresh_detail.__refresh_tick_set.end(); ++it)
		{
            *it += 24 * 3600;
        }
		it = refresh_detail.__refresh_tick_set.begin();
		refresh_detail.__next_refresh_tick = *it + 24 * 3600;
	}

	return 0;
}

int RankSystem::clear_rank_show_data_by_rank_type(const int rank_type)
{
	JUDGE_RETURN(this->rank_show_pannel_.count(rank_type) > 0, 0);
	// rank_panel_[rank_type]保存有同一指针，所以不在此处回收空间
	this->rank_show_pannel_[rank_type].clear();
	return 0;
}

int RankSystem::rank_cache_time_up(const Time_Value& now_time)
{
//	if(this->rank_cache_tick_.check_timeout(RankSystem::TIMEUP_SAVE, now_time) == true
//			&& this->rank_cache_tick_.check_cache(RANK_TYPE_SINGLE_SCRIPT_ZYFM) == true)
//	{
//		this->request_save_rank_data(RANK_SINGLE_SCRIPT_ZYFM);
//		this->cache_tick().update_cache(RANK_TYPE_SINGLE_SCRIPT_ZYFM, false);
//		this->rank_cache_tick_.update_timeup_tick(RankSystem::TIMEUP_SAVE, now_time);
//	}

    if (this->realtime_save_tick_ < now_time)
    {
        this->realtime_save_tick_ = now_time + Time_Value(30);
        for (IntSet::iterator iter = this->cache_rank_type_.begin();
                iter != this->cache_rank_type_.end(); ++iter)
        {
            this->request_save_rank_data(*iter);
        }
        this->cache_rank_type_.clear();
        this->check_and_clear_offline_data();
    }
	return 0;
}

RankSystem::CachedTimer::CachedTimer():rank_system_(0)
{
	/*NULL*/
}

RankSystem::CachedTimer::~CachedTimer()
{
	/*NULL*/
}

int RankSystem::CachedTimer::type()
{
	return GTT_ML_ONE_SECOND;
}

int RankSystem::CachedTimer::handle_timeout(const Time_Value& now_time)
{
	this->rank_system_->rank_cache_time_up(now_time);
	return 0;
}

int RankSystem::update_realtime_rank_data(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto31401601 *, request, msg, -1);
    
    int rank_type = request->rank_type();
    RankRecordMap &rank_map = this->rank_pannel_[rank_type];
    RankRecordVec &rank_vec = this->rank_show_pannel_[rank_type];

    RankRecord *rank_rec = 0;
    RankRecordMap::iterator iter = rank_map.find(request->role_id());
    if (iter != rank_map.end())
    {
        rank_rec = iter->second;
        rank_rec->__single_player_info.__role_id = request->role_id();
        rank_rec->__single_player_info.__role_name = request->role_name();
		rank_rec->__single_player_info.__league_name = request->league_name();
		rank_rec->__single_player_info.__vip_type = request->vip_type();
        rank_rec->__single_player_info.__achive_tick = request->achieve_tick();
        rank_rec->__single_player_info.__rank_type = rank_type;
        rank_rec->__single_player_info.__rank_value = request->rank_value();
        rank_rec->__single_player_info.__fight_force = request->rank_force();
        rank_rec->__single_player_info.__ext_value = request->ext_value();
//		rank_rec->__single_player_info.__worship_num = request->worship_num();
//		rank_rec->__single_player_info.__is_worship = request->is_worship();
    }
    else
    {
    	if(this->hide_player_.find(request->role_id()) == this->hide_player_.end())
    	{
			rank_rec = this->rank_record_pool()->pop();
			rank_rec->__single_player_info.__role_id = request->role_id();
			rank_rec->__single_player_info.__role_name = request->role_name();
			rank_rec->__single_player_info.__league_name = request->league_name();
			rank_rec->__single_player_info.__vip_type = request->vip_type();
			rank_rec->__single_player_info.__achive_tick = request->achieve_tick();
			rank_rec->__single_player_info.__rank_type = rank_type;
			rank_rec->__single_player_info.__rank_value = request->rank_value();
			rank_rec->__single_player_info.__fight_force = request->rank_force();
			rank_rec->__single_player_info.__ext_value = request->ext_value();
			rank_rec->__single_player_info.__cur_rank = rank_vec.size() + 1;
//			rank_rec->__single_player_info.__worship_num = request->worship_num();
//			rank_rec->__single_player_info.__is_worship = request->is_worship();
			rank_rec->__vec_index = rank_vec.size();
			rank_map[request->role_id()] = rank_rec;
			rank_vec.push_back(rank_rec);
    	}
    }

    JUDGE_RETURN(rank_rec != 0, -1);

    this->sort_realtime_rank_data(rank_rec);

    // 只保留排行所需人数
    {
    	int max_rank_amount = CONFIG_INSTANCE->rank_json()["default_ranker_amount"].asInt();
    	const Json::Value &rank_pannel_json = CONFIG_INSTANCE->rank_pannel(rank_type);
    	if (rank_pannel_json != Json::Value::null && rank_pannel_json.isMember("ranker_amount"))
			max_rank_amount = rank_pannel_json["ranker_amount"].asInt();
    	if (max_rank_amount <= 0)
    		max_rank_amount = 100;
    	while (int(rank_vec.size()) > max_rank_amount)
    	{
    		RankRecord *del_rec = rank_vec[rank_vec.size() - 1];
    		rank_map.erase(del_rec->__single_player_info.__role_id);
    		rank_vec.pop_back();
    		this->push_rank_record(del_rec);
    	}
    }

    this->cache_rank_type_.insert(rank_type);
    return 0;
}

void RankSystem::sort_realtime_rank_data(RankRecord *rank_rec)
{
    RankRecordVec &rank_vec = this->rank_show_pannel_[rank_rec->__single_player_info.__rank_type];

    int cur_rank_index = rank_rec->__vec_index;
//    int rank_type = rank_rec->__single_player_info.__rank_type;
    int begin_index = 0, end_index = cur_rank_index, mid_index = 0, sel_index = -1;
    while (begin_index < end_index)
    {
        mid_index = (end_index - begin_index) / 2 + begin_index;
        RankRecord *mid_rec = rank_vec[mid_index];
        if (rank_record_cmp(rank_rec, mid_rec))
        {
            sel_index = mid_index;
            end_index = mid_index;
        }
        else
        {
            begin_index = mid_index + 1;
        }
    }


	if (sel_index >= 0)
	{
		RankRecord *cur_rank_rec = 0;
		for (int i = cur_rank_index - 1; i >= sel_index; -- i)
		{
			cur_rank_rec = rank_vec[i];
			cur_rank_rec->__single_player_info.__last_rank = cur_rank_rec->__single_player_info.__cur_rank;
			cur_rank_rec->__single_player_info.__cur_rank = i + 2;
			cur_rank_rec->__vec_index = i + 1;
			rank_vec[i + 1] = cur_rank_rec;
		}
		rank_rec->__single_player_info.__last_rank = rank_rec->__single_player_info.__cur_rank;
		rank_rec->__single_player_info.__cur_rank = sel_index + 1;
		rank_rec->__vec_index = sel_index;
		rank_vec[sel_index] = rank_rec;
	}

}

void RankSystem::clear_realtime_rank(const int rank_type)
{
	this->clear_rank_data_by_rank_type(rank_type);
	this->cache_rank_type_.insert(rank_type);
}

int RankSystem::map_act_fetch_level_fight_rank_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400036*, request, -1);

	int rank_type = request->rank_type();
	int size = request->size();
	MSG_USER("act fetch RankSystem %d, %d", rank_type, size);

	Proto31400036 rank_info;
	rank_info.set_rank_type(rank_type);

	RankShowPannel& rank_pannel = RANK_SYS->rank_show_pannel();
	RankRecordVec& rank_vec = rank_pannel[rank_type];

	int msg_num = std::min<int>(size, rank_vec.size());
	for(int i = 0; i < msg_num; ++i)
	{
		ProtoRankRecord* proto_record = rank_info.add_rank_list();
		RankRecord* &cur_record = rank_vec[i];
		cur_record->serialize(proto_record);
	}
	return LOGIC_MONITOR->dispatch_to_all_map(&rank_info);
//	return LOGIC_MONITOR->dispatch_to_aim_sid(head, &rank_info);
}

int RankSystem::map_act_fetch_level_fight_rank_info(int rank_type, int size)
{
	MSG_USER("act fetch RankSystem %d, %d", rank_type, size);

	Proto31400036 rank_info;
	rank_info.set_rank_type(rank_type);

	RankShowPannel& rank_pannel = RANK_SYS->rank_show_pannel();
	RankRecordVec& rank_vec = rank_pannel[rank_type];

	int msg_num = std::min<int>(size, rank_vec.size());
	for(int i = 0; i < msg_num; ++i)
	{
		ProtoRankRecord* proto_record = rank_info.add_rank_list();
		RankRecord* &cur_record = rank_vec[i];
		cur_record->serialize(proto_record);
	}
	return LOGIC_MONITOR->dispatch_to_all_map(&rank_info);
}

int RankSystem::update_player_name(const Int64 role_id, const std::string &name)
{
	for (RankPannel::iterator pannel_iter = this->rank_pannel_.begin(); pannel_iter != this->rank_pannel_.end(); ++pannel_iter)
	{
		RankRecordMap &record_map = pannel_iter->second;
		RankRecordMap::iterator record_iter = record_map.find(role_id);
		JUDGE_CONTINUE(record_iter != record_map.end());

		RankRecord *record = record_iter->second;
		record->__single_player_info.__role_name = name;
	}
	return 0;
}

int RankSystem::req_get_hide_player()
{
	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	data_map->push_multithread_query(DBRankHide::COLLECTION);
	if(TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_RANK_HIDE_ROLE, data_map, LOGIC_MONITOR->logic_unit()) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}
	return 0;
}
int RankSystem::after_get_hide_player(Transaction* transaction)
{
	JUDGE_RETURN(transaction != NULL, -1);
	if (transaction->detail().__error != 0)
	{
		transaction->rollback();
		return transaction->detail().__error;
	}

	MongoDataMap* data_map = transaction->fetch_mongo_data_map();
	if (data_map == 0)
	{
		transaction->rollback();
		return -1;
	}

	LongSet hide_set;
	MongoData* mongo_data = NULL;
	if(data_map->find_data(DBRankHide::COLLECTION, mongo_data) == 0)
	{
		auto_ptr<TQueryCursor> data_cursor = mongo_data->multithread_cursor();

		while(data_cursor->more())
		{
			BSONObj res = data_cursor->next();
			BSONObj role_set = res.getObjectField(DBRankHide::ROLE_ID.c_str());
			BSONObjIterator id_it(role_set);
			while(id_it.more())
			{
				hide_set.insert(id_it.next().numberLong());
			}
		}
	}
	transaction->summit();
	bool update = false;
	if(this->hide_player_.size() != hide_set.size())
	{
		update = true;
	}
	this->hide_player_ = hide_set;
	if(update)
	{
		for(RankPannel::const_iterator it = this->rank_pannel_.begin(); it != this->rank_pannel_.end(); ++it)
		{
			this->request_refresh_rank_data(it->first);
		}
	}
	return 0;
}
LongSet& RankSystem::get_hide_player()
{
	return this->hide_player_;
}

int RankSystem::update_act_flower_rank(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400039*, request, -1);
//	bool refresh = request->refresh();
//	//send
//	{
//		const ProtoActFlower& pro = request->send();
//		Int64 role_id = pro.role_id();
//		if(role_id > 0)
//		{
//			LogicPlayer* player  = NULL;
//			if(LOGIC_MONITOR->find_player(role_id, player) == 0 && player != NULL)
//			{
//				player->self_wedding_info().__act_total_send_flower = pro.flower_num();
//				if(refresh)
//				{
//					Proto31401601 req;
//					req.set_rank_type(RANK_ACT_SEND_FLOWER);
//					req.set_role_id(player->role_id());
//					req.set_role_name(player->name());
//					req.set_achieve_tick(Time_Value::gettimeofday().sec());
//					req.set_rank_value(pro.flower_num());
//					RANK_SYS->update_rank_data(&req);
//					BackSetActDetail::ActTypeItem* act_item = LOGIC_ACTIVITY_SYS->find_item_by_type(BackSetActDetail::ACT_RANK, RANK_ACT_SEND_FLOWER, -1);
//					if(act_item != NULL)
//					{
//						int index = (int)act_item->act_item_set_.size() - 1;
//						int size = act_item->act_item_set_[index].cond_value_[1];
//						RANK_SYS->map_act_fetch_level_fight_rank_info(RANK_ACT_SEND_FLOWER, size);
//					}
//				}
//			}
//		}
//	}
//	//recv
//	{
//		const ProtoActFlower& pro = request->recive();
//		Int64 role_id = pro.role_id();
//		if(role_id > 0)
//		{
//			LogicPlayer* player  = NULL;
//			if(LOGIC_MONITOR->find_player(role_id, player) == 0 && player != NULL)
//			{
//				player->self_wedding_info().__act_total_recv_flower = pro.flower_num();
//				if(refresh)
//				{
//					Proto31401601 req;
//					req.set_rank_type(RANK_ACT_RECV_FLOWER);
//					req.set_role_id(player->role_id());
//					req.set_role_name(player->name());
//					req.set_achieve_tick(Time_Value::gettimeofday().sec());
//					req.set_rank_value(pro.flower_num());
//					RANK_SYS->update_rank_data(&req);
//					BackSetActDetail::ActTypeItem* act_item = LOGIC_ACTIVITY_SYS->find_item_by_type(BackSetActDetail::ACT_RANK, RANK_ACT_RECV_FLOWER, -1);
//					if(act_item != NULL)
//					{
//						int index = (int)act_item->act_item_set_.size() - 1;
//						int size = act_item->act_item_set_[index].cond_value_[1];
//						RANK_SYS->map_act_fetch_level_fight_rank_info(RANK_ACT_RECV_FLOWER, size);
//					}
//				}
//			}
//		}
//	}
	return 0;
}

int RankSystem::reset_act_flower_rank(Transaction* transaction)
{
	JUDGE_RETURN(transaction != NULL, -1);
	if (transaction->detail().__error != 0)
	{
		transaction->rollback();
		return transaction->detail().__error;
	}

	MongoDataMap* data_map = transaction->fetch_mongo_data_map();
	if (data_map == 0)
	{
		transaction->rollback();
		return -1;
	}

	MongoData* mongo_data = NULL;
	if(data_map->find_data(DBRanker::COLLECTION, mongo_data) == 0)
	{
		auto_ptr<TQueryCursor> data_cursor = mongo_data->multithread_cursor();

		while(data_cursor->more())
		{
			BSONObj res = data_cursor->next();
			Int64 role_id = res[DBRanker::ID].numberLong();
			BSONObjBuilder builder;
			if(data_map->other_value_ & BackSetActDetail::RECIVE_FLOWER)
			{
				builder << DBRanker::ACT_RECV_FLOWER << 0;
			}
			if(data_map->other_value_ & BackSetActDetail::SEND_FLOWER)
			{
				builder << DBRanker::ACT_SEND_FLOWER << 0;
			}
			data_map->push_update(DBRanker::COLLECTION, BSON(DBRanker::ID << role_id), builder.obj());
		}
	}
	transaction->summit();
	return 0;
}

int RankSystem::check_and_clear_offline_data(void)
{
//	time_t now_t = ::time(NULL);
//	RankSystem::PlayerOfflineDataPoolPack::IndexMap index_map = this->offline_data_map_.fetch_index_map();
//	for (RankSystem::PlayerOfflineDataPoolPack::IndexMap::iterator iter = index_map.begin();
//			iter != index_map.end(); ++iter)
//	{
//		PlayerOfflineData *offline_data = this->offline_data_map_.find_object(iter->first);
//		JUDGE_CONTINUE(offline_data != NULL);
//		JUDGE_CONTINUE((offline_data->mount_beast_tick_ + 86400) <= now_t);
//
//		this->offline_data_map_.unbind_and_push(iter->first, offline_data);
//	}
	return 0;
}
