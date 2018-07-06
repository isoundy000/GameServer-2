/*
 * MMORankPannel.cpp
 *
 *  Created on: Mar 5, 2014
 *      Author: louis
 */

#include "GameField.h"
#include "MMORankPannel.h"
#include "RankSystem.h"
#include "MapLogicPlayer.h"
#include "MapPlayerEx.h"
#include "LogicPlayer.h"
#include "MapMonitor.h"

#include "MMORole.h"
#include "MMOBeast.h"
#include "DBCommon.h"

#include "MongoConnector.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMORankPannel::MMORankPannel() {
	// TODO Auto-generated constructor stub

}

MMORankPannel::~MMORankPannel() {
	// TODO Auto-generated destructor stub
}

int MMORankPannel::check_error_role(Int64 role_id)
{
	return CACHED_INSTANCE->mmo_role()->check_validate_create_role(role_id);
}

void MMORankPannel::load_rank_pannel_data()
{
	LongMap error_role;

	RankPannel& rank_pannel = RANK_SYS->rank_pannel();
	RankShowPannel& rank_show_pannel = RANK_SYS->rank_show_pannel();

	const Json::Value& cfg = CONFIG_INSTANCE->rank_json();
	JUDGE_RETURN(cfg != Json::Value::null, );

	BSONObj hide_role = CACHED_CONNECTION.findOne(DBRankHide::COLLECTION, BSONObj());
	BSONObj role_set = hide_role.getObjectField(DBRankHide::ROLE_ID.c_str());

	LongSet &hide_set = RANK_SYS->get_hide_player();
	hide_set.clear();

	BSONObjIterator id_it(role_set);
	while(id_it.more())
	{
		hide_set.insert(id_it.next().numberLong());
	}

	for(int i = 0; i < (int)cfg["rank_type"].size(); ++i)
	{
		int rank_type = cfg["rank_type"][i].asInt();

		BSONObj resource_obj = CACHED_CONNECTION.findOne(DBRank::COLLECTION,
				QUERY(DBRank::ID << rank_type));

//		JUDGE_CONTINUE(resource_obj.isEmpty() == false);

		RankRecordMap record_map;
		RankRecordVec record_vec;

		if(resource_obj.hasField(DBRank::RANK_RECORD.c_str()))
		{
			BSONObj info = resource_obj.getObjectField(DBRank::RANK_RECORD.c_str());
			JUDGE_CONTINUE(info.isEmpty() == false);

			BSONObjIterator it(info);
			while(it.more())
			{
				BSONObj res = it.next().embeddedObject();

				Int64 role_id = res[DBRank::RankDetail::PLAYER_ID].numberLong();
				JUDGE_CONTINUE(hide_set.find(role_id) == hide_set.end());
				if (MMORankPannel::check_error_role(role_id) == false)
				{
					error_role[role_id] = true;
					continue;
				}

				RankRecord* record = RANK_SYS->pop_rank_record();
				JUDGE_CONTINUE(record != NULL);

				DBCommon::bson_to_rank_record(record, res);
                record->__vec_index = record_vec.size();
				record_map.insert(RankRecordMap::value_type(record->__single_player_info.__role_id, record));
				record_vec.push_back(record);
			}
		}

		rank_pannel.insert(RankPannel::value_type(rank_type, record_map));
		rank_show_pannel.insert(RankShowPannel::value_type(rank_type, record_vec));
	}

	for (LongMap::iterator iter = error_role.begin(); iter != error_role.end(); ++iter)
	{
		CACHED_CONNECTION.remove(Role::COLLECTION, QUERY(DBRanker::ID << iter->first), true);
		CACHED_CONNECTION.remove(DBRanker::COLLECTION, QUERY(DBRanker::ID << iter->first), true);
	}
}


