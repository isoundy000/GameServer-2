/*
 * TeamPanel.cpp
 *
 *  Created on: Jul 22, 2013
 *      Author: peizhibi
 */

#include "TeamPanel.h"
#include "ProtoDefine.h"
#include "LogicPlayer.h"
#include "LogicMonitor.h"
#include "TeamPlatform.h"
#include "MongoDataMap.h"

TeamSetInfo::TeamSetInfo(void) :fb_confirm_tick_(0),
	borcast_recuit_tick_(0), auto_invite_(1), auto_accept_(1)
{ /*NULL*/ }

void TeamSetInfo::reset(void)
{
	this->fb_confirm_tick_ = 0;
	this->borcast_recuit_tick_ = 0;
	this->team_applier_map_.clear();
	this->team_inviter_map_.clear();

	this->auto_accept_ = 1;
	this->auto_invite_ = 1;
}

bool TeamSetInfo::validate_applier(int64_t role_id)
{
	BLongMap::iterator iter = this->team_applier_map_.find(role_id);
	JUDGE_RETURN(iter != this->team_applier_map_.end(), true);

	return GameCommon::validate_time_span(iter->second, 15);
}

bool TeamSetInfo::validate_inviter(int64_t role_id)
{
	BLongMap::iterator iter = this->team_inviter_map_.find(role_id);
	JUDGE_RETURN(iter != this->team_inviter_map_.end(), true);

	return GameCommon::validate_time_span(iter->second, 15);
}

bool TeamSetInfo::validate_borcast_recuit(void)
{
	return GameCommon::validate_time_span(this->borcast_recuit_tick_, 15);
}

int OfflineTeamerInfo::make_up_teamer_info(ProtoTeamer* teamer_info)
{
	teamer_info->set_role_id(this->role_id_);
	teamer_info->set_role_name(this->teamer_info_.__name);
	teamer_info->set_role_level(this->teamer_info_.__level);
	teamer_info->set_role_force(this->teamer_info_.__fight_force);
	teamer_info->set_role_career(this->teamer_info_.__career);
	teamer_info->set_role_sex(this->teamer_info_.__sex);
	teamer_info->set_vip_type(this->teamer_info_.__vip_type);
	teamer_info->set_online_flag(GameEnum::TEAMER_OFFLINE);

	return 0;
}

int ReplacementInfo::make_up_teamer_info(ProtoTeamer* teamer_info)
{
	teamer_info->set_role_id(this->teamer_info_.__role_id);
	teamer_info->set_role_name(this->teamer_info_.__name);
	teamer_info->set_role_level(this->teamer_info_.__level);
	teamer_info->set_role_force(this->teamer_info_.__fight_force);
	teamer_info->set_role_career(this->teamer_info_.__career);
	teamer_info->set_role_sex(this->teamer_info_.__sex);
	teamer_info->set_vip_type(this->teamer_info_.__vip_type);
	teamer_info->set_online_flag(GameEnum::TEAMER_REPLACEMENT);

	return 0;
}

void ReplacementInfo::set_replacement_info(ReplacementRoleInfo* rpm_role_info)
{
	JUDGE_RETURN(NULL != rpm_role_info, ;);
	::memcpy(&(this->teamer_info_), rpm_role_info, sizeof(ReplacementRoleInfo));
}

TeamPanel::TeamPanel(void) :
    leader_id_(0), team_state_(GameEnum::TEAM_STATE_NORMAL),
    fb_id_(0),no_enter_nums_(0), rpm_recomand_info_(NULL)
{
	this->team_index_ = GlobalIndex::global_team_index_++;
}

void TeamPanel::reset(void)
{
	this->leader_id_			= 0;
	this->fb_id_				= 0;

	if (this->rpm_recomand_info_ != NULL)
	{
		LOGIC_MONITOR->rpm_recomand_info_pool()->push(this->rpm_recomand_info_);
	}
	this->rpm_recomand_info_	= NULL;

	this->team_state_			= GameEnum::TEAM_STATE_NORMAL;
	this->no_enter_nums_ 		= 0;

	this->leader_name_.clear();
	this->teamer_set_.clear();
	this->offline_teamer_info_.clear();
	this->fb_wait_confirm_set_.clear();
	this->fb_ready_set_.clear();
	this->use_times_set_.clear();
	this->replacement_set_.clear();
	this->rpm_teamer_info_.clear();
}

