/*
 * MMOInvestSystem.cpp
 *
 *  Created on: 2016年11月15日
 *      Author: lzy
 */

#include "InvestRechargeSys.h"
#include "MMOInvestSystem.h"
#include "MongoDataMap.h"
#include "MongoData.h"
#include "MongoException.h"
#include "MongoConnector.h"
#include <mongo/client/dbclient.h>
#include "GameField.h"
#include "DBCommon.h"
#include "MapLogicPlayer.h"

MMOInvestSystem::MMOInvestSystem() {
	// TODO Auto-generated constructor stub
}

MMOInvestSystem::~MMOInvestSystem() {
	// TODO Auto-generated destructor stub
}

int MMOInvestSystem::load_player(MapLogicPlayer* player)
{
	BSONObj res = this->conection().findOne(InvestRecharger::COLLECTION,
			QUERY(InvestRecharger::ID << player->role_id()));
	if (res.isEmpty() == true)
	{
		return this->load_old_version(player);
	}
	else
	{
		typedef InvestRecharge::InvestMap DB_INVEST;
		InvestRechargeDetail &detail = player->invest_recharge_detail();

		detail.__is_buy = res[DB_INVEST::IS_BUY].numberInt();
		detail.__vip_level = res[DB_INVEST::VIP_LEVEL].numberInt();
		detail.__buy_time.set_tick(res[DB_INVEST::BUY_TIME].numberLong());

		IntVec invest_vec;
		DBCommon::bson_to_int_vec(invest_vec, res.getObjectField(
				InvestRecharge::InvestMap::INVEST_REWARDS.c_str()));
		for (uint i = 0; i < invest_vec.size(); ++ i)
		{
			detail.__invest_rewards[i + 1] = ::std::max(invest_vec[i], 1) - 1;
		}

		IntVec vip_vec;
		DBCommon::bson_to_int_vec(vip_vec, res.getObjectField(
				InvestRecharge::InvestMap::VIP_REWARDS.c_str()));
		for (uint i = 0; i < vip_vec.size(); ++ i)
		{
			detail.__vip_rewards[i + 1] = ::std::max(vip_vec[i], 1) - 1;
		}
	}

	return 0;
}

int MMOInvestSystem::load_old_version(MapLogicPlayer* player)
{
	BSONObj res = this->conection().findOne(InvestRecharge::COLLECTION,
		mongo::Query().sort(BSON(InvestRecharge::VERSION << -1)));
	JUDGE_RETURN(res.isEmpty() == false, 0);

	BSONObjIterator bson_iter = res.getObjectField(InvestRecharge::INVEST_RECHARGE.c_str());
	typedef InvestRecharge::InvestMap DB_INVEST;
	while(bson_iter.more())
	{
		BSONObj info_bson = bson_iter.next().Obj();

		Int64 id = info_bson[DB_INVEST::ID].numberLong();
		JUDGE_CONTINUE(id == player->role_id());

		InvestRechargeDetail &detail = player->invest_recharge_detail();
		detail.__buy_time.set_tick(info_bson[DB_INVEST::BUY_TIME].numberLong());
		detail.__is_buy = info_bson[DB_INVEST::IS_BUY].numberInt();
		detail.__vip_level = info_bson[DB_INVEST::VIP_LEVEL].numberInt();

		IntVec invest_vec;
		DBCommon::bson_to_int_vec(invest_vec,info_bson.getObjectField(
				InvestRecharge::InvestMap::INVEST_REWARDS.c_str()));
		for (uint i = 0; i < invest_vec.size(); ++ i)
		{
			detail.__invest_rewards[i + 1] = ::std::max(invest_vec[i], 1) - 1;
		}

		IntVec vip_vec;
		DBCommon::bson_to_int_vec(vip_vec,info_bson.getObjectField(
				InvestRecharge::InvestMap::VIP_REWARDS.c_str()));
		for (uint i = 0; i < vip_vec.size(); ++ i)
		{
			detail.__vip_rewards[i + 1] = ::std::max(vip_vec[i], 1) - 1;
		}

		if (detail.__is_buy == true)
		{
			player->check_invest_normal_reward();
			player->check_invest_vip_reward();
		}

		break;
	}

	return 0;
}

int MMOInvestSystem::update_data(MapLogicPlayer *player, MongoDataMap *data_map)
{
	InvestRechargeDetail &detail = player->invest_recharge_detail();
//	JUDGE_RETURN(detail.__is_buy == true, -1);

	IntVec invest_normal_vec, invest_vip_vec;
	for (uint i = 1; i <= detail.__invest_rewards.size(); ++i)
	{
		invest_normal_vec.push_back(detail.__invest_rewards[i] + 1);
		invest_vip_vec.push_back(detail.__vip_rewards[i] + 1);
	}

	BSONObjBuilder builder;
	builder	<< InvestRecharge::InvestMap::IS_BUY << detail.__is_buy
			<< InvestRecharge::InvestMap::BUY_TIME << detail.__buy_time.create_tick_
			<< InvestRecharge::InvestMap::VIP_LEVEL << detail.__vip_level
			<< InvestRecharge::InvestMap::INVEST_REWARDS << invest_normal_vec
			<< InvestRecharge::InvestMap::VIP_REWARDS << invest_vip_vec
			<< InvestRecharge::InvestMap::SAVE_FLAG << int(1);

    data_map->push_update(InvestRecharger::COLLECTION, BSON(
    		InvestRecharger::ID << player->role_id()), builder.obj(), true);
	return 0;
}