int MMORankPannel::update_data(RankPannel& rank_pannel,
		MongoDataMap *mongo_data_map, const int rank_type)
{
	JUDGE_RETURN(rank_pannel.count(rank_type) > 0, -1);

	RankRecordMap& record_vec = rank_pannel[rank_type];
	JUDGE_RETURN(record_vec.size() > 0, -1);

	BSONVec rank_set;
	RankRecordMap::iterator it = record_vec.begin();
	for (; it != record_vec.end(); ++it)
	{
		RankRecord* &record = it->second;
		JUDGE_CONTINUE(record != NULL);
		rank_set.push_back(DBCommon::rank_record_to_bson(record));
	}

	BSONObjBuilder builder;
	builder << DBRank::RANK_RECORD << rank_set;

	mongo_data_map->push_update(DBRank::COLLECTION,
	BSON(DBRank::ID << rank_type), builder.obj(), true);

	return 0;
}

int MMORankPannel::update_data(RankShowPannel& rank_pannel, MongoDataMap *mongo_data_map, const int rank_type)
{
	JUDGE_RETURN(rank_pannel.count(rank_type) > 0, -1);

	RankRecordVec& record_vec = rank_pannel[rank_type];
	JUDGE_RETURN(record_vec.size() > 0, -1);

	BSONVec rank_set;
	RankRecordVec::iterator it = record_vec.begin();
	for (; it != record_vec.end(); ++it)
	{
		RankRecord* &record = *it;
		JUDGE_CONTINUE(record != NULL);
		rank_set.push_back(DBCommon::rank_record_to_bson(record));
	}

	BSONObjBuilder builder;
	builder << DBRank::RANK_RECORD << rank_set;

	mongo_data_map->push_update(DBRank::COLLECTION,
	BSON(DBRank::ID << rank_type), builder.obj(), true);

	return 0;
}

int MMORankPannel::push_equip(BSONVec &equip_vec, GamePackage* package)
{
	JUDGE_RETURN(package != NULL, 0);

	BSONVec item_vec;
	item_vec.reserve(package->item_list_map_.size());

	ItemListMap::iterator it = package->item_list_map_.begin();
	for(; it != package->item_list_map_.end(); ++it)
	{
		PackageItem* item = it->second;
		JUDGE_CONTINUE(item != NULL);

		item_vec.push_back(GameCommon::item_to_bson(item));
	}

	BSONObjBuilder builder;
	builder << Package::Pack::PACK_TYPE << package->type()
			<< Package::Pack::PACK_ITEM << item_vec;

	equip_vec.push_back(builder.obj());

	return 0;
}

int MMORankPannel::update_date(LogicPlayer* player,
		MongoDataMap *mongo_data_map)
{
	BSONObjBuilder build;
	build << DBRanker::CAREER << player->role_detail().__career
			<< DBRanker::LEAGUE_ID << player->role_detail().__league_id
			<< DBRanker::KILL_NORMAL << player->role_detail().kill_normal_
			<< DBRanker::KILL_EVIL << player->role_detail().kill_evil_
			<< DBRanker::WEEK_TICK << Int64(player->role_detail().week_reset_tick_.sec())
			<< DBRanker::SEND_FLOWER << player->total_send_flower()
			<< DBRanker::RECV_FLOWER << player->total_recv_flower()
			<< DBRanker::ACT_SEND_FLOWER << player->act_total_send_flower()
			<< DBRanker::ACT_RECV_FLOWER << player->act_total_recv_flower();

	League* league = player->find_league(player->role_detail().__league_id);
	std::string league_name;
	if(NULL != league)
		league_name = league->league_name_;
	build << DBRanker::LEAGUE_NAME << league_name;

	mongo_data_map->push_update(DBRanker::COLLECTION,
			BSON(DBRanker::ID << player->role_id()), build.obj(), true);
	return 0;
}

