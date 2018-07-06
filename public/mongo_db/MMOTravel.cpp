/*
 * MMOTravel.cpp
 *
 *  Created on: Nov 17, 2016
 *      Author: peizhibi
 */

#include "MMOTravel.h"
#include "GameField.h"
#include "BackField.h"
#include "MapPlayerEx.h"
#include "GameCommon.h"
#include "DBCommon.h"
#include "MongoDataMap.h"
#include "TrvlArenaMonitor.h"
#include "TrvlWeddingMonitor.h"
#include "TrvlRechargeMonitor.h"
#include "PoolMonitor.h"
#include "MongoDataMap.h"
#include "MongoData.h"
#include "TQueryCursor.h"
#include "MapMonitor.h"
#include "MapMapStruct.h"

#include "MongoException.h"
#include "MongoConnector.h"
#include <mongo/client/dbclient.h>
using namespace mongo;


MMOTravel::MMOTravel() {
	// TODO Auto-generated constructor stub
}

MMOTravel::~MMOTravel() {
	// TODO Auto-generated destructor stub
}

int MMOTravel::load_player_tarena(MapPlayerEx* player)
{
	BSONObj res = this->conection().findOne(DBBattler::COLLECTION,
			QUERY(DBBattler::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	TArenaDetail& detail = player->tarena_detail();
    BSONObj detail_obj = res.getObjectField(DBBattler::TARENA.c_str());

	detail.stage_ = std::max<int>(detail_obj[DBBattler::Tarena::STAGE].numberInt(), 1);
	detail.score_ = detail_obj[DBBattler::Tarena::SCORE].numberInt();
	detail.adjust_tick_ = detail_obj[DBBattler::Tarena::ADJUST_TICK].numberLong();
	detail.draw_flag_ = detail_obj[DBBattler::Tarena::DRAW].numberInt();
	detail.win_times_ = detail_obj[DBBattler::Tarena::WIN_TIMES].numberInt();
	detail.get_exploit_ = detail_obj[DBBattler::Tarena::GET_EXPLOIT].numberInt();
	detail.attend_times_ = detail_obj[DBBattler::Tarena::ATTEND_TIMES].numberInt();

	GameCommon::bson_to_map(detail.draw_win_, detail_obj.getObjectField(
			DBBattler::Tarena::DRAW_WIN.c_str()), true);
	return 0;
}

int MMOTravel::update_data(MapPlayerEx* player, MongoDataMap *data_map)
{
	TArenaDetail& detail = player->tarena_detail();

	BSONVec draw_bson_vec;
	GameCommon::map_to_bson(draw_bson_vec, detail.draw_win_, false, true);

	BSONObjBuilder tarena_builder;
	tarena_builder << DBBattler::Tarena::STAGE << detail.stage_
			<< DBBattler::Tarena::SCORE << detail.score_
			<< DBBattler::Tarena::ADJUST_TICK << detail.adjust_tick_
			<< DBBattler::Tarena::DRAW << detail.draw_flag_
			<< DBBattler::Tarena::WIN_TIMES << detail.win_times_
			<< DBBattler::Tarena::GET_EXPLOIT << detail.get_exploit_
			<< DBBattler::Tarena::ATTEND_TIMES << detail.attend_times_
			<< DBBattler::Tarena::DRAW_WIN << draw_bson_vec;

    BSONObjBuilder builder;
    builder << DBBattler::TARENA << tarena_builder.obj();
    data_map->push_update(DBBattler::COLLECTION, BSON(DBBattler::ID << player->role_id()),
    		builder.obj(), true);
	return 0;
}

void MMOTravel::clear_all_tarena_role()
{
	BSONObj empty_obj;
	GameCommon::request_remove_mmo_begin(DBTarenaRole::COLLECTION, empty_obj, false);
}

void MMOTravel::load_tarena_role(TrvlArenaMonitor* monitor)
{
	auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBTarenaRole::COLLECTION);
	while (cursor->more())
	{
		BSONObj res = cursor->next();
		JUDGE_CONTINUE(res.isEmpty() == false);

		TrvlArenaRole* arena = monitor->find_and_pop(res[DBTarenaRole::ID].numberLong());
		JUDGE_CONTINUE(arena != NULL);

		arena->sync_  = res[DBTarenaRole::SYNC].numberInt();
		arena->stage_ = std::max<int>(res[DBTarenaRole::STAGE].numberInt(), 1);
		arena->score_ = res[DBTarenaRole::SCORE].numberInt();
		arena->update_tick_ = res[DBTarenaRole::UPDATE_TICK].numberLong();

		DBCommon::bson_to_base_server(*arena, res.getObjectField(DBTarenaRole::SERVER.c_str()));
		DBCommon::bson_to_base_member(*arena,res.getObjectField(DBTarenaRole::ROLE.c_str()));
	}
}

void MMOTravel::save_tarena_role(TrvlArenaRole* arena)
{
	JUDGE_RETURN(arena != NULL, ;);

    BSONObjBuilder builder;
    builder << DBTarenaRole::SYNC << arena->sync_
    		<< DBTarenaRole::SERVER << DBCommon::base_server_to_bson(*arena)
    		<< DBTarenaRole::ROLE << DBCommon::base_member_to_bson(*arena)
    		<< DBTarenaRole::STAGE << arena->stage_
    		<< DBTarenaRole::SCORE << arena->score_
    		<< DBTarenaRole::UPDATE_TICK << arena->update_tick_;

	GameCommon::request_save_mmo_begin(DBTarenaRole::COLLECTION,
			BSON(DBTarenaRole::ID << arena->id_), BSON("$set" << builder.obj()));
}

void MMOTravel::load_wedding_rank(TrvlWeddingMonitor* monitor)
{
	auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBWeddingRank::COLLECTION);
	while (cursor->more())
	{
		BSONObj res = cursor->next();
		JUDGE_CONTINUE(res.isEmpty() == false);

		TrvlWeddingRank* rank_info = monitor->pop_rank_info(res[DBWeddingRank::ID].numberInt());
		JUDGE_CONTINUE(rank_info != NULL);

		rank_info->rank_ = res[DBWeddingRank::ID].numberInt();
		rank_info->tick_ = res[DBWeddingRank::TICK].numberLong();

		DBCommon::bson_to_base_server(*rank_info, res.getObjectField(DBWeddingRank::SERVER.c_str()));
		DBCommon::bson_to_base_member(rank_info->player1_, res.getObjectField(DBWeddingRank::PLAYER1.c_str()));
		DBCommon::bson_to_base_member(rank_info->player2_, res.getObjectField(DBWeddingRank::PLAYER2.c_str()));
	}
}