int MMOInvestSystem::load_data()
{
	BSONObj res = CACHED_CONNECTION.findOne(InvestRecharge::COLLECTION,
		mongo::Query().sort(BSON(InvestRecharge::VERSION << -1)));
	JUDGE_RETURN(res.isEmpty() == false, 0);

//	BSONObj sys_map = res[InvestRecharge::INVEST_RECHARGE].Obj();
//	JUDGE_RETURN(!sys_map.isEmpty(), 0);
//
//	BSONObjIterator bson_iter(sys_map);
//	while(bson_iter.more())
//	{
//		BSONObj info_bson = bson_iter.next().Obj();
//		Int64 id = info_bson[DB_INVEST::ID].numberLong();
//		InvestRechargeDetail &detail = invest_map[id];
//		detail.__buy_time = info_bson[DB_INVEST::BUY_TIME].numberLong();
//		detail.__get_reward = info_bson[DB_INVEST::GET_REWARD].numberInt();
//		detail.__invest_index = info_bson[DB_INVEST::INVEST_INDEX].numberInt();
//		detail.__is_buy = info_bson[DB_INVEST::IS_BUY].numberInt();
//		detail.__reward_time = info_bson[DB_INVEST::REWARD_TIME].numberLong();
//		detail.__vip_level = info_bson[DB_INVEST::VIP_LEVEL].numberInt();
//
//		IntVec invest_vec;
//		DBCommon::bson_to_int_vec(invest_vec,info_bson.getObjectField(InvestRecharge::InvestMap::INVEST_REWARDS.c_str()));
//		for (int i = 0; i < (int)invest_vec.size(); ++ i)
//			detail.__invest_rewards[i + 1] = ::std::max(invest_vec[i], 1) - 1;
//
//		invest_vec.clear();
//		DBCommon::bson_to_int_vec(invest_vec,info_bson.getObjectField(InvestRecharge::InvestMap::VIP_REWARDS.c_str()));
//		for (int i = 0; i < (int)invest_vec.size(); ++ i)
//			detail.__vip_rewards[i + 1] = ::std::max(invest_vec[i], 1) - 1;
//	}

	BSONObjIterator daily_iter(res.getObjectField(InvestRecharge::SERIAL_DAILY.c_str()));
	DBCommon::bson_to_long_map(BackIR_SYS->serial_daily(), daily_iter);

	BSONObjIterator rebate_iter(res.getObjectField(InvestRecharge::SERIAL_REBATE.c_str()));
	DBCommon::bson_to_long_map(BackIR_SYS->serial_rebate(), rebate_iter);

	BSONObjIterator invest_iter(res.getObjectField(InvestRecharge::SERIAL_INVEST.c_str()));
	DBCommon::bson_to_long_map(BackIR_SYS->serial_invest(), invest_iter);

	return 0;
}

int MMOInvestSystem::update_data(MongoDataMap *mongo_data)
{
//	InvestMap &invest_map = BackIR_SYS->invest_map();
//	std::vector<BSONObj> bson_vec;
//	for(InvestMap::iterator iter = invest_map.begin(); iter != invest_map.end(); iter++)
//	{
//		Int64 id = iter->first;
//		InvestRechargeDetail &detail = iter->second;
//
//		IntVec invest_normal_vec, invest_vip_vec;
//		for (int i = 1; i <= (int)detail.__invest_rewards.size(); ++i)
//		{
//			invest_normal_vec.push_back(detail.__invest_rewards[i] + 1);
//			invest_vip_vec.push_back(detail.__vip_rewards[i] + 1);
//		}
//
//		bson_vec.push_back(BSON( DB_INVEST::ID << id
//				<< DB_INVEST::IS_BUY << detail.__is_buy
//				<< DB_INVEST::BUY_TIME << detail.__buy_time
//				<< DB_INVEST::REWARD_TIME << detail.__reward_time
//				<< DB_INVEST::GET_REWARD << detail.__get_reward
//				<< DB_INVEST::INVEST_INDEX << detail.__invest_index
//				<< DB_INVEST::VIP_LEVEL << detail.__vip_level
//				<< DB_INVEST::INVEST_REWARDS << invest_normal_vec
//				<< DB_INVEST::VIP_REWARDS << invest_vip_vec));
//	}
//
//	MSG_USER("INVEST_SYS TEST %d", (int)invest_map.size());

	BSONVec serial_daily, serial_rebate, serial_invest;
	DBCommon::long_map_to_bson(serial_daily, BackIR_SYS->serial_daily());
	DBCommon::long_map_to_bson(serial_rebate, BackIR_SYS->serial_rebate());
	DBCommon::long_map_to_bson(serial_invest, BackIR_SYS->serial_invest());

	BSONObjBuilder builder;
	builder << InvestRecharge::SERIAL_DAILY << serial_daily
			<< InvestRecharge::SERIAL_REBATE << serial_rebate
			<< InvestRecharge::SERIAL_INVEST << serial_invest;

	mongo_data->push_update(InvestRecharge::COLLECTION,
			BSON(InvestRecharge::VERSION << 1), builder.obj(), true);
	return 0;
}

void MMOInvestSystem::ensure_all_index(void)
{
	this->conection().ensureIndex(InvestRecharge::COLLECTION, BSON(InvestRecharge::VERSION << -1), true);
	this->conection().ensureIndex(InvestRecharger::COLLECTION, BSON(InvestRecharger::ID << 1), true);
}