int MMORankPannel::update_date(MapLogicPlayer* player, MongoDataMap *mongo_data_map)
{
	//calc gamepackage item : all equip
	BSONVec player_equip, equip_vec, mount_vec;

//	GamePackage* equip_package = player->find_package(GameEnum::INDEX_EQUIP);
//	MMORankPannel::push_equip(equip_vec, equip_package);

	GamePackage* mount_package = player->find_package(GameEnum::INDEX_MOUNT);
	MMORankPannel::push_equip(equip_vec, mount_package);

	GamePackage* god_package = player->find_package(GameEnum::INDEX_GOD_SOLIDER);
	MMORankPannel::push_equip(equip_vec, god_package);

	GamePackage* magic_package = player->find_package(GameEnum::INDEX_MAGIC_WEAPON);
	MMORankPannel::push_equip(equip_vec, magic_package);

	GamePackage* wing_package = player->find_package(GameEnum::INDEX_XIANYU);
	MMORankPannel::push_equip(equip_vec, wing_package);

	GamePackage* beast_package = player->find_package(GameEnum::INDEX_BEAST);
	MMORankPannel::push_equip(equip_vec, beast_package);

	GamePackage* beast_wu_package = player->find_package(GameEnum::INDEX_BEAST_WU);
	MMORankPannel::push_equip(equip_vec, beast_wu_package);

	GamePackage* beast_weapon_package = player->find_package(GameEnum::INDEX_BEAST_WEAPON);
	MMORankPannel::push_equip(equip_vec, beast_weapon_package);

	GamePackage* beast_wing_package = player->find_package(GameEnum::INDEX_BEAST_WING);
	MMORankPannel::push_equip(equip_vec, beast_wing_package);

	GamePackage* beast_mao_package = player->find_package(GameEnum::INDEX_BEAST_MAO);
	MMORankPannel::push_equip(equip_vec, beast_mao_package);

	GamePackage* tian_gang_package = player->find_package(GameEnum::INDEX_TIAN_GANG);
	MMORankPannel::push_equip(equip_vec, tian_gang_package);

	GamePackage* package = player->find_package(GameEnum::INDEX_EQUIP);
	if(NULL != package)
	{
		ItemListMap::iterator it = package->item_list_map_.begin();
		for(; it != package->item_list_map_.end(); ++it)
		{
			PackageItem* item = it->second;
			JUDGE_CONTINUE(item != NULL);

			player_equip.push_back(GameCommon::item_to_bson(item));
		}
	}

	MountDetail& mount_detail = player->mount_detail();

	BSONObjBuilder build;
	build << DBRanker::EQUIP_LIST << player_equip
		  << DBRanker::EQUIP_SET << equip_vec
	      << DBRanker::MOUNT_GRADE << mount_detail.mount_grade_
	      << DBRanker::MOUNT_SHAPE << mount_detail.mount_shape_
		  << DBRanker::IS_ON_MOUNT << mount_detail.on_mount_;

	for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
	{
		MountDetail& fun_mount = player->mount_detail(i);
		string mount_grade;
		string mount_force;
		switch (i)
		{
			case GameEnum::FUN_MOUNT:
				mount_grade = DBRanker::FUN_MOUNT_GRADE;
				mount_force = DBRanker::FUN_MOUNT_FORCE;
				break;
			case GameEnum::FUN_GOD_SOLIDER:
				mount_grade = DBRanker::FUN_GOD_SOLIDER_GRADE;
				mount_force = DBRanker::FUN_GOD_SOLIDER_FORCE;
				break;
			case GameEnum::FUN_MAGIC_EQUIP:
				mount_grade = DBRanker::FUN_MAGIC_EQUIP_GRADE;
				mount_force = DBRanker::FUN_MAGIC_EQUIP_FORCE;
				break;
			case GameEnum::FUN_XIAN_WING:
				mount_grade = DBRanker::FUN_XIAN_WING_GRADE;
				mount_force = DBRanker::FUN_XIAN_WING_FORCE;
				break;
			case GameEnum::FUN_LING_BEAST:
				mount_grade = DBRanker::FUN_LING_BEAST_GRADE;
				mount_force = DBRanker::FUN_LING_BEAST_FORCE;
				break;
			case GameEnum::FUN_BEAST_EQUIP:
				mount_grade = DBRanker::FUN_BEAST_EQUIP_GRADE;
				mount_force = DBRanker::FUN_BEAST_EQUIP_FORCE;
				break;
			case GameEnum::FUN_BEAST_MOUNT:
				mount_grade = DBRanker::FUN_BEAST_MOUNT_GRADE;
				mount_force = DBRanker::FUN_BEAST_MOUNT_FORCE;
				break;
			case GameEnum::FUN_BEAST_WING:
				mount_grade = DBRanker::FUN_BEAST_WING_GRADE;
				mount_force = DBRanker::FUN_BEAST_WING_FORCE;
				break;
			case GameEnum::FUN_BEAST_MAO:
				mount_grade = DBRanker::FUN_BEAST_MAO_GRADE;
				mount_force = DBRanker::FUN_BEAST_MAO_FORCE;
				break;
			case GameEnum::FUN_TIAN_GANG:
				mount_grade = DBRanker::FUN_TIAN_GANG_GRADE;
				mount_force = DBRanker::FUN_TIAN_GANG_FORCE;
				break;
		}
		build	<< mount_grade << fun_mount.mount_grade_
				<< mount_force << fun_mount.total_prop_.force_;

		BSONVec skill_vec;
		DBCommon::skill_map_to_bson(skill_vec, fun_mount.skill_map_);

		BSONObjBuilder builder;
		builder << DBRanker::OFF_MOUNT_TYPE << i
				<< DBRanker::OFF_MOUNT_OPEN << fun_mount.open_
			    << DBRanker::OFF_MOUNT_GRADE << fun_mount.mount_grade_
			    << DBRanker::OFF_MOUNT_SHAPE << fun_mount.mount_shape_
			    << DBRanker::OFF_ACT_SHAPE << fun_mount.act_shape_
			    << DBRanker::OFF_MOUNT_SKILL << skill_vec
			    << DBRanker::OFF_MOUNT_FORCE << fun_mount.total_prop_.force_
			    << DBRanker::OFF_MOUNT_PROP << DBCommon::fight_property_to_bson(fun_mount.fight_prop_)
				<< DBRanker::OFF_MOUNT_TEMP << DBCommon::fight_property_to_bson(fun_mount.temp_prop_);

		mount_vec.push_back(builder.obj());
	}
	build << DBRanker::MOUNT_SET << mount_vec;

	mongo_data_map->push_update(DBRanker::COLLECTION,
	BSON(DBRanker::ID << player->role_id()), build.obj(), true);

	return 0;
}

