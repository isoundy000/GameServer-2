/*
 * GameCommon.cpp
 *
 *  Created on: Jun 30, 2013
 *      Author: peizhibi
 */

#include "GameCommon.h"
#include "GameField.h"
#include "GameConfig.h"
#include "PoolMonitor.h"
#include "ProtoDefine.h"
#include "MongoConnector.h"

#include <mongo/client/dbclient.h>

#include "MapStruct.h"
#include "FightStruct.h"
#include "MapLogicPlayer.h"

#include "GameFont.h"
#include "GameFighter.h"
#include "MapPlayerEx.h"
#include "MapMonitor.h"
#include "RankStruct.h"
#include "ActivityTipsSystem.h"
#include "BackField.h"
#include "GameAI.h"
#include "MapBeast.h"
#include "MLGameSwither.h"
#include "LogicStruct.h"
#include "SessionManager.h"
#include "LogicMonitor.h"
#include "DaemonServer.h"
#include "Scene.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

ItemObj GameCommon::NullItemObj;
std::string GameCommon::NullString;

const uint16_t GameCommon::crc16_table[256]=
{
	 0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	 0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	 0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
	 0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	 0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
	 0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
	 0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	 0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	 0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	 0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
	 0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	 0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	 0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
	 0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	 0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
	 0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	 0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
	 0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
	 0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	 0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
	 0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	 0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	 0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
	 0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
	 0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
	 0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	 0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
	 0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	 0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	 0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	 0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	 0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

TipsPlayer::TipsPlayer(EntityCommunicate* player) : tips_info_(new Proto80400216)
{
	this->player_ = player;
}

TipsPlayer::~TipsPlayer()
{
	Proto80400216* tips_info = this->tips_info_.get();
	JUDGE_RETURN(tips_info->tips_set_size() > 0 || tips_info->tips_str_set_size() > 0, ;);
	this->player_->respond_to_client(ACTIVE_TIPS_INFO, tips_info);
}

void TipsPlayer::push_tips(int type, int id, int amount)
{
	TipsItem* tips_item = this->tips_info_.get()->add_tips_set();
	tips_item->set_type(type);

	tips_item->set_id(id);
	tips_item->set_amount(amount);
}

void TipsPlayer::push_goods(const ItemObjVec& item_set)
{
	for (ItemObjVec::const_iterator iter = item_set.begin();
			iter != item_set.end(); ++iter)
	{
		this->push_goods(iter->id_, iter->amount_);
	}
}

void TipsPlayer::push_goods(int id, int amount)
{
	this->push_tips(GameEnum::TIPS_ITEM, id, amount);
}

void TipsPlayer::push_money(const Money& money)
{
	if (money.__gold > 0)
	{
		this->push_goods(GameEnum::ITEM_MONEY_UNBIND_GOLD, money.__gold);
	}

	if (money.__bind_gold > 0)
	{
		this->push_goods(GameEnum::ITEM_MONEY_BIND_GOLD, money.__bind_gold);
	}
}

void TipsPlayer::push_tips_str(const char* str)
{
	this->tips_info_.get()->add_tips_str_set(str);
}

void TipsPlayer::push_tips_msg_id(int msg_id, ...)
{
	const int buf_size = 1024;
	char buf[buf_size] = {0};

	const Json::Value &msg_format_json = CONFIG_INSTANCE->tips_msg_format(msg_id);
	JUDGE_RETURN(msg_format_json != Json::Value::null, ;);

	const std::string &format_str = msg_format_json.asString();
	if(format_str.length() <= 0)
	{
		MSG_USER("format error!!! tips_msg_format(%d)", msg_id);
		return ;
	}

    va_list arg_list;
    va_start(arg_list, msg_id);

	::vsnprintf(buf, buf_size - 1, format_str.c_str(), arg_list);
	buf[buf_size - 1] = '\0';

	va_end(arg_list);

	return this->push_tips_str(buf);
}

int GameSwitcherDetail::serialize(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30101401*, res, -1);
	return 0;
}

int GameSwitcherDetail::unserialize(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30101401*, res, -1);
	return 0;
}

/*
 * sort compare function
 */
bool GameCommon::pair_comp_by_asc(const PairObj& first, const PairObj& second)
{
	return first.value_ < second.value_;
}

bool GameCommon::pair_comp_by_desc(const PairObj& first, const PairObj& second)
{
	return first.value_ > second.value_;
}

bool GameCommon::three_comp_by_asc(const ThreeObj& first, const ThreeObj& second)
{
	if (first.value_ != second.value_)
	{
		return first.value_ < second.value_;
	}

	return first.tick_ < second.tick_;
}

bool GameCommon::three_comp_by_desc(const ThreeObj& first, const ThreeObj& second)
{
	if (first.value_ != second.value_)
	{
		return first.value_ > second.value_;
	}

	return first.tick_ < second.tick_;
}

bool GameCommon::three_comp_by_desc_b(const ThreeObj& first, const ThreeObj& second)
{
	if (first.value_ != second.value_)
	{
		return first.value_ > second.value_;
	}

	return first.tick_ > second.tick_;
}

bool GameCommon::four_comp_by_desc(const FourObj& first, const FourObj& second)
{
	if (first.first_value_ != second.first_value_)
	{
		return first.first_value_ > second.first_value_;
	}

	if (first.second_value_ != second.second_value_)
	{
		return first.second_value_ > second.second_value_;
	}

	return first.tick_ < second.tick_;
}


void GameCommon::bson_to_item(PackageItem *item, const BSONObj& item_obj)
{
    item->__index 	= item_obj[Package::PackItem::INDEX].numberInt();
    item->__id 		= item_obj[Package::PackItem::ID].numberInt();
    item->__amount 	= item_obj[Package::PackItem::AMOUNT].numberInt();
    item->__bind 	= item_obj[Package::PackItem::BIND].numberInt();
    item->__use_times 	= item_obj[Package::PackItem::USE_TIMES].numberInt();
    item->__timeout 	= item_obj[Package::PackItem::TIME_OUT].numberLong();
    item->__new_tag 	= item_obj[Package::PackItem::NEW_TAG].numberInt();
    item->__unique_id 	= item_obj[Package::PackItem::UNIQUE_ID].numberLong();

    GameCommon::bson_to_map(item->__tips_time_map,item_obj.getObjectField(
    		Package::PackItem::TIPS_TIME_MAP.c_str()));

    GameCommon::bson_to_map(item->__tips_status_map,item_obj.getObjectField(
    		Package::PackItem::TIPS_STATUS_MAP.c_str()));

    if (GameCommon::item_is_equipment(item->__id))
    {
    	item->__equipment.strengthen_level_ 	= item_obj[Package::PackEquip::REFINE_LEVEL].numberInt();
    	item->__equipment.__refine_degree 	= item_obj[Package::PackEquip::REFINE_DEGREE].numberInt();
    	item->__equipment.__luck_value 		= item_obj[Package::PackEquip::LUCK_VALUE].numberInt();

    	GameCommon::bson_to_map(item->__equipment.__jewel_detail, item_obj.getObjectField(
    			Package::PackEquip::JEWEL_SETS.c_str()));

    	IntMap refine_map;
    	GameCommon::bson_to_map(refine_map, item_obj.getObjectField(
    			Package::PackEquip::REFINE_SETS.c_str()));

    	MoldingSpiritDetail &molding_detail = item->__equipment.__molding_detail;
    	molding_detail.__nature_level_map[GameEnum::MOLDING_SPIRIT_ATTACK] = item_obj[Package::PackEquip::MOLDING_ATTACK_LEVEL].numberInt();
    	molding_detail.__nature_level_map[GameEnum::MOLDING_SPIRIT_DEFENSE] = item_obj[Package::PackEquip::MOLDING_DEFENCE_LEVEL].numberInt();
    	molding_detail.__nature_level_map[GameEnum::MOLDING_SPIRIT_BLOOD] = item_obj[Package::PackEquip::MOLDING_HEALTH_LEVEL].numberInt();
    	molding_detail.__nature_level_map[GameEnum::MOLDING_SPIRIT_ALL_NATURE] = item_obj[Package::PackEquip::MOLDING_ALL_LEVEL].numberInt();
    	molding_detail.__nature_schedule_map[GameEnum::MOLDING_SPIRIT_ATTACK] = item_obj[Package::PackEquip::MOLDING_ATTACK_SCHEDULE].numberInt();
    	molding_detail.__nature_schedule_map[GameEnum::MOLDING_SPIRIT_DEFENSE] = item_obj[Package::PackEquip::MOLDING_DEFENCE_SCHEDULE].numberInt();
    	molding_detail.__nature_schedule_map[GameEnum::MOLDING_SPIRIT_BLOOD] = item_obj[Package::PackEquip::MOLDING_HEALTH_SCHEDULE].numberInt();

    	item->__equipment.set_refine(refine_map);

    }
}

BSONObj GameCommon::item_to_bson(PackageItem *item)
{
    BSONObjBuilder builder;

	BSONVec time_map, status_map;
	GameCommon::map_to_bson(time_map, item->__tips_time_map);
	GameCommon::map_to_bson(status_map, item->__tips_status_map);

    builder << Package::PackItem::INDEX << item->__index
        << Package::PackItem::ID << item->__id
        << Package::PackItem::AMOUNT << item->__amount
        << Package::PackItem::BIND << item->__bind
        << Package::PackItem::NEW_TAG << item->__new_tag
        << Package::PackItem::USE_TIMES << item->__use_times
        << Package::PackItem::TIME_OUT << item->__timeout
        << Package::PackItem::UNIQUE_ID << item->__unique_id
        << Package::PackItem::TIPS_TIME_MAP << time_map
        << Package::PackItem::TIPS_STATUS_MAP << status_map;

    if (GameCommon::item_is_equipment(item->__id))
    {
    	BSONVec jewel_info;
    	GameCommon::map_to_bson(jewel_info, item->__equipment.__jewel_detail);

    	IntMap refine_map;
    	item->__equipment.fetch_refine(refine_map);

    	BSONVec refine_info;
    	GameCommon::map_to_bson(refine_info, refine_map);

    	MoldingSpiritDetail &molding_detail = item->__equipment.__molding_detail;
    	builder << Package::PackEquip::REFINE_LEVEL << item->__equipment.strengthen_level_
    			<< Package::PackEquip::REFINE_DEGREE << item->__equipment.__refine_degree
    			<< Package::PackEquip::JEWEL_SETS << jewel_info
    			<< Package::PackEquip::REFINE_SETS << refine_info
    	    	<< Package::PackEquip::MOLDING_ATTACK_LEVEL << molding_detail.__nature_level_map[GameEnum::MOLDING_SPIRIT_ATTACK]
    	    	<< Package::PackEquip::MOLDING_DEFENCE_LEVEL << molding_detail.__nature_level_map[GameEnum::MOLDING_SPIRIT_DEFENSE]
    	    	<< Package::PackEquip::MOLDING_HEALTH_LEVEL << molding_detail.__nature_level_map[GameEnum::MOLDING_SPIRIT_BLOOD]
    	    	<< Package::PackEquip::MOLDING_ALL_LEVEL << molding_detail.__nature_level_map[GameEnum::MOLDING_SPIRIT_ALL_NATURE]
    	    	<< Package::PackEquip::MOLDING_ATTACK_SCHEDULE << molding_detail.__nature_schedule_map[GameEnum::MOLDING_SPIRIT_ATTACK]
    	    	<< Package::PackEquip::MOLDING_DEFENCE_SCHEDULE << molding_detail.__nature_schedule_map[GameEnum::MOLDING_SPIRIT_DEFENSE]
    	    	<< Package::PackEquip::MOLDING_HEALTH_SCHEDULE << molding_detail.__nature_schedule_map[GameEnum::MOLDING_SPIRIT_BLOOD]
    			;
    }

    return builder.obj();
}

void GameCommon::bson_to_money(Money& money, const BSONObj& money_obj)
{
    money.__gold = money_obj[Package::Money::GOLD].numberInt();
    money.__bind_gold = money_obj[Package::Money::BIND_GOLD].numberInt();

    money.__copper = money_obj[Package::Money::COPPER].numberInt();
    money.__bind_copper = money_obj[Package::Money::BIND_COPPER].numberInt();
}

BSONObj GameCommon::money_to_bson(const Money& money)
{
	return BSON(Package::Money::GOLD << money.__gold
            << Package::Money::COPPER << money.__copper
            << Package::Money::BIND_GOLD << money.__bind_gold
            << Package::Money::BIND_COPPER << money.__bind_copper);
}

void GameCommon::map_to_bson(BSONVec& bson_vec, const IntMap& obj_map,
		bool value_except_zero, bool key_include_zero)
{
	bson_vec.reserve(obj_map.size());

	for (IntMap::const_iterator iter = obj_map.begin(); iter != obj_map.end(); ++iter)
	{
		if (key_include_zero == false)
		{
			JUDGE_CONTINUE(iter->first != 0);
		}

		if (value_except_zero == true)
		{
			JUDGE_CONTINUE(iter->second != 0);
		}

		bson_vec.push_back(BSON(DBPairObj::KEY << iter->first
				<< DBPairObj::VALUE << iter->second));
	}
}

void GameCommon::bson_to_map(IntMap& obj_map, BSONObjIterator iter, bool key_include_zero)
{
	while (iter.more())
	{
		BSONObj obj = iter.next().embeddedObject();

		int obj_key = obj[DBPairObj::KEY].numberInt();
		int obj_value = obj[DBPairObj::VALUE].numberInt();

		if (key_include_zero == false)
		{
			JUDGE_CONTINUE(obj_key != 0);
		}

		obj_map[obj_key] = obj_value;
	}
}

void GameCommon::map_to_proto(ProtoPairMap* proto_map, const IntMap& obj_map)
{
	for (IntMap::const_iterator iter = obj_map.begin(); iter != obj_map.end(); ++iter)
	{
		ProtoPairObj* pair = proto_map->add_obj_list();
		pair->set_obj_id(iter->first);
		pair->set_obj_value(iter->second);
	}
}

void GameCommon::proto_to_map(IntMap& obj_map, const ProtoPairMap& proto_map)
{
	for (int i = 0; i < proto_map.obj_list_size(); ++i)
	{
		const ProtoPairObj& pair = proto_map.obj_list(i);
		obj_map[pair.obj_id()] = pair.obj_value();
	}
}

void GameCommon::vector_to_proto(ProtoPairMap* proto_map, const IntVec& obj_vec)
{
	for (IntVec::const_iterator iter = obj_vec.begin(); iter != obj_vec.end(); ++iter)
	{
		ProtoPairObj* pair = proto_map->add_obj_list();
		pair->set_obj_id(*iter);
	}
}

void GameCommon::protot_to_vector(IntVec& obj_vec, const ProtoPairMap& proto_map)
{
	int total_size = proto_map.obj_list_size();
	obj_vec.reserve(total_size);

	for (int i = 0; i < total_size; ++i)
	{
		const ProtoPairObj& pair = proto_map.obj_list(i);
		obj_vec.push_back(pair.obj_id());
	}
}

bool GameCommon::validate_money(const Money& money)
{
//	JUDGE_RETURN(money >= 0, false);
	return money.__gold > 0 || money.__bind_gold > 0
			|| money.__copper > 0 || money.__bind_copper > 0;
}

bool GameCommon::validate_item_id(int item_id)
{
	JUDGE_RETURN(item_id > 0, false);

	const Json::Value& item_conf = CONFIG_INSTANCE->item(item_id);
	return item_conf != Json::Value::null;
}

bool GameCommon::is_movable_coord(int scene_id, const MoverCoord &coord)
{
    int mpt_value = CONFIG_INSTANCE->coord_mpt_value(scene_id,
    		coord.pos_x(), coord.pos_y());
    return GameCommon::is_movable_mpt(mpt_value);
}

bool GameCommon::is_movable_coord_no_border(int scene_id, const MoverCoord& coord)
{
	if (GameCommon::is_movable_coord(scene_id, coord) == false)
	{
		return false;
	}

	if (CONFIG_INSTANCE->is_border_coord_for_rand(scene_id,
			coord.pos_x(), coord.pos_y()) == true)
	{
		return false;
	}

	return true;
}

