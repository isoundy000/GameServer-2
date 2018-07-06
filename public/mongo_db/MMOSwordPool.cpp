/*
 * MMOSwordPool.cpp
 *
 *  Created on: 2016年10月12日
 *      Author: lyw
 */

#include "GameField.h"
#include "MMOSwordPool.h"
#include "LogicPlayer.h"
#include "MapLogicPlayer.h"
#include "MongoDataMap.h"
#include "MongoConnector.h"
#include "DBCommon.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOSwordPool::MMOSwordPool() {
	// TODO Auto-generated constructor stub

}

MMOSwordPool::~MMOSwordPool() {
	// TODO Auto-generated destructor stub
}

void MMOSwordPool::ensure_all_index(void)
{
	BEGIN_CATCH
	this->conection().ensureIndex(DBSwordPool::COLLECTION, BSON(DBSwordPool::ID << 1), true);
	END_CATCH
}

int MMOSwordPool::load_player_spool(MapLogicPlayer *player)
{
BEGIN_CATCH
	BSONObj res = this->conection().findOne(DBSwordPool::COLLECTION, QUERY(DBSwordPool::ID << player->role_id()));
    if (res.isEmpty())
        return 0;

    SwordPoolDetail &spool_detail = player->sword_pool_detail();
    spool_detail.level_ = res[DBSwordPool::LEVEL].numberInt();
    spool_detail.exp_ = res[DBSwordPool::EXP].numberInt();
    spool_detail.open_ = res[DBSwordPool::OPEN].numberInt();
    spool_detail.stype_lv_ = res[DBSwordPool::STYLE_LV].numberInt();

    Int64 refresh_tick = res[DBSwordPool::REFRESH_TICK].numberLong();
    spool_detail.refresh_tick_ = Time_Value(refresh_tick);

    BSONObjIterator iter(res.getObjectField(DBSwordPool::LAST_POOL_TASK_INFO.c_str()));
    while (iter.more())
    {
    	BSONObj lpool_obj = iter.next().embeddedObject();
    	int task_id = lpool_obj[DBSwordPool::PoolTaskInfo::TASK_ID].numberInt();

    	// 判断配置是否还有该任务
    	if (CONFIG_INSTANCE->sword_pool_task(task_id) == Json::Value::null)
    		continue;

    	SwordPoolDetail::PoolTaskInfo &info = spool_detail.last_task_map_[task_id];
    	info.task_id_ = task_id;
    	info.total_num_ = lpool_obj[DBSwordPool::PoolTaskInfo::TOTAL_NUM].numberInt();
    	info.left_num_ = lpool_obj[DBSwordPool::PoolTaskInfo::LEFT_NUM].numberInt();
    }

    BSONObjIterator it(res.getObjectField(DBSwordPool::TODAY_POOL_TASK_INFO.c_str()));
	while (it.more())
	{
		BSONObj tpool_obj = it.next().embeddedObject();
		int task_id = tpool_obj[DBSwordPool::PoolTaskInfo::TASK_ID].numberInt();

		// 判断配置是否还有该任务
		if (CONFIG_INSTANCE->sword_pool_task(task_id) == Json::Value::null)
		    continue;

		SwordPoolDetail::PoolTaskInfo &info = spool_detail.today_task_map_[task_id];
		info.task_id_ = task_id;
		info.total_num_ = tpool_obj[DBSwordPool::PoolTaskInfo::TOTAL_NUM].numberInt();
		info.left_num_ = tpool_obj[DBSwordPool::PoolTaskInfo::LEFT_NUM].numberInt();
	}
	return 0;
END_CATCH
    return -1;
}