int MMORankPannel::update_date(MapPlayerEx* player,MongoDataMap *mongo_data_map)
{
	BSONObjBuilder build;
	build 	<< DBRanker::NAME << player->role_detail().__name
			<< DBRanker::FIGHT_FORCE << player->role_detail().__fight_force
			<< DBRanker::KILL_VALUE << player->killed_info().killing_value_
			<< DBRanker::KILL_NUM << player->killed_info().kill_num_
			<< DBRanker::FIGHT_LEVEL << player->level()
			<< DBRanker::EXPERIENCE << player->fight_detail().__experience
			<< DBRanker::LABEL << player->get_cur_label()
			<< DBRanker::WEAPON << player->get_shape_item_id(GameEnum::EQUIP_WEAPON)
			<< DBRanker::CLOTHES
			<< player->get_shape_item_id(GameEnum::EQUIP_YIFU)
			<< DBRanker::FASHION_WEAPON
			<< player->fetch_fashion_color()
			<< DBRanker::FASHION_CLOTHES
			<< player->fetch_fashion_id()
			<< DBRanker::ATTACK
			<< player->fight_detail().__attack_total_i(player)
			<< DBRanker::DEFENCE
			<< player->fight_detail().__defence_total_i(player)
			<< DBRanker::MAX_BLOOD
			<< player->fight_detail().__blood_total_i(player)
			<< DBRanker::ATTACK_UPPER
			<< int(player->fight_detail().__attack_upper_total(player))
			<< DBRanker::ATTACK_LOWER
			<< int(player->fight_detail().__attack_lower_total(player))
			<< DBRanker::DEFENCE_UPPER
			<< int(player->fight_detail().__defence_upper_total(player))
			<< DBRanker::DEFENCE_LOWER
			<< int(player->fight_detail().__defence_lower_total(player))
			<< DBRanker::CRIT
			<< player->fight_detail().__crit_total_i(player)
			<< DBRanker::TOUGHNESS
			<< player->fight_detail().__toughness_total_i(player)
			<< DBRanker::HIT
			<< player->fight_detail().__hit_total_i(player)
			<< DBRanker::DODGE
			<< player->fight_detail().__avoid_total_i(player)
			<< DBRanker::LUCKY
			<< player->fight_detail().__lucky_total_i(player)
			<< DBRanker::CRIT_HIT
			<< (player->fight_detail().__crit_hurt_multi_total(player) * 100)
			<< DBRanker::DAMAGE
			<< (player->fight_detail().__damage_rate_total(player) * 100)
			<< DBRanker::REDUCTION
			<< (player->fight_detail().__reduction_rate_total(player) * 100)
			<< DBRanker::GLAMOUR
			<< player->fight_detail().__glamour
			<< DBRanker::CAREER
			<< player->role_detail().__career
			<< Role::SEX
			<< player->role_detail().__sex
			<< DBRanker::VIP_STATUS
			<< player->vip_detail().__vip_level;

	BSONVec skill_vec;
	build << Skill::SKILL << skill_vec;

	mongo_data_map->push_update(DBRanker::COLLECTION,
			BSON(DBRanker::ID << player->role_id()), build.obj(), true);
	return 0;
}

