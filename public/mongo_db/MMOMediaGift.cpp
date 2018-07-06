/*
 * MMOMediaGift.cpp
 *
 *  Created on: Aug 13, 2014
 *      Author: root
 */

#include "GameField.h"
#include "MMOMediaGift.h"
#include "MongoConnector.h"
#include "MapLogicPlayer.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOMediaGift::MMOMediaGift() {
	// TODO Auto-generated constructor stub

}

MMOMediaGift::~MMOMediaGift() {
	// TODO Auto-generated destructor stub
}

int MMOMediaGift::load_player_media_gift(MapLogicPlayer *player)
{
BEGIN_CATCH
    BSONObj res = this->conection().findOne(DBMediaGiftPlayer::COLLECTION,
    		QUERY(DBMediaGiftPlayer::ID << (long long int)(player->role_id())));

    if (res.isEmpty() == true)
        return -1;

    GameCommon::bson_to_map(player->media_gift_detail().__gift_use_times,
    		res.getObjectField(DBMediaGiftPlayer::USE_TIMES_LIST.c_str()));

    GameCommon::bson_to_map(player->media_gift_detail().__gift_use_tags,
    		res.getObjectField(DBMediaGiftPlayer::USE_TAGS_LIST.c_str()));

    GameCommon::bson_to_map(player->media_gift_detail().__gift_use_tick,
    		res.getObjectField(DBMediaGiftPlayer::USE_TICK_LIST.c_str()));

	return 0;

END_CATCH

	return -1;
}
int MMOMediaGift::update_data(MapLogicPlayer *player, MongoDataMap* mongo_data)
{
	BSONVec use_times_bsonset;
	GameCommon::map_to_bson(use_times_bsonset, player->media_gift_detail().__gift_use_times);

	BSONVec use_tags_bsonset;
	GameCommon::map_to_bson(use_tags_bsonset, player->media_gift_detail().__gift_use_tags);

	BSONVec use_tick_bsonset;
	GameCommon::map_to_bson(use_tick_bsonset, player->media_gift_detail().__gift_use_tick);

	BSONObjBuilder builder;
	builder << DBMediaGiftPlayer::USE_TIMES_LIST << use_times_bsonset
			<< DBMediaGiftPlayer::USE_TAGS_LIST << use_tags_bsonset
			<< DBMediaGiftPlayer::USE_TICK_LIST << use_tick_bsonset;

	mongo_data->push_update(DBMediaGiftPlayer::COLLECTION, BSON(DBMediaGiftPlayer::ID
			<< (long long int)player->role_id()), builder.obj(), true);

	return 0;
}

void MMOMediaGift::ensure_all_index(void)
{
	this->conection().ensureIndex(DBMediaGiftPlayer::COLLECTION, BSON(DBMediaGiftPlayer::ID << 1), true);
}