bool GameCommon::is_nearby_aim(const IntPair& cur_coord, const IntPair& aim_coord)
{
	int abs_x = std::abs(cur_coord.first - aim_coord.first);
	int abs_y = std::abs(cur_coord.second - aim_coord.second);

	return abs_x <= 30 && abs_y <= 30;
}

int GameCommon::fetch_rand_coord(MoverCoord& aim_coord, int scene_id)
{
	int x_len = CONFIG_INSTANCE->mpt(scene_id).__x_len;
	const GameConfig::CoordIndexList& index_list = CONFIG_INSTANCE->move_coord_list(scene_id);

	while (true)
	{
		int mpt_index = index_list[std::rand() % index_list.size()];
		aim_coord.set_pos(mpt_index % x_len, mpt_index / x_len);

		if (GameCommon::is_movable_coord_no_border(scene_id, aim_coord) == true)
		{
			break;
		}
	}

	return 0;
}

MoverCoord GameCommon::fetch_rand_conf_pos(const Json::Value& pos_conf)
{
	MoverCoord enter_pos;

	if (pos_conf[0u].isArray() == false)
	{
		enter_pos.set_pixel(pos_conf[0u].asInt(), pos_conf[1u].asInt());
	}
	else
	{
		int rand_index = std::rand() % pos_conf.size();
		enter_pos.set_pixel(pos_conf[rand_index][0u].asInt(),
				pos_conf[rand_index][1u].asInt());
	}

	return enter_pos;
}

void GameCommon::fetch_rang_coord(LongSet& coord_set, const MoverCoord& location, int range)
{
	int cur_x = location.pos_x();
	int cur_y = location.pos_y();

    for (int pos_x = cur_x - range; pos_x <= cur_x + range; ++pos_x)
    {
    	JUDGE_CONTINUE(pos_x >= 0);
    	for (int pos_y = cur_y - 1; pos_y <= cur_y + 1; ++pos_y)
    	{
    		JUDGE_CONTINUE(pos_y >= 0);
    		coord_set.insert(merge_int_to_long(pos_x, pos_y));
    	}
    }
}

int GameCommon::fetch_area_id_by_coord(const int scene_id, const MoverCoord &coord)
{
    int mpt_value = CONFIG_INSTANCE->coord_mpt_value(scene_id, coord.pos_x(), coord.pos_y());
    int area_id = ((mpt_value & 0x0FF) >> 4);
    return area_id;
}

bool GameCommon::is_movable_mpt(int mpt_value)
{
	return mpt_value % 2 == 0;
}

bool GameCommon::is_normal_scene(int scene_id)
{
    return (scene_id / 10000 == 1);
}

bool GameCommon::is_travel_scene(const int scene_id)
{
	return (scene_id / 10000 == 7);
}

bool GameCommon::is_script_scene(int scene_id)
{
	int type = CONFIG_INSTANCE->scene_set_type(scene_id);
	if (type > 0)
	{
		return type / 1000 == 2;
	}
	else
	{
		return (scene_id / 10000 == 2);
	}
}

bool GameCommon::is_value_in_config(const Json::Value& value_set, int value)
{
	uint total = value_set.size();

	for (uint i = 0; i < total; ++i)
	{
		JUDGE_CONTINUE(value_set[i].asInt() == value);
		return true;
	}

	return false;
}

int GameCommon::fetch_skill_sex_bit(int skill)
{
	return (skill / 1000) % 10;
}

int GameCommon::fetch_skill_fun_bit(int skill)
{
	return (skill / 10000) % 10000;
}

int GameCommon::fetch_skill_fun_type(int fun_id)
{
	if (fun_id == 4)
	{
		return GameEnum::SKILL_FUN_PASSIVE;
	}
	else if (fun_id >= 5 && fun_id <= 14)
	{
		return GameEnum::SKILL_FUN_MOUNT;
	}
	else if (fun_id == 16)
	{
		return GameEnum::SKILL_FUN_RAMA;
	}
	else if (fun_id == 2)
	{
		return GameEnum::SKILL_FUN_SHEN_LUO;
	}
	else if (fun_id > 100)
	{
		return GameEnum::SKILL_FUN_TRANSFER;
	}

	return 0;
}

int GameCommon::fetch_skill_id_fun_type(int skill)
{
	int fun_bit = GameCommon::fetch_skill_fun_bit(skill);
	return GameCommon::fetch_skill_fun_type(fun_bit);
}

bool GameCommon::is_world_boss_scene(const int scene_id)
{
	return (scene_id > GameEnum::WORLD_BOSS_SCENE_ID_READY && scene_id < GameEnum::WORLD_BOSS_SCENE_ID_END);
}

bool GameCommon::is_trvl_wboss_scene(const int scene_id)
{
	return (scene_id > GameEnum::TRVL_WBOSS_SCENE_ID_READY && scene_id < GameEnum::TRVL_WBOSS_SCENE_ID_END);
}

bool GameCommon::is_hiddien_beast_model_scene(const int scene_id)
{
    return false;
}

bool GameCommon::is_arr_scene_level_limit(const int scene_id, const int level)
{
	const Json::Value &scene_json = CONFIG_INSTANCE->scene(scene_id);
	JUDGE_RETURN(scene_json.isMember("level") == true, true);

	//如果是只有一个，则是最低值
	if (scene_json["level"].size() == 1)
	{
		return level >= scene_json["level"][0u].asInt();
	}
	else
	{
		return level >= scene_json["level"][0u].asInt()
				&& level <= scene_json["level"][1u].asInt();
	}
}

int GameCommon::fetch_mover_type(int mover_lower_id)
{
	if (mover_lower_id <= 0)
	{
		return MOVER_TYPE_UNKOWN;
	}
	else if (mover_lower_id < BASE_OFFSET_BEAST)
	{
		//玩家
		return MOVER_TYPE_PLAYER;
	}
	else if (mover_lower_id < BASE_OFFSET_MONSTER)
	{
		//宠物
		return MOVER_TYPE_BEAST;
	}
	else if (mover_lower_id < BASE_OFFSET_AIDROP)
	{
		//怪物
		return MOVER_TYPE_MONSTER;
	}
	else
	{
		//怪物掉落
		return MOVER_TYPE_AIDROP;
	}
}

std::string GameCommon::fight_prop_name(int prop_id)
{

	switch (prop_id)
	{
	// property level two
	case GameEnum::BLOOD_MAX:
	{
		return GameName::BLOOD_MAX;
	}
	case GameEnum::MAGIC_MAX:
	{
		return GameName::MAGIC_MAX;
	}
	case GameEnum::ATTACK:
	{
		return GameName::ATTACK;
	}
	case GameEnum::DEFENSE:
	{
		return GameName::DEFENSE;
	}
	case GameEnum::CRIT:
	{
		return GameName::CRIT;
	}
	case GameEnum::TOUGHNESS:
	{
		return GameName::TOUGHNESS;
	}
	case GameEnum::HIT:
	{
		return GameName::HIT;
	}
	case GameEnum::AVOID:
	{
		return GameName::AVOID;
	}
	case GameEnum::LUCKY:
	{
		return GameName::LUCK;
	}
	case GameEnum::ATTACK_UPPER:
	{
		return GameName::ATTACK_MAX;
	}
	case GameEnum::ATTACK_LOWER:
	{
		return GameName::ATTACK_MIN;
	}
	case GameEnum::DEFENCE_UPPER:
	{
		return GameName::DEFENCE_MAX;
	}
	case GameEnum::DEFENCE_LOWER:
	{
		return GameName::DEFENCE_MIN;
	}
	case GameEnum::DAMAGE_MULTI:
	{	//注意：伤害加成和伤害减免是按倍率算的，所以这里返回的是DAMAGE和REDUCTION
		return GameName::DAMAGE_MULTI;
	}
	case GameEnum::REDUCTION_MULTI:{
		return GameName::REDUCTION_MULTI;
	}
	default:
	{
		return GameCommon::NullString;
	}
	}
}

void GameCommon::stl_split(StringVec& str_set, const std::string& des_str,
		const std::string& delim_str)
{
	size_t last = 0;
	size_t index = des_str.find_first_of(delim_str, last);

	while (index != std::string::npos)
	{
		str_set.push_back(des_str.substr(last, index - last));

		last = index + 1;
		index = des_str.find_first_of(delim_str, last);
	}

	if (index - last > 0)
	{
		str_set.push_back(des_str.substr(last, index - last));
	}
}

void GameCommon::string_hidden_replace(string& dst_str, const string& find_str)
{
	string rep_str(find_str.size(), '*');
	return GameCommon::string_replace(dst_str, find_str, rep_str);
}

void GameCommon::string_replace(string& dst_str, const string& find_str,
		const string& rep_str)
{
	string::size_type pos = 0;
	string::size_type find_size = find_str.size();
	string::size_type rep_size = rep_str.size();

	while ((pos = dst_str.find(find_str, pos)) != string::npos)
	{
		dst_str.replace(pos, find_size, rep_str);
		pos += rep_size;
	}
}

int GameCommon::mod_next(int cur_value, int total_size)
{
	cur_value += 1;
	return cur_value % total_size;
}

int GameCommon::rand_value(int min_value, int max_value)
{
	if (min_value >= max_value)
	{
		return min_value;
	}

	int diff_value = max_value - min_value + 1;
	return min_value + std::rand() % diff_value;
}

int GameCommon::rand_value(const Json::Value& range_conf)
{
	int min_value = range_conf[0u].asInt();
	int max_value = range_conf[1u].asInt();
	return GameCommon::rand_value(min_value, max_value);
}

int GameCommon::adjust_positive_integer(int cur, int total)
{
	if (cur > 0 && cur <= total)
	{
		return cur;
	}

	if (cur <= 0)
	{
		return 1;
	}

	return total;
}

int GameCommon::up_number(double number)
{
	int up_value = number * 10000;
	int check_value = int(number) * 10000;

	if (up_value == check_value)
	{
		return number;
	}
	else
	{
		return number + 1;
	}
}

int GameCommon::validate_name_length(const string& name, int max_length)
{
	int name_length = ::string_utf8_len(name.c_str());
	return name_length > 0 && name_length <= max_length;
}

int GameCommon::validate_chinese_name_length(const string& name, int max_length)
{
	int name_length = ::chinese_utf8_len(name.c_str());
	return name_length > 0 && name_length <= max_length;
}

int GameCommon::double_compare(double left, double right)
{
	double diff = left - right;
	if (std::fabs(diff) < 0.00001)
	{
		return 0;
	}
	else
	{
		return diff > 0 ?  1 : -1;
	}
}

bool GameCommon::is_zero(double number)
{
	return GameCommon::double_compare(number, 0) == 0;
}

double GameCommon::div_percent(double number)
{
	return number / GameEnum::DAMAGE_ATTR_PERCENT;
}

LongPair GameCommon::fetch_map_max_value(const LongMap& long_map)
{
	LongPair ret;

	ret.first = -1;
	ret.second = INT_MIN;

	for (LongMap::const_iterator iter = long_map.begin(); iter != long_map.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second > ret.second);

		ret.first = iter->first;
		ret.second = iter->second;
	}

	return ret;
}

void GameCommon::fetch_rand_index(IntVec& index_vec, int total_size)
{
	index_vec.reserve(total_size);

	for (int i = 0; i < total_size; ++i)
	{
		index_vec.push_back(i);
	}

	for (int i = 0; i < total_size; ++i)
	{
		int swap_index = std::rand() % total_size;
		JUDGE_CONTINUE(i != swap_index);
		std::swap(index_vec[i], index_vec[swap_index]);
	}
}

const Json::Value& GameCommon::skill_detail(FighterSkill* fighter_skill)
{
	JUDGE_RETURN(fighter_skill != NULL, Json::Value::null);
	return GameCommon::skill_detail(fighter_skill->__skill_id,
			fighter_skill->__level);
}

const Json::Value& GameCommon::skill_detail(int skill_id, int org_skill_lvl)
{
    int skill_lvl = org_skill_lvl;
	JUDGE_RETURN(skill_id > 0, Json::Value::null);
	JUDGE_RETURN(skill_lvl > 0, Json::Value::null);

    return CONFIG_INSTANCE->skill_detail(skill_id, org_skill_lvl);
}

const Json::Value& GameCommon::json_by_index(const Json::Value &json, uint index)
{
    if (json.isArray() == false)
    {
        return json;
    }

    uint total = json.size();
    if (total <= 0)
    {
    	return Json::Value::null;
    }

    if (index < 0)
    {
    	return json[0u];
    }

    if (index >= total)
    {
        return json[total - 1];
    }

    return json[index];
}

const Json::Value& GameCommon::json_by_level(const Json::Value &json, uint level)
{
	if (level > 0)
	{
		return GameCommon::json_by_index(json, level -1);
	}
	else
	{
		return GameCommon::json_by_index(json, 0);
	}
}

int GameCommon::validate_fighter(GameFighter* fighter, int type)
{
	JUDGE_RETURN(fighter != NULL, false);

	JUDGE_RETURN(fighter->is_death() == false, false);
	JUDGE_RETURN(fighter->is_active() == true, false);

	return true;
}

int GameCommon::validate_attacker(GameFighter* fighter, int type)
{
	return true;
}

int GameCommon::is_copy_player_skill(FighterSkill* fighter_skill)
{
	int skill_id = 400011001;

    for (int i = 0; i < 4; ++i)
    {

    	if (skill_id + i == fighter_skill->__skill_id ||
    		skill_id + i + 1000 == fighter_skill->__skill_id)
    	return 1;
    }
	return 0;
}

int GameCommon::validate_skill_usage(GameFighter* fighter, FighterSkill* fighter_skill)
{
    const Json::Value &skill_json = fighter_skill->conf();

    if (skill_json.isMember("left_blood_percent") == true)
    {
    	int left_per = fighter_skill->level_conf(skill_json["left_blood_percent"]);
    	JUDGE_RETURN(fighter->validate_left_blood(left_per) == true, -1);
    }

    if (skill_json.isMember("useLvl") == true)
    {
    	JUDGE_RETURN(fighter->level() >= fighter_skill->__use_level, -1);
    }

    return 0;
}

int GameCommon::validate_skill_target(FighterSkill* skill, GameFighter* attacker,
		GameFighter* target)
{
    string type_str = skill->__target_type;

    //自己
    if (GameCommon::check_config_value(type_str, GameEnum::SKILL_TARGET_SELF) == true)
    {
    	return GameCommon::is_self_relation(attacker, target) ? 0 : ERROR_TARGET_NO_ATTACK;
    }

    //主人
    if (GameCommon::check_config_value(type_str, GameEnum::SKILL_TARGET_SELF_OWNER) == true)
	{
    	return GameCommon::is_master_relation(attacker, target) ? 0 : ERROR_TARGET_NO_ATTACK;
	}

	Scene* scene = target->fetch_scene();
	JUDGE_RETURN(scene != NULL, ERROR_TARGET_NO_ATTACK);

	bool is_player = target->is_player();
	bool is_monster = target->is_monster();

    for (uint i = 0; i < type_str.size(); ++i)
    {
    	JUDGE_CONTINUE(type_str[i] == '1');

    	switch (i)
    	{
    	case GameEnum::SKILL_TARGET_ENEMY:
    	{
    		JUDGE_CONTINUE(is_player == true);
    		JUDGE_CONTINUE(GameCommon::is_enemy_relation(attacker, target, scene) == true);
    		return 0;
    	}

    	case GameEnum::SKILL_TARGET_ALLY:
    	{
    		JUDGE_CONTINUE(is_player == true);
    		JUDGE_CONTINUE(GameCommon::is_ally_relation(attacker, target, scene) == true);
    		return 0;
    	}

    	case GameEnum::SKILL_TARGET_MONSTER:
    	{
    		JUDGE_CONTINUE(is_monster == true);
    		JUDGE_CONTINUE(target->fighter_sort() != GameEnum::FIGHTER_GATHER);
    		JUDGE_CONTINUE(GameCommon::is_monster_relation(attacker, target, scene) == true);
    		return 0;
    	}

    	case GameEnum::SKILL_TARGET_BOSS:
    	{
    		JUDGE_CONTINUE(is_monster == true);
    		JUDGE_CONTINUE(target->fighter_sort() == GameEnum::FIGHTER_BOSS);
    		JUDGE_CONTINUE(GameCommon::is_boss_relation(attacker, target, scene) == true);
    		return 0;
    	}
    	}
    }

    return ERROR_TARGET_NO_ATTACK;
}

