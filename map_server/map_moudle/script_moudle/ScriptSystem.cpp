/*
 * ScriptSystem.cpp
 *
 *  Created on: 2016年10月26日
 *      Author: lyw
 */

#include "ScriptSystem.h"
#include "MMOScriptHistory.h"
#include "MapMonitor.h"
#include "MapPlayerEx.h"

ScriptSystem::ScriptSystem() {
	// TODO Auto-generated constructor stub
	this->legend_player_.reset();
	this->sword_player_.reset();
	this->role_map_.clear();
	this->double_script_map_.clear();
}

ScriptSystem::~ScriptSystem() {
	// TODO Auto-generated destructor stub
}

void ScriptSystem::star()
{
	MMOScriptHistory::load_data(this->legend_player_, GameEnum::SCRIPT_T_LEGEND_TOP);
	MMOScriptHistory::load_data(this->sword_player_, GameEnum::SCRIPT_T_SWORD_TOP);
	MMOScriptHistory::load_couples(this->role_map_);

	this->sort_rank(GameEnum::SCRIPT_T_SWORD_TOP);

	MSG_USER("ScriptSystem start!");
}

void ScriptSystem::stop()
{
	MMOScriptHistory::update_data(this->legend_player_, GameEnum::SCRIPT_T_LEGEND_TOP);
	MMOScriptHistory::update_data(this->sword_player_, GameEnum::SCRIPT_T_SWORD_TOP);
	MMOScriptHistory::update_couples(this->role_map_);

	MSG_USER("ScriptSystem stop!");
}

LegendTopPlayer &ScriptSystem::top_player(int script_type)
{
	if (script_type == GameEnum::SCRIPT_T_LEGEND_TOP)
		return this->legend_player_;
	else
		return this->sword_player_;
}

ThreeObjVec &ScriptSystem::rank_vec(int script_type)
{
	if (script_type == GameEnum::SCRIPT_T_LEGEND_TOP)
		return this->legend_rank_vec_;
	else
		return this->sword_rank_vec_;
}

LegendTopPlayer::PlayerInfo *ScriptSystem::player_info(int script_type, Int64 role_id)
{
	if (script_type == GameEnum::SCRIPT_T_LEGEND_TOP)
	{
		JUDGE_RETURN(this->legend_player_.player_map_.count(role_id) > 0, NULL);
		return &(this->legend_player_.player_map_[role_id]);
	}
	else
	{
		JUDGE_RETURN(this->sword_player_.player_map_.count(role_id) > 0, NULL);
		return &(this->sword_player_.player_map_[role_id]);
	}
}

int ScriptSystem::sweep_update_top_rank(int gate_id, Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto30400912 *, request, msg, -1);

	this->update_legend_top_rank_info(request->role_id(), request->role_name(),
			request->fight_score(), request->script_type(), request->floor());

	if (request->script_type() == GameEnum::SCRIPT_T_LEGEND_TOP)
		this->fetch_rank_info(gate_id, request->role_id());
	else
		this->fetch_sword_rank_info(gate_id, request->role_id(), 1);

	return 0;
}

int ScriptSystem::update_legend_top_rank(MapPlayer* player, int script_type, int floor)
{
//	this->check_rank_reset();
	JUDGE_RETURN(player != NULL, 0);
	JUDGE_RETURN(script_type == GameEnum::SCRIPT_T_LEGEND_TOP ||
			script_type == GameEnum::SCRIPT_T_SWORD_TOP, 0);

	this->update_legend_top_rank_info(player->role_id(), player->name(),
			player->role_detail().__fight_force, script_type, floor);

	return 0;
}

