/*
 * MMOScript.cpp
 *
 * Created on: 2014-01-13 22:21
 *     Author: lyz
 */

#include "MMOScript.h"
#include "GameField.h"

#include "MongoConnector.h" 
#include "MongoDataMap.h"
#include "MapPlayerEx.h"

#include "MongoException.h"
#include <mongo/client/dbclient.h>


MMOScript::~MMOScript(void)
{ /*NULL*/ }

int MMOScript::load_player_script(MapPlayerEx *player)
{
BEGIN_CATCH
    
    BSONObj res = this->conection().findOne(DBScript::COLLECTION, QUERY(DBScript::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, 0);

    ScriptPlayerDetail &detail = player->script_detail();
    detail.__script_id = res[DBScript::SCRIPT_ID].numberInt();
    detail.__script_sort = res[DBScript::SCRIPT_SORT].numberInt();
    detail.__prev_scene = res[DBScript::PREV_SCENE].numberInt();
    detail.__prev_coord.set_pixel(res[DBScript::PREV_PIXEL_X].numberInt(),
    		res[DBScript::PREV_PIXEL_Y].numberInt());
    detail.__prev_blood = res[DBScript::PREV_BLOOD].numberInt();
    detail.__prev_magic = res[DBScript::PREV_MAGIC].numberInt();
    detail.__trvl_total_pass = res[DBScript::TRVL_TOTAL_PASS].numberInt();
    detail.__skill_id = res[DBScript::SKILL_ID].numberInt();

    // type_record
    {
    	BSONObjIterator iter(res.getObjectField(DBScript::TYPE_RECORD.c_str()));
    	while (iter.more())
    	{
    		BSONObj obj = iter.next().embeddedObject();
    		ScriptPlayerDetail::TypeRecord &type_record = detail.__type_map[obj[DBScript::TypeRecord::SCRIPT_TYPE].numberInt()];
    		type_record.reset();
    		type_record.__script_type = obj[DBScript::TypeRecord::SCRIPT_TYPE].numberInt();
    		type_record.__max_script_sort = obj[DBScript::TypeRecord::MAX_SCRIPT_SORT].numberInt();
    		type_record.__pass_wave = obj[DBScript::TypeRecord::PASS_WAVE].numberInt();
    		type_record.__pass_chapter = obj[DBScript::TypeRecord::PASS_CHAPTER].numberInt();
    		type_record.__start_wave = obj[DBScript::TypeRecord::START_WAVE].numberInt();
    		type_record.__start_chapter = obj[DBScript::TypeRecord::START_CHAPTER].numberInt();
    		type_record.__notify_chapter = obj[DBScript::TypeRecord::NOTIFY_CHAPTER].numberInt();
    		type_record.__is_sweep = obj[DBScript::TypeRecord::IS_SWEEP].numberInt();
    		type_record.__used_times_tick.set(obj[DBScript::TypeRecord::USED_TIMES_TICK_SEC].numberInt(),
    				obj[DBScript::TypeRecord::USED_TIMES_TICK_USEC].numberInt());
    	}
    }
    // wave_chapter_info
    {
    	BSONObjIterator iter(res.getObjectField(DBScript::SCRIPT_WAVE_RECORD.c_str()));
    	while (iter.more())
    	{
    		BSONObj obj = iter.next().embeddedObject();
    		ScriptPlayerDetail::ScriptWaveRecord &special_record = detail.__script_wave_map[obj[DBScript::ScriptWaveInfo::SCRIPT_WAVE_ID].numberInt()];
    		special_record.reset();
    		special_record.__script_wave_id = obj[DBScript::ScriptWaveInfo::SCRIPT_WAVE_ID].numberInt();
    		special_record.__is_get = obj[DBScript::ScriptWaveInfo::IS_GET].numberInt();
    	}
    }
    // record
    {
        BSONObjIterator iter(res.getObjectField(DBScript::RECORD.c_str()));
        while (iter.more())
        {
            BSONObj obj = iter.next().embeddedObject();
            ScriptPlayerDetail::ScriptRecord &record = detail.__record_map[obj[DBScript::Record::SCRIPT_SORT].numberInt()];
            record.reset();
            record.__script_sort = obj[DBScript::Record::SCRIPT_SORT].numberInt();
            record.__used_times = obj[DBScript::Record::USED_TIMES].numberInt();
            record.__buy_times = obj[DBScript::Record::BUY_TIMES].numberInt();
            record.__couple_buy_times = obj[DBScript::Record::COUPLE_BUY_TIME].numberInt();
            record.__used_times_tick.set(obj[DBScript::Record::USED_TIMES_TICK_SEC].numberInt(),
                    obj[DBScript::Record::USED_TIMES_TICK_USEC].numberInt());
            record.__enter_script_tick.sec(obj[DBScript::Record::ENTER_SCRIPT_TICK].numberInt());
            record.__progress_id = obj[DBScript::Record::PROGRESS_ID].numberLong();
            record.__best_use_tick = obj[DBScript::Record::BEST_USE_TICK].numberInt();
            record.__is_first_pass = obj[DBScript::Record::IS_FIRST_PASS].numberInt();
            record.__day_pass_times = obj[DBScript::Record::DAY_PASS_TIMES].numberInt();
            record.__is_even_enter = obj[DBScript::Record::IS_EVEN_ENTER].numberInt();
            record.__protect_beast_index = obj[DBScript::Record::PROTECT_BEAST_INDEX].numberInt();
        }
    }
    //问鼎江湖/武林论剑
    {
//    	ScriptPlayerDetail::LegendTopInfo &top_rec = detail.__legend_top_info;
    	BSONObj legend_obj = res.getObjectField(DBScript::LEGEND_TOP_INFO.c_str());
    	BSONObj sword_obj = res.getObjectField(DBScript::SWORD_TOP_INFO.c_str());
    	this->load_top_info(player, GameEnum::SCRIPT_T_LEGEND_TOP, legend_obj);
    	this->load_top_info(player, GameEnum::SCRIPT_T_SWORD_TOP, sword_obj);
//    	{
//    		top_rec.__pass_floor = top_obj[DBScript::LegendTopInfo::PASS_FLOOR].numberInt();
//    		top_rec.__today_rank = top_obj[DBScript::LegendTopInfo::TODAY_RANK].numberInt();
//    		top_rec.__is_sweep = top_obj[DBScript::LegendTopInfo::IS_SWEEP].numberInt();
//
//    		BSONObjIterator floor_iter(top_obj.getObjectField(DBScript::LegendTopInfo::FLOOR_INFO.c_str()));
//    		while (floor_iter.more())
//    		{
//    			BSONObj floor_obj = floor_iter.next().embeddedObject();
//    			int floor_id = floor_obj[DBScript::LegendTopInfo::FloorInfo::FLOOR_ID].numberInt();
//    			ScriptPlayerDetail::LegendTopInfo::FloorInfo &info = top_rec.__piece_map[floor_id];
//    			info.reset();
//    			info.__floor_id = floor_id;
//    			info.__pass_tick = floor_obj[DBScript::LegendTopInfo::FloorInfo::PASS_TICK].numberInt();
//    			info.__totay_pass_flag = floor_obj[DBScript::LegendTopInfo::FloorInfo::TODAY_PASS_FLAG].numberInt();
//    		}
//    	}
    }
    {
        ScriptPlayerDetail::PieceRecord &piece_rec = detail.__piece_record;
        BSONObj piece_obj = res.getObjectField(DBScript::PIECE_RECORD.c_str());
        {
            piece_rec.__pass_piece = piece_obj[DBScript::PieceRecord::PASS_PIECE].numberInt();
            piece_rec.__pass_chapter = piece_obj[DBScript::PieceRecord::PASS_CHAPTER].numberInt();

            piece_rec.__piece_star_award_map.clear();
            BSONObjIterator piece_star_iter(piece_obj.getObjectField(DBScript::PieceRecord::PIECE_STAR_AWARD.c_str()));
            while (piece_star_iter.more())
            {
            	int key = piece_star_iter.next().numberInt(), value = 0;
            	if (piece_star_iter.more())
            		value = piece_star_iter.next().numberInt();
            	piece_rec.__piece_star_award_map[key] = value;
            }

            BSONObjIterator chapter_iter(piece_obj.getObjectField(DBScript::PieceRecord::PIECE_CHAPTER.c_str()));
            while (chapter_iter.more())
            {
                BSONObj chapter_obj = chapter_iter.next().embeddedObject();
                int chapter_key = chapter_obj[DBScript::PieceRecord::PieceChapter::CHAPTER_KEY].numberInt();
                ScriptPlayerDetail::PieceRecord::PieceChapterInfo &info = piece_rec.__pass_chapter_map[chapter_key];
                info.reset();
                info.__chapter_key = chapter_key;
                info.__used_sec = chapter_obj[DBScript::PieceRecord::PieceChapter::USED_SEC].numberInt();
                info.__used_times = chapter_obj[DBScript::PieceRecord::PieceChapter::USED_TIMES].numberInt();
                info.__totay_pass_flag = chapter_obj[DBScript::PieceRecord::PieceChapter::TODAY_PASS_FLAG].numberInt();
            }
        }
    }
    {
    	BSONObjIterator iter(res.getObjectField(DBScript::FIRST_SCRIPT.c_str()));
    	while (iter.more())
    	{
    		detail.__first_script_vc.push_back(iter.next().numberInt());
    	}
    }
    return 0;
    
END_CATCH
    MSG_USER("ERROR load player script %ld", player->role_id());
    return -1;
}

int MMOScript::update_data(MapPlayerEx *player, MongoDataMap *mongo_data)
{
    ScriptPlayerDetail &detail = player->script_detail();

    std::vector<BSONObj> record_vc, chapter_vc, legend_vc, sword_vc, type_vc, special_vc;
    {
    	for (ScriptPlayerDetail::TypeRecordMap::iterator iter = detail.__type_map.begin();
    			iter != detail.__type_map.end(); ++iter)
    	{
    		ScriptPlayerDetail::TypeRecord &type_record = iter->second;
    		type_vc.push_back(BSON(DBScript::TypeRecord::SCRIPT_TYPE << type_record.__script_type
    				<< DBScript::TypeRecord::MAX_SCRIPT_SORT << type_record.__max_script_sort
    				<< DBScript::TypeRecord::PASS_WAVE << type_record.__pass_wave
    				<< DBScript::TypeRecord::PASS_CHAPTER << type_record.__pass_chapter
    				<< DBScript::TypeRecord::START_WAVE << type_record.__start_wave
    				<< DBScript::TypeRecord::START_CHAPTER << type_record.__start_chapter
    				<< DBScript::TypeRecord::NOTIFY_CHAPTER << type_record.__notify_chapter
    				<< DBScript::TypeRecord::USED_TIMES_TICK_SEC << int(type_record.__used_times_tick.sec())
    				<< DBScript::TypeRecord::USED_TIMES_TICK_USEC << int(type_record.__used_times_tick.usec())
    				<< DBScript::TypeRecord::IS_SWEEP << type_record.__is_sweep));
    	}
    }

    {
    	for (ScriptPlayerDetail::ScriptWaveMap::iterator iter = detail.__script_wave_map.begin();
    			iter != detail.__script_wave_map.end(); ++iter)
    	{
    		ScriptPlayerDetail::ScriptWaveRecord &special_recial = iter->second;
    		special_vc.push_back(BSON(DBScript::ScriptWaveInfo::SCRIPT_WAVE_ID << special_recial.__script_wave_id
    				<< DBScript::ScriptWaveInfo::IS_GET << special_recial.__is_get));
    	}
    }

    {
        for (ScriptPlayerDetail::ScriptRecordMap::iterator iter = detail.__record_map.begin();
                iter != detail.__record_map.end(); ++iter)
        {
            ScriptPlayerDetail::ScriptRecord &record = iter->second;
            record_vc.push_back(BSON(DBScript::Record::SCRIPT_SORT << record.__script_sort
                        << DBScript::Record::USED_TIMES << record.__used_times
                        << DBScript::Record::USED_TIMES_TICK_SEC << int(record.__used_times_tick.sec())
                        << DBScript::Record::USED_TIMES_TICK_USEC << int(record.__used_times_tick.usec())
                        << DBScript::Record::ENTER_SCRIPT_TICK << int(record.__enter_script_tick.sec())
                        << DBScript::Record::PROGRESS_ID << record.__progress_id
                        << DBScript::Record::BEST_USE_TICK << record.__best_use_tick
                        << DBScript::Record::IS_FIRST_PASS << record.__is_first_pass
                        << DBScript::Record::BUY_TIMES << record.__buy_times
                        << DBScript::Record::COUPLE_BUY_TIME << record.__couple_buy_times
                        << DBScript::Record::DAY_PASS_TIMES << record.__day_pass_times
                        << DBScript::Record::IS_EVEN_ENTER << record.__is_even_enter
                        << DBScript::Record::PROTECT_BEAST_INDEX << record.__protect_beast_index));
        }
    }
    ScriptPlayerDetail::PieceRecord &piece_rec = detail.__piece_record;
    {
        for (ScriptPlayerDetail::PieceRecord::PassChapterMap::iterator iter = piece_rec.__pass_chapter_map.begin();
                iter != piece_rec.__pass_chapter_map.end(); ++iter)
        {
            ScriptPlayerDetail::PieceRecord::PieceChapterInfo &chapter_info = iter->second;
            chapter_vc.push_back(BSON(DBScript::PieceRecord::PieceChapter::CHAPTER_KEY << chapter_info.__chapter_key
                        << DBScript::PieceRecord::PieceChapter::USED_SEC << chapter_info.__used_sec
                        << DBScript::PieceRecord::PieceChapter::USED_TIMES << chapter_info.__used_times
                        << DBScript::PieceRecord::PieceChapter::TODAY_PASS_FLAG << chapter_info.__totay_pass_flag));
        }
    }
    ScriptPlayerDetail::LegendTopInfo &legend_rec = detail.__legend_top_info;
    {
    	for (ScriptPlayerDetail::LegendTopInfo::FloorMap::iterator iter = legend_rec.__piece_map.begin();
    			iter != legend_rec.__piece_map.end(); ++iter)
    	{
    		ScriptPlayerDetail::LegendTopInfo::FloorInfo &floor_info = iter->second;
    		legend_vc.push_back(BSON(DBScript::LegendTopInfo::FloorInfo::FLOOR_ID << floor_info.__floor_id
    				<< DBScript::LegendTopInfo::FloorInfo::PASS_TICK << floor_info.__pass_tick
    				<< DBScript::LegendTopInfo::FloorInfo::TODAY_PASS_FLAG << floor_info.__totay_pass_flag));
    	}
    }

    ScriptPlayerDetail::LegendTopInfo &sword_rec = detail.__sword_top_info;
    {
        for (ScriptPlayerDetail::LegendTopInfo::FloorMap::iterator iter = sword_rec.__piece_map.begin();
        		iter != sword_rec.__piece_map.end(); ++iter)
        {
        	ScriptPlayerDetail::LegendTopInfo::FloorInfo &floor_info = iter->second;
        	sword_vc.push_back(BSON(DBScript::LegendTopInfo::FloorInfo::FLOOR_ID << floor_info.__floor_id
        			<< DBScript::LegendTopInfo::FloorInfo::PASS_TICK << floor_info.__pass_tick
        			<< DBScript::LegendTopInfo::FloorInfo::TODAY_PASS_FLAG << floor_info.__totay_pass_flag));
        }
    }

    IntVec piece_star_vc;
    {
    	for (IntMap::iterator iter = piece_rec.__piece_star_award_map.begin();
    			iter != piece_rec.__piece_star_award_map.end(); ++iter)
    	{
    		piece_star_vc.push_back(iter->first);
    		piece_star_vc.push_back(iter->second);
    	}
    }

    BSONObjBuilder builder;
    builder << DBScript::SCRIPT_ID << detail.__script_id
        << DBScript::SCRIPT_SORT << detail.__script_sort
        << DBScript::PREV_SCENE << detail.__prev_scene
        << DBScript::PREV_PIXEL_X << detail.__prev_coord.pixel_x()
        << DBScript::PREV_PIXEL_Y << detail.__prev_coord.pixel_y()
        << DBScript::PREV_BLOOD << detail.__prev_blood
        << DBScript::PREV_MAGIC << detail.__prev_magic
        << DBScript::TRVL_TOTAL_PASS << detail.__trvl_total_pass
        << DBScript::FIRST_SCRIPT << detail.__first_script_vc
        << DBScript::SKILL_ID << detail.__skill_id
        << DBScript::RECORD << record_vc
        << DBScript::TYPE_RECORD << type_vc
        << DBScript::SCRIPT_WAVE_RECORD << special_vc
        << DBScript::PIECE_RECORD << BSON(DBScript::PieceRecord::PASS_PIECE << piece_rec.__pass_piece
                << DBScript::PieceRecord::PASS_CHAPTER << piece_rec.__pass_chapter
                << DBScript::PieceRecord::PIECE_STAR_AWARD << piece_star_vc
                << DBScript::PieceRecord::PIECE_CHAPTER << chapter_vc)
        << DBScript::LEGEND_TOP_INFO << BSON(DBScript::LegendTopInfo::PASS_FLOOR << legend_rec.__pass_floor
        		<< DBScript::LegendTopInfo::TODAY_RANK << legend_rec.__today_rank
        		<< DBScript::LegendTopInfo::IS_SWEEP << legend_rec.__is_sweep
        		<< DBScript::LegendTopInfo::FLOOR_INFO << legend_vc)
        << DBScript::SWORD_TOP_INFO << BSON(DBScript::LegendTopInfo::PASS_FLOOR << sword_rec.__pass_floor
        		<< DBScript::LegendTopInfo::TODAY_RANK << sword_rec.__today_rank
        		<< DBScript::LegendTopInfo::IS_SWEEP << sword_rec.__is_sweep
        		<< DBScript::LegendTopInfo::FLOOR_INFO << sword_vc);

    mongo_data->push_update(DBScript::COLLECTION,
            BSON(DBScript::ID << player->role_id()),
            builder.obj(), true);

    return 0;
}

void MMOScript::load_top_info(MapPlayerEx *player, int script_type, BSONObj &obj)
{
	ScriptPlayerDetail::LegendTopInfo &top_rec = player->top_info(script_type);
	top_rec.__pass_floor = obj[DBScript::LegendTopInfo::PASS_FLOOR].numberInt();
	top_rec.__today_rank = obj[DBScript::LegendTopInfo::TODAY_RANK].numberInt();
	top_rec.__is_sweep = obj[DBScript::LegendTopInfo::IS_SWEEP].numberInt();

	BSONObjIterator floor_iter(obj.getObjectField(DBScript::LegendTopInfo::FLOOR_INFO.c_str()));
	while (floor_iter.more())
	{
		BSONObj floor_obj = floor_iter.next().embeddedObject();
		int floor_id = floor_obj[DBScript::LegendTopInfo::FloorInfo::FLOOR_ID].numberInt();
		ScriptPlayerDetail::LegendTopInfo::FloorInfo &info = top_rec.__piece_map[floor_id];
		info.reset();
		info.__floor_id = floor_id;
		info.__pass_tick = floor_obj[DBScript::LegendTopInfo::FloorInfo::PASS_TICK].numberInt();
		info.__totay_pass_flag = floor_obj[DBScript::LegendTopInfo::FloorInfo::TODAY_PASS_FLAG].numberInt();
	}
}

void MMOScript::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(DBScript::COLLECTION, BSON(DBScript::ID << 1), true);
END_CATCH
}