void MMOTravel::save_wedding_rank(TrvlWeddingRank* rank_info)
{
	JUDGE_RETURN(rank_info != NULL, ;);

	BSONObjBuilder builder;
	builder << DBWeddingRank::TICK << rank_info->tick_
			<< DBWeddingRank::SERVER << DBCommon::base_server_to_bson(*rank_info)
			<< DBWeddingRank::PLAYER1 << DBCommon::base_member_to_bson(rank_info->player1_)
			<< DBWeddingRank::PLAYER2 << DBCommon::base_member_to_bson(rank_info->player2_);

	GameCommon::request_save_mmo_begin(DBWeddingRank::COLLECTION,
			BSON(DBWeddingRank::ID << rank_info->rank_), BSON("$set" << builder.obj()));
}

void MMOTravel::save_recharge_rank(TrvlRechargeMonitor* monitor)
{
	BSONVec get_vec, cost_vec, recharge_back_vec;
	BLongMap get_map = monitor->fetch_long_map(TrvlRechargeMonitor::TYPE_GET);
	for (BLongMap::iterator iter = get_map.begin(); iter != get_map.end(); ++iter)
	{
		TrvlRechargeRank* rank_info = monitor->find_role(iter->first, TrvlRechargeMonitor::TYPE_GET);
		JUDGE_CONTINUE(rank_info != NULL && rank_info->amount_ > 0);

		get_vec.push_back(BSON(DBRechargeRank::ROLE_ID << rank_info->id_
				<< DBRechargeRank::AMOUNT << rank_info->amount_
				<< DBRechargeRank::TICK << rank_info->tick_
				<< DBRechargeRank::SERVER << DBCommon::base_server_to_bson(*rank_info)
				<< DBRechargeRank::ROLE << DBCommon::base_member_to_bson(*rank_info)));
	}

	BLongMap cost_map = monitor->fetch_long_map(TrvlRechargeMonitor::TYPE_COST);
	for (BLongMap::iterator iter = cost_map.begin(); iter != cost_map.end(); ++iter)
	{
		TrvlRechargeRank* rank_info = monitor->find_role(iter->first, TrvlRechargeMonitor::TYPE_COST);
		JUDGE_CONTINUE(rank_info != NULL && rank_info->amount_ > 0);

		cost_vec.push_back(BSON(DBRechargeRank::ROLE_ID << rank_info->id_
				<< DBRechargeRank::AMOUNT << rank_info->amount_
				<< DBRechargeRank::TICK << rank_info->tick_
				<< DBRechargeRank::SERVER << DBCommon::base_server_to_bson(*rank_info)
				<< DBRechargeRank::ROLE << DBCommon::base_member_to_bson(*rank_info)));
	}

	BLongMap recharge_back_map = monitor->fetch_long_map(TrvlRechargeMonitor::TYPE_BACK_RECHAGE);
	for (BLongMap::iterator iter = recharge_back_map.begin(); iter != recharge_back_map.end(); ++iter)
	{
		TrvlRechargeRank* rank_info = monitor->find_role(iter->first, TrvlRechargeMonitor::TYPE_BACK_RECHAGE);
		JUDGE_CONTINUE(rank_info != NULL && rank_info->amount_ > 0);

		recharge_back_vec.push_back(BSON(DBRechargeRank::ROLE_ID << rank_info->id_
				<< DBRechargeRank::AMOUNT << rank_info->amount_
				<< DBRechargeRank::TICK << rank_info->tick_
				<< DBRechargeRank::SERVER << DBCommon::base_server_to_bson(*rank_info)
				<< DBRechargeRank::ROLE << DBCommon::base_member_to_bson(*rank_info)
				<< DBRechargeRank::ACTIVITY_ID << rank_info->activity_id_
				<< DBRechargeRank::RANK << rank_info->rank_));
	}

	BSONObjBuilder builder;
	builder << DBRechargeRank::GET_RANK << get_vec
			<< DBRechargeRank::COST_RANK << cost_vec
			<< DBRechargeRank::RECHARGE_BACK_RANK << recharge_back_vec;

	GameCommon::request_save_mmo_begin(DBRechargeRank::COLLECTION,
			BSON(DBRechargeRank::ID << 0), BSON("$set" << builder.obj()));
}

