/*
 * MMOOfflineRewards.cpp
 *
 *  Created on: 2016年8月19日
 *      Author: lzy
 */



#include "MMOOfflineRewards.h"
#include "GameField.h"
#include "MongoConnector.h"
#include "MapLogicPlayer.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOOfflineRewards::MMOOfflineRewards()
{
	// TODO Auto-generated constructor stub
}

MMOOfflineRewards::~MMOOfflineRewards()
{
	// TODO Auto-generated destructor stub
}


int MMOOfflineRewards::load_player_offline_rewards(MapLogicPlayer * player)
{
	BEGIN_CATCH
		OfflineRewardsDetail &offline_rewards_detail = player->offline_rewards_detail();
		offline_rewards_detail.reset();
		BSONObj obj = this->conection().findOne(DBOfflineRewards::COLLECTION, QUERY(DBOfflineRewards::ID << (long long int) player->role_id()));
		if(true == obj.isEmpty())
			return 0;

		offline_rewards_detail.__offline_sum = obj[DBOfflineRewards::OFFLINE_SUM].numberLong();
		offline_rewards_detail.__logout_time = obj[DBOfflineRewards::LOGOUT_TIME].numberLong();
		offline_rewards_detail.__longest_time = obj[DBOfflineRewards::LONGEST_TIME].numberLong();
		offline_rewards_detail.__received_mark = obj[DBOfflineRewards::RECEIVED_MARK].numberInt();

		return 0;

	END_CATCH
		return -1;
}

int MMOOfflineRewards::update_data(MapLogicPlayer *player, MongoDataMap *data_map)
{
	OfflineRewardsDetail &offline_rewards_detail = player->offline_rewards_detail();

	BSONObjBuilder builder;

	builder << DBOfflineRewards::OFFLINE_SUM << offline_rewards_detail.__offline_sum
			<< DBOfflineRewards::LOGOUT_TIME << offline_rewards_detail.__logout_time
			<< DBOfflineRewards::LONGEST_TIME << offline_rewards_detail.__longest_time
			<< DBOfflineRewards::RECEIVED_MARK << offline_rewards_detail.__received_mark;

	data_map->push_update(DBOfflineRewards::COLLECTION,
			BSON(DBOfflineRewards::ID << (long long int) player->role_id()),
			builder.obj(), true);

	return 0;

}

void MMOOfflineRewards::ensure_all_index(void)
{
	this->conection().ensureIndex(DBOfflineRewards::COLLECTION, BSON(DBOfflineRewards::ID << 1), true);
}

