/*
 * AIDropPack.cpp
 *
 *  Created on: Jan 16, 2014
 *      Author: peizhibi
 */

#include "AIDropPack.h"
#include "TipsEnum.h"
#include "AIManager.h"
#include "MapPlayer.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"

int AIDropPack::AIDropTimer::type()
{
	return GTT_MAP_ONE_SECOND;
}

int AIDropPack::AIDropTimer::handle_timeout(const Time_Value &tv)
{
	JUDGE_RETURN(this->drop_pack_ != NULL, -1);

	this->cancel_timer();
	this->drop_pack_->exit_scene();

	return 0;
}

AIDropPack::AIDropTimer::AIDropTimer()
{
	this->drop_pack_ = NULL;
}

int AIDropPack::RecycleTimer::type()
{
	return GTT_MAP_ONE_SECOND;
}

int AIDropPack::RecycleTimer::handle_timeout(const Time_Value &tv)
{
	this->cancel_timer();
	this->drop_pack_->recycle_self();
	return 0;
}

AIDropPack::AIDropPack()
{
	// TODO Auto-generated constructor stub
	this->drop_timer_.drop_pack_ = this;
	this->recycle_timer_.drop_pack_ = this;
	this->drop_detail_.drop_id_ = MAP_MONITOR->generate_drop_id();
}

AIDropPack::~AIDropPack()
{
	// TODO Auto-generated destructor stub
}

int AIDropPack::client_sid(void)
{
	return 0;
}

int64_t AIDropPack::entity_id(void)
{
	return this->drop_detail_.drop_id_;
}

int AIDropPack::enter_scene(const int type)
{
	this->sign_in();
	GameMover::enter_scene(type);

	if (this->drop_detail_.recycle_tick_ == Time_Value::zero)
	{
		this->drop_detail_.recycle_tick_ = GameCommon::fetch_time_value(AIDropDetail::MAX_DROP_TIME);
	}

	this->drop_timer_.schedule_timer(this->drop_detail_.recycle_tick_);

	return 0;
}

int AIDropPack::exit_scene(const int type)
{
	this->drop_timer_.cancel_timer();
	GameMover::exit_scene(type);

	this->sign_out();
	this->recycle_timer_.schedule_timer(3);

	return 0;
}

int AIDropPack::make_up_appear_info_base(Block_Buffer *buff, const bool send_by_gate)
{
    Proto80400112 appear_info;
    appear_info.set_drop_id(this->drop_detail_.drop_id_);
    appear_info.set_no_auto_pickup(this->drop_detail_.no_auto_pickup_);
    appear_info.set_drop_type(this->drop_detail_.drop_type_);
    appear_info.set_item_id(this->drop_detail_.item_obj_.__id);
    appear_info.set_amount(this->drop_detail_.item_obj_.__amount);

    appear_info.set_pixel_x(this->mover_detial_.__location.pixel_x());
    appear_info.set_pixel_y(this->mover_detial_.__location.pixel_y());

    int left_protect_tick = GameCommon::left_time(this->drop_detail_.drop_tick_.sec());
    appear_info.set_left_protect_sec(left_protect_tick);

    for (LongMap::iterator iter = this->drop_detail_.player_map_.begin();
    		iter != this->drop_detail_.player_map_.end(); ++iter)
    {
    	appear_info.add_viewer_set(iter->first);
    }

    for (LongMap::iterator iter = this->drop_detail_.no_view_.begin();
    		iter != this->drop_detail_.no_view_.end(); ++iter)
    {
    	appear_info.add_no_viewer_set(iter->first);
    }

    ProtoClientHead head;
    head.__recogn = ACTIVE_AIDROP_APPEAR;
    return this->make_up_client_block(buff, &head, &appear_info);
}

int AIDropPack::drop_id()
{
	return this->drop_detail_.drop_id_;
}

int AIDropPack::reset()
{
	this->drop_timer_.cancel_timer();
	this->recycle_timer_.cancel_timer();

	this->reset_mover();
	this->drop_detail_.reset();

	return 0;
}

int AIDropPack::recycle_self()
{
	return AIDROP_PACKAGE->unbind_and_push(this->entity_id(), this);
}

int AIDropPack::is_have_goods()
{
	return this->drop_detail_.money_map_.empty() == false
			|| this->drop_detail_.item_obj_.__id > 0;
}

int AIDropPack::pick_up_shout_drop_goods(MapPlayer *player)
{
	return 0;
}

int AIDropPack::validate_pick_up(Int64 role_id)
{
	switch (this->drop_detail_.drop_type_)
	{
	case GameEnum::DROP_TYPE_PERSON:
	case GameEnum::DROP_TYPE_TEAM:
	case GameEnum::DROP_TYPE_RANDOM_HURT:
	case GameEnum::DROP_TYPE_MAX_HURT_PERSON:
	{
		if (this->drop_detail_.drop_tick_ >= Time_Value::gettimeofday())
		{
			if (this->drop_detail_.team_share_ == 1)
			{
				// 队伍共享的道具要是没有被队友正在拾取的道具
				if (this->drop_detail_.pick_up_prep_ == true)
					return ERROR_ALREADY_PICKUP;
			}

			JUDGE_RETURN(this->drop_detail_.player_map_.count(role_id) > 0, ERROR_NO_PICKUP_RIGHT);
			return 0;
		}
		else
		{
			// 过了保护时间, 所有人可以拾取
			JUDGE_RETURN(this->drop_detail_.pick_up_prep_ == false, ERROR_ALREADY_PICKUP);
			return 0;
		}
	}
	case GameEnum::DROP_TYPE_ALL:
	{
		JUDGE_RETURN(this->drop_detail_.pick_up_prep_ == false, ERROR_ALREADY_PICKUP);
		return 0;
	}
	}

	return ERROR_NO_PICKUP_RIGHT;
}