int MMORankPannel::update_date_player_script_zyfm(const RankRecord* rank_record, MongoDataMap *mongo_data_map)
{
	BSONObjBuilder builder;

	builder << DBRanker::ZYFM_PASS_KEY << rank_record->__single_player_info.__rank_value
			<< DBRanker::ZYFM_PASS_TICK << rank_record->__single_player_info.__achive_tick;

	mongo_data_map->push_update(DBRanker::COLLECTION,
			BSON(DBRanker::ID << rank_record->__single_player_info.__role_id), builder.obj(), true);

	return 0;
}

int MMORankPannel::load_rank_data(MongoDataMap* data_map, const int rank_type)
{
	BSONObj obj;
	switch (rank_type)
	{
	case TRANS_LOAD_FIGHT_FORCE_RANK_DATA:
		data_map->push_query(DBRanker::COLLECTION, obj);
		break;
	case TRANS_LOAD_KILL_VALUE_RANK_DATA:
	    data_map->push_query(DBRanker::COLLECTION, obj);
		break;
	case TRANS_LOAD_HERO_RANK_DATA:
		data_map->push_query(DBRanker::COLLECTION, obj);
		break;
	case TRANS_LOAD_FIGHT_LEVEL_RANK_DATA:
		data_map->push_query(DBRanker::COLLECTION, obj);
		break;
	case TRANS_LOAD_PET_RANK_DATA:
		data_map->push_query(DBRanker::COLLECTION, obj);
		break;
	case TRANS_LOAD_MOUNT_RANK_DATA:
		data_map->push_query(DBRanker::COLLECTION, obj);
		break;
	case TRANS_LOAD_SCRIPT_ZYFM_RANK_DATA:
		data_map->push_query(DBRanker::COLLECTION, obj);
		break;
	case TRANS_LOAD_RANK_FUN_INFO:
		data_map->push_query(DBRanker::COLLECTION, obj);
		break;
		/*
	case TRANS_LOAD_RANK_FUN_MOUNT_INFO:
		data_map->push_query(DBRanker::COLLECTION, obj);
		break;
	case TRANS_LOAD_RANK_FUN_GOD_SOLIDER_INFO:
		data_map->push_query(DBRanker::COLLECTION, obj);
		break;
	case TRANS_LOAD_RANK_FUN_MAGIC_EQUIP_INFO:
		data_map->push_query(DBRanker::COLLECTION, obj);
		break;
	case TRANS_LOAD_RANK_FUN_XIAN_WING_INFO:
		data_map->push_query(DBRanker::COLLECTION, obj);
		break;
	case TRANS_LOAD_RANK_FUN_LING_BEAST_INFO:
		data_map->push_query(DBRanker::COLLECTION, obj);
		break;
	case TRANS_LOAD_RANK_FUN_BEAST_EQUIP_INFO:
		data_map->push_query(DBRanker::COLLECTION, obj);
		break;
	case TRANS_LOAD_RANK_FUN_BEAST_MOUNT_INFO:
		data_map->push_query(DBRanker::COLLECTION, obj);
		break;
		*/
	default:
		break;
	}
	return 0;
}

