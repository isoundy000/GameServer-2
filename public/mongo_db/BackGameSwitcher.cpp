/*
 * BackGameSwitcher.cpp
 *
 *  Created on: Dec 20, 2014
 *      Author: jinxing
 */

#include "BackGameSwitcher.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>
#include "BackField.h"
#include "GameCommon.h"
#include "MongoConnector.h"
#include "MongoDataMap.h"

BackGameSwitcher::BackGameSwitcher()
{
	// TODO Auto-generated constructor stub

}

BackGameSwitcher::~BackGameSwitcher()
{
	// TODO Auto-generated destructor stub
}

int BackGameSwitcher::direct_load_data(GameSwitcherDetail& detail)
{
	auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBBackSwitcher::COLLECTION);
	while (cursor->more())
	{
		BSONObj res = cursor->next();
		BackGameSwitcher::set_detail_value_by_bson(res, detail);
	}

	return 0;
}

int BackGameSwitcher::load_data_request(MongoDataMap* data_map)
{
	data_map->push_multithread_query(DBBackSwitcher::COLLECTION);
	return 0;
}

int BackGameSwitcher::set_detail_value_by_bson(BSONObj &res, GameSwitcherDetail& detail)
{
	JUDGE_RETURN(res.isEmpty() == false, -1);

	std::string name = res[DBBackSwitcher::NAME].str();
	int value = res[DBBackSwitcher::VALUE].numberInt();

	detail.switcher_map_[name] = (value > 0 ? true : false);
	return 0;
}

int BackGameSwitcher::save_game_modify(BackGameModifyVec& update_vec)
{
	for (BackGameModifyVec::iterator iter = update_vec.begin();
			iter != update_vec.end(); ++iter)
	{
		JUDGE_CONTINUE(!iter->name.empty());
		BSONObjBuilder builder;
		builder << DBBackModify::ROLE_ID << 0
				<< DBBackModify::LEAGUE_ID << 0
				<< DBBackModify::IS_UPDATE << 0;
		GameCommon::request_save_mmo_begin(DBBackModify::COLLECTION,
				BSON(DBBackModify::NAME << iter->name),
				BSON("$set" << builder.obj()));
	}
	return 0;
}

string BackGameSwitcher::get_49you_qq()
{
	BSONObj ret_fields = BSON(DBBackModify::VALUE << 1);
	BSONObj res = CACHED_CONNECTION.findOne(DBBackModify::COLLECTION, QUERY(DBBackModify::NAME << GameModifyName::you49_qq), &ret_fields);
	if (res.isEmpty() == true)
		return "";
	return res[DBBackModify::VALUE].str();
}


string BackGameSwitcher::get_fashion_box()
{
	BSONObj ret_fields = BSON(DBBackModify::VALUE << 1);
	BSONObj res = CACHED_CONNECTION.findOne(DBBackModify::COLLECTION, QUERY(DBBackModify::NAME << GameModifyName::fashion_box), &ret_fields);
	if (res.isEmpty() == true)
		return "";
	return res[DBBackModify::VALUE].str();
}

int BackGameSwitcher::get_lucky_table(BackGameModify& modify)
{
	Int64 now_tick = Time_Value::gettimeofday().sec();
	auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBBackModify::COLLECTION, QUERY(DBBackModify::NAME << GameModifyName::lucky_table));
	std::vector<Int64> remove_set;
	while (cursor->more())
	{
		BSONObj res = cursor->next();
		JUDGE_CONTINUE(res.isEmpty() == false);
		if(res[DBBackModify::OPEN_STATE].numberInt() == 0 && res[DBBackModify::IS_UPDATE].numberLong() == 0)
		{
			remove_set.push_back(res[DBBackModify::ACTIVITY_ID].numberLong());
			continue;
		}
		Int64 start_tick[] = {0,0},end_tick[] = {0,0};
		BSONObjIterator iter(res.getObjectField(DBBackModify::START_TICK.c_str()));
		int i = 0;
		while (iter.more())
		{
			start_tick[i++] = iter.next().numberLong();
		}
		iter = res.getObjectField(DBBackModify::END_TICK.c_str());
		i = 0;
		while (iter.more())
		{
			end_tick[i++] = iter.next().numberLong();
		}
		i = 0;
		int remove_flog = 0;
		for(; i < 2; ++i)
		{
			if(now_tick > end_tick[i])
				++remove_flog;
			if(start_tick[i] <= now_tick && now_tick < end_tick[i])
				break;
		}
		if(remove_flog == 2)
			remove_set.push_back(res[DBBackModify::ACTIVITY_ID].numberLong());
		JUDGE_CONTINUE(i < 2);
		modify.is_update = 1;
		modify.value_str = res[DBBackModify::VALUE].str();
		modify.ltable.activity_id = res[DBBackModify::ACTIVITY_ID].numberLong();
		modify.ltable.open_state = res[DBBackModify::OPEN_STATE].numberInt();
		i = 0;
		for(; i < 2; ++i)
		{
			modify.ltable.start_tick[i] = start_tick[i];
			modify.ltable.end_tick[i] = end_tick[i];
		}
		break;
	}
	if(!remove_set.empty())
	{
		for(uint i = 0; i < remove_set.size(); ++i)
		{
			CACHED_CONNECTION.remove(DBBackModify::COLLECTION,
					QUERY(DBBackModify::NAME << GameModifyName::lucky_table << DBBackModify::ACTIVITY_ID << remove_set[i]));
		}
	}
	return 0;
}

void BackGameSwitcher::ensure_all_index(void)
{
// 索引由 PHP 维护
//BEGIN_CATCH
//	this->conection().ensureIndex(DBBackSwitcher::COLLECTION, BSON(DBBackSwitcher::NAME << 1), true);
//END_CATCH
}
