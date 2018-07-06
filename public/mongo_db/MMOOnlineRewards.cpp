/*
 * MMOOnlineRewards.cpp
 *
 *  Created on: 2014-1-16
 *      Author: root
 */

#include "MMOOnlineRewards.h"
#include "GameField.h"
#include "MongoConnector.h"
#include "MapLogicPlayer.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOOnlineRewards::MMOOnlineRewards()
{
	// TODO Auto-generated constructor stub
}

MMOOnlineRewards::~MMOOnlineRewards()
{
	// TODO Auto-generated destructor stub
}

int MMOOnlineRewards::load_player_online_rewards(MapLogicPlayer * player)
{
	BEGIN_CATCH
		OnlineRewardsDetail &online_rewards_detail = player->online_rewards_detail();
		online_rewards_detail.reset();
		BSONObj obj = this->conection().findOne(DBOnlineRewards::COLLECTION, QUERY(DBOnlineRewards::ID << (long long int) player->role_id()));
		if(true == obj.isEmpty())
			return 0;

	    BSONObj list = obj[DBOnlineRewards::AWARD_LIST].Obj();
	    mmo_list_load_from_bson(list, player);

		online_rewards_detail.__stage = obj[DBOnlineRewards::STAGE].numberInt();
		online_rewards_detail.__pre_stage = obj[DBOnlineRewards::PRE_STAGE].numberInt();
		online_rewards_detail.__login_time = obj[DBOnlineRewards::LOGIN_TIME].numberLong();
		online_rewards_detail.__online_sum = obj[DBOnlineRewards::ONLINE_SUM].numberLong();
		online_rewards_detail.__received_mark = obj[DBOnlineRewards::RECEIVED_MARK].numberInt();
		long tick = online_rewards_detail.__login_time;
		MSG_USER("LOAD ==> stage %d, tick:%s, mark:%d",
				online_rewards_detail.__stage,
				ctime(&tick),
				online_rewards_detail.__received_mark
				);
		return 0;

	END_CATCH
		return -1;
}

int MMOOnlineRewards::mmo_list_dump_to_bson(MapLogicPlayer *player, BSONObj& res_obj)
{
	JUDGE_RETURN(player != NULL, -1);
	IntVec temp = player->get_award_list();
	typedef DBOnlineRewards::Award_List  DB_TEMP;

	std::vector<BSONObj> bson_vec;
	for(IntVec::iterator iter = temp.begin(); iter != temp.end(); iter++)
	{
		bson_vec.push_back(BSON( DB_TEMP::AWARD_NUM << *iter));
	}
	BSONObjBuilder builder;
	builder << DBOnlineRewards::AWARD_LIST << bson_vec;
	res_obj = builder.obj();
	return 0;
}

int MMOOnlineRewards::mmo_list_load_from_bson(BSONObj& bson_obj, MapLogicPlayer *player)
{
	JUDGE_RETURN(player != NULL, -1);
	IntVec &temp = player->get_award_list();
	temp.clear();
	typedef DBOnlineRewards::Award_List  DB_TEMP;

	JUDGE_RETURN(!bson_obj.isEmpty(), 0);

	BSONObjIterator list_iter(bson_obj[DBOnlineRewards::AWARD_LIST].Obj());
	while(list_iter.more())
	{
		BSONObj list_bson = list_iter.next().Obj();
		int num = list_bson[DB_TEMP::AWARD_NUM].numberInt();
		temp.push_back(num);
	}

	return 0;
}

int MMOOnlineRewards::update_data(MapLogicPlayer *player, MongoDataMap *data_map)
{
	BSONObj list;
	MMOOnlineRewards::mmo_list_dump_to_bson(player, list);

	BSONObjBuilder builder;
	OnlineRewardsDetail &online_rewards_detail = player->online_rewards_detail();

	builder << DBOnlineRewards::STAGE << online_rewards_detail.__stage
			<< DBOnlineRewards::PRE_STAGE << online_rewards_detail.__pre_stage
			<< DBOnlineRewards::LOGIN_TIME << online_rewards_detail.__login_time
			<< DBOnlineRewards::ONLINE_SUM << online_rewards_detail.__online_sum
			<< DBOnlineRewards::RECEIVED_MARK << online_rewards_detail.__received_mark
			<< DBOnlineRewards::AWARD_LIST << list;

	data_map->push_update(DBOnlineRewards::COLLECTION,
			BSON(DBOnlineRewards::ID << player->role_id()),builder.obj(), true);

	MSG_USER("SAVE <== stage %d, tick:%ld, mark:%d, id:%ld",
			online_rewards_detail.__stage, online_rewards_detail.__login_time,
			online_rewards_detail.__received_mark, player->role_id());

	return 0;
}
void MMOOnlineRewards::ensure_all_index(void)
{
	this->conection().ensureIndex(DBOnlineRewards::COLLECTION, BSON(DBOnlineRewards::ID << 1), true);
}