void MMORankPannel::save_offline_data(PlayerOfflineData* offline_data, int save_type, int direct_save)
{
	JUDGE_RETURN(offline_data->role_id_ > 0, ;);

	BSONObjBuilder builder;

	switch (save_type)
	{
	case GameEnum::OFFLINE_DT_BEAST_MOUNT:
	{
		break;
	}

	default:
	{
		return ;
	}
	}

    if (direct_save == false)
    {
		GameCommon::request_save_mmo_begin(DBOfflineData::COLLECTION,
				BSON(DBOfflineData::ID << offline_data->role_id_),
				BSON("$set" << builder.obj()));
    }
    else
    {
    	CACHED_CONNECTION.update(DBOfflineData::COLLECTION,
				BSON(DBOfflineData::ID << offline_data->role_id_),
				BSON("$set" << builder.obj()), true);
    }
}


int MMORankPannel::save_beast_detail_on_pet_rank(DBShopMode* shop_mode)
{
	return 0;
}

int MMORankPannel::fix_rank_manager_next_refresh(RankRefreshManager &rank_manager)
{
BEGIN_CATCH
	IntMap refresh_tick_map;
	auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBRank::COLLECTION);
	while(cursor->more())
	{
		BSONObj res = cursor->next();
		int rank_type = res[DBRank::RankDetail::RANK_TYPE].numberInt();
		int refresh_tick = res[DBRank::RankDetail::LAST_REFRESH_TICK].numberInt();

		refresh_tick_map[rank_type] = refresh_tick;
	}

	for(RankRefreshManager::iterator it = rank_manager.begin();
			it != rank_manager.end(); ++it)
	{
		RankRefreshDetail &refresh_detail = it->second;
		refresh_detail.__last_refresh_tick = refresh_tick_map[it->first];

		int cur_day_stamp = current_day(0, 0).sec();
		int cur_time = Time_Value::gettimeofday().sec();

		if(1 == refresh_detail.__refresh_type)
		{
			//最后一次刷新是今天之前
			if(refresh_detail.__last_refresh_tick < cur_day_stamp)
			{
				if(refresh_detail.__refresh_tick_set.size() == 0)
				{
					refresh_detail.__next_refresh_tick = cur_time;
					continue;
				}

				IntVec::reverse_iterator iter = refresh_detail.__refresh_tick_set.rbegin();
				if(refresh_detail.__last_refresh_tick < (*iter - Time_Value::DAY))
				{
					refresh_detail.__next_refresh_tick = cur_time;
					continue;
				}
			}

			IntVec::iterator iter = refresh_detail.__refresh_tick_set.begin();
			for(; iter != refresh_detail.__refresh_tick_set.end(); ++iter)
			{
				JUDGE_CONTINUE(refresh_detail.__last_refresh_tick <= *iter);
				refresh_detail.__next_refresh_tick = *iter;
				break;
			}
		}
		else
		{// 每天0点刷新类型
			if(refresh_detail.__last_refresh_tick < cur_day_stamp)
			{
				refresh_detail.__next_refresh_tick = cur_time;
			}
		}
	}

	return 0;

