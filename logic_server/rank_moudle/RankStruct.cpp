/*
 * RankStruct.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: louis
 */

#include "RankStruct.h"
#include "ProtoDefine.h"

RankerShapeInfo::RankerShapeInfo(void) :
    __sex(0), __weapon(0), __clothes(0), __label(0), __mount(0), __pet(0)
{ /*NULL*/ }

void RankerShapeInfo::reset()
{
	::memset(this, 0, sizeof(RankerShapeInfo));
}

BaseRankInfo::BaseRankInfo() :
    __cur_rank(0), __last_rank(0), __achive_tick(0), __role_id(0), 
    __rank_type(0), __rank_value(0), __fight_force(0), __ext_value(0),
    __additional_id(0),__vip_type(0),__worship_num(0),__is_worship(0)
{ /*NULL*/ }

void BaseRankInfo::reset(void)
{
	this->__cur_rank=0;
	this->__last_rank=0;
	this->__achive_tick=0;
	this->__role_id=0;
	this->__rank_type=0;
	this->__rank_value=0;
	this->__additional_id=0;
	this->__fight_force=0;
	this->__ext_value=0;
	this->__role_name.clear();
	this->__league_name.clear();
	this->__vip_type=0;
	this->__worship_num=0;
	this->__is_worship=0;
//	this->__shaper_info.reset();
}

RankRecord::RankRecord(int p) : __vec_index(-1)
{ /*NULL*/ }

void RankRecord::reset(void)
{
    __vec_index = -1;
    __single_player_info.reset();
}

void RankRecord::serialize(ProtoRankRecord* proto_record)
{
	proto_record->set_cur_rank(this->__single_player_info.__cur_rank);
	proto_record->set_last_rank(this->__single_player_info.__last_rank);
	proto_record->set_role_id(this->__single_player_info.__role_id);
	proto_record->set_display_content(this->__single_player_info.__role_name);
	proto_record->set_league_name(this->__single_player_info.__league_name);
	proto_record->set_rank_value(this->__single_player_info.__rank_value);
	proto_record->set_additional_id(this->__single_player_info.__additional_id);
	proto_record->set_vip_type(this->__single_player_info.__vip_type);
	proto_record->set_worship_num(this->__single_player_info.__worship_num);
	proto_record->set_is_worship(this->__single_player_info.__is_worship);
}

void RankRecord::unserialize(const ProtoRankRecord* proto_record)
{
	this->__single_player_info.__cur_rank = proto_record->cur_rank();
	this->__single_player_info.__last_rank = proto_record->last_rank();
	this->__single_player_info.__rank_value = proto_record->rank_value();
	this->__single_player_info.__role_id = proto_record->role_id();
	this->__single_player_info.__role_name = proto_record->display_content();
	this->__single_player_info.__league_name = proto_record->league_name();
	this->__single_player_info.__vip_type = proto_record->vip_type();
//	this->__single_player_info.__worship_num = proto_record->worship_num();
	this->__single_player_info.__is_worship = proto_record->is_worship();
}

RankRefreshDetail::RankRefreshDetail(void) :
    __refresh_type(0), __rank_type(0), __refresh_interval(0),
    __last_refresh_tick(0), __next_refresh_tick(0)

{ /*NULL*/ }

void RankRefreshDetail::reset(void)
{
	this->__refresh_type=0;
	this->__rank_type=0;
	this->__last_refresh_tick=0;
	this->__next_refresh_tick=0;
	this->__refresh_interval=0;
	this->__refresh_tick_set.clear();
}

void PlayerOfflineData::reset(void)
{
	this->role_id_ = 0;

	this->mount_beast_tick_ = 0;
	this->mount_beast_info_.clear();
}

bool rank_record_cmp(const RankRecord *left, const RankRecord *right)
{
    switch (left->__single_player_info.__rank_type)
    {
        case RANK_FIGHT_FORCE:
        case RANK_FIGHT_LEVEL:
        case RANK_FUN_MOUNT:
        case RANK_FUN_GOD_SOLIDER:
        case RANK_FUN_MAGIC_EQUIP:
        case RANK_FUN_XIAN_WING:
        case RANK_FUN_LING_BEAST:
        case RANK_FUN_BEAST_EQUIP:
        case RANK_FUN_BEAST_MOUNT:
        case RANK_FUN_BEAST_WING:
        case RANK_FUN_BEAST_MAO:
        case RANK_FUN_TIAN_GANG:
        {
            if (left->__single_player_info.__rank_value == right->__single_player_info.__rank_value)
                return left->__single_player_info.__ext_value > right->__single_player_info.__ext_value;
            return left->__single_player_info.__rank_value > right->__single_player_info.__rank_value;
            break;
        }
        case RANK_KILL_VALUE:
        case RANK_HERO:
        {
             if (left->__single_player_info.__rank_value == right->__single_player_info.__rank_value)
            	 return left->__single_player_info.__ext_value < right->__single_player_info.__ext_value;
             return left->__single_player_info.__rank_value > right->__single_player_info.__rank_value;
             break;
        }
        case RANK_SINGLE_SCRIPT_ZYFM:
        {
            if (left->__single_player_info.__rank_value == right->__single_player_info.__rank_value)
            {
                if (left->__single_player_info.__achive_tick == right->__single_player_info.__achive_tick)
                    return left->__single_player_info.__fight_force > right->__single_player_info.__fight_force;
                return left->__single_player_info.__achive_tick < right->__single_player_info.__achive_tick;
            }
            return left->__single_player_info.__rank_value > right->__single_player_info.__rank_value;
            break;
        }
        default:
        {
            if (left->__single_player_info.__rank_value == right->__single_player_info.__rank_value)
                return left->__single_player_info.__achive_tick < right->__single_player_info.__achive_tick;
            return left->__single_player_info.__rank_value > right->__single_player_info.__rank_value;
            break;
        }
    }
}