int AIDropPack::set_pick_up_prep(Int64 role_id)
{
	switch (this->drop_detail_.drop_type_)
	{
	case GameEnum::DROP_TYPE_PERSON:
	case GameEnum::DROP_TYPE_TEAM:
	case GameEnum::DROP_TYPE_RANDOM_HURT:
	case GameEnum::DROP_TYPE_MAX_HURT_PERSON:
	{
		this->drop_detail_.player_map_.erase(role_id);
		this->drop_detail_.no_view_[role_id] = true;
        if (this->drop_detail_.drop_tick_ < Time_Value::gettimeofday() || this->drop_detail_.team_share_ == 1)
        {
			this->drop_detail_.pick_up_prep_ = true;
        }
		break;
	}

	case GameEnum::DROP_TYPE_ALL:
	{
		this->drop_detail_.pick_up_prep_ = true;
		break;
	}
	}
	return 0;
}

int AIDropPack::pick_up_suceess()
{
	switch (this->drop_detail_.drop_type_)
	{
	case GameEnum::DROP_TYPE_PERSON:
	case GameEnum::DROP_TYPE_TEAM:
	case GameEnum::DROP_TYPE_RANDOM_HURT:
	case GameEnum::DROP_TYPE_MAX_HURT_PERSON:
	{
		JUDGE_RETURN(this->drop_detail_.player_map_.empty() == true ||
				this->drop_detail_.team_share_ == 1 ||
				this->drop_detail_.drop_tick_ < Time_Value::gettimeofday(), -1);
		this->exit_scene();
		break;
	}

	case GameEnum::DROP_TYPE_ALL:
	{
		this->exit_scene();
		break;
	}
	}
	return 0;
}

int AIDropPack::pick_up_failure(Int64 role_id)
{
	switch (this->drop_detail_.drop_type_)
	{
	case GameEnum::DROP_TYPE_PERSON:
	case GameEnum::DROP_TYPE_TEAM:
	case GameEnum::DROP_TYPE_RANDOM_HURT:
	case GameEnum::DROP_TYPE_MAX_HURT_PERSON:
	{
		this->drop_detail_.player_map_[role_id] = true;
		this->drop_detail_.no_view_.erase(role_id);
        if (this->drop_detail_.drop_tick_ < Time_Value::gettimeofday() || this->drop_detail_.team_share_ == 1)
			this->drop_detail_.pick_up_prep_ = false;
		break;
	}

	case GameEnum::DROP_TYPE_ALL:
	{
		this->drop_detail_.pick_up_prep_ = false;
		break;
	}
	}
	return 0;
}

int AIDropPack::make_up_money_info(Proto31400013* pickup_info)
{
	JUDGE_RETURN(pickup_info != NULL, -1);
	JUDGE_RETURN(this->drop_detail_.money_map_.empty() == false, -1);

	ProtoMoney* proto_money = pickup_info->mutable_add_money();
	JUDGE_RETURN(proto_money != NULL, -1);

	IntMap& money_map = this->drop_detail_.money_map_;
	proto_money->set_bind_copper(money_map[GameEnum::MONEY_BIND_COPPER]);
	proto_money->set_copper(money_map[GameEnum::MONEY_UNBIND_COPPER]);

	proto_money->set_bind_gold(money_map[GameEnum::MONEY_BIND_GOLD]);
	proto_money->set_gold(money_map[GameEnum::MONEY_UNBIND_GOLD]);

	return 0;
}

int AIDropPack::make_up_item_info(Proto31400013* pickup_info)
{
	JUDGE_RETURN(pickup_info != NULL, -1);
	JUDGE_RETURN(this->drop_detail_.item_obj_.__id > 0, -1);

	ProtoItem* proto_item = pickup_info->mutable_add_item();
	JUDGE_RETURN(proto_item != NULL, -1);

	this->drop_detail_.item_obj_.serialize(proto_item);
	return 0;
}

int AIDropPack::sign_and_enter_scene(const MoverCoord& ai_coord, Scene* scene)
{
	JUDGE_RETURN(scene != NULL, -1);

	this->init_mover_scene(scene);
	this->mover_detial_.__location = ai_coord;

	AIDROP_PACKAGE->bind_object(this->drop_id(), this);
	return this->enter_scene();
}

AIDropDetail& AIDropPack::drop_detail()
{
	return this->drop_detail_;
}

void AIDropPack::calc_pickup_protect_tick(const Json::Value &json)
{
    int protect_tick = AIDropDetail::MAX_DELAY_ALL_TIME;
    if(this->is_in_script_mode())
    {
    	protect_tick = AIDropDetail::MAX_SCIRPT_ALL_TIME;
    }

    this->drop_detail_.drop_tick_ = GameCommon::fetch_add_time_value(protect_tick);
}

void AIDropPack::calc_recycle_tick(const Json::Value &json, Scene *scene)
{
    double recycle_tick = AIDropDetail::MAX_DROP_TIME;
    if (json.isMember("recycle_last"))
    {
        recycle_tick = std::min<double>(json["recycle_last"].asDouble(), Time_Value::MINUTE * 5);
    }
    this->drop_detail_.recycle_tick_ = GameCommon::fetch_time_value(recycle_tick);
}