END_CACHE_CATCH
	return -1;
}

int MMORankPannel::save_rank_manager_last_refresh(const RankRefreshManager &rank_manager)
{
BEGIN_CATCH
	for(RankRefreshManager::const_iterator iter = rank_manager.begin();
			iter != rank_manager.end(); ++iter)
	{
		BSONObjBuilder builder;

		builder << DBRank::RankDetail::LAST_REFRESH_TICK
				<< iter->second.__last_refresh_tick;

		CACHED_CONNECTION.update(DBRank::COLLECTION,
				QUERY(DBRank::RankDetail::RANK_TYPE << iter->first),
				BSON("$set" << builder.obj()), true);
	}

	return 0;

END_CACHE_CATCH
	return -1;
}

int MMORankPannel::update_rank_last_refresh(MongoDataMap *data_map, const int rank_type, const int refresh_tick)
{
	data_map->push_update(DBRank::COLLECTION,
			BSON(DBRank::RankDetail::RANK_TYPE << rank_type),
			BSON(DBRank::RankDetail::LAST_REFRESH_TICK << refresh_tick));
	return 0;
}

void MMORankPannel::ensure_all_index()
{
	this->conection().ensureIndex(DBRank::COLLECTION, BSON(DBRank::ID << 1), true);
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::ID << 1), true);

	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::FIGHT_FORCE << 1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::KILL_VALUE << -1 << DBRanker::FIGHT_LEVEL << 1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::KILL_NUM << -1 << DBRanker::FIGHT_LEVEL << 1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::KILL_NORMAL << -1 << DBRanker::FIGHT_LEVEL << 1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::KILL_EVIL << -1 << DBRanker::FIGHT_LEVEL << 1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::FIGHT_LEVEL << -1 << DBRanker::EXPERIENCE << -1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::Pet::PET_FORCE << 1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::MOUNT_GRADE << 1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::FUN_MOUNT_GRADE << -1 << DBRanker::FUN_MOUNT_FORCE << -1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::FUN_GOD_SOLIDER_GRADE << -1 << DBRanker::FUN_GOD_SOLIDER_FORCE << -1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::FUN_MAGIC_EQUIP_GRADE << -1 << DBRanker::FUN_MAGIC_EQUIP_FORCE << -1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::FUN_XIAN_WING_GRADE << -1 << DBRanker::FUN_XIAN_WING_FORCE << -1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::FUN_LING_BEAST_GRADE << -1 << DBRanker::FUN_LING_BEAST_FORCE << -1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::FUN_BEAST_EQUIP_GRADE << -1 << DBRanker::FUN_BEAST_EQUIP_FORCE << -1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::FUN_BEAST_MOUNT_GRADE << -1 << DBRanker::FUN_BEAST_MOUNT_FORCE << -1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::FUN_BEAST_WING_GRADE << -1 << DBRanker::FUN_BEAST_WING_FORCE << -1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::FUN_BEAST_MAO_GRADE << -1 << DBRanker::FUN_BEAST_MAO_FORCE << -1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::FUN_TIAN_GANG_GRADE << -1 << DBRanker::FUN_TIAN_GANG_FORCE << -1));

	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::ZYFM_PASS_KEY << -1
			<< DBRanker::ZYFM_PASS_TICK << 1 << DBRanker::FIGHT_FORCE << -1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::WING_LEVEL << -1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::SEND_FLOWER << -1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::RECV_FLOWER << -1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::ACT_SEND_FLOWER << -1));
	this->conection().ensureIndex(DBRanker::COLLECTION, BSON(DBRanker::ACT_RECV_FLOWER << -1));

	this->conection().ensureIndex(DBOfflineData::COLLECTION, BSON(DBOfflineData::ID << 1), true);
//	this->conection().ensureIndex(DBRankSystem::COLLECTION, BSON(DBRankSystem::RANK_TYPE << 1), true);
}
