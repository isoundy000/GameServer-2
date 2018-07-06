/*
 * MMOCollectChests.cpp
 *
 *  Created on: 2016年8月8日
 *      Author: lzy0927
 */


#include "MLCollectChests.h"
#include "MMOCollectChests.h"
#include "GameField.h"
#include "MongoConnector.h"
#include "MapLogicPlayer.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOCollectChests::MMOCollectChests()
{
	// TODO Auto-generated constructor stub
}

MMOCollectChests::~MMOCollectChests()
{
	// TODO Auto-generated destructor stub
}

int MMOCollectChests::load_player_collect_chests(MapLogicPlayer * player)
{
	BSONObj res = this->conection().findOne(DBCollectChests::COLLECTION,
				QUERY(DBCollectChests::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	player->collect_chests().collect_num = res[DBCollectChests::COLLECT_NUM].numberInt();
	player->collect_chests().reset_tick = res[DBCollectChests::NEXT_TICK].numberLong();

	return 0;
}
int MMOCollectChests::update_data(MapLogicPlayer *player, MongoDataMap *data_map)
{
//	Collect_Chests &collect_chests = player->collect_chests();


	BSONObjBuilder builder;
	builder << DBCollectChests::COLLECT_NUM<< player->collect_chests().collect_num
			<< DBCollectChests::NEXT_TICK<< player->collect_chests().reset_tick;


	data_map->push_update(DBCollectChests::COLLECTION,
			BSON(DBCollectChests::ID << player->role_id()),
			builder.obj(), true);

	return 0;
}


void MMOCollectChests::ensure_all_index(void)
{
	this->conection().ensureIndex(DBCollectChests::COLLECTION, BSON(DBCollectChests::ID << 1), true);
}
