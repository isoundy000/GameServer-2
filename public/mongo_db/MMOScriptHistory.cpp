/*
 * MMOScriptHistory.cpp
 *
 * Created on: 2014-05-09 16:47
 *     Author: lyz
 */

#include "MMOScriptHistory.h"
#include "GlobalScriptHistory.h"
#include "GameField.h"
#include "MongoConnector.h"
#include "MongoDataMap.h"
#include "GameCommon.h"
#include "DBCommon.h"

#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOScriptHistory::~MMOScriptHistory(void)
{ /*NULL*/ }

int MMOScriptHistory::load_script_chapter_info(GlobalScriptHistory *history)
{
//BEGIN_CATCH
//    BSONObj res = this->conection().findOne(DBScriptHistory::COLLECTION,
//            BSON(DBScriptHistory::SCRIPT_SORT << int(GameEnum::SCRIPT_SORT_CLIMB_TOWER)));
//
//    HistoryChapterRecord history_rec;
//    BSONObjIterator iter(res.getObjectField(DBScriptHistory::CHAPTER_REC.c_str()));
//    while (iter.more())
//    {
//        BSONObj obj = iter.next().embeddedObject();
//        history_rec.reset();
//        history_rec.__chapter_key = obj[DBScriptHistory::ChapterRec::CHAPTER_KEY].numberInt();
//        history_rec.__best_use_tick = obj[DBScriptHistory::ChapterRec::BEST_USE_TICK].numberInt();
//        history_rec.__first_top_level_player = obj[DBScriptHistory::ChapterRec::FIRST_ID].numberLong();
//        history_rec.__first_top_level_role_name = obj[DBScriptHistory::ChapterRec::FIRST_NAME].str();
//
//        history->bind_chapter_rec(history_rec.__chapter_key, history_rec);
//    }
//
//    return 0;
//END_CATCH
//    return -1;
	return 0;
}

int MMOScriptHistory::update_script_chapter_info(GlobalScriptHistory *history, MongoDataMap *data_map)
{
//    GlobalScriptHistory::ChapterRecMap &chapter_rec_map = history->chapter_rec_map();
//
//    std::vector<BSONObj> chapter_vc;
//    for (GlobalScriptHistory::ChapterRecMap::iterator iter = chapter_rec_map.begin(); iter != chapter_rec_map.end(); ++iter)
//    {
//        HistoryChapterRecord &chapter_rec = iter->second;
//        chapter_vc.push_back(BSON(DBScriptHistory::ChapterRec::CHAPTER_KEY << chapter_rec.__chapter_key
//                    << DBScriptHistory::ChapterRec::BEST_USE_TICK << chapter_rec.__best_use_tick
//                    << DBScriptHistory::ChapterRec::FIRST_ID << chapter_rec.__first_top_level_player
//                    << DBScriptHistory::ChapterRec::FIRST_NAME << chapter_rec.__first_top_level_role_name));
//    }
//
//    BSONObjBuilder builder;
//    builder << DBScriptHistory::SCRIPT_SORT << GameEnum::SCRIPT_SORT_CLIMB_TOWER
//        << DBScriptHistory::CHAPTER_REC << chapter_vc;
//
//    data_map->push_update(DBScriptHistory::COLLECTION,
//            BSON(DBScriptHistory::SCRIPT_SORT << int(GameEnum::SCRIPT_SORT_CLIMB_TOWER)),
//            builder.obj(), true);

    return 0;
}