int GameCommon::is_self_relation(GameFighter* attacker, GameFighter* target)
{
	return attacker->fighter_id() == target->fighter_id();
}

// target is player
int GameCommon::is_enemy_relation(GameFighter* attacker, GameFighter* target, Scene* scene)
{
	if (GameCommon::is_self_relation(attacker, target) == true)
	{
		return false;
	}

	if (GameCommon::is_master_relation(attacker, target) == true)
	{
		return false;
	}

	if (scene->camp_split() == true)
	{
		// 区分阵营
		JUDGE_RETURN(scene->is_validate_attack_player(attacker, target) == true, false);
		return attacker->camp_id() != target->camp_id();
	}

	if (attacker->is_plenary_pk())
	{
		// 全体PK
		return true;
	}

	if (attacker->is_pk_state() == false)
	{
		return false;
	}

	if (attacker->is_team_pk())
	{
		// 队伍PK
		JUDGE_RETURN(attacker->team_id() > 0, true);
		return attacker->team_id() != target->team_id();
	}

	if (attacker->is_league_pk())
	{
		// 帮派PK
		JUDGE_RETURN(attacker->league_id() > 0, true);
		return attacker->league_id() != target->league_id();
	}

	if (attacker->is_same_pk_state(PK_DITHESISM))
	{
		// 善恶模式下只能攻击红名或攻击自己的玩家
		JUDGE_RETURN(target->fetch_name_color() >= GameEnum::NAME_WHITE, true);
		return attacker->is_attack_by_id(target->mover_id());
	}

	return false;
}

//target is player
int GameCommon::is_ally_relation(GameFighter* attacker, GameFighter* target, Scene* scene)
{
	if (GameCommon::is_self_relation(attacker, target) == true)
	{
		return false;
	}

	if (scene->camp_split() == true)
	{
		return attacker->camp_id() == target->camp_id();
	}

	if (attacker->is_team_pk()) // 队伍PK
	{
		JUDGE_RETURN(attacker->team_id() > 0, false);
		return attacker->team_id() == target->team_id();
	}

	if (attacker->is_league_pk()) // 帮派PK
	{
		JUDGE_RETURN(attacker->league_id() > 0, false);
		return attacker->league_id() == target->league_id();
	}

	return false;
}

int GameCommon::is_master_relation(GameFighter* attacker, GameFighter* target)
{
	GameFighter* master = attacker->fetch_self_owner();
	JUDGE_RETURN(master != NULL, false);

	return master->fighter_id() == target->fighter_id();
}

int GameCommon::is_monster_relation(GameFighter* attacker, GameFighter* target, Scene* scene)
{
	if (scene->camp_split() == true)
	{
		// 区分阵营
		JUDGE_RETURN(scene->is_validate_attack_monster(attacker, target) == true, false);
		return attacker->camp_id() != target->camp_id();
	}

	return target->fighter_sort() == GameEnum::FIGHTER_MONSTER;
}

int GameCommon::is_boss_relation(GameFighter* attacker, GameFighter* target, Scene* scene)
{
	if (scene->camp_split() == true)
	{
		// 区分阵营
		return attacker->camp_id() != target->camp_id();
	}

	return target->fighter_sort() == GameEnum::FIGHTER_BOSS;
}

bool GameCommon::is_base_skill(int skill_type)
{
	return (skill_type % 100) == 1;
}

bool GameCommon::is_jump_skill(int skill_type)
{
	return (skill_type % 100) == 2;
}

bool GameCommon::is_angry_skill(int skill_type)
{
	return (skill_type % 100) == 3;
}

bool GameCommon::is_base_skill(FighterSkill* skill)
{
	return GameCommon::is_base_skill(skill->__skill_type);
}

bool GameCommon::is_jump_skill(FighterSkill* skill)
{
	return GameCommon::is_jump_skill(skill->__skill_type);
}

bool GameCommon::is_angry_skill(FighterSkill* skill)
{
	return GameCommon::is_angry_skill(skill->__skill_type);
}

bool GameCommon::is_passive_prop_skill(int skill_type)
{
	return (skill_type / 100) == 3;
}

bool GameCommon::is_base_skill(const int skill_id, const int mover_type)
{
	const Json::Value& skill_json = CONFIG_INSTANCE->skill(skill_id);
	return GameCommon::is_base_skill(skill_json["server_type"].asInt());
}

bool GameCommon::is_boss(int monster_sort)
{
	const Json::Value& monster_config = CONFIG_INSTANCE->monster(monster_sort);
	JUDGE_RETURN(monster_config.empty() == false, false);

	return monster_config["boss"].asInt() == 1;
}

bool GameCommon::is_multi_target_skill(FighterSkill* fighter_skill)
{
	switch (fighter_skill->__aoe_type)
	{
	case GameEnum::SKILL_AOE_CUR_AIM_TARGET: // 当目标对象
	case GameEnum::SKILL_AOE_SELF_OWNER_TARGET:	// 自身主人
	case GameEnum::SKILL_AOE_SELF_TARGET:		// 自身
	{
		return false;
	}

	case GameEnum::SKILL_AOE_TARGET_CIRCLE:    	// 以目标为圆心
	case GameEnum::SKILL_AOE_SELF_CIRCLE:      	// 以自身为圆心
	case GameEnum::SKILL_AOE_SELF_SECTOR:      	// 以自身为扇形
	case GameEnum::SKILL_AOE_SELF_RECT:        	// 以自身为矩形中心
	case GameEnum::SKILL_AOE_SCENE_LEFT_SIDE:	// 自身屏幕左侧
	case GameEnum::SKILL_AOE_SCENE_RIGHT_SIDE:	// 自身屏幕右侧
	case GameEnum::SKILL_AOE_SELF_RING:			// 自身屏幕左侧
	case GameEnum::SKILL_AOE_TARGET_POINT_CIRCLE:
	{
		return true;
	}
	}

	return false;
}

bool GameCommon::check_config_value(const string& type_str, uint type_index)
{
    if (type_str.length() > type_index && type_str[type_index] == '1')
    {
    	return true;
    }
    else
    {
    	return false;
    }
}

void GameCommon::copy_player(OfflineRoleDetail* aim_detail, MapPlayerEx* player)
{
	GameCommon::copy_role_property(aim_detail, player);
	GameCommon::copy_fight_property(aim_detail, player);
}

void GameCommon::copy_beast(OfflineBeastDetail* aim_detail, MapBeast* beast)
{
	BeastDetail* beast_detail = beast->fetch_beast_detail();
	aim_detail->beast_name_ = beast_detail->beast_name_;
	aim_detail->beast_sort_ = beast_detail->beast_sort_;

	beast->fetch_prop_map(aim_detail->prop_map_);
	beast->fetch_skill_map(aim_detail->skill_set_);
}

void GameCommon::copy_role_property(OfflineRoleDetail* aim_detail, MapPlayerEx* player)
{
	aim_detail->role_id_ = player->role_id();
	aim_detail->role_info_ = player->role_detail();
    if (aim_detail->role_info_.__level <= 0)
    {
    	aim_detail->role_info_.__level = player->level();
    }

    aim_detail->cur_label_ = player->get_cur_label();
    aim_detail->magic_weapon_info_.__weapon_id = player->magic_weapon_id();
    aim_detail->magic_weapon_info_.__weapon_level = player->magic_weapon_lvl();
}

void GameCommon::copy_fight_property(OfflineRoleDetail* aim_detail, MapPlayerEx* player)
{
	FightDetail& src_detail = player->fight_detail();

	aim_detail->total_speed_ = player->speed_base_total_i();
	aim_detail->attack_lower_ = src_detail.__attack_lower_total(player);
	aim_detail->attack_upper_ = src_detail.__attack_upper_total(player);
	aim_detail->defence_lower_ = src_detail.__defence_lower_total(player);
	aim_detail->defence_upper_ = src_detail.__defence_upper_total(player);

	aim_detail->total_hit_ = src_detail.__hit_total(player);
	aim_detail->total_avoid_ = src_detail.__avoid_total(player);
	aim_detail->total_crit_ = src_detail.__crit_total(player);
	aim_detail->total_toughness_ = src_detail.__toughness_total(player);
	aim_detail->total_lucky_ = src_detail.__lucky_total(player);

	aim_detail->total_blood_ = src_detail.__blood_total(player);
	aim_detail->total_magic_ = src_detail.__magic_total(player);

	aim_detail->total_damage_ = src_detail.__damage_total(player);
	aim_detail->total_reduction_ = src_detail.__reduction_total(player);

	aim_detail->skill_map_.clear();
	aim_detail->skill_map_[player->base_skill_id()] = 1;
    aim_detail->shape_map_ = player->shape_detail();

    SkillMap &skill_map = player->fight_detail().__skill_map;
    for (SkillMap::iterator iter = skill_map.begin(); iter != skill_map.end(); ++iter)
    {
    	aim_detail->skill_map_[iter->first] = iter->second->__level;
    }
}

void GameCommon::copy_player(Proto30400432* trans_info, const OfflineRoleDetail& offline_detail)
{
	ProtoRoleInfo* proto_role = trans_info->mutable_role_info();
	proto_role->set_role_id(offline_detail.role_id_);
	proto_role->set_role_name(offline_detail.role_info_.name());

	proto_role->set_role_sex(offline_detail.role_info_.__sex);
	proto_role->set_role_level(offline_detail.role_info_.__level);
	proto_role->set_role_career(offline_detail.role_info_.__career);

	ProtoFightPro* proto_fight = trans_info->mutable_fight_prop();
	proto_fight->set_hit(offline_detail.total_hit_);
	proto_fight->set_avoid(offline_detail.total_avoid_);
	proto_fight->set_crit(offline_detail.total_crit_);
	proto_fight->set_toughness(offline_detail.total_toughness_);
	proto_fight->set_blood(offline_detail.total_blood_);
	proto_fight->set_magic(offline_detail.total_magic_);
	proto_fight->set_speed(offline_detail.total_speed_);
	proto_fight->set_damage(offline_detail.total_damage_);
	proto_fight->set_reduction(offline_detail.total_reduction_);

	for (IntMap::const_iterator iter = offline_detail.skill_map_.begin();
			iter != offline_detail.skill_map_.end(); ++iter)
	{
		ProtoPairObj* pair_obj = trans_info->add_skill_set();

		pair_obj->set_obj_id(iter->first);
		pair_obj->set_obj_value(iter->second);
	}

	for (IntMap::const_iterator iter = offline_detail.shape_map_.begin();
			iter != offline_detail.shape_map_.end(); ++iter)
	{
		ProtoPairObj* pair_obj = trans_info->add_shape_set();

		pair_obj->set_obj_id(iter->first);
		pair_obj->set_obj_value(iter->second);
	}

    trans_info->set_kill_num(offline_detail.kill_info_.first);
    trans_info->set_killing_value(offline_detail.kill_info_.second);
    trans_info->set_cur_label(offline_detail.cur_label_);
    trans_info->set_weapon_id(offline_detail.magic_weapon_info_.__weapon_id);
    trans_info->set_weapon_level(offline_detail.magic_weapon_info_.__weapon_level);
    trans_info->set_equip_refine_lvl(offline_detail.equie_refine_lvl_);
}

void GameCommon::copy_beast(Proto30400432* trans_info, const OfflineBeastDetail& offline_detail)
{
	JUDGE_RETURN(offline_detail.beast_id_ > 0, ;);
	JUDGE_RETURN(offline_detail.beast_sort_ > 0, ;);

	ProtoOfflineBeast* proto_beast = trans_info->mutable_offline_beast();
	proto_beast->set_beast_id(offline_detail.beast_id_);
	proto_beast->set_beast_sort(offline_detail.beast_sort_);
	proto_beast->set_beast_name(offline_detail.beast_name_);

	for (IntVec::const_iterator iter = offline_detail.skill_set_.begin();
			iter != offline_detail.skill_set_.end(); ++iter)
	{
		ProtoPairObj* tmp = proto_beast->add_skill_set();
		tmp->set_obj_id(*iter);
		tmp->set_obj_value( 1 );
	}

	for (IntMap::const_iterator iter = offline_detail.prop_map_.begin();
			iter != offline_detail.prop_map_.end(); ++iter)
	{
		ProtoPairObj* pair_obj = proto_beast->add_prop_set();
		pair_obj->set_obj_id(iter->first);
		pair_obj->set_obj_value(iter->second);
	}
}

int GameCommon::fetch_name_color(int kill_num, int killing_value)
{
	static int PURPLE_START = CONFIG_INSTANCE->killing(GameEnum::NAME_PURPLE)["kill_num"].asInt();
	static int BLUE_START = CONFIG_INSTANCE->killing(GameEnum::NAME_BLUE)["kill_num"].asInt();

	static int YELLOW_START = CONFIG_INSTANCE->killing(GameEnum::NAME_YELLOW)["yellow_start"].asInt();
	static int RED_START = CONFIG_INSTANCE->killing(GameEnum::NAME_RED)["red_start"].asInt();

	if (kill_num >= PURPLE_START)
	{
		return GameEnum::NAME_PURPLE;
	}

	if (kill_num >= BLUE_START)
	{
		return GameEnum::NAME_BLUE;
	}

	if (killing_value < YELLOW_START)
	{
		return GameEnum::NAME_WHITE;
	}

	if (killing_value < RED_START)
	{
		return GameEnum::NAME_YELLOW;
	}

	return GameEnum::NAME_RED;
}

int GameCommon::fetch_kill_value(int name_color)
{
	return CONFIG_INSTANCE->killing(name_color)["kill_value"].asInt();
}

int GameCommon::fetch_base_skill(int mover_type, int flag)
{
	static int MALE_BASE_SKILL	 	= 400011004;
	static int FEMALE_BASE_SKIL		= 400012004;
	static int MONSTER_BASE_SKILL	= 400030001;
	static int R_MONSTER_BASE_SKILL	= 400030002;
	static int BEAST_BASE_SKILL 	= 400070001;
	static int MAGIC_BAST_SKILL		= 400090001;

	switch (mover_type)
	{
	case MOVER_TYPE_PLAYER:
	{
		return flag == 1 ? MALE_BASE_SKILL : FEMALE_BASE_SKIL;
	}
	case MOVER_TYPE_BEAST:
	{
		return flag == 0 ? BEAST_BASE_SKILL : MAGIC_BAST_SKILL;
	}
	case MOVER_TYPE_MONSTER:
	{
		return flag == 1 ? MONSTER_BASE_SKILL : R_MONSTER_BASE_SKILL;
	}
	}

	return 0;
}

bool GameCommon::is_operate_activity(int activity_id)
{
	return (activity_id / 10000) == 5;
}

bool GameCommon::is_daily_activity(int activity_id)
{
	return (activity_id / 10000) == 7;
}

bool GameCommon::validate_time_span(Int64 last_tick, int span_time)
{
	Int64 now_span = ::time(NULL) - last_tick;
	return now_span >= span_time;
}

int GameCommon::validate_array_index(const Json::Value &array_conf, int index)
{
	return index >= 0 && uint(index) < array_conf.size();
}

int GameCommon::adjust_array_index(const Json::Value &array_conf, int index)
{
	JUDGE_RETURN(index >= 0, 0);

	int max_size = array_conf.size();
	JUDGE_RETURN(index >= max_size, index);

	return max_size - 1;
}

const Json::Value &GameCommon::fetch_index_config(const Json::Value &array_conf, int index)
{
	JUDGE_RETURN(array_conf.size() > 0, Json::Value::null);

	int adjust_index = GameCommon::adjust_array_index(array_conf, index);
	return array_conf[adjust_index];
}

tm GameCommon::fetch_cur_tm()
{
	tm cur_tm;
	GameCommon::fetch_cur_tm(cur_tm);

	return cur_tm;
}

void GameCommon::fetch_cur_tm(tm& cur_tm)
{
	time_t now_tick = ::time(NULL);
	::localtime_r(&now_tick, &cur_tm);
}

int GameCommon::left_time(Int64 finish_tick)
{
	Int64 now_tick = ::time(NULL);

	if (finish_tick > now_tick)
	{
		return finish_tick - now_tick;
	}
	else
	{
		return 0;
	}
}

