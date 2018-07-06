/*
 * MMORechargeRankConfig.cpp
 *
 *  Created on: Jan 21, 2015
 *      Author: LiJin
 */

#include "LogicStruct.h"
#include "BackField.h"
#include "GameField.h"
#include "LogicMonitor.h"
#include "MMORechargeRankConfig.h"
#include "MapLogicPlayer.h"
#include "DBCommon.h"
#include "MLBackDailyRechargeSys.h"

#include "MongoDataMap.h"
#include "MongoConnector.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>


MMORechargeRewards::MMORechargeRewards()
{
	// TODO Auto-generated constructor stub

}

MMORechargeRewards::~MMORechargeRewards()
{
	// TODO Auto-generated destructor stub
}

int MMORechargeRewards::load_player_recharge_rewards(MapLogicPlayer *player)
{
BEGIN_CATCH
    BSONObj res = this->conection().findOne(DBRechargeRewards::COLLECTION,
    		QUERY(DBRechargeRewards::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, -1);

    RechargeDetail &recharge_detail = player->recharge_detail();

    recharge_detail.__recharge_money = res[DBRechargeRewards::RECHARGE_MONEY].numberInt();
    recharge_detail.__recharge_times = res[DBRechargeRewards::RECHARGE_TIMES].numberInt();
    recharge_detail.__feedback_awards = res[DBRechargeRewards::FEEDBACK_AWARDS].numberInt();
    recharge_detail.__last_recharge_time = res[DBRechargeRewards::LAST_RECHARGE_TIME].numberLong();
    recharge_detail.__first_recharge_time = res[DBRechargeRewards::FIRST_RECHARGE_TIME].numberLong();
    recharge_detail.__love_gift_index = res[DBRechargeRewards::LOVE_GIFT_INDEX].numberInt();
    recharge_detail.__love_gift_recharge = res[DBRechargeRewards::LOVE_RECHARGE_MONEY].numberInt();
    recharge_detail.__recharge_type = std::max(res[DBRechargeRewards::RECHARGE_TYPE].numberInt(), 1);

    DBCommon::bson_to_int_vec(recharge_detail.__recharge_awards,
    		res.getObjectField(DBRechargeRewards::RECHARGE_AWARDS.c_str()));

    GameCommon::bson_to_map(recharge_detail.__recharge_map,
    		res.getObjectField(DBRechargeRewards::RECHARGE_MAP.c_str()));

	{
		if(recharge_detail.__recharge_money > 0 && recharge_detail.__recharge_awards.empty() && recharge_detail.__love_gift_index < 1)
		{
			recharge_detail.__love_gift_index = 1;
		}
	}

	RebateRechargeDetail &rebate_recharge = player->rebate_recharge_dtail();
	if(res.hasField(DBRechargeRewards::REBATE_RECHARGE.c_str()))
	{
		BSONObj rebate_bson = res[DBRechargeRewards::REBATE_RECHARGE].Obj();
		rebate_recharge.__rebate_times = rebate_bson[DBRechargeRewards::RebateRecharge::REBATE_TIMES].numberInt();
		rebate_recharge.__last_buy_time = rebate_bson[DBRechargeRewards::RebateRecharge::LAST_BUY_TIME].numberLong();
	}

    DailyRechargeDetail &daily_recharge = player->daily_recharge_dtail();
	if(res.hasField(DBRechargeRewards::DAILY_RECHARGE.c_str()))
	{
		typedef DBRechargeRewards::DailyRecharge DBDailyRecharge;
		BSONObj daily_bson = res[DBRechargeRewards::DAILY_RECHARGE].Obj();
		if(!daily_bson.isEmpty())
		{
			daily_recharge.__first_recharge_gold = daily_bson[DBDailyRecharge::FIRST_RECHARGE_GOLD].numberInt();

			int last_time = daily_bson[DBDailyRecharge::LAST_RECHARGE_TIME].numberLong();

			daily_recharge.__last_recharge_time = last_time;
			daily_recharge.__today_recharge_gold = daily_bson[DBDailyRecharge::TODAY_RECHARGE_GOLD].numberInt();
			daily_recharge.__act_recharge_times = daily_bson[DBDailyRecharge::ACT_RECHARGE_TIMES].numberInt();
			daily_recharge.__act_has_mail = daily_bson[DBDailyRecharge::ACT_HAS_MAIL].numberInt();

			IntVec daily_vec;

			DBCommon::bson_to_int_vec(daily_vec,daily_bson.getObjectField(DBDailyRecharge::DAILY_RECHARGE_REWARDS.c_str()));

			for (int i = 0; i < (int)daily_vec.size(); ++ i)
				daily_recharge.__daily_recharge_rewards[i] = ::std::max(daily_vec[i], 1) - 1;
		}
	}

	if(res.hasField(DBRechargeRewards::TOTAL_RECHARGE.c_str()))
	{
		typedef DBRechargeRewards::TotalRecharge DBTotalRecharge;
		BSONObj total_bson = res[DBRechargeRewards::TOTAL_RECHARGE].Obj();

		if(!total_bson.isEmpty())
		{
			TotalRechargeDetail &total_recharge = player->total_recharge_detail();
			DBCommon::bson_to_int_vec(total_recharge.__reward_states,
					total_bson.getObjectField(DBTotalRecharge::REWARD_STATES.c_str()));
		}
	}


	return 0;

END_CATCH

	return -1;
}

int MMORechargeRewards::update_data(MapLogicPlayer *player, MongoDataMap* mongo_data)
{
	typedef DBRechargeRewards::DailyRecharge DBDailyRecharge;
	typedef DBRechargeRewards::RebateRecharge DBRebateRecharge;

	BSONObjBuilder daily_builder;
	BSONObjBuilder rebate_builder;
	BSONObjBuilder invest_builder;

	RebateRechargeDetail &rebate_recharge = player->rebate_recharge_dtail();
	DailyRechargeDetail &daily_recharge = player->daily_recharge_dtail();

	rebate_builder << DBRebateRecharge::REBATE_TIMES << rebate_recharge.__rebate_times
			<<	DBRebateRecharge::LAST_BUY_TIME << rebate_recharge.__last_buy_time;

	IntVec daily_vec;
	for (int i = 0; i < (int)daily_recharge.__daily_recharge_rewards.size(); ++i)
	{
		daily_vec.push_back(daily_recharge.__daily_recharge_rewards[i] + 1);
	}

	daily_builder << DBDailyRecharge::TODAY_RECHARGE_GOLD << daily_recharge.__today_recharge_gold
			<< DBDailyRecharge::FIRST_RECHARGE_GOLD << daily_recharge.__first_recharge_gold
			<< DBDailyRecharge::LAST_RECHARGE_TIME << daily_recharge.__last_recharge_time
			<< DBDailyRecharge::ACT_RECHARGE_TIMES << daily_recharge.__act_recharge_times
			<< DBDailyRecharge::ACT_HAS_MAIL << daily_recharge.__act_has_mail
			<< DBDailyRecharge::DAILY_RECHARGE_REWARDS << daily_vec;

	TotalRechargeDetail &total_recharge = player->total_recharge_detail();

	BSONObjBuilder total_recharge_builder;
	total_recharge_builder << DBRechargeRewards::TotalRecharge::REWARD_STATES << total_recharge.__reward_states;

	RechargeDetail &recharge_detail = player->recharge_detail();

	BSONVec recharge_map_bson;
	GameCommon::map_to_bson(recharge_map_bson, recharge_detail.__recharge_map);

	BSONObjBuilder builder;
	builder << DBRechargeRewards::RECHARGE_MONEY << recharge_detail.__recharge_money
			<< DBRechargeRewards::RECHARGE_TIMES << recharge_detail.__recharge_times
			<< DBRechargeRewards::FEEDBACK_AWARDS << recharge_detail.__feedback_awards
			<< DBRechargeRewards::LAST_RECHARGE_TIME << recharge_detail.__last_recharge_time
			<< DBRechargeRewards::FIRST_RECHARGE_TIME << recharge_detail.__first_recharge_time
			<< DBRechargeRewards::RECHARGE_TYPE << recharge_detail.__recharge_type
			<< DBRechargeRewards::RECHARGE_MAP << recharge_map_bson
			<< DBRechargeRewards::RECHARGE_AWARDS << recharge_detail.__recharge_awards
			<< DBRechargeRewards::LOVE_GIFT_INDEX << recharge_detail.__love_gift_index
			<< DBRechargeRewards::LOVE_RECHARGE_MONEY << recharge_detail.__love_gift_recharge
			<< DBRechargeRewards::DAILY_RECHARGE << daily_builder.obj()
			<< DBRechargeRewards::REBATE_RECHARGE << rebate_builder.obj()
			<< DBRechargeRewards::TOTAL_RECHARGE << total_recharge_builder.obj();

	BSONObj obj = builder.obj();
	mongo_data->push_update(DBRechargeRewards::COLLECTION, BSON(DBRechargeRewards::ID
			<< player->role_id()), obj, true);

	return 0;
}

void MMORechargeRewards::load_daily_recharge_mail_role(MLBackDailyRechargeSys *sys)
{
	BSONObj res = CACHED_CONNECTION.findOne(DBDailyRechargeRoleMail::COLLECTION,
	    	QUERY(DBDailyRechargeRoleMail::ID << 0));
	JUDGE_RETURN(res.isEmpty() == false, ;);

	DailyMap &daily_map = sys->daily_map();

	BSONObjIterator iter(res.getObjectField(DBDailyRechargeRoleMail::MAIL_MAP.c_str()));
	while (iter.more())
	{
		BSONObj obj = iter.next().embeddedObject();
		Int64 role_id = obj[DBDailyRechargeRoleMail::ROLE_ID].numberLong();

		DailyRechargeDetail &daily_recharge = daily_map[role_id];
		daily_recharge.__first_recharge_gold = obj[DBDailyRechargeRoleMail::FIRST_RECHARGE_TIMES].numberInt();

		IntVec daily_vec;

		DBCommon::bson_to_int_vec(daily_vec,obj.getObjectField(DBDailyRechargeRoleMail::DAILY_RECHARGE.c_str()));

		for (int i = 0; i < (int)daily_vec.size(); ++ i)
			daily_recharge.__daily_recharge_rewards[i] = ::std::max(daily_vec[i], 1) - 1;
	}
}

void MMORechargeRewards::save_daily_recharge_mail_role(MLBackDailyRechargeSys *sys)
{
	DailyMap &daily_map = sys->daily_map();

	if (daily_map.size() <= 0)
	{
		GameCommon::request_remove_mmo_begin(DBDailyRechargeRoleMail::COLLECTION,
				BSON(DBDailyRechargeRoleMail::ID << 0));
		return;
	}

	BSONVec recharge_vec;
	for (DailyMap::iterator iter = daily_map.begin(); iter != daily_map.end(); ++iter)
	{
		DailyRechargeDetail &daily_recharge = iter->second;

		IntVec daily_vec;
		for (int i = 0; i < (int)daily_recharge.__daily_recharge_rewards.size(); ++i)
		{
			daily_vec.push_back(daily_recharge.__daily_recharge_rewards[i] + 1);
		}

		recharge_vec.push_back(BSON(DBDailyRechargeRoleMail::ROLE_ID << iter->first
				<< DBDailyRechargeRoleMail::FIRST_RECHARGE_TIMES << daily_recharge.__first_recharge_gold
				<< DBDailyRechargeRoleMail::DAILY_RECHARGE << daily_vec));
	}

	BSONObjBuilder builder;
	builder << DBDailyRechargeRoleMail::MAIL_MAP << recharge_vec;

	GameCommon::request_save_mmo_begin(DBDailyRechargeRoleMail::COLLECTION,
			BSON(DBDailyRechargeRoleMail::ID << 0), BSON("$set" << builder.obj()));
}

void MMORechargeRewards::ensure_all_index(void)
{
	this->conection().ensureIndex(DBRechargeRewards::COLLECTION, BSON(DBRechargeRewards::ID << 1), true);
	this->conection().ensureIndex(DBDailyRechargeRoleMail::COLLECTION, BSON(DBDailyRechargeRoleMail::ID << 1), true);
}
