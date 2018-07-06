/*
 * MMOIllustration.cpp
 *
 *  Created on: 2016年7月25日
 *      Author: lzy0927
 */

#include "GameHeader.h"
#include "MapStruct.h"
#include "MapLogicPlayer.h"
#include "GameField.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>
#include "MMOIllustration.h"
#include "MongoConnector.h"
#include "MapLogicIllustration.h"

using namespace mongo;


MMOIllustration::MMOIllustration() {
	// TODO Auto-generated constructor stub

}

MMOIllustration::~MMOIllustration() {
	// TODO Auto-generated destructor stub
}


int MMOIllustration::load_player_illus(MapLogicPlayer *player)
{

	IntSet &illus_group_list = player->get_group_list();
	IntMap &illus_list = player->get_illus_list();
	illus_group_list.clear();
	illus_list.clear();
BEGIN_CATCH
	BSONObj res = this->conection().findOne(DBIllus::COLLECTION,
			QUERY(DBIllus::ID << player->role_id()));

	if(res.isEmpty())
		return 0;
	player->get_open() = res[DBIllus::OPEN].numberInt();
    BSONObj illus_group = res[DBIllus::ILLUS_GROUP].Obj();
    this->mmo_illus_group_load_from_bson(illus_group, player);

    BSONObj illus = res[DBIllus::ILLUS].Obj();
    this->mmo_illus_load_from_bson(illus, player);


END_CATCH
	return -1;
}

int MMOIllustration::update_data(MapLogicPlayer *player, MongoDataMap *data_map)
{


//	MSG_USER("test5 work %d /n", (int)player->role_id());

	BSONObj illus_bson;
	mmo_illus_dump_to_bson(player, illus_bson);

	BSONObj illus_group_bson;
	mmo_illus_group_dump_to_bson(player, illus_group_bson);

	BSONObjBuilder builder;
	builder << DBIllus::ILLUS << illus_bson
			<< DBIllus::ILLUS_GROUP << illus_group_bson
			<< DBIllus::OPEN << player->get_open();
	data_map->push_update(DBIllus::COLLECTION,
			BSON(DBIllus::ID << player->role_id()),
			builder.obj(), true);
	return 0;
}

void MMOIllustration::ensure_all_index(void)
{
	this->conection().ensureIndex(DBIllus::COLLECTION, BSON(DBIllus::ID << 1), true);
}

int MMOIllustration::mmo_illus_dump_to_bson(MapLogicPlayer *player, BSONObj& res_obj)
{
	JUDGE_RETURN(player != NULL, -1);

	IntMap &illus_list = player->get_illus_list();
	typedef DBIllus::Illus  DB_ILLUS;

	std::vector<BSONObj> bson_vec;
	for(IntMap::iterator iter = illus_list.begin(); iter != illus_list.end(); iter++)
	{
		int illus_id = iter->first;
		int illus_level = iter->second;

//		 MSG_USER("test6 work %d %d/n",illus_id, illus_level);

		bson_vec.push_back(BSON( DB_ILLUS::ILLUS_ID << illus_id
				<<  DB_ILLUS::ILLUS_LEVEL << illus_level
				<<  DB_ILLUS::ILLUS_NUM << 0));
	}

	BSONObjBuilder builder;
	builder << DBIllus::ILLUS << bson_vec;
	res_obj = builder.obj();
	// MSG_DEBUG("DailyLiveness %s", res_obj.toString().c_str());
	return 0;
}

int MMOIllustration::mmo_illus_load_from_bson(BSONObj& bson_obj, MapLogicPlayer *player)
{
	JUDGE_RETURN(player != NULL, -1);

	IntMap &illus_list = player->get_illus_list();
	illus_list.clear();
	typedef DBIllus::Illus  DB_ILLUS;

	//MSG_DEBUG("DailyLiveness %s", bson_obj.toString().c_str());

	JUDGE_RETURN(!bson_obj.isEmpty(), 0);

	BSONObjIterator illus_iter(bson_obj[DBIllus::ILLUS].Obj());
	while(illus_iter.more())
	{
		BSONObj illus_bson = illus_iter.next().Obj();
		int illus_id = illus_bson[DB_ILLUS::ILLUS_ID].numberInt();
		int illus_level = illus_bson[DB_ILLUS::ILLUS_LEVEL].numberInt();

		illus_list.insert(IntMap::value_type(illus_id, illus_level));
	}

	return 0;
}


int MMOIllustration::mmo_illus_group_dump_to_bson(MapLogicPlayer *player, BSONObj& res_obj)
{
	JUDGE_RETURN(player != NULL, -1);

	IntSet &illus_group_list = player->get_group_list();
	typedef DBIllus::Illus_group  DB_ILLUS;

	std::vector<BSONObj> bson_vec;
	for(IntSet::iterator iter = illus_group_list.begin(); iter != illus_group_list.end(); iter++)
	{
		int illus_group_id = *iter;
//		int illus_group_type = 0;//test

//		 MSG_USER("test7 work %d /n",illus_group_id);

		bson_vec.push_back(BSON( DB_ILLUS::GROUP_ID << illus_group_id));
	}

	BSONObjBuilder builder;
	builder << DBIllus::ILLUS_GROUP << bson_vec;
	res_obj = builder.obj();
	// MSG_DEBUG("DailyLiveness %s", res_obj.toString().c_str());
	return 0;
}

int MMOIllustration::mmo_illus_group_load_from_bson(BSONObj& bson_obj, MapLogicPlayer *player)
{
	JUDGE_RETURN(player != NULL, -1);

	IntSet &illus_group_list = player->get_group_list();
	illus_group_list.clear();

	typedef DBIllus::Illus_group  DB_ILLUS;

	//MSG_DEBUG("DailyLiveness %s", bson_obj.toString().c_str());

	JUDGE_RETURN(!bson_obj.isEmpty(), 0);

	BSONObjIterator illus_group_iter(bson_obj[DBIllus::ILLUS_GROUP].Obj());
	while(illus_group_iter.more())
	{
		BSONObj illus_bson = illus_group_iter.next().Obj();
		int group_id = illus_bson[DB_ILLUS::GROUP_ID].numberInt();

//		int group_type = illus_bson[DB_ILLUS::GROUP_TYPE].numberInt();

		illus_group_list.insert(group_id);
	}

	return 0;
}