int GameCommon::left_time_span(Int64 last_tick, int span_time)
{
	int passed_time = ::time(NULL) - last_tick;
	return std::max(0, span_time - passed_time);
}

int GameCommon::next_minute()
{
	tm now_tm = GameCommon::fetch_cur_tm();
	return Time_Value::MINUTE - now_tm.tm_sec;
}

int GameCommon::next_hour()
{
	tm now_tm = GameCommon::fetch_cur_tm();
	return Time_Value::HOUR - now_tm.tm_min * Time_Value::MINUTE - now_tm.tm_sec;
}

int GameCommon::next_day()
{
	tm now_tm = GameCommon::fetch_cur_tm();
	return Time_Value::DAY - now_tm.tm_hour * Time_Value::HOUR
			- now_tm.tm_min * Time_Value::MINUTE - now_tm.tm_sec;
}

int GameCommon::next_week()
{
	tm now_tm = GameCommon::fetch_cur_tm();
	return Time_Value::WEEK - now_tm.tm_wday * Time_Value::DAY - now_tm.tm_hour * Time_Value::HOUR
			- now_tm.tm_min * Time_Value::MINUTE - now_tm.tm_sec;
}

int GameCommon::next_month()
{
	return GameCommon::next_month_start_zero() - ::time(NULL);
}

time_t GameCommon::today_zero()
{
	return GameCommon::day_zero(::time(NULL));
}

time_t GameCommon::week_zero()
{
	tm zero_tm = GameCommon::fetch_cur_tm();
	zero_tm.tm_wday = 0;
	zero_tm.tm_hour = 0;
	zero_tm.tm_min 	= 0;
	zero_tm.tm_sec 	= 0;

	time_t week_zero = ::mktime(&zero_tm);
	int day_num = zero_tm.tm_wday == 0 ? 7 : zero_tm.tm_wday;

	week_zero -= (day_num - 1) * Time_Value::DAY;
	return week_zero;
}

time_t GameCommon::day_zero(time_t tick)
{
	tm zero_tm;
	::localtime_r(&tick, &zero_tm);

	zero_tm.tm_hour = zero_tm.tm_min = zero_tm.tm_sec = 0;
	return ::mktime(&zero_tm);
}

time_t GameCommon::next_month_start_zero()
{
    struct tm p;
    ::memset(&p, 0, sizeof(struct tm));

	tm cur_tm = GameCommon::fetch_cur_tm();
	if (cur_tm.tm_mon == 11)
	{
		p.tm_year = cur_tm.tm_year + 1;
		p.tm_mon = 0;
	}
	else
	{
		p.tm_year = cur_tm.tm_year;
		p.tm_mon = cur_tm.tm_mon + 1;
	}

	p.tm_mday = 1;
	return ::mktime(&p);
}

int GameCommon::is_current_day(Int64 check_tick)
{
	Int64 zero_tick = GameCommon::today_zero();
	return check_tick >= zero_tick;
}

int GameCommon::day_interval(Int64 first_tick, Int64 second_tick)
{
	Int64 first = GameCommon::day_zero(first_tick);
	Int64 second = GameCommon::day_zero(second_tick);

	return (std::abs(long(first - second)) / Time_Value::DAY);
}

int GameCommon::day_interval(int second, int type)
{
	JUDGE_RETURN(second > 0, 0);

	if (type == 0)
	{
		return GAME_PAGE_SIZE(second, Time_Value::DAY);
	}
	else
	{
		return second / Time_Value::DAY;
	}
}

int GameCommon::is_continue_day(Int64 check_tick)
{
	return GameCommon::is_continue_day(::time(NULL), check_tick);
}

int GameCommon::is_continue_day(Int64 first_tick, Int64 second_tick)
{
	Int64 first = GameCommon::day_zero(first_tick);
	Int64 second = GameCommon::day_zero(second_tick);

	return std::abs(long(first - second)) == Time_Value::DAY;
}

bool GameCommon::is_same_week(const Time_Value &tv1)
{
	tm week_tm = GameCommon::fetch_cur_tm();
	week_tm.tm_wday = 0;
	week_tm.tm_hour = 0;
	week_tm.tm_min  = 0;
	week_tm.tm_sec  = 0;

	time_t week_zero = ::mktime(&week_tm);

	int day_num = week_tm.tm_wday == 0 ? 7 : week_tm.tm_wday;
	week_zero -= (day_num - 1) * Time_Value::DAY;

	return tv1.sec() >= week_zero;
}

bool GameCommon::is_same_month(const Time_Value &tv1, const Time_Value &tv2)
{
	tm tm1, tm2;
	time_t t1 = tv1.sec(), t2 = tv2.sec();

	::localtime_r(&t1, &tm1);
	::localtime_r(&t2, &tm2);

	if (tm1.tm_year == tm2.tm_year && tm1.tm_mon == tm2.tm_mon)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void GameCommon::make_up_reward_items(RewardInfo& reward_info, int reward_id)
{
	const Json::Value &reward_json = CONFIG_INSTANCE->reward(reward_id);
	JUDGE_RETURN(reward_json.empty() == false, ;);
	GameCommon::make_up_reward_items(reward_info, reward_json);
}

void GameCommon::make_up_reward_items(RewardInfo& reward_info, const ThreeObjVec& obj_vec)
{
	JUDGE_RETURN(obj_vec.empty() == false, ;);

	for (ThreeObjVec::const_iterator iter = obj_vec.begin(); iter != obj_vec.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->id_ > 0);
		const Json::Value& reward_json = CONFIG_INSTANCE->reward(iter->id_);
		GameCommon::make_up_reward_items(reward_info, reward_json);
	}
}

void GameCommon::make_up_reward_items(RewardInfo& reward_info, const Json::Value& reward_json)
{
	reward_info.id_ = reward_json["id"].asInt();
	GameCommon::make_up_fix_reward_items(reward_info, reward_json);

	int total_size = 9;
	for (int i = 1; i <= total_size; ++i)
	{
		GameCommon::make_up_rand_reward_items(reward_info, i, reward_json);
	}
}

void GameCommon::make_up_fix_reward_items(RewardInfo& reward_info, const Json::Value& reward_json)
{
	JUDGE_RETURN(reward_json.isMember("fix_goods_list") == true, ;);
	return GameCommon::make_up_conf_items(reward_info, reward_json["fix_goods_list"]);
}

int GameCommon::make_up_rand_reward_items(RewardInfo& reward_info, int rand_type,
		const Json::Value& reward_json)
{
	static string field_str[] = {
			"choose_times_a", "choose_goods_list_a", "choose_count_a",
			"choose_times_b", "choose_goods_list_b", "choose_count_b",
			"choose_times_c", "choose_goods_list_c", "choose_count_c",
			"choose_times_d", "choose_goods_list_d", "choose_count_d",
			"choose_times_e", "choose_goods_list_e", "choose_count_e",
			"choose_times_f", "choose_goods_list_f", "choose_count_f",
			"choose_times_g", "choose_goods_list_g", "choose_count_g",
			"choose_times_h", "choose_goods_list_h", "choose_count_h",
			"choose_times_i", "choose_goods_list_i", "choose_count_i"
	};

	int index = (rand_type - 1) * 3;

	string first_str = field_str[index];
	JUDGE_RETURN(reward_json.isMember(first_str) == true, -1);

	JUDGE_RETURN(GameCommon::check_reward_open_condition(
			reward_info, rand_type, reward_json) == true, -1);

	string sec_str = field_str[index + 1];
	string third_str = field_str[index + 2];

	int total_rand = reward_json[first_str].asInt(); //总概率
	int total_size = reward_json[sec_str].size();
	JUDGE_RETURN(total_size > 0, -1);

	int rand_times = std::max<int>(reward_json[third_str].asInt(), 1);	//次数
	for (int times_index = 0; times_index < rand_times; ++times_index)
	{
		JUDGE_CONTINUE(GameCommon::validate_cur_rand(total_rand) == true);

		if (rand_type == 8)
		{
			GameCommon::choose_goods_list_b(reward_info, reward_json[sec_str]);
		}
		else
		{
			GameCommon::choose_goods_list(reward_info, reward_json[sec_str]);
		}
	}

	return reward_info.select_index_;
}

int GameCommon::choose_goods_list(RewardInfo& reward_info, const Json::Value& choose_json)
{
	int is_array = true;
	int total_size = choose_json.size();

	int total_cur_rand = 0;
	int cur_rand = std::rand() % GameEnum::DAMAGE_ATTR_PERCENT;

	if (choose_json[0u].isArray() == false)
	{
		is_array = false;
		total_size = 1;
	}

	for (int i = 0; i < total_size; ++i)
	{
		ItemObj obj = (is_array == true)
				? (GameCommon::make_up_itemobj(choose_json[i]))
				: (GameCommon::make_up_itemobj(choose_json));
		JUDGE_CONTINUE(obj.validate() == true);

		total_cur_rand += obj.rand_;
		JUDGE_CONTINUE(cur_rand < total_cur_rand);

		reward_info.add_rewards(obj);
		reward_info.select_index_ = i;
		break;
	}

	return 0;
}

int GameCommon::choose_goods_list_b(RewardInfo& reward_info, const Json::Value& choose_json)
{
	JUDGE_RETURN(reward_info.has_player() == true, -1);

	ItemObjVec obj_vec;
	GameCommon::make_up_conf_items_b(obj_vec, choose_json);

	int total_size = obj_vec.size();
	JUDGE_RETURN(total_size > 0, -1);

	int select_index = -1;

	int total_rand = 0;		//计算概率总基数
	int total_cur_rand = 0;

	IntMap rand_index_map;
	IntMap attend_index_map;
	for (int i = 0; i < total_size; ++i)
	{
		if (obj_vec[i].rand_start_times_ > 0)
		{
			int history_times = reward_info.history_use_times(obj_vec[i].id_);
			if (obj_vec[i].no_rand_times_ > 0
					&& history_times >= obj_vec[i].no_rand_times_)
			{
				select_index = i;
				attend_index_map[i] = true;
			}
			else if (history_times >= obj_vec[i].rand_start_times_)
			{
				total_rand += obj_vec[i].rand_;
				attend_index_map[i] = true;
			}

			rand_index_map[i] = true;
		}
		else
		{
			total_rand += obj_vec[i].rand_;
		}
	}

	if (select_index >= 0)
	{
		reward_info.add_rewards(obj_vec[select_index]);
		reward_info.select_index_ = select_index;
	}
	else
	{
		JUDGE_RETURN(total_rand > 0, -1);

		int cur_rand = std::rand() % total_rand;
		for (int i = 0; i < total_size; ++i)
		{
			if (obj_vec[i].rand_start_times_ > 0)
			{
				JUDGE_CONTINUE(attend_index_map.count(i) > 0);
			}

			total_cur_rand += obj_vec[i].rand_;
			JUDGE_CONTINUE(cur_rand < total_cur_rand);

			reward_info.add_rewards(obj_vec[i]);
			reward_info.select_index_ = i;
			select_index = i;

			break;
		}
	}

	if (attend_index_map.count(select_index) > 0)
	{
		reward_info.remve_use_times(obj_vec[select_index].id_);
	}

	for (IntMap::iterator iter = rand_index_map.begin();
			iter != rand_index_map.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->first != select_index);
		reward_info.add_use_times(obj_vec[iter->first].id_);
	}

	return 0;
}

int GameCommon::check_reward_open_condition(RewardInfo& reward_info, int rand_type,
		const Json::Value& reward_json)
{
	switch (rand_type)
	{
	case 5:	//开服天数
	{
		JUDGE_RETURN(reward_info.player_ != NULL, false);
		JUDGE_RETURN(reward_json.isMember("open_condition_value_e") == true, false);

		int need_day = reward_json["open_condition_value_e"].asInt() - 1;
		return reward_info.player_->role_detail().validate_server_open_day(std::max<int>(need_day, 0));
	}
	case 6:	//节日活动
	{
		JUDGE_RETURN(reward_json.isMember("open_condition_value_f") == true, false);

		int act_index = reward_json["open_condition_value_f"].asInt();
		if (reward_info.player_ != NULL)
		{
			return reward_info.player_->validate_festival_activity(act_index);
		}
		else if (reward_info.logic_player_ != NULL)
		{
			return reward_info.logic_player_->logic_validate_festival_activity(act_index);
		}
		else
		{
			return false;
		}
	}
	case 7:	//合服天数
	{
		JUDGE_RETURN(reward_info.has_player() == true, false);
		JUDGE_RETURN(reward_json.isMember("open_condition_value_g") == true, false);

		if (reward_info.player_ != NULL)
		{
			int max_day = reward_json["open_condition_value_g"].asInt();
			return reward_info.player_->role_detail().validate_max_combine_day(max_day);
		}

		if (reward_info.logic_player_ != NULL)
		{
			int max_day = reward_json["open_condition_value_g"].asInt();
			return reward_info.logic_player_->role_detail().validate_max_combine_day(max_day);
		}

		return false;
	}
	case 9:	//五一等大型活动
	{
		JUDGE_RETURN(reward_info.has_player() == true, false);

		if (reward_info.player_ != NULL)
		{
			return reward_info.player_->validate_big_act_time();
		}

		if (reward_info.logic_player_ != NULL)
		{
			return reward_info.logic_player_->logic_validate_big_act_time();
		}

		return false;
	}
	default:
	{
		return true;
	}
	}
}

void GameCommon::make_up_conf_items(ItemObjVec& items, const Json::Value& json)
{
	int is_array = true;
	int total_size = json.size();

	if (json[0u].isArray() == false)
	{
		is_array = false;
		total_size = 1;
	}

	items.reserve(total_size);
	for (int i = 0; i < total_size; ++i)
	{
		ItemObj obj = (is_array == true)
				? GameCommon::make_up_itemobj(json[i])
				: GameCommon::make_up_itemobj(json);
		JUDGE_CONTINUE(obj.validate() == true);
		items.push_back(obj);
	}
}

void GameCommon::make_up_conf_items_b(ItemObjVec& items, const Json::Value& json)
{
	int total_size = json.size();
	items.reserve(total_size);

	for (int i = 0; i < total_size; ++i)
	{
		ItemObj obj = GameCommon::make_up_itemobj_b(json[i]);
		JUDGE_CONTINUE(obj.validate() == true);
		items.push_back(obj);
	}
}

void GameCommon::make_up_conf_items(RewardInfo& reward_info, const Json::Value& json)
{
	int total = json.size();
	for (int i = 0; i < total; ++i)
	{
		ItemObj obj = GameCommon::make_up_itemobj(json[i]);
		JUDGE_CONTINUE(obj.validate() == true);
		reward_info.add_rewards(obj);
	}
}

void GameCommon::json_to_int_set(IntSet& i_set, const Json::Value& json)
{
	if (json.isArray() == true)
	{
		int total = json.size();
		for (int i = 0; i < total; ++i)
		{
			i_set.insert(json[i].asInt());
		}
	}
	else
	{
		i_set.insert(json.asInt());
	}
}


bool GameCommon::validate_cur_rand(int use_rate)
{
	JUDGE_RETURN(use_rate > 0, false);

	int cur_rate = ::rand() % GameEnum::DAMAGE_ATTR_PERCENT;
	return cur_rate < use_rate;
}

int GameCommon::rand_list_num(const Json::Value& rand_conf)
{
	int total_size = rand_conf.size();
	int cur_rate = ::rand() % GameEnum::DAMAGE_ATTR_PERCENT;

	int rand_num = 0;
	for (int i = 0; i < total_size; ++i)
	{
		rand_num += rand_conf[i].asInt();
		JUDGE_CONTINUE(cur_rate < rand_num);
		return i;
	}

	return 0;
}

bool GameCommon::validate_range_rand(int key, const Json::Value& rand_conf)
{
	int total_size = rand_conf.size();
	JUDGE_RETURN(total_size > 0, false);

	key = std::min<int>(rand_conf[total_size - 1][1u].asInt(), key);
	for (int i = 0; i < total_size; ++i)
	{
		int start = rand_conf[i][0u].asInt();
		int end = rand_conf[i][1u].asInt();
		JUDGE_CONTINUE(key >= start && key <= end);

		int use_rate = rand_conf[i][2u].asInt();
		return GameCommon::validate_cur_rand(use_rate);
	}

	return false;
}

