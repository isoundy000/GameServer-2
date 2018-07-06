/*
 * MMOVip.cpp
 *
 *  Created on: 2013-12-27
 *      Author: root
 */

#include "GameField.h"
#include "MMOVip.h"
#include "MongoConnector.h"
#include "MapLogicPlayer.h"
#include "MapPlayerEx.h"
#include "LogicPlayer.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOVip::MMOVip()
{
	// TODO Auto-generated constructor stub

}

int MMOVip::load_player_vip(MapLogicPlayer *player)
{
	BSONObj res = this->conection().findOne(Vip::COLLECTION,
			QUERY(Vip::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	MLVipDetail& vip_detail = player->vip_detail();
	MMOVip::load_base_vip(vip_detail, res);

	vip_detail.__check_flag = res[Vip::CHECK_FLAG].numberInt();
    vip_detail.__weekly_tick.sec(res[Vip::WEEKLY_TICK].numberLong());
    vip_detail.__super_get_type = res[Vip::SUPER_VIP_TYPE].numberInt();

    GameCommon::bson_to_map(vip_detail.__is_given,
    			res.getObjectField(Vip::IS_GIVEN.c_str()));

    GameCommon::bson_to_map(vip_detail.__weekly_given,
    			res.getObjectField(Vip::WEEKLY_GIVEN.c_str()));

	return 0;
}

int MMOVip::load_player_vip(MapPlayerEx* player)
{
	BSONObj res = this->conection().findOne(Vip::COLLECTION,
			QUERY(Vip::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	return MMOVip::load_base_vip(player->vip_detail(), res);
}

int MMOVip::load_player_vip(LogicPlayer* player)
{
	BSONObj res = this->conection().findOne(Vip::COLLECTION,
			QUERY(Vip::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	return MMOVip::load_base_vip(player->vip_detail(), res);
}

int MMOVip::load_base_vip(BaseVipDetail& base_vip, const BSONObj& res)
{
	base_vip.set_vip_type(res[Vip::TYPE].numberInt());
	base_vip.__start_time = res[Vip::START_TIME].numberLong();
	base_vip.__expired_time = res[Vip::EXPIRED_TIME].numberLong();
	return 0;
}

int MMOVip::update_data(MapLogicPlayer *player, MongoDataMap *mongo_data)
{
	MLVipDetail &vip_detail = player->vip_detail();

    BSONVec bson_set;
    GameCommon::map_to_bson(bson_set, vip_detail.__is_given);

    BSONVec weekly_bson_set;
    GameCommon::map_to_bson(weekly_bson_set, vip_detail.__weekly_given);

	BSONObjBuilder builder;
	builder	<< Vip::TYPE << vip_detail.__vip_type
			<< Vip::EXPIRED_TIME << vip_detail.__expired_time
			<< Vip::START_TIME << vip_detail.__start_time
			<< Vip::CHECK_FLAG << vip_detail.__check_flag
			<< Vip::IS_GIVEN << bson_set
			<< Vip::WEEKLY_GIVEN << weekly_bson_set
			<< Vip::WEEKLY_TICK << (Int64)vip_detail.__weekly_tick.sec()
			<< Vip::SUPER_VIP_TYPE << vip_detail.__super_get_type;

	mongo_data->push_update(Vip::COLLECTION,
			BSON(Vip::ID << player->role_id()), builder.obj(), true);

	return 0;
}

void MMOVip::ensure_all_index(void)
{
	this->conection().ensureIndex(Vip::COLLECTION, BSON(Vip::ID << 1), true);
}

MMOExpRestore::MMOExpRestore()
{
	// TODO Auto-generated constructor stub
}

int MMOExpRestore::load_player_exp_restore(MapLogicPlayer *player)
{
BEGIN_CATCH
	StorageRecordSet &storage_record_set = player->storage_record_set();
	ExpRestoreDetail &er_detail = player->exp_restore_detail();
	LongMap &stamp_level_map = player->stamp_level_map();
	LongMap &stamp_vip_map = player->stamp_vip_map();
	StorageStageInfo &storage_stage_info = er_detail.__storage_stage_info;

	 BSONObj res = this->conection().findOne(DBExpRestore::COLLECTION,
			 QUERY(DBExpRestore::ID << player->role_id()));

	 JUDGE_RETURN(res.isEmpty() == false, 0);

	er_detail.__check_timestamp.set(res[DBExpRestore::CHECK_TIMESTAMP].numberInt(), 0);
	er_detail.__vip_type_record = res[DBExpRestore::VIP_TYPE_REC].numberInt();
	er_detail.__vip_start_time = res[DBExpRestore::VIP_START_TIME].numberInt();
	er_detail.__vip_expried_time = res[DBExpRestore::VIP_EXPRIED_TIME].numberInt();

	BSONObjIterator pre_iter = res.getObjectField(DBExpRestore::PRE_ACT_MAP.c_str());
	while(pre_iter.more())
	{
		BSONObj obj = pre_iter.next().embeddedObject();
		int id = obj[DBExpRestore::PreActMap::ACT_ID].numberInt();
		int ext_type = obj[DBExpRestore::PreActMap::EXT_TYPE].numberInt();
		int times = obj[DBExpRestore::PreActMap::TIMES].numberInt();
		er_detail.__pre_act_map[id].__ext_type = ext_type;
		er_detail.__pre_act_map[id].__times = times;
	}

	BSONObjIterator now_iter = res.getObjectField(DBExpRestore::NOW_ACT_MAP.c_str());
	while(now_iter.more())
	{
		BSONObj obj = now_iter.next().embeddedObject();
		int id = obj[DBExpRestore::NowActMap::ACT_ID].numberInt();
		int ext_type = obj[DBExpRestore::NowActMap::EXT_TYPE].numberInt();
		int times = obj[DBExpRestore::NowActMap::TIMES].numberInt();
		er_detail.__now_act_map[id].__ext_type = ext_type;
		er_detail.__now_act_map[id].__times = times;
	}

	BSONObjIterator lvl_iter = res.getObjectField(DBExpRestore::LEVEL_RECORD.c_str());

	while (lvl_iter.more())
	{
		BSONObj obj = lvl_iter.next().embeddedObject();

		int64_t timestamp_sec = obj[DBExpRestore::LevelRecord::REC_TIMESTAMP].numberLong();
		int lvl = obj[DBExpRestore::LevelRecord::LEVEL].numberInt();

		stamp_level_map[timestamp_sec] = lvl;
	}

	BSONObjIterator vip_iter = res.getObjectField(DBExpRestore::VIP_RECORD.c_str());

	while (vip_iter.more())
	{
		BSONObj obj = vip_iter.next().embeddedObject();

		int64_t timestamp_sec = obj[DBExpRestore::VipRecord::REC_TIMESTAMP].numberLong();
		int vip_type = obj[DBExpRestore::VipRecord::VIP_TYPE].numberInt();

		stamp_vip_map[timestamp_sec] = vip_type;
	}

	BSONObjIterator record_iter = res.getObjectField(DBExpRestore::STORAGE_RECORD.c_str());
	while(record_iter.more())
	{
		BSONObj record_obj = record_iter.next().embeddedObject();

		StorageRecord record;
		record.__storage_id = record_obj[DBExpRestore::ActivityExpRestore::STORAGE_ID].numberInt();
		record.__record_timestamp.set(record_obj[DBExpRestore::ActivityExpRestore::REC_TIMESTAMP].numberInt(), 0);
		record.__finish_count = record_obj[DBExpRestore::ActivityExpRestore::FINISH_COUNT].numberInt();
		record.__storage_valid = record_obj[DBExpRestore::ActivityExpRestore::STORAGE_VALID].trueValue();
		storage_record_set.push_back(record);
	}

	BSONObjIterator stage_info_iter = res.getObjectField(DBExpRestore::STORAGE_STAGE_INFO.c_str());
	while(stage_info_iter.more())
	{
		BSONObj timestamp_stage = stage_info_iter.next().embeddedObject();

		int stage_id = timestamp_stage[DBExpRestore::StorageStageInfo::STORAGE_ID].numberInt();

		TimestampStageMap &timestamp_stage_map = storage_stage_info[stage_id];
		BSONObjIterator timestamp_stage_iter = timestamp_stage.getObjectField(DBExpRestore::StorageStageInfo::TIMESTAMP_STAGE.c_str());
		while(timestamp_stage_iter.more())
		{
			BSONObj stage_rec = timestamp_stage_iter.next().embeddedObject();
			int64_t time_sec = stage_rec[DBExpRestore::StorageStageInfo::TimestampStage::REC_TIMESTAMP].numberLong();
			int stage = stage_rec[DBExpRestore::StorageStageInfo::TimestampStage::STAGE].numberInt();

			timestamp_stage_map[time_sec] = stage;
		}
	}

END_CATCH
    return -1;
}

int MMOExpRestore::update_data(MapLogicPlayer *player, MongoDataMap *mongo_data)
{
	StorageRecordSet &storage_record_set = player->storage_record_set();
	ExpRestoreDetail &er_detail = player->exp_restore_detail();
	LongMap &stamp_level_map = player->stamp_level_map();
	LongMap &stamp_vip_map = player->stamp_vip_map();
	StorageStageInfo &storage_stage_info = er_detail.__storage_stage_info;

	Time_Value timestamp = next_day(0, 0, Time_Value::gettimeofday());
	int max_day = player->exp_restore_max_day();

	BSONVec bson_set_pre;
	bson_set_pre.reserve(er_detail.__pre_act_map.size());
	for(RestoreMap::iterator iter = er_detail.__pre_act_map.begin(); iter != er_detail.__pre_act_map.end(); ++iter)
	{
		bson_set_pre.push_back(BSON(DBExpRestore::PreActMap::ACT_ID << iter->first
				<< DBExpRestore::PreActMap::EXT_TYPE << iter->second.__ext_type
				<< DBExpRestore::PreActMap::TIMES << iter->second.__times));
	}

	BSONVec bson_set_now;
	bson_set_now.reserve(er_detail.__now_act_map.size());
	for(RestoreMap::iterator iter = er_detail.__now_act_map.begin(); iter != er_detail.__now_act_map.end(); ++iter)
	{
		bson_set_now.push_back(BSON(DBExpRestore::NowActMap::ACT_ID << iter->first
				<< DBExpRestore::NowActMap::EXT_TYPE << iter->second.__ext_type
				<< DBExpRestore::NowActMap::TIMES << iter->second.__times));
	}

	BSONVec bson_set;
	bson_set.reserve(storage_record_set.size());
	for(StorageRecordSet::iterator iter = storage_record_set.begin(); iter != storage_record_set.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->__record_timestamp.sec() >= timestamp.sec() - Time_Value::DAY * max_day);

		bson_set.push_back(BSON(DBExpRestore::ActivityExpRestore::STORAGE_ID << iter->__storage_id
				<< DBExpRestore::ActivityExpRestore::REC_TIMESTAMP << (long long int)(iter->__record_timestamp.sec())
				<< DBExpRestore::ActivityExpRestore::FINISH_COUNT << iter->__finish_count
				<< DBExpRestore::ActivityExpRestore::STORAGE_VALID << iter->__storage_valid));
	}

	// 等级信息
	BSONVec bson_set_level;
	bson_set_level.reserve(stamp_level_map.size());
	for(LongMap::iterator iter = stamp_level_map.begin(); iter != stamp_level_map.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->first >= timestamp.sec() - Time_Value::DAY * max_day);
		JUDGE_CONTINUE(iter->first <= timestamp.sec());
		bson_set_level.push_back(BSON(DBExpRestore::LevelRecord::REC_TIMESTAMP << (long long int)(iter->first)
				<< DBExpRestore::LevelRecord::LEVEL << iter->second));
	}

	// vip变化记录
	BSONVec bson_set_vip;
	bson_set_vip.reserve(stamp_vip_map.size());
	for(LongMap::iterator iter = stamp_vip_map.begin(); iter != stamp_vip_map.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->first >= timestamp.sec() - Time_Value::DAY * max_day);
		JUDGE_CONTINUE(iter->first <= timestamp.sec());
		bson_set_vip.push_back(BSON( DBExpRestore::VipRecord::REC_TIMESTAMP << (long long int)(iter->first)
				<< DBExpRestore::VipRecord::VIP_TYPE << iter->second));
	}

	BSONVec bson_set_stage;
	bson_set_stage.reserve(storage_stage_info.size());
	for(StorageStageInfo::iterator iter_i = storage_stage_info.begin(); iter_i != storage_stage_info.end(); ++iter_i)
	{
		TimestampStageMap &timestamp_stage_map = iter_i->second;
		int storage_id = iter_i->first;

		BSONVec bson_set_n;
		for(TimestampStageMap::iterator iter_n = timestamp_stage_map.begin(); iter_n != timestamp_stage_map.end(); ++iter_n)
		{
			JUDGE_CONTINUE(iter_n->first <= timestamp.sec());

			TimestampStageMap::iterator iter_n_next = iter_n;
			++iter_n_next;

			if(iter_n_next != timestamp_stage_map.end() && iter_n_next->second >= iter_n->second && iter_n_next->first > iter_n->first)
			{// 记录使用的时间戳是当日0时,所以max_day+1
				int64_t data_expired_time =  timestamp.sec() - Time_Value::DAY * (max_day + 1);
				JUDGE_CONTINUE(iter_n->first >= data_expired_time && iter_n_next->first >= data_expired_time);
			}

			bson_set_n.push_back(BSON(DBExpRestore::StorageStageInfo::TimestampStage::REC_TIMESTAMP << (long long int) iter_n->first
					<< DBExpRestore::StorageStageInfo::TimestampStage::STAGE << iter_n->second));
		}

		bson_set_stage.push_back(BSON(DBExpRestore::StorageStageInfo::STORAGE_ID << storage_id
				<< DBExpRestore::StorageStageInfo::TIMESTAMP_STAGE << bson_set_n));
	}

	BSONObjBuilder builder;
	builder << DBExpRestore::CHECK_TIMESTAMP << (long long int)er_detail.__check_timestamp.sec()
			<< DBExpRestore::VIP_TYPE_REC << er_detail.__vip_type_record
			<< DBExpRestore::VIP_START_TIME << er_detail.__vip_start_time
			<< DBExpRestore::VIP_EXPRIED_TIME << er_detail.__vip_expried_time
			<< DBExpRestore::STORAGE_RECORD << bson_set
			<< DBExpRestore::PRE_ACT_MAP << bson_set_pre
			<< DBExpRestore::NOW_ACT_MAP << bson_set_now
			<< DBExpRestore::LEVEL_RECORD << bson_set_level
			<< DBExpRestore::VIP_RECORD << bson_set_vip
			<< DBExpRestore::STORAGE_STAGE_INFO << bson_set_stage;

	mongo_data->push_update(DBExpRestore::COLLECTION, BSON(DBExpRestore::ID
			<< (long long int)player->role_id()), builder.obj(), true);

	return 0;
}

void MMOExpRestore::ensure_all_index(void)
{
	this->conection().ensureIndex(DBExpRestore::COLLECTION, BSON(DBExpRestore::ID << 1), true);
}