int MMOSwordPool::update_data(MapLogicPlayer *player, MongoDataMap *mongo_data)
{
	std::vector<BSONObj> lpool_vc;
	std::vector<BSONObj> tpool_vc;
	SwordPoolDetail &spool_detail = player->sword_pool_detail();
	for (SwordPoolDetail::LastTaskInfoMap::iterator iter = spool_detail.last_task_map_.begin();
			iter != spool_detail.last_task_map_.end(); ++iter)
	{
		SwordPoolDetail::PoolTaskInfo &info = iter->second;
		lpool_vc.push_back(BSON(DBSwordPool::PoolTaskInfo::TASK_ID << info.task_id_
				<< DBSwordPool::PoolTaskInfo::TOTAL_NUM << info.total_num_
		        << DBSwordPool::PoolTaskInfo::LEFT_NUM << info.left_num_));
	}
	for (SwordPoolDetail::TodayTaskInfoMap::iterator iter = spool_detail.today_task_map_.begin();
			iter != spool_detail.today_task_map_.end(); ++iter)
	{
		SwordPoolDetail::PoolTaskInfo &info = iter->second;
		tpool_vc.push_back(BSON(DBSwordPool::PoolTaskInfo::TASK_ID << info.task_id_
				<< DBSwordPool::PoolTaskInfo::TOTAL_NUM << info.total_num_
				<< DBSwordPool::PoolTaskInfo::LEFT_NUM << info.left_num_));
	}

	Int64 refresh_tick = spool_detail.refresh_tick_.sec();

	BSONObjBuilder builder;
	builder << DBSwordPool::LEVEL << spool_detail.level_
	        << DBSwordPool::EXP << spool_detail.exp_
	        << DBSwordPool::OPEN << spool_detail.open_
	        << DBSwordPool::STYLE_LV << spool_detail.stype_lv_
	        << DBSwordPool::REFRESH_TICK << refresh_tick
	        << DBSwordPool::LAST_POOL_TASK_INFO << lpool_vc
	        << DBSwordPool::TODAY_POOL_TASK_INFO << tpool_vc;
	mongo_data->push_update(DBSwordPool::COLLECTION,
			BSON(DBSwordPool::ID << player->role_id()),builder.obj(), true);

	return 0;
}


MMOHiddenTreasure::MMOHiddenTreasure() {
	// TODO Auto-generated constructor stub

}

MMOHiddenTreasure::~MMOHiddenTreasure() {
	// TODO Auto-generated destructor stub
}

void MMOHiddenTreasure::ensure_all_index(void)
{
BEGIN_CATCH
	this->conection().ensureIndex(DBHiddenTreasure::COLLECTION, BSON(DBHiddenTreasure::ID << 1), true);
END_CATCH
}

int MMOHiddenTreasure::load_player_hi_treasure(MapLogicPlayer *player)
{
BEGIN_CATCH
	BSONObj res = this->conection().findOne(DBHiddenTreasure::COLLECTION,
			QUERY(DBHiddenTreasure::ID << player->role_id()));
    if (res.isEmpty())
        return 0;

    HiddenTreasureDetail& hi_treasure = player->hi_treasure_detail();
    hi_treasure.day_ = res[DBHiddenTreasure::DAY].numberInt();
    hi_treasure.open_ = res[DBHiddenTreasure::OPEN].numberInt();
    hi_treasure.get_status_ = res[DBHiddenTreasure::GET_STATUS].numberInt();
    hi_treasure.refresh_tick_ = res[DBHiddenTreasure::REFRESH_TICK].numberLong();

    BSONObjIterator iter(res.getObjectField(DBHiddenTreasure::BUY_MAP.c_str()));
    while (iter.more())
    {
    	BSONObj obj = iter.next().embeddedObject();
    	int obj_key = obj[DBPairObj::KEY].numberInt();
    	int obj_value = obj[DBPairObj::VALUE].numberInt();

    	JUDGE_CONTINUE(obj_key != 0);
    	hi_treasure.buy_map_[obj_key] = obj_value;
    }

	return 0;
END_CATCH
	return -1;
}

