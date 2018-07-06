/*
 * MapTinyPlayer.cpp
 *
 *  Created on: Apr 1, 2014
 *      Author: peizhibi
 */

#include "MapTinyPlayer.h"
#include "MapMonitor.h"
#include "MapPlayerEx.h"
#include "ProtoDefine.h"
#include "MapLogicStruct.h"
#include "Scene.h"

MapTinyPlayer::MapTinyPlayer()
{
	// TODO Auto-generated constructor stub
}

MapTinyPlayer::~MapTinyPlayer()
{
	// TODO Auto-generated destructor stub
}

void MapTinyPlayer::reset_tiny()
{
	this->blood_container_.reset();
    this->attack_set.clear();
    this->cur_select_role = 0;
    this->jump_value_.stop_time();
}

void MapTinyPlayer::reset_tiny_everyday()
{
	this->blood_container_.non_tips_ = false;
}

void MapTinyPlayer::tiny_fb_recover_blood()
{
	JUDGE_RETURN(this->is_blood_full() == false, ;);
	JUDGE_RETURN(this->is_death() == false, ;);

    this->fighter_restore_all();
}

int MapTinyPlayer::fetch_buy_blood_value()
{
	int item_id = CONFIG_INSTANCE->blood_cont("blood_item")[0u][0u].asInt();
	const Json::Value& effect = CONFIG_INSTANCE->prop(item_id)["effect"];
	if(effect.isArray() == true)
	{
		for(uint i = 0;i < effect.size(); i++)
		{
			if(effect[i]["name"] == "add_blood")
				return effect[i]["value"].asInt();
		}
	}
	else
	{
		return effect["value"].asInt();
	}
	return 0;
}

int MapTinyPlayer::notify_blood_contain_info()
{
	this->blood_container_.copy_to_status();
	this->notify_update_status(&this->blood_container_.status_);
	return 0;
}

int MapTinyPlayer::check_recover_blood_scene()
{
	return true;
}

int MapTinyPlayer::check_and_add_cur_blood(int add_value)
{
	this->blood_container_.cur_blood_ += add_value;
	this->blood_container_.adjust_cur_blood();
	return this->notify_blood_contain_info();
}

int MapTinyPlayer::check_and_use_health(ProtoItem* proto_item, int add_value)
{
	JUDGE_RETURN(this->blood_container_.left_capacity() > 0, ERROR_BLOOD_CONTAINER_FULL);
	return this->check_and_add_cur_blood(add_value);
}

int MapTinyPlayer::check_timeout_cont_blood()
{
//	JUDGE_RETURN(this->is_fight_state() == false, -1);
	JUDGE_RETURN(this->blood_container_.cur_blood_ > 0, -1);
	JUDGE_RETURN(this->check_recover_blood_scene() == true, -1);

	int blood_differ = this->fetch_blood_differ();
	JUDGE_RETURN(blood_differ > 0, -1);
	JUDGE_RETURN(this->blood_container_.arrive_time_add() == true, -1);

	int add_value = 0;
	add_value = std::min<int>(this->total_recover_blood(), blood_differ);
	add_value = std::min<int>(add_value, this->blood_container_.cur_blood_);

	int blood_container_modify = this->correct_add_blood_by_status(add_value);
	this->check_and_add_cur_blood(-1 * blood_container_modify);
	this->modify_blood_by_fight(-1 * add_value,	FIGHT_TIPS_USE_PROPS);

	this->blood_container_.check_time_ = 0;
	return 0;
}

int MapTinyPlayer::set_auto_cont_blood(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400005*, request, RETURN_AUTO_USE_BLOOD);
	JUDGE_RETURN(this->blood_container_.left_capacity() > 0, -1);

	// 1: auto package, 2: auto buy, 3: request package
	if (request->use_type() == 2)
	{
		int add_value = this->fetch_buy_blood_value();
		JUDGE_RETURN(add_value > 0 && add_value <= this->blood_container_.left_capacity(), -1);
	}
	else
	{
		request->set_left_capacity(this->blood_container_.left_capacity());
	}

	request->set_use_times(request->use_times() + 1);
	return this->send_to_logic_thread(*request);
}

int MapTinyPlayer::set_cont_blood_notips(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400006*, request, RETURN_BLOOD_CONTAIN_NOTIPS);

	if (request->tips_flag() == false)
	{
		this->blood_container_.non_tips_ = true;
	}
	else
	{
		this->blood_container_.non_tips_ = false;
	}

	FINER_PROCESS_NOTIFY(RETURN_BLOOD_CONTAIN_NOTIPS);
}

void MapTinyPlayer::start_jump_value_time()
{
	const Json::Value& detail = CONFIG_INSTANCE->skill_detail(400000001);
	this->jump_value_.sub1_ = detail["max_value"].asInt();
	this->jump_value_.start_interval(detail["interval"].asInt());
}

void MapTinyPlayer::check_jump_value_timeout()
{
	JUDGE_RETURN(this->fight_detail_.__jump < this->jump_value_.sub1_, ;);

	int ret = this->jump_value_.update_time();
	JUDGE_RETURN(ret == true, ;);

	this->jump_value_.reset_time();
	this->update_fighter_jump_value(1);
}