int GameCommon::game_page_size(int total_size, int page_count)
{
	JUDGE_RETURN(total_size > 0, GameEnum::DEFAULT_TOTAL_PAGE);
	return GAME_PAGE_SIZE(total_size, page_count);
}

void GameCommon::game_page_info(PageInfo& page_info, int cur_page,
		int total_count, int page_size)
{
	//起始页码为1
	cur_page = std::max(cur_page, 1);

	page_info.total_page_ = GameCommon::game_page_size(total_count, page_size);
	page_info.cur_page_ = std::min<int>(cur_page, page_info.total_page_);

	page_info.start_index_ = (page_info.cur_page_ - 1) * page_size;
	page_info.total_count_ = total_count;
}

DBShopMode* GameCommon::pop_shop_mode(int trans_recogn)
{
	DBShopMode* shop_mode = POOL_MONITOR->shop_mode_pool()->pop();
	JUDGE_RETURN(shop_mode != NULL, NULL);

	shop_mode->recogn_ = trans_recogn;
	return shop_mode;
}

bool GameCommon::is_male(int sex_type)
{
	return sex_type == GameEnum::SEX_MALE;
}

bool GameCommon::is_female(int sex_type)
{
	return sex_type == GameEnum::SEX_FEMALE;
}

bool GameCommon::is_max_level(int level)
{
	return level >= MAX_PLAYER_LEVEL;
}

bool GameCommon::is_rotary_table_goods(int item_id)
{
	const Json::Value& prop_conf = CONFIG_INSTANCE->prop(item_id);
	JUDGE_RETURN(prop_conf != Json::Value::null, false);

	string prop_name = prop_conf["effect"]["name"].asString();
	JUDGE_RETURN(prop_name == PropName::ROTARY_TABLE, false);

	return true;
}

void GameCommon::set_rotary_table_index(PackageItem* pack_item)
{
	int total_rand = 0;
	int rand_value = std::rand() % 10000;

	const Json::Value& effect_conf = CONFIG_INSTANCE->prop(pack_item->__id)["effect"];
	for (uint i = 0; i < effect_conf["item_list"].size(); ++i)
	{
		total_rand += effect_conf["item_list"][i][3u].asDouble() * 100;
		JUDGE_CONTINUE(rand_value < total_rand);

		pack_item->__rotary_table.prop_index_ = i + 1;
		break;
	}
}

bool GameCommon::item_is_bind(int item_id)
{
	const Json::Value& item_conf = CONFIG_INSTANCE->item(item_id);
	return item_conf["bind"].asInt() == GameEnum::ITEM_BIND;
}

bool GameCommon::item_is_bind(PackageItem* pack_item)
{
	JUDGE_RETURN(pack_item != NULL, true);
	return pack_item->__bind != GameEnum::ITEM_NO_BIND;
}

bool GameCommon::check_item_effect_value(PackageItem* pack_item, int check_type)
{
	const Json::Value& effect_conf = pack_item->conf()["effect_prop"];
	JUDGE_RETURN(effect_conf.empty() == false, false);

	return effect_conf["change"].asInt() == check_type;
}

bool GameCommon::item_is_equipment(int item_id)
{
	return item_id / GameEnum::ID_NAME_EQUIPMENT == 1;
}

bool GameCommon::item_can_overlap(int item_id)
{
	return GameCommon::item_overlap_count(item_id) == 1 ? false : true;
}

int GameCommon::item_overlap_count(int item_id)
{
	const Json::Value& item_conf = CONFIG_INSTANCE->item(item_id);
	JUDGE_RETURN(item_conf != Json::Value::null, 1);

	int overlap_count = 1;
	if (item_conf.isMember("overlap") == true)
	{
		overlap_count = item_conf["overlap"].asInt();
	}
	else if (GameCommon::item_is_equipment(item_id) == false)
	{
		overlap_count = GameEnum::MAX_OVERLAP_SIZE;
	}

	return overlap_count;
}

bool GameCommon::item_can_onsell(PackageItem* pack_item)
{
	JUDGE_RETURN(pack_item != NULL, false);
	JUDGE_RETURN(GameCommon::item_is_bind(pack_item) == false, false);

	return CONFIG_INSTANCE->market(pack_item->__id) != Json::Value::null;
}

int GameCommon::fetch_item_type(int item_id)
{
	const Json::Value& item_conf = CONFIG_INSTANCE->item(item_id);
	return item_conf["type"].asInt();
}

std::string GameCommon::item_name(int item_id)
{
	const Json::Value& item_conf = CONFIG_INSTANCE->item(item_id);
	return item_conf["name"].asString();
}

std::string GameCommon::item_name_color(int item_id)
{
	const Json::Value& item_conf = CONFIG_INSTANCE->item(item_id);

	int item_color = item_conf["color"].asInt();
	std::string color_name = CONFIG_INSTANCE->tiny("item_color")[item_color].asString();

	return "[" + color_name +"]" + item_conf["name"].asString() + "[-]";
}

std::string GameCommon::make_up_color_name(int color_id)
{
	static string color_name_set[] = {"color1", "color2", "color3",
			"color4", "color5", "color6", "color7", "color8", "color9",
			"color10", "color11", "color12", "color13", "color14", "color15",
			"color16", "color17", "color18", "color19", "color20"};
	return color_name_set[color_id - 1];
}

Money GameCommon::item_cost(const int item_id, const int item_amount, const int money_type)
{
    return Money();
}

int GameCommon::fetch_label_type(int label_id)
{
	const Json::Value& label_json = CONFIG_INSTANCE->label(label_id);
	JUDGE_RETURN(label_json != Json::Value::null, 0);
	return label_json["limit_type"].asInt();
}

int GameCommon::json_value(const Json::Value& json_value,
		const std::string& field_name)
{
	JUDGE_RETURN(json_value.isMember(field_name) == true, 0);
	return json_value[field_name].asInt();
}

ItemObj GameCommon::make_up_itemobj(const Json::Value& conf, int times)
{
	ItemObj obj;
	obj.id_ 	= conf[0u].asInt();
	obj.amount_ = conf[1u].asInt() * times;

	int total_size = conf.size();
	if (total_size > 2)
	{
		obj.bind_ = conf[2u].asInt();
	}

	if (total_size > 3)
	{
		obj.rand_ = conf[3u].asInt();
	}

	return obj;
}

ItemObj GameCommon::make_up_itemobj_b(const Json::Value& conf)
{
	ItemObj obj;
	obj.id_ 	= conf["id"].asInt();
	obj.amount_ = conf["amount"].asInt();
	obj.bind_ 	= conf["bind"].asInt();
	obj.rand_ 	= conf["rand"].asInt();

	if (conf.isMember("rand_start_times"))
	{
		obj.rand_start_times_ = conf["rand_start_times"].asInt();
	}

	if (conf.isMember("no_rand_times"))
	{
		obj.no_rand_times_ = conf["no_rand_times"].asInt();
	}

	return obj;
}

Money GameCommon::make_up_money(int amount, const Json::Value& json_value)
{
	int money_type = GameCommon::json_value(json_value, "money_type");
	return GameCommon::make_up_money(amount, money_type);
}

Money GameCommon::make_up_money(int amount, int money_type)
{
	switch (money_type)
	{
	case GameEnum::MONEY_BIND_COPPER:
		return Money(0, 0, 0, amount);

	case GameEnum::MONEY_UNBIND_COPPER:
		return Money(0, 0, amount);

	case GameEnum::MONEY_BIND_GOLD:
		return Money(0, amount);

	case GameEnum::MONEY_UNBIND_GOLD:
		return Money(amount);
	}

	return Money(0);
}

bool GameCommon::validate_money_type(int money_type)
{
	switch (money_type)
	{
	case GameEnum::MONEY_BIND_COPPER:
	case GameEnum::MONEY_UNBIND_COPPER:
	case GameEnum::MONEY_BIND_GOLD:
	case GameEnum::MONEY_UNBIND_GOLD:
		return true;
	}

	return false;
}

void GameCommon::make_up_money_times(Money& money, int add_times)
{
	money.__bind_copper *= add_times;
	money.__copper *= add_times;
	money.__bind_gold *= add_times;
	money.__gold *= add_times;
}

int GameCommon::calc_pack_money_exchange_rate(int money_type)
{
//	const Json::Value& cfg = CONFIG_INSTANCE->tiny("money_exchange_rate");
//	JUDGE_RETURN(cfg != Json::Value::null, GameEnum::GOLD_EXCHANGE_COPPER);
//
//	int money_rate = 0;
//	if(money_type == GameEnum::MONEY_BIND_COPPER)
//	{
//		money_rate = cfg["gold_to_bind_copper"].asInt();
//		if(money_rate == 0)
//			money_rate = GameEnum::GOLD_EXCHANGE_BIND_COPPER;
//	}
//	if(money_type == GameEnum::MONEY_UNBIND_COPPER)
//	{
//		money_rate = cfg["gold_to_copper"].asInt();
//		if(money_rate == 0)
//			money_rate = GameEnum::GOLD_EXCHANGE_COPPER;
//	}
//	return money_rate;
	return 10;
}

int GameCommon::calc_pack_money_exchange_need_gold(Money& need_money, const Money& own_money, int money_rate)
{
	GameCommon::adjust_money(need_money, own_money);
	JUDGE_RETURN(need_money.__copper - own_money.__copper > 0, 0);

	int diff = need_money.__copper - own_money.__copper;
	int need_gold = diff % money_rate == 0 ?
			diff / money_rate : (diff / money_rate + 1);

	return need_gold;
}

int GameCommon::check_money_adjust_with_gold(Money& need_money, const Money& own_money)
{
	if(need_money.__bind_copper > own_money.__bind_copper &&
			need_money.__bind_copper > own_money.__bind_copper + own_money.__copper)
	{
		return 1;
	}

	if(need_money.__copper > own_money.__copper)
		return 1;

	return 0;
}

int GameCommon::check_copper_adjust_exchange_gold(const Money &need_money, const Money &own_money, int *use_gold, int *buy_bind_copper)
{
    int need_copper = need_money.__copper + need_money.__bind_copper;
    int cal_buy_copper = std::max(int(need_copper - own_money.__copper - own_money.__bind_copper), 0);
    JUDGE_RETURN(cal_buy_copper > 0, 0);

    int money_rate = GameCommon::calc_pack_money_exchange_rate(GameEnum::MONEY_BIND_COPPER);
    int need_gold = (cal_buy_copper + money_rate - 1) / money_rate;

    if (use_gold)
        *use_gold = need_gold;
    if (buy_bind_copper)
        *buy_bind_copper = need_gold * money_rate;
    return 0;
}

int GameCommon::commercial_recogn_type(int recogn)
{
	switch(recogn)
	{
	case RETURN_MARKET_ONSELL:
	case RETURN_MARKET_CONSELL:
	   	return GameEnum::COMM_SYS_AUCTION;

	case RETURN_SHOP_BUY_BACK:
	   	return GameEnum::COMM_SYS_SHOP;

	default:
		return recogn;
	}
}


void GameCommon::adjust_money(Money& need_money, const Money& own_money, int adjust_type)
{
	switch (adjust_type)
	{
	case GameEnum::MONEY_ADJUST_DEFAUL:
	{
		GameCommon::adjust_gold(need_money, own_money);
		break;
	}

	case GameEnum::MONEY_ADJUST_COPPER:
	{
		GameCommon::adjust_copper(need_money, own_money);
		break;
	}

	case GameEnum::MONEY_ADJUST_GOLD:
	{
		GameCommon::adjust_gold(need_money, own_money);
		break;
	}
	}
}

void GameCommon::adjust_copper(Money& need_money, const Money& own_money)
{
	JUDGE_RETURN(own_money.__bind_copper < need_money.__bind_copper, ;);
    JUDGE_RETURN(need_money.__bind_copper <= (own_money.__bind_copper + own_money.__copper), ;);

	int need_copper = need_money.__bind_copper;
	need_money.__bind_copper = own_money.__bind_copper;
	need_money.__copper += need_copper - own_money.__bind_copper;
}

void GameCommon::adjust_gold(Money& need_money, const Money& own_money)
{
	JUDGE_RETURN(own_money.__bind_gold < need_money.__bind_gold, ;);
    JUDGE_RETURN(need_money.__bind_gold <= (own_money.__bind_gold + own_money.__gold), ;);

	int need_gold = need_money.__bind_gold;
	need_money.__bind_gold = own_money.__bind_gold;
	need_money.__gold += need_gold - own_money.__bind_gold;
}

int GameCommon::fetch_cur_min_sec()
{
	tm now_tm;

	time_t now_tick = ::time(NULL);
	::localtime_r(&now_tick, &now_tm);

	return now_tm.tm_sec;
}

int GameCommon::fetch_cur_sec(int type)
{
	if (type == GameEnum::DAILY_ACTIVITY)
	{
		return GameCommon::fetch_cur_day_sec();
	}
	else
	{
		return GameCommon::fetch_cur_week_sec();
	}
}

int GameCommon::fetch_cur_combine_time()
{
	tm now_tm;

	time_t now_tick = ::time(NULL);
	::localtime_r(&now_tick, &now_tm);

	return now_tm.tm_hour * 100 + now_tm.tm_min;
}

int GameCommon::fetch_cur_hour_sec()
{
	tm now_tm;

	time_t now_tick = ::time(NULL);
	::localtime_r(&now_tick, &now_tm);

	return now_tm.tm_min * Time_Value::MINUTE + now_tm.tm_sec;
}

int GameCommon::fetch_cur_day_sec()
{
	tm now_tm;

	time_t now_tick = ::time(NULL);
	::localtime_r(&now_tick, &now_tm);

	return now_tm.tm_hour * Time_Value::HOUR
			+ now_tm.tm_min * Time_Value::MINUTE + now_tm.tm_sec;
}

int GameCommon::fetch_day_sec(int day_time)
{
	int hour = day_time / 100;
	int minute = day_time % 100;

	return hour * Time_Value::HOUR + minute * Time_Value::MINUTE;
}

int GameCommon::fetch_cur_week_sec()
{
	tm now_tm;

	time_t now_tick = ::time(NULL);
	::localtime_r(&now_tick, &now_tm);

	return now_tm.tm_wday * Time_Value::DAY + now_tm.tm_hour * Time_Value::HOUR
			+ now_tm.tm_min * Time_Value::MINUTE + now_tm.tm_sec;
}

int GameCommon::fetch_week_sec(int week_time)
{
	int day = week_time / 10000;
	int hour = (week_time % 10000) / 100;

	return day * Time_Value::DAY + hour * Time_Value::HOUR
			+ (week_time % 100) * Time_Value::MINUTE;
}

Time_Value GameCommon::fetch_time_value(double time_tick)
{
	return Time_Value::gettime(time_tick);
}

Time_Value GameCommon::fetch_add_time_value(double time_tick)
{
	return Time_Value::gettime(time_tick) + Time_Value::gettimeofday();
}

int GameCommon::cal_activity_info(ActivityTimeInfo& time_info, const Json::Value& activity_json)
{
    IntVec week_day;
    GameCommon::json_to_t_vec(week_day, activity_json["week"]);

    int start_time 	= GameCommon::fetch_day_sec(activity_json["start"].asInt());
    int end_time 	= GameCommon::fetch_day_sec(activity_json["end"].asInt());
    int ahead_time 	= start_time - activity_json["ahead_time"].asInt();

    if (activity_json.isMember("day_check"))
    {
    	time_info.set_week_day(week_day);
    }
    else
    {
    	time_info.day_check_ = false;
    }

	if (week_day.size() >= DAY_PER_WEEK || week_day.size() == 0 || time_info.day_check_ == true)
	{
		week_day.clear();
		week_day.push_back(0);
		time_info.set_freq_type(GameEnum::DAILY_ACTIVITY);
	}
	else
	{
		time_info.set_freq_type(GameEnum::WEEKLY_ACTIVITY);
	}

	for (IntVec::iterator iter = week_day.begin(); iter != week_day.end(); ++iter)
	{
		int day = *iter * Time_Value::DAY;
		if (ahead_time != start_time)
		{
			time_info.time_set_.push_back(day + ahead_time);
		}

    	time_info.time_set_.push_back(day + start_time);
    	time_info.time_set_.push_back(day + end_time);
	}

	time_info.active_time_ = end_time - start_time;

	if (ahead_time != start_time)
	{
		GameCommon::cal_three_activity_time(time_info);
	}
	else
	{
		GameCommon::cal_two_activity_time(time_info);
	}
    return 0;
}