int MMOHiddenTreasure::update_data(MapLogicPlayer *player, MongoDataMap *mongo_data)
{
	BSONVec buy_set;
	HiddenTreasureDetail& hi_treasure = player->hi_treasure_detail();
	for (IntMap::iterator iter = hi_treasure.buy_map_.begin();
			iter != hi_treasure.buy_map_.end(); ++iter)
	{
		buy_set.push_back(BSON(DBPairObj::KEY << iter->first
				<< DBPairObj::VALUE << iter->second));
	}

	BSONObjBuilder build;
	build << DBHiddenTreasure::DAY << hi_treasure.day_
		  << DBHiddenTreasure::OPEN << hi_treasure.open_
		  << DBHiddenTreasure::GET_STATUS << hi_treasure.get_status_
		  << DBHiddenTreasure::REFRESH_TICK << hi_treasure.refresh_tick_
		  << DBHiddenTreasure::BUY_MAP << buy_set;
	mongo_data->push_update(DBHiddenTreasure::COLLECTION, BSON(DBHiddenTreasure::ID << player->role_id()),
			build.obj(), true);

	return 0;
}

MMOFashion::MMOFashion() {
	// TODO Auto-generated constructor stub

}

MMOFashion::~MMOFashion() {
	// TODO Auto-generated destructor stub
}

void MMOFashion::ensure_all_index(void)
{
BEGIN_CATCH
	this->conection().ensureIndex(DBFashion::COLLECTION, BSON(DBFashion::ID << 1), true);
END_CATCH
}

int MMOFashion::load_player_fashion(MapLogicPlayer *player)
{
BEGIN_CATCH
	BSONObj res = this->conection().findOne(DBFashion::COLLECTION,
			QUERY(DBFashion::ID << player->role_id()));
    if (res.isEmpty())
        return 0;

    RoleFashion &fashion_detail = player->fashion_detail();
    fashion_detail.level_ = res[DBFashion::LEVEL].numberInt();
    fashion_detail.exp_ = res[DBFashion::EXP].numberInt();
    fashion_detail.select_id_ = res[DBFashion::SELECT_ID].numberInt();
    fashion_detail.sel_color_id_ = res[DBFashion::SEL_COLOR_ID].numberInt();
    fashion_detail.open_ = res[DBFashion::OPEN].numberInt();

    DBCommon::bson_to_int_vec(fashion_detail.send_set_,
    		res.getObjectField(DBFashion::SEND_SET.c_str()));

    BSONObjIterator iter(res.getObjectField(DBFashion::FASHION_SET.c_str()));
    while (iter.more())
    {
    	BSONObj obj = iter.next().embeddedObject();
    	int fashion_id = obj[DBFashion::FashionInfo::FASHION_ID].numberInt();
    	JUDGE_CONTINUE(CONFIG_INSTANCE->fashion(fashion_id) != Json::Value::null);

    	RoleFashion::FashionInfo &fashion_info = fashion_detail.fashion_map_[fashion_id];
    	fashion_info.fashion_id_ = fashion_id;
    	fashion_info.color_id_ = obj[DBFashion::FashionInfo::COLOR_ID].numberInt();
    	fashion_info.active_type_ = obj[DBFashion::FashionInfo::ACTIVE_TYPE].numberInt();
    	fashion_info.is_permanent_ = obj[DBFashion::FashionInfo::IS_PERMANENT].numberInt();
    	fashion_info.active_tick_ = obj[DBFashion::FashionInfo::ACTIVE_TICK].numberLong();
    	fashion_info.end_tick_ = obj[DBFashion::FashionInfo::END_TICK].numberLong();

    	DBCommon::bson_to_int_vec(fashion_info.color_set_,
    			obj.getObjectField(DBFashion::FashionInfo::COLOR_SET.c_str()));
    }

	return 0;
END_CATCH
	return -1;
}