BloodContainer& MapTinyPlayer::blood_container()
{
	return this->blood_container_;
}

int MapTinyPlayer::sync_transfer_tiny()
{
	Proto30400111 tiny_info;

	tiny_info.set_cur_blood(this->blood_container_.cur_blood_);
	tiny_info.set_check_flag(this->blood_container_.check_flag_);
	tiny_info.set_check_tick(this->blood_container_.check_time_);

	tiny_info.set_non_tips(this->blood_container_.non_tips_);
	tiny_info.set_everyday_tick(this->blood_container_.everyday_tick_);
	tiny_info.set_last_tips_tick(this->blood_container_.last_tips_tick_);

	return this->send_to_other_scene(this->scene_id(), tiny_info);
}

int MapTinyPlayer::read_transfer_tiny(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400111*, request, -1);

	this->blood_container_.cur_blood_ = request->cur_blood();
	this->blood_container_.check_flag_ = request->check_flag();
	this->blood_container_.check_time_ = request->check_tick();

	this->blood_container_.non_tips_ = request->non_tips();
	this->blood_container_.everyday_tick_ = request->everyday_tick();
	this->blood_container_.last_tips_tick_ = request->last_tips_tick();

	return 0;
}

void MapTinyPlayer::insert_attack_me(Int64 fighter_id)
{
	MapPlayerEx* self = this->self_player();
	MapPlayerEx* fighter = this->find_player(fighter_id);
	JUDGE_RETURN(self != NULL && fighter != NULL,);

	if(self->is_attack_by_id(fighter->role_id()) == false)
		return;

	if(this->attack_set.empty())
	{
		this->cur_select_role = fighter_id;
		this->notify_attack(this->cur_select_role);
	}

	this->attack_set.insert(fighter_id);
}

void MapTinyPlayer::check_attack_me()
{
	JUDGE_RETURN(this->attack_set.empty() == false, ;)
	JUDGE_RETURN(this->condition_attack_me(this->cur_select_role) < 0, ;);

	this->attack_set.erase(this->cur_select_role);

	int min_distance = INT_MAX;
	int dis_tmp = 0;

	Int64 selected_role = 0;
	for(BLongSet::iterator iter = this->attack_set.begin();iter != this->attack_set.end();)
	{
		dis_tmp = this->condition_attack_me(*iter);
		if(dis_tmp < 0)
		{
			this->attack_set.erase(iter++);
			continue;
		}
		if(dis_tmp < min_distance)
		{
			min_distance = dis_tmp;
			selected_role = *iter;
		}
		++iter;
	}

	this->cur_select_role = selected_role;
	this->notify_attack(this->cur_select_role);
}

int MapTinyPlayer::condition_attack_me(Int64 fighter_id)
{
	JUDGE_RETURN(fighter_id > 0,-1);
	MapPlayerEx* fighter = NULL,*self = NULL;
	fighter = this->find_player(fighter_id);
	if(fighter == NULL || fighter->is_death() || fighter->scene_id() != this->scene_id() || fighter->space_id() != this->space_id())
	{
		return -1;
	}
	int distance = ::coord_offset_grid(fighter->location(), this->location());
	if(distance > 20)
		return -1;

	self = dynamic_cast<MapPlayerEx*>(this);
	if(self->is_attack_by_id(fighter->role_id()) == false)
		return -1;

	return distance;
}

void MapTinyPlayer::notify_attack(Int64 fighter_id)
{
	Proto80400383 notify;
	MapPlayerEx* player = NULL;
	player = this->find_player(fighter_id);
	if(player != NULL)
	{
		notify.set_attack_career(player->role_detail().__career);
		notify.set_attack_name(player->role_name());
		notify.set_attack_id(player->role_id());
		notify.set_pixel_x(player->location().pixel_x());
		notify.set_pixel_y(player->location().pixel_y());
	}
	else
	{
		notify.set_attack_id(0);
	}
	this->respond_to_client(ACTIVE_BE_ATTACK_NOTIFY, &notify);
}

void MapTinyPlayer::clear_attack_me()
{
	this->attack_set.clear();
	this->cur_select_role = 0;
	this->notify_attack(0);
}

int MapTinyPlayer::check_and_add_event_cut(Int64 attackor, int test)
{
	Scene* scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, -1);

	if (test == false)
	{
		JUDGE_RETURN(scene->has_event_cut() == true, -1);
	}

	MapPlayerEx* player = scene->find_player(attackor);
	JUDGE_RETURN(player != NULL, -1);

	return player->update_event_cut();
}

int MapTinyPlayer::update_event_cut()
{
	static double last = CONFIG_INSTANCE->buff(BasicStatus::EVENTCUT)["last"].asDouble();

	int total_kill = 1 + this->find_status_value(BasicStatus::EVENTCUT);
    return this->insert_defender_status(this, BasicStatus::EVENTCUT,
    		0, last, 0, total_kill);
}