void ScriptSystem::sort_rank(int script_type)
{
	ThreeObjVec &sort_vec = this->rank_vec(script_type);
	sort_vec.clear();

	const Json::Value &script_json = CONFIG_INSTANCE->script(GameEnum::SCRIPT_SORT_LEGEND_TOP); //问鼎 论剑最大层数一样
	int top_rank = script_json["top_rank"].asInt();

	LegendTopPlayer &top_player = this->top_player(script_type);
	for (LegendTopPlayer::PlayerInfoMap::iterator iter = top_player.player_map_.begin();
			iter != top_player.player_map_.end(); ++iter)
	{
		LegendTopPlayer::PlayerInfo &rank_player = iter->second;

		ThreeObj obj;
		obj.id_ = rank_player.player_id_;
		obj.tick_ = rank_player.tick_;
		obj.value_ = rank_player.floor_;
		sort_vec.push_back(obj);
	}
	int rank = 1;
	std::sort(sort_vec.begin(), sort_vec.end(), GameCommon::three_comp_by_desc);

	for (ThreeObjVec::iterator iter = sort_vec.begin();
			iter != sort_vec.end(); ++iter)
	{
		LegendTopPlayer::PlayerInfo &rank_player = top_player.player_map_[iter->id_];
		if (rank <= top_rank)
		{
			rank_player.rank_ = rank;
			++rank;
		}
		else
		{
			top_player.player_map_.erase(iter->id_);
		}
	}
}

int ScriptSystem::update_legend_top_rank_info(Int64 role_id, std::string name, int fight_score, int script_type, int floor)
{
	LegendTopPlayer &top_player = this->top_player(script_type);

	LegendTopPlayer::PlayerInfo &player_info = top_player.player_map_[role_id];
	player_info.player_id_ = role_id;
	player_info.name_ = name;
	player_info.fight_score_ = fight_score;

	if (floor > player_info.floor_)
	{
		player_info.floor_ = floor;
		player_info.tick_ = ::time(NULL);
	}

	this->sort_rank(script_type);

#ifdef LOCAL_DEBUG
	MMOScriptHistory::update_data(this->legend_player_, GameEnum::SCRIPT_T_LEGEND_TOP);
	MMOScriptHistory::update_data(this->sword_player_, GameEnum::SCRIPT_T_SWORD_TOP);
#endif

	return 0;
}

int ScriptSystem::request_fetch_rank_info(int gate_id, Int64 role_id, Message* msg)
{
//	this->check_rank_reset();
	DYNAMIC_CAST_RETURN(Proto30400911 *, request, msg, -1);

	if (request->script_type() == GameEnum::SCRIPT_T_LEGEND_TOP)
		this->fetch_rank_info(gate_id, role_id, request->num1(), request->num2());
	else if (request->script_type() == GameEnum::SCRIPT_T_SWORD_TOP)
		this->fetch_sword_rank_info(gate_id, role_id, request->page());

	return 0;
}

int ScriptSystem::fetch_rank_info(int gate_id, Int64 role_id, int num1, int num2)
{
	Proto50400940 respond;
	for (LegendTopPlayer::PlayerInfoMap::iterator iter = this->legend_player_.player_map_.begin();
			iter != this->legend_player_.player_map_.end(); ++iter)
	{
		LegendTopPlayer::PlayerInfo &player_info = iter->second;
		if (player_info.rank_ >= num1 && player_info.rank_ <= num2)
		{
			ProtoLegendTopRank *legend_rank = respond.add_legend_rank();
			player_info.serialize(legend_rank);
		}

		JUDGE_CONTINUE(player_info.player_id_ == role_id);
		ProtoLegendTopRank *my_rank = respond.mutable_my_rank();
		player_info.serialize(my_rank);
	}

//	MSG_USER("Proto50400940: %s", respond.Utf8DebugString().c_str());
	return MAP_MONITOR->dispatch_to_client_from_gate(gate_id, role_id, &respond);
}