void MMOTravel::load_recharge_rank(TrvlRechargeMonitor* monitor)
{
	BSONObj res = CACHED_CONNECTION.findOne(DBRechargeRank::COLLECTION, QUERY(DBRechargeRank::ID << 0));
	JUDGE_RETURN(res.isEmpty() == false, ;);

	BSONObjIterator get_iter = res.getObjectField(DBRechargeRank::GET_RANK.c_str());
	while (get_iter.more())
	{
		BSONObj obj = get_iter.next().embeddedObject();
		MMOTravel::add_recharge_rank_info(monitor, obj, TrvlRechargeMonitor::TYPE_GET);
	}

	BSONObjIterator cost_iter = res.getObjectField(DBRechargeRank::COST_RANK.c_str());
	while (cost_iter.more())
	{
		BSONObj obj = cost_iter.next().embeddedObject();
		MMOTravel::add_recharge_rank_info(monitor, obj, TrvlRechargeMonitor::TYPE_COST);
	}

	BSONObjIterator recharge_back_iter = res.getObjectField(DBRechargeRank::RECHARGE_BACK_RANK.c_str());
	while (recharge_back_iter.more())
	{
		BSONObj obj = recharge_back_iter.next().embeddedObject();
		MMOTravel::add_recharge_rank_info(monitor, obj, TrvlRechargeMonitor::TYPE_BACK_RECHAGE);
	}
	std::sort(monitor->recharge_back_vec().begin(), monitor->recharge_back_vec().end(), GameCommon::three_comp_by_desc);
}

void MMOTravel::add_recharge_rank_info(TrvlRechargeMonitor* monitor, BSONObj &obj, int type)
{
	TrvlRechargeRank *rank_info = monitor->find_and_pop(obj[DBRechargeRank::ROLE_ID].numberLong(), type);
	JUDGE_RETURN(rank_info != NULL, ;);

	rank_info->rank_ = obj[DBRechargeRank::RANK].numberInt();
	rank_info->amount_ = obj[DBRechargeRank::AMOUNT].numberInt();
	rank_info->tick_   = obj[DBRechargeRank::TICK].numberLong();
	rank_info->activity_id_ = obj[DBRechargeRank::ACTIVITY_ID].numberInt();

	DBCommon::bson_to_base_server(*rank_info, obj.getObjectField(DBRechargeRank::SERVER.c_str()));
	DBCommon::bson_to_base_member(*rank_info,obj.getObjectField(DBRechargeRank::ROLE.c_str()));

	if (type == TrvlRechargeMonitor::TYPE_BACK_RECHAGE)
	{
		ThreeObj obj;
		obj.id_ = rank_info->id_;
		obj.value_ = rank_info->amount_;
		obj.tick_ = rank_info->tick_;
		monitor->recharge_back_vec().push_back(obj);
	}
}

void MMOTravel::remove_recharge_rank()
{
	GameCommon::request_remove_mmo_begin(DBRechargeRank::COLLECTION,
			BSON(DBLeague::ID << 0));
}

void MMOTravel::request_correct_rank_info(void)
{
    MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
    data_map->push_multithread_query(DBCorrectTrvlRank::COLLECTION, BSON(DBCorrectTrvlRank::UPDATE_FLAG << 1));
    if (TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_CORRECT_TRVL_RANK, data_map, MAP_MONITOR->map_unit()) != 0)
    {
        POOL_MONITOR->mongo_data_map_pool()->push(data_map);
    }
}