int MMOFashion::update_data(MapLogicPlayer *player, MongoDataMap *mongo_data)
{
	BSONVec fashion_vec;
	RoleFashion &fashion_detail = player->fashion_detail();
	for (RoleFashion::FashionInfoMap::iterator iter = fashion_detail.fashion_map_.begin();
			iter != fashion_detail.fashion_map_.end(); ++iter)
	{
		RoleFashion::FashionInfo &fashion_info = iter->second;
		fashion_vec.push_back(BSON(DBFashion::FashionInfo::FASHION_ID << fashion_info.fashion_id_
				<< DBFashion::FashionInfo::COLOR_ID << fashion_info.color_id_
				<< DBFashion::FashionInfo::ACTIVE_TYPE << fashion_info.active_type_
				<< DBFashion::FashionInfo::IS_PERMANENT << fashion_info.is_permanent_
				<< DBFashion::FashionInfo::ACTIVE_TICK << fashion_info.active_tick_
				<< DBFashion::FashionInfo::END_TICK << fashion_info.end_tick_
				<< DBFashion::FashionInfo::COLOR_SET << fashion_info.color_set_));
	}

	BSONObjBuilder build;
	build << DBFashion::LEVEL << fashion_detail.level_
		  << DBFashion::EXP << fashion_detail.exp_
		  << DBFashion::OPEN << fashion_detail.open_
		  << DBFashion::SELECT_ID << fashion_detail.select_id_
		  << DBFashion::SEL_COLOR_ID << fashion_detail.sel_color_id_
		  << DBFashion::SEND_SET << fashion_detail.send_set_
		  << DBFashion::FASHION_SET << fashion_vec;
	mongo_data->push_update(DBFashion::COLLECTION, BSON(DBFashion::ID << player->role_id()),
			build.obj(), true);

	return 0;
}

void MMOFashion::fetch_player_fashion(LeagueMember &member)
{
	BSONObj res = CACHED_CONNECTION.findOne(DBFashion::COLLECTION,
			QUERY(DBFashion::ID << member.role_index_));
	if (res.isEmpty())
		return ;

	member.fashion_id_ = res[DBFashion::SELECT_ID].numberInt();
	member.fashion_color_ = res[DBFashion::SEL_COLOR_ID].numberInt();
}

MMOTransfer::MMOTransfer() {
	// TODO Auto-generated constructor stub

}

MMOTransfer::~MMOTransfer() {
	// TODO Auto-generated destructor stub
}

void MMOTransfer::ensure_all_index(void)
{
BEGIN_CATCH
	this->conection().ensureIndex(DBTransfer::COLLECTION, BSON(DBTransfer::ID << 1), true);
END_CATCH
}