int ScriptSystem::fetch_sword_rank_info(int gate_id, Int64 role_id, int page)
{
	PageInfo page_info;
	GameCommon::game_page_info(page_info, page, this->sword_rank_vec_.size(), RANK_PAGE);

	Proto50400942 respond;
	respond.set_cur_page(page_info.cur_page_);
	respond.set_total_page(page_info.total_page_);

	for (int i = page_info.start_index_; i < ::std::min(page_info.total_count_,
			page_info.start_index_ + RANK_PAGE); ++i)
	{
		Int64 player_id = this->sword_rank_vec_[i].id_;
		LegendTopPlayer::PlayerInfo *player_info = this->player_info(GameEnum::SCRIPT_T_SWORD_TOP, player_id);
		JUDGE_CONTINUE(player_info != NULL);

		ProtoLegendTopRank *sword_rank = respond.add_sword_rank();
		player_info->serialize(sword_rank);
	}

	LegendTopPlayer::PlayerInfo *my_info = this->player_info(GameEnum::SCRIPT_T_SWORD_TOP, role_id);
	if (my_info != NULL)
	{
		ProtoLegendTopRank *my_rank = respond.mutable_my_rank();
		my_info->serialize(my_rank);
	}

	return MAP_MONITOR->dispatch_to_client_from_gate(gate_id, role_id, &respond);
}

void ScriptSystem::check_rank_reset()
{
	Time_Value nowtime = Time_Value::gettimeofday();
	if (this->legend_player_.refresh_tick_ <= nowtime.sec())
	{
		this->legend_player_.refresh_tick_ = next_day(0, 0, nowtime).sec();
		this->legend_player_.player_map_.clear();
	}
}

void ScriptSystem::test_rank_reset()
{
//	this->legend_player_.player_map_.clear();
//
//#ifdef LOCAL_DEBUG
//	MMOScriptHistory::update_data(this->legend_player_);
//#endif
}

int ScriptSystem::add_couple_script_times(Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto30400914 *, request, msg, -1);

	Int64 role_id = request->role_id();
	this->role_map_[role_id] = ::time(NULL);

	return 0;
}

int ScriptSystem::login_fetch_couple_script_times(int sid, Int64 role_id, Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto30400915 *, request, msg, -1);

	int prev_scene_id = request->prev_scene_id();

	JUDGE_RETURN(this->role_map_.count(role_id) > 0, 0);
	JUDGE_RETURN(GameCommon::is_current_day(this->role_map_[role_id]) == true, 0);

	this->role_map_.erase(role_id);

	Proto30400916 respond;
	respond.set_state(1);

	return MAP_MONITOR->dispatch_to_scene(sid, role_id, prev_scene_id, &respond);
}

int ScriptSystem::set_double_script(int gate_id, Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto30400917 *, request, msg, -1);

	this->double_script_map_.clear();
	for (int i = 0; i < request->value_set_size(); ++i)
	{
		int script_type = request->value_set(i);
		JUDGE_CONTINUE(script_type != 0);

		this->double_script_map_[script_type] = true;
	}

	return MAP_MONITOR->dispatch_to_logic(request);
}

int ScriptSystem::fetch_script_mult(int script_type)
{
	JUDGE_RETURN(this->double_script_map_.count(script_type) > 0, 1);

	return 2;
}

int ScriptSystem::sweep_fetch_script_mult(int sid, Int64 role_id, Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto30400918 *, request, msg, -1);

	int prev_scene_id = request->prev_scene_id();

	int mult = 0;
	if (request->script_sort() > 0)
	{
		const Json::Value &script_json = CONFIG_INSTANCE->script(request->script_sort());
		JUDGE_RETURN(script_json != Json::Value::null, 0);

		int script_type = script_json["type"].asInt();
		mult = this->fetch_script_mult(script_type);
	}
	else
	{
		mult = this->fetch_script_mult(request->scrit_type());
	}

	Proto30400919 req;
	req.set_type(request->type());
	req.set_script_sort(request->script_sort());
	req.set_scrit_type(request->scrit_type());
	req.set_mult(mult);
	return MAP_MONITOR->dispatch_to_scene(sid, role_id, prev_scene_id, &req);
}