void GameCommon::cal_one_activity_time(ActivityTimeInfo& time_info,
		const Json::Value& activity_json, int type)
{
    IntVec week_day;
    GameCommon::json_to_t_vec(week_day, activity_json["week"]);

    int start_time 	= GameCommon::fetch_day_sec(activity_json["start"].asInt());
    int end_time 	= GameCommon::fetch_day_sec(activity_json["end"].asInt());
    int ahead_time 	= start_time - activity_json["ahead_time"].asInt();

	if (week_day.size() >= DAY_PER_WEEK || week_day.size() == 0)
	{
		week_day.clear();
		week_day.push_back(0);
		time_info.set_freq_type(GameEnum::DAILY_ACTIVITY);
	}
	else
	{
		time_info.set_freq_type(GameEnum::WEEKLY_ACTIVITY);
	}

	for (IntVec::iterator iter = week_day.begin(); iter != week_day.end(); ++iter)
	{
		int day = *iter * Time_Value::DAY;
		switch(type)
		{
		case 0:
		{
			time_info.time_set_.push_back(day + start_time);
			break;
		}
		case 1:
		{
			time_info.time_set_.push_back(day + end_time);
			break;
		}
		case 2:
		{
			time_info.time_set_.push_back(day + ahead_time);
			break;
		}
		}
	}

	time_info.time_span_ = 1;

	int total = time_info.fetch_time_group();
    int cur_sec = GameCommon::fetch_cur_sec(time_info.freq_type_);

    for (int i = 0; i < total; ++i)
    {
    	time_info.time_index_ = i;

    	int start_time 	= time_info.time_set_[i * time_info.time_span_];
    	if (cur_sec < start_time)
    	{
    		time_info.refresh_time_ = start_time - cur_sec;
    		break;
    	}

    	JUDGE_CONTINUE(i == total - 1);

		// next cycle
		time_info.time_index_ 	= 0;
		time_info.refresh_time_ = time_info.time_cycle_ + time_info.time_set_[0] - cur_sec;
    }
}

void GameCommon::cal_two_activity_time(ActivityTimeInfo& time_info)
{
	time_info.time_span_ = 2;

	int total = time_info.fetch_time_group();
    int cur_sec = GameCommon::fetch_cur_sec(time_info.freq_type_);

    for (int i = 0; i < total; ++i)
    {
       	time_info.time_index_ = i;

    	int start_time 	= time_info.time_set_[i * time_info.time_span_];
    	int end_time 	= time_info.time_set_[i * time_info.time_span_ + 1];

    	if (cur_sec < start_time)
    	{
    		time_info.refresh_time_ = start_time - cur_sec;
    		time_info.cur_state_ 	= GameEnum::ACTIVITY_STATE_NO_START;
    		break;
    	}
    	else if (cur_sec < end_time)
    	{
    		time_info.refresh_time_ = end_time - cur_sec;
    		time_info.cur_state_ 	= GameEnum::ACTIVITY_STATE_START;
    		break;
    	}

		JUDGE_CONTINUE(i == total - 1);

		// next cycle
		time_info.time_index_ 	= 0;
		time_info.cur_state_ 	= GameEnum::ACTIVITY_STATE_NO_START;
		time_info.refresh_time_ = time_info.time_cycle_ + time_info.time_set_[0] - cur_sec;
    }
}

void GameCommon::cal_three_activity_time(ActivityTimeInfo& time_info)
{
	time_info.time_span_ = 3;

	int total = time_info.fetch_time_group();
    int cur_sec = GameCommon::fetch_cur_sec(time_info.freq_type_);

    for (int i = 0; i < total; ++i)
    {
       	time_info.time_index_ = i;

    	int ahead_time 	= time_info.time_set_[i * time_info.time_span_];
    	int start_time 	= time_info.time_set_[i * time_info.time_span_ + 1];
    	int end_time 	= time_info.time_set_[i * time_info.time_span_ + 2];

    	if (cur_sec < ahead_time)
    	{
    		time_info.refresh_time_ = ahead_time - cur_sec;
    		time_info.cur_state_ 	= GameEnum::ACTIVITY_STATE_NO_START;
    		break;
    	}
    	else if (cur_sec < start_time)
    	{
    		time_info.refresh_time_ = start_time - cur_sec;
    		time_info.cur_state_ 	= GameEnum::ACTIVITY_STATE_AHEAD;
    		break;
    	}
    	else if (cur_sec < end_time)
    	{
    		time_info.refresh_time_ = end_time - cur_sec;
    		time_info.cur_state_ 	= GameEnum::ACTIVITY_STATE_START;
    		break;
    	}

		JUDGE_CONTINUE(i == total - 1);

		// next cycle
		time_info.time_index_ 	= 0;
		time_info.cur_state_ 	= GameEnum::ACTIVITY_STATE_NO_START;
		time_info.refresh_time_ = time_info.time_cycle_ + time_info.time_set_[0] - cur_sec;
    }
}

void GameCommon::make_up_chat_broadcast_info(Proto80200006* chat_info,
			const ProtoBrocastNewInfo* brocast_info)
{
	chat_info->set_shout_id(brocast_info->shout_id());
	chat_info->set_role_id(brocast_info->role());
	chat_info->set_scene_id(brocast_info->scene_id());
	chat_info->set_channel_type(brocast_info->channel_type());

	for(int i = 0; i < brocast_info->shout_detail_list_size(); ++i)
	{
		const ProtoShoutDetail& sync_detail = brocast_info->shout_detail_list(i);
		ProtoShoutDetail *proto_detail = chat_info->add_shout_detail_list();
		*proto_detail = sync_detail;
	}
}

int GameCommon::make_up_session(ProtoSession* proto_session, const string& account)
{
	SessionDetail *session = 0;
	JUDGE_RETURN(SESSION_MANAGER->find_account_session(
			account, session) == 0, -1);

	proto_session->set_account(session->__account);
	proto_session->set_session(session->__session);
	return 0;
}

void GameCommon::announce(const ShoutInfo& shout, BrocastParaVec* para_vec, int scene_id)
{
	const Json::Value& conf = CONFIG_INSTANCE->brocast(shout.id_);
	JUDGE_RETURN(conf.empty() == false, ;);

	Proto30200123 brocast_info;

	ProtoBrocastNewInfo* proto_info = brocast_info.mutable_brocast_info();
	proto_info->set_role(shout.role_);
	proto_info->set_shout_id(shout.id_);
	proto_info->set_group_id(shout.group_);
	proto_info->set_scene_id(scene_id);

	switch (conf["server_type"].asInt())
	{
	default:
	case 1:	//all
	{
		proto_info->set_channel_type(CHANNEL_SYSTEM);
		break;
	}
	case 2:	//league
	{
		proto_info->set_channel_type(CHANNEL_LEAGUE);
		break;
	}
	case 3:	// team
	{
		proto_info->set_channel_type(CHANNEL_TEAM);
		break;
	}
	case 4:	//scene
	{
		proto_info->set_channel_type(CHANNEL_SCENE);
		break;
	}
	case 5:	//self
	{
		return ;
	}
	}

	if (para_vec != NULL)
	{
		GameCommon::make_up_broadcast_info(proto_info, shout.id_, *para_vec);
	}

#ifdef LOCAL_DEBUG
	bool map_flag 	= MapMonitorSingle::instance()->is_inited();
	bool logic_flag = LogicMonitorSingle::instance()->is_inited();
	JUDGE_RETURN(map_flag == true && logic_flag == true, ;);

	if (map_flag == 1)
	{
		MAP_MONITOR->dispatch_to_chat(&brocast_info);
	}
	else
	{
		LOGIC_MONITOR->dispatch_to_chat(SCENE_CHAT, &brocast_info);
	}
#else
	if (DAEMON_SERVER->is_server_map() == true)
	{
		MAP_MONITOR->dispatch_to_chat(&brocast_info);
	}
	else
	{
		LOGIC_MONITOR->dispatch_to_chat(SCENE_CHAT, &brocast_info);
	}
#endif
}

void GameCommon::announce(Int64 group_id, const ShoutInfo& shout, BrocastParaVec* para_vec)
{
	ShoutInfo cur_shout = shout;
	cur_shout.group_ = group_id;
	GameCommon::announce(cur_shout, para_vec);
}

void GameCommon::trvl_announce(const ShoutInfo& shout, BrocastParaVec* para_vec)
{
	const Json::Value& conf = CONFIG_INSTANCE->brocast(shout.id_);
	JUDGE_RETURN(conf.empty() == false, ;);

	Proto30200123 brocast_info;
	ProtoBrocastNewInfo* proto_info = brocast_info.mutable_brocast_info();
	proto_info->set_role(shout.role_);
	proto_info->set_shout_id(shout.id_);
	proto_info->set_group_id(shout.group_);

	//跨服只处理世界频道
	proto_info->set_channel_type(CHANNEL_SYSTEM);

	if (para_vec != NULL)
	{
		GameCommon::make_up_broadcast_info(proto_info, shout.id_, *para_vec);
	}

	Proto30100607 info;
	info.set_brocast_info(brocast_info.SerializeAsString());

#ifdef LOCAL_DEBUG
	bool map_flag 	= MapMonitorSingle::instance()->is_inited();
	bool logic_flag = LogicMonitorSingle::instance()->is_inited();
	JUDGE_RETURN(map_flag == true && logic_flag == true, ;);

	MAP_MONITOR->dispatch_to_logic_in_all_server(&info);
#else
	if (DAEMON_SERVER->is_server_map() == true)
	{
		MAP_MONITOR->dispatch_to_logic_in_all_server(&info);
	}
#endif
}

void GameCommon::make_up_broadcast_info(ProtoBrocastNewInfo* brocast_info,
			int shout_id, const BrocastParaVec& para_vec)
{
	JUDGE_RETURN(para_vec.empty() == false, );

	for(uint i = 0; i < para_vec.size(); ++i)
	{
		ProtoShoutDetail* proto_shout  = brocast_info->add_shout_detail_list();
		int parse_type                 = para_vec[i].__parse_type;
		proto_shout->set_parse_type(parse_type);
		switch(parse_type)
		{
		case GameEnum::PARSE_TYPE_INT:
		{
			int value                   = para_vec[i].__parse_data.__parse_int;
			proto_shout->set_single_value(value);
			break;
		}

		case GameEnum::PARSE_TYPE_ITEM_ID:
		case GameEnum::PARSE_TYPE_INT_64:
		{
			Int64 single_id             = para_vec[i].__parse_data.__parse_int_64;
			proto_shout->set_single_id(single_id);
			break;
		}

		case GameEnum::PARSE_TYPE_STRING:
		{
			int content_length          = ::strlen(para_vec[i].__parse_data.__parse_string);
			proto_shout->set_single_content(para_vec[i].__parse_data.__parse_string, content_length);
			break;
		}

		case GameEnum::PARSE_TYPE_PROTOBROCASTROLE:
		{
			ProtoBrocastRole* proto_role = proto_shout->mutable_role_info();
			proto_role->set_role_id(para_vec[i].__parse_data.__parse_role_info.__role_id);
			proto_role->set_role_name(para_vec[i].__parse_data.__parse_role_info.__role_name,
					::strlen(para_vec[i].__parse_data.__parse_role_info.__role_name));
			proto_role->set_team_state(para_vec[i].__parse_data.__parse_role_info.__team_status);
			break;
		}
		case GameEnum::PARSE_TYPE_ITEM_TIPS:
		{
			ProtoShoutItem *proto_item = proto_shout->mutable_item_tips();
			*proto_item = *(para_vec[i].__shout_item);
			break;
		}

		default:
			break;
		}
	}
}

void GameCommon::push_brocast_para_int(BrocastParaVec& para_vec, int single_value)
{
	BrocastPara para;
	para.__parse_type = GameEnum::PARSE_TYPE_INT;
	para.__parse_data.__parse_int = single_value;

	para_vec.push_back(para);
}

void GameCommon::push_brocast_para_int64(BrocastParaVec& para_vec, Int64 single_value)
{
	BrocastPara para;
	para.__parse_type = GameEnum::PARSE_TYPE_INT_64;
	para.__parse_data.__parse_int_64 = single_value;

	para_vec.push_back(para);
}

void GameCommon::push_brocast_para_item_name(BrocastParaVec& para_vec, int item_id)
{
	const Json::Value &item_json = CONFIG_INSTANCE->item(item_id);
	JUDGE_RETURN(item_json.empty() == false, ;);

	GameCommon::push_brocast_para_string(para_vec, item_json["name"].asString());
}

void GameCommon::push_brocast_para_reward(BrocastParaVec& para_vec, RewardInfo *reward_info)
{
	string name = "";
	for (ItemObjVec::iterator item_iter = reward_info->item_vec_.begin();
			item_iter != reward_info->item_vec_.end(); ++item_iter)
	{
		ItemObj& item_obj = *item_iter;
		const Json::Value &item_json = CONFIG_INSTANCE->item(item_obj.id_);
		JUDGE_CONTINUE(item_json != Json::Value::null);

		std::string item_name = item_json["name"].asString();
		if (name != "")
		{
			name.append(",");
		}
		name.append(item_name);
	}
	IntMap::const_iterator iter = reward_info->resource_map_.begin();
	for (; iter != reward_info->resource_map_.end(); ++iter)
	{
		const Json::Value &item_json = CONFIG_INSTANCE->item(iter->first);
		JUDGE_CONTINUE(item_json != Json::Value::null);

		std::string item_name = item_json["name"].asString();
		if (name != "")
		{
			name.append(",");
		}
		name.append(item_name);
	}
	GameCommon::push_brocast_para_string(para_vec, name);
}

void GameCommon::push_brocast_para_reward_id(BrocastParaVec& para_vec, int reward_id)
{
	RewardInfo reward_info(false);

	const Json::Value& reward_json = CONFIG_INSTANCE->reward(reward_id);
	JUDGE_RETURN(reward_json != Json::Value::null, ;);

	GameCommon::make_up_reward_items(reward_info, reward_json);
	GameCommon::push_brocast_para_reward(para_vec, &reward_info);
}

void GameCommon::push_brocast_para_item_info(BrocastParaVec& para_vec, const int item_id,
				const int item_bind, const int item_amount)
{
	std::stringstream ss;
	const Json::Value &item_json = CONFIG_INSTANCE->item(item_id);
	JUDGE_RETURN(item_json != Json::Value::null, ;);

	std::string item_name = item_json["name"].asString();
	ss << item_name << "x" << item_amount;
	GameCommon::push_brocast_para_string(para_vec, ss.str());
}

void GameCommon::push_brocast_para_skill(BrocastParaVec& para_vec, int skill_id)
{
	const Json::Value &skill_conf = CONFIG_INSTANCE->skill(skill_id);
	GameCommon::push_brocast_para_string(para_vec, skill_conf["name"].asString());
}

void GameCommon::push_brocast_para_string(BrocastParaVec& para_vec, const std::string& content)
{
	BrocastPara para;
	para.__parse_type = GameEnum::PARSE_TYPE_STRING;
	::strncpy(para.__parse_data.__parse_string,
			content.c_str(), MAX_COMMON_NAME_LENGTH);
	para.__parse_data.__parse_string[MAX_COMMON_NAME_LENGTH] = '\0';

	para_vec.push_back(para);
}

int GameCommon::time_to_sec(const Time_Value &tick)
{
    return tick.sec() + (tick.usec() > 5e5 ? 1 : 0);
}

int GameCommon::time_to_mdate(const time_t tick)
{
#ifndef NEW_LOCALTIME
    struct tm *tm_time;
    tm_time = localtime(&tick);
    return tm_time->tm_mday;
#else
    struct tm tm_time;
    g_localtime(tick, &tm_time);
    return tm_time.tm_mday;
#endif
}

int GameCommon::time_to_ydate(const time_t tick)
{
#ifndef NEW_LOCALTIME
    struct tm *tm_time;
    tm_time = localtime(&tick);
    return tm_time->tm_yday;
#else
    struct tm tm_time;
    g_localtime(tick, &tm_time);
    return tm_time.tm_yday;
#endif
}