void TeamPanel::notify_all_teamer(int recogn, Message* msg)
{
	for (LongList::iterator iter = this->teamer_set_.begin();
			iter != this->teamer_set_.end(); ++iter)
	{
		LogicPlayer* player = NULL;
		JUDGE_CONTINUE(LOGIC_MONITOR->find_player(*iter, player) == 0);

		player->respond_to_client(recogn, msg);
	}

}

int TeamPanel::online_count()
{
	return this->teamer_set_.size() + this->replacement_set_.size() - this->offline_teamer_info_.size();
}

int TeamPanel::teamer_count()
{
	return this->teamer_set_.size() + this->replacement_set_.size();
}

bool TeamPanel::teamer_full()
{
	return (this->teamer_set_.size()+ this->replacement_set_.size()) >= GameEnum::MAX_TEAMER_COUNT;
}

bool TeamPanel::is_leader(int64_t role_id)
{
	return this->leader_id_ == role_id;
}

bool TeamPanel::validate_teamer(int64_t role_id)
{
	LongList::iterator iter = std::find(this->teamer_set_.begin(),
			this->teamer_set_.end(), role_id);
	return iter != this->teamer_set_.end();
}

bool TeamPanel::validate_replacement(int64_t role_id)
{
	LongList::iterator iter = std::find(this->replacement_set_.begin(),
			this->replacement_set_.end(), role_id);
	return iter != this->replacement_set_.end();
}

bool TeamPanel::push_teamer(int64_t role_id, int push_type)
{
	JUDGE_RETURN(this->validate_teamer(role_id) == false, false);

	if (push_type == GameEnum::TEAMER_PUSH_BACK)
	{
		// back
		this->teamer_set_.push_back(role_id);
	}
	else
	{
		// front
		this->teamer_set_.push_front(role_id);
	}

	return true;
}

bool TeamPanel::erase_teamer(int64_t role_id)
{
	this->teamer_set_.remove(role_id);
	this->offline_teamer_info_.erase(role_id);
	this->fb_ready_set_.erase(role_id);
	this->use_times_set_.erase(role_id);
	this->fb_wait_confirm_set_.erase(role_id);

	return true;
}

bool TeamPanel::erase_replacement(int64_t role_id)
{
	this->replacement_set_.remove(role_id);
	this->rpm_teamer_info_.erase(role_id);

	return true;
}

RpmRecomandInfo* TeamPanel::rpm_recomand_info()
{
	return this->rpm_recomand_info_;
}

// 离线超时的玩家退出队伍(return true 表示有玩家被删除 =_=# )
bool TeamPanel::check_and_handle_offline()
{
	JUDGE_RETURN(this->offline_teamer_info_.empty() == false, false);
	JUDGE_RETURN(GameEnum::TEAM_STATE_FB_INSIDE != this->team_state_, false);//如果是副本队伍不清除离线队员
	LongVec offline_set;
	Int64 now_tick = ::time(NULL);

	// check
	for (OfflineTeamerMap::iterator iter = this->offline_teamer_info_.begin();
			iter != this->offline_teamer_info_.end(); ++iter)
	{
		int team_offline_time = CONFIG_INSTANCE->tiny("team_offline_time").asInt();
		JUDGE_CONTINUE(now_tick - iter->second.offline_tick_ >= team_offline_time);
		offline_set.push_back(iter->first);
	}

	JUDGE_RETURN(offline_set.empty() == false, false);

	// remove
	for (LongVec::iterator iter = offline_set.begin(); iter != offline_set.end(); ++iter)
	{
		MSG_USER("offline player quit");
		this->erase_teamer(*iter);
		this->offline_teamer_info_.erase(*iter);
		this->save_offline_teamer_info(*iter, true);
		TEAM_PLANTFORM->pop_offline_teamer(*iter);
	}

	return true;
}

LogicPlayer* TeamPanel::find_online_teamer(bool include_leader)
{
	LogicPlayer* player = NULL;

	for (LongList::iterator iter = this->teamer_set_.begin();
			iter != this->teamer_set_.end(); ++iter)
	{
		if (include_leader == false)
		{
			JUDGE_CONTINUE(*iter != this->leader_id_);
		}

		JUDGE_BREAK(LOGIC_MONITOR->find_player(*iter, player) != 0);
	}

	return player;
}

int TeamPanel::save_offline_teamer_info(int64_t role_id, bool is_leave)
{
	return 0;
}
