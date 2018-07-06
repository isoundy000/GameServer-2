/*
 * MMOTreasures.cpp
 *
 *  Created on: 2016年12月1日
 *      Author: lzy
 */

#include "MMOTreasures.h"
#include "GameField.h"
#include "MongoConnector.h"
#include "MapLogicPlayer.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOTreasures::MMOTreasures()
{
	// TODO Auto-generated constructor stub
}

MMOTreasures::~MMOTreasures()
{
	// TODO Auto-generated destructor stub
}


int MMOTreasures::load_player_treasures_info(MapLogicPlayer * player)
{
	BEGIN_CATCH
		TreasuresDetail &info = player->treasures_detail();
		info.reset();
		BSONObj obj = this->conection().findOne(DBTreasures::COLLECTION, QUERY(DBTreasures::ID << (long long int) player->role_id()));
		if(true == obj.isEmpty())
			return 0;

		info.free_times_ = obj[DBTreasures::FREE_TIMES].numberInt();
		info.game_index_ = obj[DBTreasures::GAME_INDEX].numberInt();
		info.game_length_ = obj[DBTreasures::GAME_LENGTH].numberInt();
		info.reset_tick_ = obj[DBTreasures::RESET_TICK].numberLong();
		info.reset_times_ = obj[DBTreasures::RESET_TIMES].numberInt();

	    BSONObj bson_obj = obj[DBTreasures::ITEM_LIST].Obj();
		JUDGE_RETURN(!bson_obj.isEmpty(), 0);

		BSONObjIterator iter = obj.getObjectField(DBTreasures::ITEM_LIST.c_str());
		while(iter.more())
		{
			BSONObj obj = iter.next().embeddedObject();
			int id = obj[DBTreasures::Item_list::ID].numberInt();
			int amount = obj[DBTreasures::Item_list::AMOUNT].numberInt();
			int bind = obj[DBTreasures::Item_list::BIND].numberInt();
			int index = obj[DBTreasures::Item_list::INDEX].numberInt();

			ItemObj temp(id, amount, bind, index);
			info.item_list_.push_back(temp);
		}

		return 0;

	END_CATCH
		return -1;
}

int MMOTreasures::update_data(MapLogicPlayer *player, MongoDataMap *data_map)
{
	TreasuresDetail &info = player->treasures_detail();

	BSONObjBuilder builder;

	std::vector<BSONObj> bson_vec;
	for (ItemObjVec::iterator it = info.item_list_.begin();
			it != info.item_list_.end(); ++it)
	{
		ItemObj& obj = (*it);
		bson_vec.push_back(BSON( DBTreasures::Item_list::ID  << obj.id_
							  << DBTreasures::Item_list::AMOUNT  << obj.amount_
							  << DBTreasures::Item_list::BIND  << obj.bind_
							  << DBTreasures::Item_list::INDEX  << obj.index_));
	}

	builder << DBTreasures::FREE_TIMES << info.free_times_
			<< DBTreasures::GAME_INDEX << info.game_index_
			<< DBTreasures::GAME_LENGTH << info.game_length_
			<< DBTreasures::RESET_TICK << info.reset_tick_
			<< DBTreasures::RESET_TIMES << info.reset_times_
			<< DBTreasures::ITEM_LIST << bson_vec;

	data_map->push_update(DBTreasures::COLLECTION,
			BSON(DBTreasures::ID << player->role_id()),
			builder.obj(), true);

	return 0;

}

void MMOTreasures::ensure_all_index(void)
{
	this->conection().ensureIndex(DBTreasures::COLLECTION, BSON(DBTreasures::ID << 1), true);
}




