/*
 * MapEscort.cpp
 *
 *  Created on: 2016年10月24日
 *      Author: lzy
 */

#include "MapEscort.h"
#include "MapPlayer.h"
#include "MapPlayerEx.h"
#include "ProtoDefine.h"
#include "MapMonitor.h"

Escort_detail::Escort_detail()
{
	Escort_detail::reset();
}

int Escort_detail::escort_type()
{
	return this->escort_type_;
}

void Escort_detail::reset()
{
	this->car_index_ = 0;
	this->protect_id = 0;
	this->escort_type_ = 0;
	this->escort_times_ = 1;
	this->total_exp_ = 0;
	this->start_tick_ = 0;
	this->till = 0;
	this->target_level = 0;
	this->protect_map.clear();
}

void Escort_detail::set_escort_type(int type)
{
	type = std::min<int>(4, type);
	this->escort_type_ = std::max<int>(1, type);
}

MapEscort::MapEscort(void)
{ /*NULL*/ }

MapEscort::~MapEscort(void)
{ /*NULL*/ }

Escort_detail &MapEscort::get_escort_detail()
{
	return this->escort_detail_;
}

void MapEscort::reset()
{
	this->escort_detail_.reset();
}

int MapEscort::get_activity_status(int from)
{
	Proto30400453 info;
	info.set_from(from);
	return this->monitor()->dispatch_to_logic(this, &info);
}

int MapEscort::request_get_activity_status(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400453*, request, -1);
	JUDGE_RETURN(request->cycle_id() != 0, -1);

	Escort_detail& item = this->escort_detail_;
	item.set_escort_type(item.escort_type());

	if (request->type())
	{
		item.escort_times_ = 2;
	}
	else
	{
		item.escort_times_ = 1;
	}

	//经验加成
	int base_exp = CONFIG_INSTANCE->role_level(0, this->level())["convoy_exp"][item.escort_type() - 1].asInt();
	int add_scale = CONFIG_INSTANCE->vip(this->vip_type())["escort_extra_exp"].asInt();
	item.total_exp_ = base_exp * (1 + GameCommon::div_percent(add_scale)) * item.escort_times_;

	MapPlayerEx *player = this->self_player();
	player->cache_tick().update_cache(MapPlayer::CACHE_ESCORT);
	player->record_other_serial(ESCORT_SERIAL, 1, item.escort_type(),
			item.escort_times_, item.total_exp_);

	switch(request->from())
	{
	case GameEnum::ESCORT_INFO_OPEN :
		player->respond_escort_open_info();
		break;
	case GameEnum::ESCORT_INFO_UPGRADE :
		player->respond_escort_upgrade_info();
		break;
	case GameEnum::ESCORT_INFO_SELECT :
		player->respond_escort_start_info();
	}
	return 0;
}

int MapEscort::generate_escort_info()
{
	JUDGE_RETURN(this->escort_detail_.escort_type() == 0, -1);

    const Json::Value &convoy = CONFIG_INSTANCE->convoy_json();
	int escort_type = GameCommon::rand_list_num(convoy["level_prob_list"]) + 1;
	this->escort_detail_.set_escort_type(escort_type);

	return 0;
}

int MapEscort::start_protect_player(Int64 player_id)
{
	const Json::Value &convoy = CONFIG_INSTANCE->convoy_json();
	int protect_limit = convoy["protect_limit"].asInt() == 0 ? 5 : convoy["protect_limit"].asInt();

	JUDGE_RETURN((int)this->escort_detail_.protect_map.size() < protect_limit, -1);

	this->escort_detail_.protect_map.insert(player_id);
	return 0;
}

int MapEscort::protect_someone(Int64 protect_id)
{
	JUDGE_RETURN(this->escort_detail_.protect_id == 0, -1);

	this->escort_detail_.protect_id = protect_id;
	return 0;
}

int MapEscort::stop_protect_player()
{
	JUDGE_RETURN(this->escort_detail_.protect_id != 0, -1);

	MapPlayerEx* player = NULL;
	JUDGE_RETURN(MAP_MONITOR->find_player(this->escort_detail_.protect_id, player) == 0, -2)

	MapPlayer* own = this->self_player();
	player->get_escort_detail().protect_map.erase(own->role_id());

	this->escort_detail_.protect_id = 0;
	return 0;
}


int MapEscort::sync_transfer_escort()
{
	Proto30400116 escort_info;
	escort_info.set_car_index(this->escort_detail_.car_index_);
	escort_info.set_escort_exp(this->escort_detail_.escort_times_);
	escort_info.set_escort_type(this->escort_detail_.escort_type());
	escort_info.set_pre_exp(this->escort_detail_.total_exp_);
	escort_info.set_protect_id(this->escort_detail_.protect_id);
	escort_info.set_till(this->escort_detail_.till);
	escort_info.set_start_tick(this->escort_detail_.start_tick_);
	escort_info.set_target_level(this->escort_detail_.target_level);

	for (LongSet::iterator it = this->escort_detail_.protect_map.begin();
			it != this->escort_detail_.protect_map.end(); ++it)
	{
		escort_info.add_protect_list(*it);
	}

	return this->send_to_other_scene(this->scene_id(), escort_info);
}

int MapEscort::read_transfer_escort(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400116*, request, -1);

	this->escort_detail_.car_index_ = request->car_index();
	this->escort_detail_.escort_times_ = request->escort_exp();
	this->escort_detail_.escort_type_ = request->escort_type();
	this->escort_detail_.total_exp_ = request->pre_exp();
	this->escort_detail_.protect_id = request->protect_id();
	this->escort_detail_.start_tick_ = request->start_tick();
	this->escort_detail_.target_level = request->target_level();
	this->escort_detail_.till = request->till();

	for (int i = 0; i < request->protect_list_size(); ++i)
	{
		Int64 protect_id = request->protect_list(i);
		this->escort_detail_.protect_map.insert(protect_id);
	}

	return 0;
}