int GameCommon::time_to_date(const time_t tick)
{
	struct timeval tv;
	struct timezone tz;

	memset(&tv, 0, sizeof(tv));
	memset(&tz, 0, sizeof(tz));

	gettimeofday(&tv, &tz);
	int local_tick = tick - (60 * tz.tz_minuteswest);
	return local_tick / Time_Value::DAY;
}

MongoConnector* GameCommon::mongodb_connection(pthread_t tid)
{
	static Thread_Mutex mutex;
	static std::map<pthread_t, MongoConnector*> conn_map;

	GUARD(Thread_Mutex, obj, mutex);
	if (conn_map.count(tid) == 0)
	{
		conn_map[tid] = new MongoConnector;
	}

	return conn_map[tid];
}

int GameCommon::db_load_mode_begin(int trans_recogn, BaseUnit* base_unit,
		Int64 role_id)
{
	DBShopMode* shop_mode = POOL_MONITOR->shop_mode_pool()->pop();
	JUDGE_RETURN(shop_mode != NULL, -1);

	shop_mode->recogn_ = trans_recogn;
	return GameCommon::db_load_mode_begin(shop_mode, base_unit, role_id);
}

int GameCommon::db_load_mode_begin(DBShopMode* shop_mode, BaseUnit* base_unit,
		Int64 role_id)
{
	return TRANSACTION_MONITOR->request_mongo_transaction(role_id, TRANS_LOAD_SHOP_MODE,
			DB_SHOP_LOAD_MODE, shop_mode, POOL_MONITOR->shop_mode_pool(), base_unit);
}

void GameCommon::request_save_role_int(Int64 role_index,
		const string& field_name, int value)
{
    BSONObjBuilder builder;
    builder << field_name << value;
	GameCommon::request_save_mmo_begin(Role::COLLECTION, BSON(Role::ID << role_index),
			BSON("$set" << builder.obj()), false);
}

void GameCommon::request_save_role_long(Int64 role_index,
		const std::string& field_name, Int64 value)
{
    BSONObjBuilder builder;
    builder << field_name << value;
	GameCommon::request_save_mmo_begin(Role::COLLECTION, BSON(Role::ID << role_index),
			BSON("$set" << builder.obj()), false);
}

int GameCommon::request_save_mmo_begin(const string& table_name, const BSONObj& query,
		const BSONObj& content, int upgrade_type, int trans_recogn, long index)
{
	DBTradeMode* trade_mode = POOL_MONITOR->trade_mode_pool()->pop();
	JUDGE_RETURN(trade_mode != NULL, -1);

	trade_mode->table_name_ = table_name;
	trade_mode->operate_flag_ = upgrade_type;

	*(trade_mode->query_) = query;
	*(trade_mode->content_) = content;

	return TRANSACTION_MONITOR->request_mongo_transaction(index, trans_recogn, DB_TRADE_MODE,
			trade_mode, POOL_MONITOR->trade_mode_pool());
}

int GameCommon::request_remove_mmo_begin(const string& table_name, const BSONObj& query,
		int just_one, int trans_recogn, long index)
{
	DBTradeMode* trade_mode = POOL_MONITOR->trade_mode_pool()->pop();
	JUDGE_RETURN(trade_mode != NULL, -1);

	trade_mode->table_name_ = table_name;
	trade_mode->operate_flag_ = just_one;
	*(trade_mode->query_) = query;

	return TRANSACTION_MONITOR->request_mongo_transaction(index, trans_recogn,
			DB_TRADE_MODE, trade_mode, POOL_MONITOR->trade_mode_pool());
}

int GameCommon::request_save_sys_mail(Int64 role_id, int mail_id)
{
	FontPair quit_font = FONT2(mail_id);

	MailInformation* mail_info = GameCommon::create_sys_mail(mail_id);
	return GameCommon::request_save_mail(role_id, mail_info);
}

int GameCommon::request_save_mail_content(Int64 role_id, MailInformation* mail_info, int push_pool)
{
	JUDGE_RETURN(mail_info != NULL, -1);
	mail_info->mail_content_ = mail_info->makeup_content_;
	return GameCommon::request_save_mail(role_id, mail_info, push_pool);
}

int GameCommon::request_save_mail(Int64 role_id, MailInformation* mail_info, int push_pool)
{
	JUDGE_RETURN(mail_info != NULL, -1);

	mail_info->receiver_id_ = role_id;
	return GameCommon::request_save_mail(mail_info, push_pool);
}

int GameCommon::request_save_mail(MailInformation* mail_info, int push_pool)
{
	JUDGE_RETURN(mail_info != NULL, -1);

    BSONObj mail_obj = GameCommon::mail_info_to_bson(mail_info);

	GameCommon::request_save_mmo_begin(MailOffline::COLLECTION,
			BSON(MailOffline::MAIL_ID << mail_info->mail_index_),
			BSON("$set" << mail_obj), true, TRANS_SAVE_MAIL_OFFLINE);

	JUDGE_RETURN(push_pool == true, -1);

	mail_info->recycle_goods();
	POOL_MONITOR->mail_info_pool()->push(mail_info);

	return 0;
}

BSONObj GameCommon::mail_info_to_bson(MailInformation *mail_info)
{
    BSONVec goods_vec;
	for (ItemListMap::iterator iter = mail_info->goods_map_.begin();
			iter != mail_info->goods_map_.end(); ++iter)
	{
		PackageItem* pack_item = iter->second;
		goods_vec.push_back(GameCommon::item_to_bson(pack_item));
	}

	BSONObjBuilder builder;
	builder << MailOffline::ROLE_ID << mail_info->receiver_id_
			<< MailOffline::SENDER_ID << mail_info->sender_id_
			<< MailOffline::SENDER_NAME << mail_info->sender_name_
			<< MailOffline::TYPE << mail_info->mail_type_
			<< MailOffline::FORMAT << mail_info->mail_format_
			<< MailOffline::FLAG << int(false)
			<< MailOffline::TIME << mail_info->send_time_
			<< MailOffline::TITLE << mail_info->mail_title_
			<< MailOffline::CONTENT << mail_info->mail_content_
			<< Package::MONEY << GameCommon::money_to_bson(mail_info->money_)
			<< MailOffline::GOODS << goods_vec
            << MailOffline::LABEL << mail_info->label_id_
			<< MailOffline::SENDER_VIP << mail_info->sender_vip_;

    return builder.obj();
}

MailInformation* GameCommon::create_base_mail(const string& mail_title,
		const string& mail_content, int mail_type, int mail_format)
{
	MailInformation* mail_info = POOL_MONITOR->mail_info_pool()->pop();
	JUDGE_RETURN(mail_info != NULL, NULL);

	mail_info->mail_title_ = mail_title;
	mail_info->mail_content_ = mail_content;

	mail_info->mail_type_ = mail_type;
	mail_info->mail_format_ = mail_format;
	mail_info->send_time_ = ::time(NULL);

	return mail_info;
}

MailInformation* GameCommon::create_pri_mail(const string& mail_title,
		const string& mail_content)
{
	return GameCommon::create_base_mail(mail_title, mail_content,
			GameEnum::MAIL_PRIVATE,0);
}

MailInformation* GameCommon::create_sys_mail(const string& mail_title,
		const string& mail_content, int format)
{
	return GameCommon::create_base_mail(mail_title, mail_content,
			GameEnum::MAIL_SYSTEM, format);
}

MailInformation* GameCommon::create_sys_mail(const FontPair& font_pair, int format)
{
	MailInformation* mail_info = GameCommon::create_base_mail(font_pair.first,
			font_pair.second, GameEnum::MAIL_SYSTEM, format);

	static string system_mail_name = CONFIG_INSTANCE->const_set_conf(
			"sys_mail_name")["remark"].asString();
	mail_info->sender_name_ = system_mail_name;

	return mail_info;
}

MailInformation* GameCommon::create_sys_mail(int font_index)
{
	FontPair font_pair = FONT2(font_index);
	return GameCommon::create_sys_mail(font_pair,font_index);
}

time_t GameCommon::make_time_value(int year, int month, int day, int hour, int minute)
{
    struct tm p;
    memset(&p, 0, sizeof(struct tm));

    p.tm_year = year - 1900;
    p.tm_mon = month - 1;
    p.tm_mday = day;
    p.tm_hour = hour;
    p.tm_min = minute;

    return ::mktime(&p);
}

void GameCommon::make_time_value(Time_Value &time_value, int year,
		int month, int day, int hour, int minute)
{
	time_t tt = GameCommon::make_time_value(year, month, day, hour, minute);
    time_value.set(tt, 0);
}

bool GameCommon::mover_in_area_json(const MoverCoord &coord, const Json::Value& area_arr_json)
{
	JUDGE_RETURN(area_arr_json != Json::Value::null && area_arr_json.size() == 2, false);
	int lt_x = area_arr_json[0u][0u].asInt();
	int lt_y = area_arr_json[0u][1u].asInt();
	int rb_x = area_arr_json[1u][0u].asInt();
	int rb_y = area_arr_json[1u][1u].asInt();
	if(GameCommon::mover_in_area(coord, lt_x, lt_y, rb_x, rb_y))
	{
		return true;
	}
	return false;
}

bool GameCommon::mover_in_area(const MoverCoord &coord, int lt_x, int lt_y, int rb_x, int rb_y)
{
	if( lt_x <= coord.pixel_x() && lt_y <= coord.pixel_y() &&
			rb_x >= coord.pixel_x() && rb_y >= coord.pixel_y())
		return true;
	return false;
}

int GameCommon::map_sync_activity_tips_no_start(int event_id, int sub_value)
{
	Proto30100601 request;
	request.set_activity_id(event_id);
	request.set_sub_value(sub_value);

	request.set_activity_state(GameEnum::ACTIVITY_STATE_NO_START);
	return MAP_MONITOR->dispatch_to_logic(&request);
}

int GameCommon::map_sync_activity_tips_ahead(const PairObj& event, int left_time, int sub_value)
{
	Proto30100601 request;
	request.set_activity_id(event.id_);
	request.set_sub_value(sub_value);

	request.set_activity_state(GameEnum::ACTIVITY_STATE_AHEAD);
	request.set_ahead_time(left_time);

	if (event.value_ == true)
	{
		return MAP_MONITOR->dispatch_to_logic_in_all_server(&request);
	}
	else
	{
		return MAP_MONITOR->dispatch_to_logic(&request);
	}
}

int GameCommon::map_sync_activity_tips_start(const PairObj& event, int end_time, int sub_value)
{
	Proto30100601 request;
	request.set_activity_id(event.id_);
	request.set_sub_value(sub_value);

	request.set_end_time(end_time);
	request.set_activity_state(GameEnum::ACTIVITY_STATE_START);

	if (event.value_ == true)
	{
		return MAP_MONITOR->dispatch_to_logic_in_all_server(&request);
	}
	else
	{
		return MAP_MONITOR->dispatch_to_logic(&request);
	}
}

int GameCommon::map_sync_activity_tips_stop(const PairObj& event, int left_times, int sub_value)
{
	Proto30100601 request;
	request.set_activity_id(event.id_);
	request.set_sub_value(sub_value);

	request.set_activity_state(GameEnum::ACTIVITY_STATE_END);
	request.set_left_times(left_times);

	if (event.value_ == true)
	{
		return MAP_MONITOR->dispatch_to_logic_in_all_server(&request);
	}
	else
	{
		return MAP_MONITOR->dispatch_to_logic(&request);
	}
}

int GameCommon::map_sync_activity_tips_next(int event_id, int left_times, int sub_value)
{
	Proto30100601 request;
	request.set_activity_id(event_id);
	request.set_sub_value(sub_value);

	request.set_activity_state(GameEnum::ACTIVITY_STATE_NEXT);
	request.set_left_times(left_times);
	return MAP_MONITOR->dispatch_to_logic(&request);
}

int GameCommon::script_vip_extra_use_times(int vip_type, int script_sort)
{
	return 0;
}

IntPair GameCommon::to_int_number(const string& str)
{
	std::stringstream sin(str);

	IntPair pair;
	char c = 0;

	if (!(sin >> pair.second))
	{
		return pair;
	}

	if (sin >> c)
	{
		return pair;
	}

	pair.first = true;
	return pair;
}

char* GameCommon::string_to_uper_case(char* string)
{
	JUDGE_RETURN(0 != string, 0);
	char *p_char = string;
	for( ; *p_char != '\0'; ++p_char )
	{
		if( *p_char >= 'a' && *p_char <= 'z')
			(*p_char) -= 'a' - 'A';
	}

	return string;
}

char* GameCommon::string_to_lower_case(char* string)
{
	JUDGE_RETURN(0 != string, 0);
	char *p_char = string;
	for( ; *p_char != '\0'; ++p_char )
	{
		if( *p_char >= 'A' && *p_char <= 'Z')
			(*p_char) += 'a' - 'A';
	}

	return string;
}

uint16_t GameCommon::crc16_byte(uint16_t crc, const uint8_t data)
{
	return (crc >> 8) ^ crc16_table[(crc ^ data) & 0xff];
}

uint16_t GameCommon::crc16(uint16_t crc, uint8_t const *buffer, size_t len)
{
	while (len--)
		crc = GameCommon::crc16_byte(crc, *buffer++);
	return crc;
}


int GameCommon::rand_mount_level(string str_info, const Json::Value &info_json)
{
	int rand_level = 0;
	if (info_json.isMember(str_info))
	{
		int down = info_json[str_info][0u].asInt(),
			up = info_json[str_info][1u].asInt();

		if (up >= down)
		{
			rand_level = std::rand() % (up - down + 1) + down;
		}
	}

	return rand_level;
}

void GameCommon::fill_rpm_role_info(ReplacementRoleInfo &info, const BSONObj &bson)
{
	info.__role_id = bson[Role::ID].numberLong();
	info.__vip_type = bson[Role::VIP_TYPE].numberInt();
	info.__sex = bson[Role::SEX].numberInt();
	info.__career = bson[Role::CAREER].numberInt();
	info.__level = bson[Role::LEVEL].numberInt();
	info.__fight_force = bson[Role::FORCE].numberInt();

	int name_len = bson[Role::NAME].str().length();
	if (name_len > MAX_NAME_LENGTH)
	    name_len = MAX_NAME_LENGTH;
	::strncpy(info.__name, bson[Role::NAME].str().c_str(), name_len);
	info.__name[name_len] = '\0';
}

const Json::Value& GameCommon::json_value_by_int_key(const Json::Value& json_value,
		const int& key)
{
	JUDGE_RETURN(!json_value.empty(), Json::Value::null);

	std::string str_key;
	GameCommon::int2str(key, str_key);
	return json_value[str_key];
}

int GameCommon::int2str(const int i, std::string &str)
{
	str.clear();
	char cstr[64] = {0};
	::snprintf(cstr, 64, "%d", i);
	str = cstr;
	return i;
}

const std::string GameCommon::int2str(const int i)
{
	std::string result;
	int2str(i, result);
	return result;
}

Int64 GameCommon::str2long(const char *nptr)
{
	int c;
	Int64 total;
	int sign;

	/* skip whitespace */
	while ( isspace((int)(unsigned char)*nptr) )
		++nptr;

	c = (int)(unsigned char)*nptr++;
	sign = c;
	if (c == '-' || c == '+')
		c = (int)(unsigned char)*nptr++;

	total = 0;

	while (isdigit(c)) {
		total = 10 * total + (c - '0');
		c = (int)(unsigned char)*nptr++;
	}

	if (sign == '-')
		return -total;
	else
		return total;
}

Json::Value::Members GameCommon::le_json_keys(const int value, const Json::Value& json)
{
	Json::Value::Members result;
	const Json::Value::Members &members = json.getMemberNames();
	for(size_t i=0; i< members.size(); i++)
	{
		JUDGE_CONTINUE(std::isdigit(members[i][0]));
		int key_int = ::atoi(members[i].c_str());
		if(key_int <= value)
			result.push_back(members[i]);
	}
	return result;
}

int GameCommon::get_item_by_money_type(int money_item)
{
	switch (money_item)
	{
	case GameEnum::ITEM_MONEY_UNBIND_GOLD:
	{
		return GameEnum::GOODS_UBNID_GOLD;
	}
	case GameEnum::ITEM_MONEY_BIND_GOLD:
	{
		return GameEnum::GOODS_BIND_GOLD;
	}
	}
	return 0;
}

