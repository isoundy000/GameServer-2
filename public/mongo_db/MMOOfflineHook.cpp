/*
 * MMOOfflineHook.cpp
 *
 *  Created on: Mar 21, 2017
 *      Author: root
 */

#include "MMOOfflineHook.h"
#include "MapLogicPlayer.h"
#include "GameField.h"
#include "DBCommon.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include "MongoConnector.h"

#include <mongo/client/dbclient.h>

using namespace mongo;

MMOOfflineHook::~MMOOfflineHook()
{

}


int MMOOfflineHook::load_player_offlinehook(MapLogicPlayer *player)
{
	Offline_Hookdetial &detial = player->offline_hook_detail();

	BSONObj res = this->conection().findOne(DBPlayerOfflineHook::COLLECTION,
			QUERY(DBPlayerOfflineHook::ID << player->role_id()));
	if (res.isEmpty() == true)
	{
		detial.offline_surplustime.sec(Offline_Hookdetial::max_surplus_time());
		return -1;
	}

	detial.offhook_playerid = res[DBPlayerOfflineHook::ID].numberLong();
	detial.offline_surplustime.sec(res[DBPlayerOfflineHook::surplus_time].numberLong());
	detial.last_offhook_starttime.sec(res[DBPlayerOfflineHook::last_start_time].numberLong());
	detial.last_offhook_endtime.sec(res[DBPlayerOfflineHook::last_end_time].numberLong());
	detial.last_offhook_time.sec(res[DBPlayerOfflineHook::last_offhook_time].numberLong());
	detial.last_offhook_exp = res[DBPlayerOfflineHook::last_reward_exp].numberLong();
	detial.one_point_five_plue_time.sec(res[DBPlayerOfflineHook::one_point_five_plue_time].numberLong());
	detial.vip_plus_time.sec(res[DBPlayerOfflineHook::vip_plus_time].numberLong());
	detial.two_plus_time.sec(res[DBPlayerOfflineHook::two_plus_time].numberLong());

	if (res.hasField(DBPlayerOfflineHook::last_reward.c_str()) )
	{
		GameCommon::bson_to_map(detial.offline_reward, res.getObjectField(DBPlayerOfflineHook::last_reward.c_str()));
	}

	if(res.hasField(DBPlayerOfflineHook::today_reward.c_str()))
	{
		GameCommon::bson_to_map(detial.today_offline_reward, res.getObjectField(DBPlayerOfflineHook::today_reward.c_str()));
	}

//	if (res.hasField(DBPlayerOfflineHook::last_reward.c_str()) )
//	{
//		GameCommon::bson_to_map(detial.offhook_costProp, res.getObjectField(DBPlayerOfflineHook::last_costProp.c_str()));
//	}

	return 0;

}

int MMOOfflineHook::update_data(MapLogicPlayer *player, MongoDataMap *mongo_data)
{
	Offline_Hookdetial &detial = player->offline_hook_detail();

	BSONVec reward,costprop,today_reward;
	GameCommon::map_to_bson(reward, detial.offline_reward, false, true);
	GameCommon::map_to_bson(costprop, detial.offhook_costProp, false, true);
	GameCommon::map_to_bson(today_reward, detial.today_offline_reward, false, true);

	BSONObjBuilder builder;
	builder << DBPlayerOfflineHook::ID << detial.offhook_playerid
			<< DBPlayerOfflineHook::surplus_time << Int64(detial.offline_surplustime.sec())
			<< DBPlayerOfflineHook::last_start_time << Int64(detial.last_offhook_starttime.sec())
			<< DBPlayerOfflineHook::last_end_time << Int64(detial.last_offhook_endtime.sec())
			<< DBPlayerOfflineHook::last_offhook_time << Int64(detial.last_offhook_time.sec())
			<< DBPlayerOfflineHook::last_reward_exp << detial.last_offhook_exp
			<< DBPlayerOfflineHook::last_reward << reward
			<< DBPlayerOfflineHook::one_point_five_plue_time << Int64(detial.one_point_five_plue_time.sec())
			<< DBPlayerOfflineHook::two_plus_time << Int64(detial.two_plus_time.sec())
			<< DBPlayerOfflineHook::vip_plus_time << Int64(detial.vip_plus_time.sec())
			<< DBPlayerOfflineHook::last_costProp << costprop
			<< DBPlayerOfflineHook::today_reward << today_reward
			;


	mongo_data->push_update(DBPlayerOfflineHook::COLLECTION,
			BSON( DBPlayerOfflineHook::ID << detial.offhook_playerid), builder.obj(), true);

	return 0;
}

void MMOOfflineHook::ensure_all_index(void)
{
BEGIN_CATCH
	this->conection().ensureIndex(DBPlayerOfflineHook::COLLECTION, BSON(DBPlayerOfflineHook::ID << 1), true);
END_CATCH
}