int MMOScriptHistory::load_data(LegendTopPlayer &legend_player, int type)
{
	BSONObj res;
	if (type == GameEnum::SCRIPT_T_LEGEND_TOP)
		res = CACHED_CONNECTION.findOne(DBLegendTopPlayer::COLLECTION,
			BSON(DBLegendTopPlayer::ID << int(0)));
	else
		res = CACHED_CONNECTION.findOne(DBLegendTopPlayer::COLLECTION2,
			BSON(DBLegendTopPlayer::ID << int(0)));

	if (res.isEmpty())
        return 0;

	legend_player.refresh_tick_ = res[DBLegendTopPlayer::REFRESH_TICK].numberLong();

	BSONObjIterator iter(res.getObjectField(DBLegendTopPlayer::PLAYER_SET.c_str()));
	while (iter.more())
	{
		BSONObj obj = iter.next().embeddedObject();
		Int64 player_id = obj[DBLegendTopPlayer::PlayerSet::PLAYER_ID].numberLong();
		LegendTopPlayer::PlayerInfo &player_info = legend_player.player_map_[player_id];
		player_info.player_id_ = player_id;
		player_info.name_ = obj[DBLegendTopPlayer::PlayerSet::NAME].str();
		player_info.fight_score_ = obj[DBLegendTopPlayer::PlayerSet::FIGHT_SCORE].numberInt();
		player_info.floor_ = obj[DBLegendTopPlayer::PlayerSet::FLOOR].numberInt();
		player_info.rank_ = obj[DBLegendTopPlayer::PlayerSet::RANK].numberInt();
		player_info.tick_ = obj[DBLegendTopPlayer::PlayerSet::TICK].numberLong();
	}

//	Time_Value nowtime = Time_Value::gettimeofday();
//	if (legend_player.refresh_tick_ <= nowtime.sec())
//	{
//		legend_player.refresh_tick_ = next_day(0, 0, nowtime).sec();
//		legend_player.player_map_.clear();
//	}
	return 0;
}

int MMOScriptHistory::update_data(LegendTopPlayer& legend_player, int type)
{
	std::vector<BSONObj> player_vc;
	for (LegendTopPlayer::PlayerInfoMap::iterator iter = legend_player.player_map_.begin();
			iter != legend_player.player_map_.end(); ++iter)
	{
		LegendTopPlayer::PlayerInfo &player_info = iter->second;
		player_vc.push_back(BSON(DBLegendTopPlayer::PlayerSet::PLAYER_ID << player_info.player_id_
						<< DBLegendTopPlayer::PlayerSet::NAME << player_info.name_
						<< DBLegendTopPlayer::PlayerSet::FIGHT_SCORE << player_info.fight_score_
						<< DBLegendTopPlayer::PlayerSet::FLOOR << player_info.floor_
						<< DBLegendTopPlayer::PlayerSet::RANK << player_info.rank_
						<< DBLegendTopPlayer::PlayerSet::TICK << player_info.tick_));
	}
	BSONObjBuilder builder;
	builder << DBLegendTopPlayer::REFRESH_TICK << legend_player.refresh_tick_
			<< DBLegendTopPlayer::PLAYER_SET << player_vc;

	if (type == GameEnum::SCRIPT_T_LEGEND_TOP)
		GameCommon::request_save_mmo_begin(DBLegendTopPlayer::COLLECTION,
				BSON(DBLegendTopPlayer::ID << int(0)), BSON("$set" << builder.obj()));
	else
		GameCommon::request_save_mmo_begin(DBLegendTopPlayer::COLLECTION2,
				BSON(DBLegendTopPlayer::ID << int(0)), BSON("$set" << builder.obj()));

	return 0;
}

int MMOScriptHistory::load_couples(LongMap &role_map)
{
	BSONObj res = CACHED_CONNECTION.findOne(DBCouplePlayer::COLLECTION,
			BSON(DBCouplePlayer::ID << int(0)));

	if (res.isEmpty())
		return 0;

	DBCommon::bson_to_long_map(role_map, res.getObjectField(DBCouplePlayer::ROLE_MAP.c_str()));
	return 0;
}

int MMOScriptHistory::update_couples(LongMap &role_map)
{
	BSONVec role_set;
	DBCommon::long_map_to_bson(role_set, role_map);

	BSONObjBuilder builder;
	builder << DBCouplePlayer::ROLE_MAP << role_set;
	GameCommon::request_save_mmo_begin(DBCouplePlayer::COLLECTION,
			BSON(DBCouplePlayer::ID << int(0)), BSON("$set" << builder.obj()));

	return 0;
}

void MMOScriptHistory::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(DBScriptHistory::COLLECTION, BSON(DBScriptHistory::SCRIPT_SORT << 1), true);
	this->conection().ensureIndex(DBLegendTopPlayer::COLLECTION, BSON(DBLegendTopPlayer::ID << 1), true);
	this->conection().ensureIndex(DBLegendTopPlayer::COLLECTION2, BSON(DBLegendTopPlayer::ID << 1), true);
END_CATCH
}

