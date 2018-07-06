/*
 * TeamPlatform.cpp
 *
 *  Created on: Jul 22, 2013
 *      Author: peizhibi
 */

#include "TeamPlatform.h"
#include "LogicPlayer.h"
#include "LogicMonitor.h"
#include "ProtoDefine.h"

const int TEAM_RECRUIT_TERMINATE	= 1;
const int TEAM_SIGN_TERMINATE		= 2;

TeamPlatform::TeamPlatform()
{
	// TODO Auto-generated constructor stub
}

TeamPlatform::~TeamPlatform()
{
	// TODO Auto-generated destructor stub
}

PoolPackage<TeamPanel>* TeamPlatform::team_panel_package()
{
	return &this->panel_package_;
}

void TeamPlatform::remove_offline_team(int team_index)
{
	this->team_index_map_.erase(team_index);
}

int TeamPlatform::push_offline_teamer(int64_t role_id, int team_index)
{
	this->offline_teamers_map_[role_id] = team_index;
	this->team_index_map_[team_index] = team_index;
	return 0;
}

int TeamPlatform::pop_offline_teamer(int64_t role_id)
{
	JUDGE_RETURN(this->offline_teamers_map_.count(role_id) > 0, -1);

	int team_index = this->offline_teamers_map_[role_id];
	this->offline_teamers_map_.erase(role_id);

	return team_index;
}

void TeamPlatform::set_team_view(int64_t role_id, int view_index)
{
	this->view_team_map_[role_id] = view_index;
}

void TeamPlatform::erase_team_view(int64_t role_id)
{
	this->view_team_map_.erase(role_id);
}

// 对同个副本排队的角色广播组队列表信息
void TeamPlatform::brocast_team_view(int view_index)
{
	for (LongMap::iterator iter = this->view_team_map_.begin();
			iter != this->view_team_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second == view_index);

		LogicPlayer* player = NULL;
		JUDGE_CONTINUE(LOGIC_MONITOR->find_player(iter->first, player) == 0);

		//player->notify_team_sign_info();
	}
}

bool TeamPlatform::validate_team_view(int64_t role_id, int open_team)
{
	JUDGE_RETURN(open_team == true, true);
	return this->view_team_map_.count(role_id) > 0;
}

int TeamPlatform::offline_handle_time_out()
{
	JUDGE_RETURN(this->team_index_map_.empty() == false, -1);

	IntMap &team_index_map = this->team_index_map_;
	for (IntMap::iterator iter = team_index_map.begin();iter != team_index_map.end(); )
	{
		TeamPanel* team_panel = this->team_panel_package()->find_object(iter->first);
		IntMap::iterator erase_iter = iter++;

		if (team_panel == NULL)
		{
			this->team_index_map_.erase(erase_iter);
		}
		else
		{
			bool is_erase = false;
			this->offline_handle_time_out(team_panel, is_erase);

			if(true == is_erase)
			{
				this->team_index_map_.erase(erase_iter);
			}
		}
	}

	return 0;
}

// 邀请&申请进入队伍超时
int TeamPlatform::team_request_time_out()
{
	for(LongList::iterator iter = this->team_replier_list_.begin();
			iter != this->team_replier_list_.end(); )
	{
		LogicPlayer* player = NULL;
		if(LOGIC_MONITOR->find_player(*iter, player) != 0)
		{
			iter = this->team_replier_list_.erase(iter);
			continue;
		}

		if(0 == player->handle_request_time_out())
			iter = this->team_replier_list_.erase(iter);
		else
			iter++;
	}

	return 0;
}

// 副本排队超时
int TeamPlatform::sign_view_time_out()
{
	return 0;
}

int TeamPlatform::update_rep_id_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100226*, request, -1);

	Int64 src_id = request->src_id();
	Int64 new_id = request->new_id();

	TeamPanel* team_panel = TEAM_PANEL_PACKAGE->find_object(request->team_id());
	JUDGE_RETURN(team_panel != NULL, -1);
	JUDGE_RETURN(team_panel->rpm_teamer_info_.count(src_id) > 0, -1);

	ReplacementInfo rep_info = team_panel->rpm_teamer_info_[src_id];
	rep_info.teamer_info_.__role_id = new_id;

	team_panel->erase_replacement(src_id);
	team_panel->rpm_teamer_info_[new_id] = rep_info;
	team_panel->replacement_set_.push_back(new_id);

	Proto80400403 id_info;
	id_info.set_new_id(new_id);
	id_info.set_src_id(src_id);
	team_panel->notify_all_teamer(ACTIVE_TEAMER_MODIFY_ID, &id_info);

	return 0;
}

int TeamPlatform::offline_handle_time_out(TeamPanel* team_panel, bool &is_erase)
{
	JUDGE_RETURN(team_panel != NULL, -1);
	JUDGE_RETURN(team_panel->check_and_handle_offline() == true, -1);

	this->notify_offline_handle(team_panel);

	is_erase = false;
	if (team_panel->offline_teamer_info_.empty() == true
			|| team_panel->teamer_set_.empty() == true)
	{
		is_erase = true;
	}

	if (team_panel->teamer_set_.empty() == true)
	{
		MSG_USER("timeout dismiss team");
		this->team_panel_package()->unbind_and_push(team_panel->team_index_,
				team_panel);
	}

	return 0;
}

// 如果没有队长则提升队长，通知队伍信息
int TeamPlatform::notify_offline_handle(TeamPanel* team_panel)
{
	JUDGE_RETURN(team_panel != NULL, -1);
	LogicPlayer* player = team_panel->find_online_teamer();

	if (player == NULL)
	{// all off line
		team_panel->leader_id_ = 0;
		team_panel->teamer_set_.clear();
		return -1;
	}

	if (team_panel->leader_id_ != player->role_id())
	{// leader off line
		player->replace_leader();
	}

	player->sync_all_teamer_team_info();
	player->notify_all_teamer_team_info();

	return 0;
}

bool TeamPlatform::can_create_team(int scene_id)
{
	const Json::Value& validate_scene = CONFIG_INSTANCE->tiny("no_create_team");
	return GameCommon::is_value_in_config(validate_scene, scene_id) == false;
}

// 如果队伍人数为0, 可解散队伍
bool TeamPlatform::can_dismiss_team(int team_count)
{
	return team_count < GameEnum::DISMISS_TEAM_COUNT;
}

void TeamPlatform::add_replier(Int64 role_id)
{
	this->team_replier_list_.remove(role_id);
	this->team_replier_list_.push_back(role_id);
}
void TeamPlatform::remove_replier(Int64 role_id)
{
	this->team_replier_list_.remove(role_id);
}
