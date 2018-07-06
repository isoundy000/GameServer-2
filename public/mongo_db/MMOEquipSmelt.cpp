/*
 * MMOEquipSmelt.cpp
 *
 *  Created on: 2016年8月9日
 *      Author: lzy0927
 */

#include "MMOEquipSmelt.h"
#include "GameField.h"
#include "MongoConnector.h"
#include "MapLogicPlayer.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOEquipSmelt::MMOEquipSmelt()
{
	// TODO Auto-generated constructor stub
}

MMOEquipSmelt::~MMOEquipSmelt()
{
	// TODO Auto-generated destructor stub
}

int MMOEquipSmelt::load_player_equip_smelt(MapLogicPlayer * player)
{
	BSONObj obj = this->conection().findOne(DBEquipSmelt::COLLECTION,
			QUERY(DBEquipSmelt::ID << player->role_id()));
	JUDGE_RETURN(obj.isEmpty() == false, -1);

	SmeltInfo &smelt_info = player->smelt_info();
	smelt_info.__smelt_level = obj[DBEquipSmelt::SMELT_LEVEL].numberInt();
	smelt_info.__smelt_exp = obj[DBEquipSmelt::SMELT_EXP].numberInt();
	smelt_info.__recommend = obj[DBEquipSmelt::RECOMMEND].numberInt();
	smelt_info.open_ = obj[DBEquipSmelt::OPEN].numberInt();
	return 0;
}
int MMOEquipSmelt::update_data(MapLogicPlayer *player, MongoDataMap *data_map)
{
	SmeltInfo &smelt_info = player->smelt_info();
	BSONObjBuilder builder;

	builder << DBEquipSmelt::SMELT_LEVEL << smelt_info.__smelt_level
			<< DBEquipSmelt::SMELT_EXP << smelt_info.__smelt_exp
			<< DBEquipSmelt::RECOMMEND << smelt_info.__recommend
			<< DBEquipSmelt::OPEN << smelt_info.open_;


	data_map->push_update(DBEquipSmelt::COLLECTION,
			BSON(DBEquipSmelt::ID << player->role_id()),
			builder.obj(), true);


	return 0;
}
void MMOEquipSmelt::ensure_all_index(void)
{
	this->conection().ensureIndex(DBEquipSmelt::COLLECTION, BSON(DBEquipSmelt::ID << 1), true);
}