int MMOTransfer::load_player_transfer(MapLogicPlayer *player)
{
BEGIN_CATCH
	BSONObj res = this->conection().findOne(DBTransfer::COLLECTION,
			QUERY(DBTransfer::ID << player->role_id()));
    if (res.isEmpty())
        return 0;

    TransferDetail &transfer_detail = player->transfer_detail();
    transfer_detail.level_ 	= res[DBTransfer::LEVEL].numberInt();
    transfer_detail.exp_ 	= res[DBTransfer::EXP].numberInt();
    transfer_detail.open_ 	= res[DBTransfer::OPEN].numberInt();
    transfer_detail.stage_ 	= res[DBTransfer::STAGE].numberInt();
    transfer_detail.last_ 	= res[DBTransfer::LAST].numberInt();
    transfer_detail.transfer_tick_ = res[DBTransfer::TRANSFER_TICK].numberLong();
    transfer_detail.active_id_ 	= res[DBTransfer::ACTIVE_ID].numberInt();
    transfer_detail.open_reward_= res[DBTransfer::OPEN_REWARD].numberInt();
    transfer_detail.gold_times_ = res[DBTransfer::GOLD_TIMES].numberInt();

    Int64 refresh_tick = res[DBTransfer::REFRESH_TICK].numberLong();
    transfer_detail.refresh_tick_ = Time_Value(refresh_tick);

    BSONObjIterator iter(res.getObjectField(DBTransfer::TRANSFER_SET.c_str()));
    while (iter.more())
    {
    	BSONObj obj = iter.next().embeddedObject();
    	int transfer_id = obj[DBTransfer::TransferSet::TRANSFER_ID].numberInt();
    	JUDGE_CONTINUE(CONFIG_INSTANCE->transfer_total(transfer_id) != Json::Value::null);

    	TransferDetail::TransferInfo &info = transfer_detail.transfer_map_[transfer_id];
    	info.transfer_id_ = transfer_id;
    	info.transfer_lv_ = obj[DBTransfer::TransferSet::TRANSFER_LV].numberInt();
    	info.is_permanent_= obj[DBTransfer::TransferSet::IS_PERMANENT].numberInt();
    	info.is_active_   = obj[DBTransfer::TransferSet::IS_ACTIVE].numberInt();
    	info.active_tick_ = obj[DBTransfer::TransferSet::ACTIVE_TICK].numberLong();
    	info.transfer_skill_ = obj[DBTransfer::TransferSet::TRANSFER_SKILL].numberInt();
    	info.end_tick_ = obj[DBTransfer::TransferSet::END_TICK].numberLong();

    	GameCommon::bson_to_map(info.skill_map_, obj.getObjectField(DBTransfer::TransferSet::SKILL_SET.c_str()));
    }

	return 0;
END_CATCH
	return -1;
}

int MMOTransfer::update_data(MapLogicPlayer *player, MongoDataMap *mongo_data)
{
	BSONVec transfer_vec;
	TransferDetail &transfer_detail = player->transfer_detail();
	for (TransferDetail::TransferInfoMap::iterator iter = transfer_detail.transfer_map_.begin();
			iter != transfer_detail.transfer_map_.end(); ++iter)
	{
		TransferDetail::TransferInfo &info = iter->second;

		BSONVec skill_vec;
		GameCommon::map_to_bson(skill_vec, info.skill_map_);

		transfer_vec.push_back(BSON(DBTransfer::TransferSet::TRANSFER_ID << info.transfer_id_
				<< DBTransfer::TransferSet::TRANSFER_LV << info.transfer_lv_
				<< DBTransfer::TransferSet::IS_PERMANENT << info.is_permanent_
				<< DBTransfer::TransferSet::IS_ACTIVE << info.is_active_
				<< DBTransfer::TransferSet::ACTIVE_TICK << info.active_tick_
				<< DBTransfer::TransferSet::TRANSFER_SKILL << info.transfer_skill_
				<< DBTransfer::TransferSet::END_TICK << info.end_tick_
				<< DBTransfer::TransferSet::SKILL_SET << skill_vec));
	}

	Int64 refresh_tick = transfer_detail.refresh_tick_.sec();

	BSONObjBuilder builder;
	builder << DBTransfer::LEVEL << transfer_detail.level_
			<< DBTransfer::EXP << transfer_detail.exp_
			<< DBTransfer::OPEN << transfer_detail.open_
			<< DBTransfer::STAGE << transfer_detail.stage_
			<< DBTransfer::TRANSFER_TICK << transfer_detail.transfer_tick_
			<< DBTransfer::LAST << transfer_detail.last_
			<< DBTransfer::ACTIVE_ID << transfer_detail.active_id_
			<< DBTransfer::OPEN_REWARD << transfer_detail.open_reward_
			<< DBTransfer::GOLD_TIMES << transfer_detail.gold_times_
			<< DBTransfer::REFRESH_TICK << refresh_tick
			<< DBTransfer::TRANSFER_SET << transfer_vec;
	mongo_data->push_update(DBTransfer::COLLECTION, BSON(DBTransfer::ID << player->role_id()),
			builder.obj(), true);

	return 0;
}

