/*
 * MMOEscort.cpp
 *
 *  Created on: 2016年10月25日
 *      Author: lzy
 */

#include "GameField.h"
#include "MMOEscort.h"
#include "MongoConnector.h"
#include "MapMonitor.h"
#include "MapPlayerEx.h"
#include "MapLogicPlayer.h"
#include "MongoDataMap.h"

#include "DBCommon.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>
using namespace mongo;

MMOEscort::~MMOEscort(void)
{ /*NULL*/ }

int MMOEscort::load_player_escort(MapPlayerEx *player)
{
    BSONObj res = this->conection().findOne(Escort::COLLECTION,
    		QUERY(Escort::ID << player->role_id()));
    JUDGE_RETURN(res.isEmpty() == false, -1);

    Escort_detail& escort_detail = player->get_escort_detail();
    escort_detail.car_index_ = res[Escort::CAR_INDEX].numberLong();
    escort_detail.escort_times_ = res[Escort::ESCORT_TIMES].numberInt();
    escort_detail.escort_type_ = res[Escort::ESCORT_TYPE].numberInt();
    escort_detail.total_exp_ = res[Escort::TOTAL_EXP].numberInt();
    escort_detail.protect_id = res[Escort::PROTECT_ID].numberLong();
    escort_detail.start_tick_ = res[Escort::START_TICK].numberLong();
    escort_detail.target_level = res[Escort::TARGET_LEVEL].numberInt();
    escort_detail.till = res[Escort::TILL].numberInt();

    BSONObj protect_list = res[Escort::PROTECT_LIST].Obj();
	BSONObjIterator iter(protect_list);
	while(iter.more())
	{
		BSONObj bson = iter.next().Obj();
		Int64 id = bson[Escort::Protect_list::PROTECT_PLAYER].numberLong();

		escort_detail.protect_map.insert(id);
	}
    return 0;
}

int MMOEscort::update_data(MapPlayerEx *player, MongoDataMap *mongo_data)
{
    Escort_detail& escort_detail = player->get_escort_detail();

	BSONVec bson_vec;
	for(LongSet::iterator iter = escort_detail.protect_map.begin();
			iter != escort_detail.protect_map.end(); iter++)
	{
		bson_vec.push_back(BSON(Escort::Protect_list::PROTECT_PLAYER << *iter));
	}

    BSONObjBuilder builder;
    builder << Escort::CAR_INDEX << escort_detail.car_index_
    		<< Escort::ESCORT_TIMES << escort_detail.escort_times_
    		<< Escort::ESCORT_TYPE << escort_detail.escort_type()
    		<< Escort::TOTAL_EXP << escort_detail.total_exp_
    		<< Escort::START_TICK << escort_detail.start_tick_
    		<< Escort::PROTECT_ID << escort_detail.protect_id
    		<< Escort::TILL << escort_detail.till
    		<< Escort::TARGET_LEVEL << escort_detail.target_level
    		<< Escort::PROTECT_LIST << bson_vec;

    mongo_data->push_update(Escort::COLLECTION, BSON(Escort::ID << player->role_id()), builder.obj(), true);
    return 0;
}

void MMOEscort::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(Escort::COLLECTION, BSON(Escort::ID << 1), true);
END_CATCH
}



