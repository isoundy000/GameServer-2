/*
 * TrvlScriptTeam.cpp
 *
 *  Created on: Sep 2, 2016
 *      Author: peizhibi
 */

#include "TrvlScriptTeam.h"
#include "ProtoPublic.pb.h"
#include "TrvlScriptMonitor.h"

BaseTeam::BaseTeam()
{
	BaseTeam::reset();
	this->index_ = GlobalIndex::global_team_index_++;
}

void BaseTeam::reset()
{
	this->type_ = 0;
	this->limit_force_ 	= 0;
	this->auto_start_ 	= 0;

	this->sceret_.clear();
	this->team_name_.clear();
	this->teamer_list_.clear();
}

void BaseTeam::push_teamer(Int64 role)
{
	JUDGE_RETURN(this->is_in_team(role) == false, ;);
	this->teamer_list_.push_back(role);
}

void BaseTeam::erase_teamer(Int64 role)
{
	this->teamer_list_.remove(role);
}

int BaseTeam::index()
{
	return this->index_;
}

int BaseTeam::is_full()
{
	return this->teamer_list_.size() >= BaseTeam::MAX_COUNT;
}

int BaseTeam::is_empty()
{
	return this->teamer_list_.empty() == true;
}

int BaseTeam::is_leader(Int64 role)
{
	if (this->teamer_list_.empty() == false)
	{
		return this->teamer_list_.front() == role;
	}
	else
	{
		return false;
	}
}

int BaseTeam::is_in_team(Int64 role)
{
	LongList::iterator iter = std::find(this->teamer_list_.begin(),
			this->teamer_list_.end(), role);
	return iter != this->teamer_list_.end();
}

Int64 BaseTeam::leader_id()
{
	JUDGE_RETURN(this->teamer_list_.empty() == false, 0);
	return this->teamer_list_.front();
}

void TrvlScriptTeam::set_team_name()
{
	JUDGE_RETURN(this->teamer_list_.empty() == false, ;);

	Int64 role = this->teamer_list_.front();
	JUDGE_RETURN(role > 0, ;);

	TrvlScriptRole* trvl_role = TRVL_SCRIPT_MONITOR->find_role(role);
	JUDGE_RETURN(trvl_role != NULL, ;);

	this->set_tick_ = ::time(NULL);
	this->team_name_ = trvl_role->name_;
	trvl_role->prep_flag_ = true;

}

void TrvlScriptTeam::make_up_team_list(ProtoTravelTeam* proto)
{
	proto->set_team_id(this->index());
	proto->set_team_name(this->team_name_);
	proto->set_limit_force(this->limit_force_);
	proto->set_teamer_amount(this->teamer_list_.size());
	proto->set_scene_id(this->scene_id_);
	proto->set_sceret_flag(this->sceret_.empty() == false);
	proto->set_auto_start(this->auto_start_);
	proto->set_start_fb(this->start_fb_);
}

void TrvlScriptTeam::reset()
{
	BaseTeam::reset();
	this->scene_id_ = 0;
	this->start_fb_ = 0;
	this->set_tick_ = 0;
	this->script_type_ = 0;
	this->type_ = BaseTeam::TRVL_SCRIPT_TEAM;
}