bool GameCommon::is_money_item(int item_id)
{
	switch (item_id)
	{
	case GameEnum::ITEM_MONEY_UNBIND_GOLD:
	case GameEnum::ITEM_MONEY_BIND_GOLD:
	{
		return true;
	}
	}

	return false;
}

bool GameCommon::is_resource_item(int item_id)
{
	switch (item_id)
	{
	case GameEnum::ITEM_ID_REPUTATION:
	case GameEnum::ITEM_ID_HONOUR:
	case GameEnum::ITEM_ID_EXPLOIT:
	case GameEnum::ITEM_ID_REIKI:
	case GameEnum::ITEM_ID_SPIRIT:
	case GameEnum::ITEM_ID_PRACTICE:
	{
		return true;
	}
	}

	return false;
}

Money GameCommon::money_item_to_money(const ItemObj &item)
{
	Money money;

	switch(item.id_)
	{
	case GameEnum::ITEM_MONEY_BIND_GOLD:
	{
		money = Money(0, item.amount_, 0);
		break;
	}
	case GameEnum::ITEM_MONEY_UNBIND_GOLD:
	{
		money = Money(item.amount_, 0, 0, 0);
		break;
	}
	}

	return money;
}

Money GameCommon::money_item_to_money(int money_item, int amount, int times)
{
	return GameCommon::money_item_to_money(ItemObj(money_item, amount * times));
}

std::string GameCommon::monster_name(int monster_sort)
{
	const Json::Value& monster_json = CONFIG_INSTANCE->monster(monster_sort);
	JUDGE_RETURN(monster_json != Json::Value::null, NullString);

	return monster_json["name"].asString();
}

std::string GameCommon::scene_name(const int scene_id)
{
    const Json::Value &scene_json = CONFIG_INSTANCE->scene(scene_id);
    JUDGE_RETURN(scene_json != Json::Value::null, NullString);

    return scene_json["name"].asString();
}

std::string GameCommon::sql_escape(const std::string &r)
{
	std::stringstream stream;
	for (std::string::const_iterator it = r.begin(); it != r.end(); it++)
	{
		if (*it == '\\' || *it == '\'')
			stream << '\\';
		stream << *it;
	}
	return stream.str();
}

uint GameCommon::format_ip_string_to_int(const std::string& ip_addr)
{
	const char* p_str = ip_addr.c_str();
	JUDGE_RETURN(p_str != 0, 0u);

	uint ip_value = 0u;
	uint sub_value = 0u;
	int dot_count = 0;

	for( ; *p_str != '\0'; ++p_str )
	{
		char ccur = *p_str;

		if(ccur >= '0' && ccur <= '9')
		{
			sub_value = sub_value * 10 + (ccur - '0');
			continue;
		}

		if(ccur == '.')
		{
			ip_value = (ip_value << 8) | sub_value;
			sub_value = 0;
			dot_count ++;

			JUDGE_RETURN(dot_count <= 3, 0u);
			continue;
		}
	}

	JUDGE_RETURN(sub_value <= 255, 0u);
	ip_value = (ip_value << 8) | sub_value;

	if(dot_count < 3)
		ip_value <<= 8 * (3 - dot_count) ;
	return ip_value;
}

int GameCommon::hex_char_to_int(char c)
{
	if(c >= '0' && c <= '9')
		return c - '0';
	else
	if(c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	else
	if(c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	return 0;
}

Int64 GameCommon::hex_to_decimal(const char* szHex,int len)
{
	int result = 0;
	for(int i = 0; i < len; ++i)
	{
		JUDGE_CONTINUE(szHex[i] != '\0');
		result += pow((float)16,(int)len - i -1) * hex_char_to_int(szHex[i]);
	}
	return result;
}

Int64 GameCommon::format_mac_string_to_int64(const std::string& mac)
{
	const char* p_str = mac.c_str();
	JUDGE_RETURN(p_str != 0, 0u);
	string split_mac = "";
	std::vector<string> vstr;
	boost::split(vstr,mac,boost::is_any_of("-:"),boost::token_compress_on);
	for(std::vector<string>::iterator iter = vstr.begin(); iter != vstr.end(); ++iter)
	{
		split_mac += *iter;
	}
	if(split_mac != "")
		return hex_to_decimal(split_mac.c_str(),16);
	return 0;
}

IntVec GameCommon::random_int_vector_from_config(const Json::Value list_json, bool is_sub/*=false*/)
{
	JUDGE_RETURN(list_json != Json::Value::null, IntVec());

	uint list_json_n = list_json.size();
	std::vector<Json::Value> json_sort_vc;
	for(uint i=0; i<list_json_n; ++i)
	{
		json_sort_vc.push_back(list_json[i]);
	}

	if( list_json.size() > 0u && (is_sub == false || list_json[0u].isObject()))
		std::random_shuffle(json_sort_vc.begin(), json_sort_vc.end());

	return GameCommon::random_int_vector_from_json_vc(json_sort_vc);
}

IntVec GameCommon::random_int_vector_from_json_vc(std::vector<Json::Value> &json_vc)
{
	IntVec result;

	int json_vc_n = json_vc.size();
	for(int i=0; i < json_vc_n; ++i)
	{
		const Json::Value &json_obj = json_vc[i];

		if(json_obj.isInt())
			result.push_back(json_obj.asInt());

		if(json_obj.isObject())
		{
			IntVec sub_int_set = GameCommon::random_int_vector_from_config(json_obj, true);
			result.insert(result.end(), sub_int_set.begin(), sub_int_set.end());
		}
	}

	return result;
}

bool GameCommon::json_int_array_exists(const Json::Value array_json, int id)
{
	uint array_n = array_json.size();
	JUDGE_RETURN(array_n > 0 && array_json[0u].isInt(), false);

	for(uint i=0; i<array_n; ++i)
	{
		if(array_json[i].asInt() == id)
			return true;
	}

	return false;
}

void GameCommon::activity_time_tick_by_time(int *begin_tick, int *end_tick, const Json::Value &json)
{
    int type = json["type"].asInt();
    if (begin_tick != NULL)
        *begin_tick = GameCommon::activity_time_tick(type, json["range_day"][0u]);
    if (end_tick != NULL)
        *end_tick = GameCommon::activity_time_tick(type, json["range_day"][1u]);
}

int GameCommon::activity_time_tick(const int type, const Json::Value &json)
{
	switch (type)
	{
	case 1:	// 开服时间
	{
		return CONFIG_INSTANCE->open_day_tick(json.asInt());
	}
	case 2:	// 指定日期
	{
        Date_Time special_date;
        special_date.year(json[0u].asInt());
        special_date.month(json[1u].asInt());
        special_date.day(json[2u].asInt());
        special_date.hour(json[3u].asInt());
        special_date.minute(json[4u].asInt());
        special_date.second(0);
        return special_date.time_sec();
	}
	}
	return 0;
}

void GameCommon::init_property_map(IntMap& prop_map)
{
	for (int prop_id = GameEnum::ATTR_BEGIN; prop_id <= GameEnum::ATTR_END; ++prop_id)
	{
		prop_map[prop_id] = 0;
	}
	for (int prop_id = GameEnum::ATTR_MULTI_BEGIN; prop_id <= GameEnum::ATTR_MULTI_END; ++prop_id)
	{
		prop_map[prop_id] = 0;
	}
}

void GameCommon::add_prop_attr(IntMap& prop_map, int prop_id, int value)
{
	if(prop_id == GameEnum::ATTACK)
	{//同时增加攻击上限和攻击下限
		prop_map[GameEnum::ATTACK_LOWER] += value;
		prop_map[GameEnum::ATTACK_UPPER] += value;
	}
	else if(prop_id == GameEnum::DEFENSE)
	{//同时增加防御上限和防御下限
		prop_map[GameEnum::DEFENCE_LOWER] += value;
		prop_map[GameEnum::DEFENCE_UPPER] += value;
	}
	else
	{
		prop_map[prop_id] += value;
	}
}

int GameCommon::check_welfare_open_condition(const std::string &item_name, const int role_level, const int create_day)
{
	const Json::Value &elements_json = CONFIG_INSTANCE->welfare_elements_json();
	JUDGE_RETURN(elements_json.isMember(item_name), ERROR_CONFIG_NOT_EXIST);

	const Json::Value &element_json = elements_json[item_name];

	if (element_json.isMember("display_lvl"))
	{
		const Json::Value &level_json = element_json["display_lvl"];
		if (level_json.size() >= 2)
		{
			if (role_level < level_json[0u].asInt() || level_json[1u].asInt() < role_level)
				return ERROR_PLAYER_LEVEL_LIMIT;
		}
		else if (level_json.size() >= 1)
		{
			if (role_level < level_json[0u].asInt())
				return ERROR_PLAYER_LEVEL_LIMIT;
		}
	}
	if (element_json.isMember("display_days"))
	{
		const Json::Value &days_json = element_json["display_days"];
		if (days_json.size() >= 2)
		{
			if (create_day < days_json[0u].asInt() || (days_json[1u].asInt() > 0 && days_json[1u].asInt() < create_day))
				return ERROR_CLIENT_OPERATE;
		}
		else if (days_json.size() >= 1)
		{
			if (create_day < days_json[0u].asInt())
				return ERROR_CLIENT_OPERATE;
		}
	}
	return 0;
}

void GameCommon::output_item_info(const ProtoItem& proto_item)
{
	MSG_USER("|||||||||||||||item info:|||||||||||||||");
	MSG_USER("index:%d",proto_item.index());
	MSG_USER("id:%d",proto_item.id());
	MSG_USER("amount:%d",proto_item.amount());
	MSG_USER("bind:%d",proto_item.bind());
	MSG_USER("use_times:%d",proto_item.use_times());
	MSG_USER("use_tick:%ld",proto_item.use_tick());
	MSG_USER("new_tag:%d",proto_item.new_tag());
	MSG_USER("unique_id:%ld",proto_item.unique_id());
	if(GameCommon::item_is_equipment(proto_item.id()))
	{
		MSG_USER("equipment info");
	    const ProtoEquip& proto_equip = proto_item.equipment();
	    MSG_USER("--refine_level:%d",proto_equip.refine_level());
	    MSG_USER("--refine_degree:%d",proto_equip.refine_degree());

		const ProtoFashionInfo& proto_fashion = proto_equip.fashion_info();
		{
		    MSG_USER("--proto_fashion info");
		    MSG_USER("----use_type:%d",proto_fashion.use_type());
		    MSG_USER("----left_sec:%d",proto_fashion.left_sec());
		    MSG_USER("----use_tick:%ld",proto_fashion.use_tick());
		    MSG_USER("----expire_tick:%ld",proto_fashion.expire_tick());
		    MSG_USER("----is_in_use:%d",proto_fashion.is_in_use());
		    MSG_USER("----vip_type:%d",proto_fashion.vip_type());
		}

	    MSG_USER("--jewel_lists info");
	    for(int i = 0; i < proto_equip.jewel_lists_size(); ++i)
	    {
	        const ProtoPairObj& tmp = proto_equip.jewel_lists(i);
	        MSG_USER("----jewel_lists:%d,%d",tmp.obj_id(),tmp.obj_value());
	    }
	}
	MSG_USER("^^^^^^^^^^^^^^^item end:^^^^^^^^^^^^^^^");
}

void GameCommon::output_item_info(PackageItem* item)
{
	ProtoItem proto_item;
	item->serialize(&proto_item);
	GameCommon::output_item_info(proto_item);
}

int GameCommon::qq_fetch_agent_vip(int agent_code)
{
	const Json::Value& agent_code_49 = CONFIG_INSTANCE->tiny("agent_code_49");
	if(agent_code_49 == Json::nullValue || !agent_code_49.isArray())
	{
//		MSG_USER("CONFIG ERROR: tiny.json:agent_code_49 IS NOT ARRAY");
		return -1;
	}

	for(uint i = 0; i < agent_code_49.size(); ++i)
	{
		JUDGE_CONTINUE(agent_code == agent_code_49[i][0u].asInt());
		return agent_code_49[i][1u].asInt();
	}

	return -1;
}

int GameCommon::reset_prop_map(IntMap& prop_map)
{
	for(int i = GameEnum::ATTR_BEGIN; i <= GameEnum::ATTR_END; ++i)
	{
		prop_map[i] = 0;
	}
	for(int i = GameEnum::ATTR_MULTI_BEGIN; i <= GameEnum::ATTR_MULTI_END; ++i)
	{
		prop_map[i] = 0;
	}
	return 0;
}

int GameCommon::is_combine_server_flag(const string& server_flag)
{
	if (server_flag.empty() == false && server_flag[0] == 'f')
	{
		return true;
	}
	else
	{
		return false;
	}
}

int GameCommon::server_flag_to_server_id(const std::string &server_flag)
{
    int combind_flag = (server_flag[0] == 'f' ? 2 : 0);
    return ::atoi(server_flag.c_str() + 2) + combind_flag * 10000;
}

bool GameCommon::is_advance_script(int script_id)
{
	if(script_id == 20101 || script_id == 20102 || script_id == 20103 || script_id == 20104
			|| script_id == 20105 || script_id == 20106 || script_id == 20107
			|| script_id == 20108 || script_id == 20109 || script_id == 20110)
		return true;
	else
		return false;
}

bool GameCommon::is_exp_script(int script_id)
{
	if (script_id == 20201 || script_id == 20202 || script_id == 20203 || script_id == 20204
			|| script_id == 20205 || script_id == 20206 || script_id == 20207
			|| script_id == 20208 || script_id == 20209 || script_id == 20210)
		return true;
	else
		return false;
}

int GameCommon::update_49_qq(LongMap& qq_set,const std::string& qq_str)
{
	Json::Reader reader;
	Json::Value qq_js;

	if(reader.parse(qq_str,qq_js) == false)
	{
		return -1;
	}
	qq_set.clear();
	const Json::Value& agent_js = CONFIG_INSTANCE->tiny("agent_code_49");
	for(uint i = 0; i < agent_js.size(); ++i)
	{
		int agent = agent_js[i][0u].asInt();
		string agent_str = GameCommon::int2str(agent);
		if(qq_js.isMember(agent_str.c_str()))
		{
			if(qq_js[agent_str].isString())
			{
				qq_set[agent] = GameCommon::str2long(qq_js[agent_str].asCString());
				MSG_USER("49you_qq:%d:%s",agent,agent_str.c_str());
			}
			else
				MSG_USER("49you_qq qq_str ERROR:%s",agent_str.c_str());
		}
		else
			MSG_USER("49you_qq: not exist:%s",agent_str.c_str());
	}
	return 0;
}

int GameCommon::rand_by_chance(std::vector<int>& multiple, int rand_count)
{
	int multiple_size = multiple.size();
	if(multiple_size <= 0)
		return -1;
	std::vector<int> vec;
	int index = 0;
	int sum_multiple = 0;
	for(std::vector<int>::iterator iter = multiple.begin(); iter != multiple.end();++iter)
	{
		sum_multiple += *iter;
		int insert_count = (*iter) * rand_count;
		for(int i = 0; i < insert_count; ++i)
		{
			vec.push_back(index);
		}
		index++;
	}
	return vec[rand()%(sum_multiple*rand_count)];
}

IntVec GameCommon::rand_red_packet(int total_money, int dispatch_count, int min_money)
{
	IntVec item_vec;
    for (int i = 1 ;i < dispatch_count; ++i)
    {
    	int sub = (total_money - (dispatch_count - i) * (min_money)) / (dispatch_count - i);//随机安全上限
        int rand_money = rand() % sub + 1;
        if(rand_money < min_money)
        {
        	rand_money = min_money;
        }
        total_money = total_money - rand_money;
        item_vec.push_back(rand_money);
        //MSG_DEBUG("money_count:%d, money:%lf, left_money:%lf\n", i, total_money, real_money);
    }
    item_vec.push_back(total_money);
    return item_vec;
}

int GameCommon::cal_day_time(int time_point)
{
	int hour = time_point / 100;
	int min = time_point % 100;
	int secs = hour * 3600 + min * 60;
	return secs;
}

bool GameCommon::comp_by_time_desc(const ServerItemRecord &first, const ServerItemRecord &second)
{
	return first.get_time_ > second.get_time_;
}