void MMOTravel::correct_rank_info_from_db(TrvlRechargeMonitor *monitor, MongoDataMap *data_map)
{
    MongoData *mongo_data = NULL;
    JUDGE_RETURN(data_map->find_data(DBCorrectTrvlRank::COLLECTION, mongo_data) == 0, ;);
    std::vector<Int64> update_id_vc;
    TrvlRechargeRank rank_info;
    auto_ptr<TQueryCursor> cursor = mongo_data->multithread_cursor();
    while (cursor->more())
    {
        BSONObj obj = cursor->next();

        Int64 role_id = obj[DBCorrectTrvlRank::ID].numberLong();
        update_id_vc.push_back(role_id);
        JUDGE_CONTINUE(role_id > 0);

        int type = obj[DBCorrectTrvlRank::TYPE].numberInt();
        JUDGE_CONTINUE(type == TrvlRechargeMonitor::TYPE_BACK_RECHAGE);

        rank_info.reset();
        rank_info.amount_ = obj[DBCorrectTrvlRank::AMOUNT].numberInt();
        rank_info.tick_ = ::time(NULL);
        rank_info.activity_id_ = obj[DBCorrectTrvlRank::ACTIVITY_ID].numberInt();
        
        DBCommon::bson_to_base_server(rank_info, obj.getObjectField(DBCorrectTrvlRank::SERVER.c_str()));
        DBCommon::bson_to_base_member(rank_info, obj.getObjectField(DBCorrectTrvlRank::ROLE.c_str()));

        TrvlRechargeRank *tar_rank_info = monitor->find_role(role_id, type);
        if (tar_rank_info != NULL)
        {
            tar_rank_info->amount_ = rank_info.amount_;
            tar_rank_info->tick_ = rank_info.tick_;
        }
        else
        {
            tar_rank_info = monitor->find_and_pop(role_id, type);
            JUDGE_CONTINUE(tar_rank_info != NULL);

            *tar_rank_info = rank_info;
        }
    }
    for (std::vector<Int64>::iterator iter = update_id_vc.begin(); iter != update_id_vc.end(); ++iter)
    {
        GameCommon::request_save_mmo_begin(DBCorrectTrvlRank::COLLECTION, BSON(DBCorrectTrvlRank::ID << (*iter)), BSON("$set" << BSON(DBCorrectTrvlRank::UPDATE_FLAG << 0)), false);
    }
}

int MMOTravel::load_wboss_info(WorldBossInfo* wboss_info)
{
	BSONObj res = CACHED_CONNECTION.findOne(DBTrvlWboss::COLLECTION,
					QUERY(DBTrvlWboss::KEY << wboss_info->scene_id_));
	JUDGE_RETURN(res.isEmpty() == false, 0);

	wboss_info->status_ = res[DBTrvlWboss::STATUS].numberInt();
	wboss_info->killer_ = res[DBTrvlWboss::KILLER].numberLong();
	wboss_info->killer_name_ = res[DBTrvlWboss::KILLER_NAME].str();
	return 0;
}

int MMOTravel::update_wboss_info(WorldBossInfo* wboss_info, int direct_save)
{
	BSONObjBuilder builder;
	builder << DBTrvlWboss::STATUS << wboss_info->status_
			<< DBTrvlWboss::KILLER << wboss_info->killer_
			<< DBTrvlWboss::KILLER_NAME << wboss_info->killer_name_;

	if (direct_save == false)
	{
		GameCommon::request_save_mmo_begin(DBTrvlWboss::COLLECTION,
				BSON(DBTrvlWboss::KEY << wboss_info->scene_id_),
				BSON("$set" << builder.obj()));
	}
	else
	{
		CACHED_CONNECTION.update(DBTrvlWboss::COLLECTION,
				BSON(DBTrvlWboss::KEY << wboss_info->scene_id_),
				BSON("$set" << builder.obj()), true);
	}

	return 0;
}

void MMOTravel::ensure_all_index(void)
{
	this->conection().ensureIndex(DBBattler::COLLECTION, BSON(DBBattler::ID << 1), true);
	this->conection().ensureIndex(DBTarenaRole::COLLECTION, BSON(DBTarenaRole::ID << 1), true);
	this->conection().ensureIndex(DBWeddingRank::COLLECTION, BSON(DBWeddingRank::ID << 1), true);
	this->conection().ensureIndex(DBRechargeRank::COLLECTION, BSON(DBRechargeRank::ID << 1), true);
    this->conection().ensureIndex(DBCorrectTrvlRank::COLLECTION, BSON(DBCorrectTrvlRank::ID << 1), true);
}
