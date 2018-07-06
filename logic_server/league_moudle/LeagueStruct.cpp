/*
 * LeagueStruct.cpp
 *
 *  Created on: Aug 8, 2013
 *      Author: peizhibi
 */

#include "LeagueStruct.h"
#include "LogicPlayer.h"
#include "LogicMonitor.h"

#include "GameFont.h"
#include "SerialRecord.h"

#include "MMORole.h"
#include "MMOLeague.h"
#include "GameField.h"

#include "ProtoDefine.h"
#include "LeagueSystem.h"
#include "GameCommon.h"
#include "OpenActivitySys.h"

LeaguerInfo::LeaguerInfo(void)
{
	this->skill_prop_set_.reserve(GameEnum::DEFAULT_VECTOR_SIZE);
	this->skill_prop_set_.push_back(GameEnum::ATTACK);
	this->skill_prop_set_.push_back(GameEnum::DEFENSE);
	this->skill_prop_set_.push_back(GameEnum::HIT);
	this->skill_prop_set_.push_back(GameEnum::AVOID);
	this->skill_prop_set_.push_back(GameEnum::BLOOD_MAX);

	LeaguerInfo::reset();
}

void LeaguerInfo::reset(void)
{
	this->buy_map_.clear();
	this->skill_map_.clear();
	this->apply_list_.clear();
	this->region_draw_.clear();
    this->wave_reward_map_.clear();

	this->open_ = 0;
	this->draw_welfare_ = 0;
	this->wand_donate_ = 0;
	this->gold_donate_ = 0;

	this->send_flag_ = true;
	this->salary_flag_ = false;

    this->leave_type_ = 0;
    this->leave_tick_ = 0;
    this->cur_contri_ = 0;
    this->store_times_ = 0;

    for (uint i = 0; i < this->skill_prop_set_.size(); ++i)
    {
    	int prop_index = this->skill_prop_set_[i];
    	this->skill_map_[prop_index] = 0;
    }

    this->day_admire_times_ = 0;
    this->type_item_num_map_.clear();
    this->day_siege_refresh_tick_ = Time_Value::zero;

}

IntMap *LeaguerInfo::reward_map(int wave)
{
	JUDGE_RETURN(this->wave_reward_map_.count(wave) > 0, NULL);
	return &(this->wave_reward_map_[wave]);
}

LeagueMember::LeagueMember(void)
{
	LeagueMember::reset();
}

void LeagueMember::reset(void)
{
	this->role_index_ = 0;

	this->join_tick_ = 0;
	this->role_name_.clear();
	this->offline_tick_ = ::time(NULL);

	this->role_sex_ = 0;
	this->vip_type_ = 0;
	this->role_lvl_ = 0;
	this->role_force_ = 0;
	this->role_career_ = 0;
	this->new_role_force_ = 0;

	this->league_pos_ = 0;
	this->cur_contribute_ = 0;
	this->today_contribute_ = 0;
	this->total_contribute_ = 0;
	this->offline_contri_ = 0;
	this->today_rank_ = 0;
	this->total_rank_ = 0;

	this->fashion_id_ = 0;
	this->fashion_color_ = 0;
	this->lrf_bet_league_id = 0;
}

int LeagueMember::left_time(void)
{
	return ::time(NULL) - this->offline_tick_;
}

LeagueApplier::LeagueApplier(void)
{
	this->role_index_ = 0;
	this->apply_tick_ = 0;

	this->vip_type_ = 0;
	this->role_lvl_ = 0;
	this->role_career_ = 0;

	this->role_force_ = 0;
	this->role_sex_ = 0;
}

LeagueLogItem::LeagueLogItem(void)
{
	this->log_tick_ = 0;
}

LeagueBossInfo::LeagueBossInfo(void)
{
	this->reset_tick_ = 0;
	LeagueBossInfo::reset();
}

void LeagueBossInfo::reset()
{
	this->boss_index_ 			= 1;
	this->boss_exp_ 			= 0;
	this->normal_summon_type_ 	= false;
	this->super_summon_type_ 	= false;
	this->normal_summon_tick_ 	= 0;
	this->super_summon_tick_ 	= 0;
	this->normal_die_tick_ 		= 0;
	this->super_die_tick_ 		= 0;
	this->super_summon_role_.clear();

	this->reset_activity_tick();
}

void LeagueBossInfo::reset_everyday()
{
	JUDGE_RETURN(GameCommon::is_current_day(this->reset_tick_) == false, ;);

	this->reset_tick_ = ::time(NULL);
	this->reset();
}

void LeagueBossInfo::reset_activity_tick()
{
	const Json::Value& star_time = CONFIG_INSTANCE->league_boss("star_time");
	const Json::Value& end_time  = CONFIG_INSTANCE->league_boss("end_time");
	Int64 day_zero = GameCommon::today_zero();
	this->start_tick_ = day_zero + star_time[0u].asInt()*3600 + star_time[1u].asInt()*60;
	this->end_tick_ = day_zero + end_time[0u].asInt()*3600 + end_time[1u].asInt()*60;
}

League::League(void)
{
    this->league_index_ = 0;
    this->league_lvl_ 	= 0;
    this->league_resource_ = 0;
    this->league_force_ = 0;
    this->league_rank_ 	= GameEnum::LEAGUE_MAX_RANK;
    this->auto_accept_ 	= false;
    this->accept_force_ = League::MIN_FORCE;
    this->flag_lvl_		= 1;
    this->flag_exp_		= 0;
    this->leader_index_	= 0;
    this->create_tick_	= 0;
    this->last_login_tick_ = 0;

    this->region_rank_ = 0;
    this->region_tick_ = 0;
    this->region_leader_reward_ = 0;
    this->last_wave_player_ = 0;
    this->league_daily_total_contribution = 0;
    this->league_impeach_.impeach_timer_.league_ = this;

    this->flag_set_.reserve(GameEnum::DEFAULT_VECTOR_SIZE);
	this->flag_set_.push_back(GameEnum::ATTACK);
	this->flag_set_.push_back(GameEnum::DEFENSE);
	this->flag_set_.push_back(GameEnum::CRIT);
	this->flag_set_.push_back(GameEnum::TOUGHNESS);
	this->flag_set_.push_back(GameEnum::HIT);
	this->flag_set_.push_back(GameEnum::AVOID);
	this->flag_set_.push_back(GameEnum::BLOOD_MAX);
	this->flag_set_.push_back(GameEnum::DAMAGE_MULTI);
	this->flag_set_.push_back(GameEnum::REDUCTION_MULTI);
}

void League::reset(void)
{
	this->league_index_ = 0;
	this->league_lvl_ = 0;

	this->auto_accept_ = false;
	this->accept_force_ = League::MIN_FORCE;

	this->league_resource_ = 0;
	this->league_force_ = 0;
	this->league_rank_ = GameEnum::LEAGUE_MAX_RANK;

	this->create_tick_ = 0;
	this->leader_index_ = 0;
	this->last_login_tick_ = 0;

	this->region_rank_ = 0;
	this->region_tick_ = 0;
	this->region_leader_reward_ = 0;
	this->last_wave_player_ = 0;
	this->league_daily_total_contribution = 0;

	this->flag_lvl_ = 1;
	this->flag_exp_ = 0;

	this->league_name_.clear();
	this->league_intro_.clear();

	this->member_map_.clear();
	this->today_contri_rank_.clear();
	this->total_contri_rank_.clear();
	this->applier_map_.clear();
	this->league_log_set_.clear();
	this->lfb_player_map_.clear();
	this->lfb_rank_vec_.clear();

	this->__lstore.reset();
}

void League::reset_everyday(bool force_reset)
{
	if (this->have_region_reward() == false)
	{
		this->region_rank_ = 0;
		this->region_tick_ = 0;
	}

	this->resource_map_.clear();

	for (MemberMap::iterator iter = this->member_map_.begin();
			iter != this->member_map_.end(); ++iter)
	{
		LeagueMember &member = iter->second;
		member.today_contribute_ = 0;
		member.today_rank_ = 0;
	}

	for (LFbPlayerMap::iterator iter = this->lfb_player_map_.begin();
			iter != this->lfb_player_map_.end(); ++iter)
	{
		LFbPlayer &lfb_player = iter->second;
		lfb_player.reset_every_day();
	}
	this->lfb_rank_vec_.clear();
	this->set_max_last_wave_player();
}

int League::leader_vip()
{
	MemberMap::iterator iter = this->member_map_.find(this->leader_index_);
	return iter != this->member_map_.end() ? iter->second.vip_type_ : 0;
}

int League::leader_career()
{
	MemberMap::iterator iter = this->member_map_.find(this->leader_index_);
	return iter != this->member_map_.end() ? iter->second.role_career_ : 0;
}

int League::leader_sex()
{
	MemberMap::iterator iter = this->member_map_.find(this->leader_index_);
	return iter != this->member_map_.end() ? iter->second.role_sex_ : 0;
}

std::string League::leader_name()
{
	MemberMap::iterator iter = this->member_map_.find(this->leader_index_);
	return iter != this->member_map_.end() ? iter->second.role_name_
			: GameCommon::NullString;
}

bool League::league_full()
{
	return this->member_map_.size() >= LEAGUE_SYSTEM->max_member(this->league_lvl_);
}

bool League::is_can_dismiss()
{
	return GameCommon::validate_time_span(this->last_login_tick_, Time_Value::WEEK) == true
		|| this->member_map_.empty() == true;
}

bool League::is_arrive_max_level()
{
	return this->league_lvl_ >= LEAGUE_SYSTEM->max_level();
}

bool League::is_leader(long role_index)
{
	return this->leader_index_ == role_index;
}

bool League::is_arrive_max_contri(int value, int source)
{
	switch(source)
	{
	case CONTRI_FROM_DONATE:
	{
		if (this->resource_map_[source] < this->fetch_max_contri(source))
		{
			return false;
		}
		else
		{
			return true;
		}

		break;
	}
	}

	return false;
}

void League::dismiss_league()
{
	LongVec member_set;
	member_set.reserve(this->member_map_.size());

	for (MemberMap::iterator iter = this->member_map_.begin();
			iter != this->member_map_.end(); ++iter)
	{
		member_set.push_back(iter->first);
	}

	for (LongVec::iterator iter = member_set.begin(); iter != member_set.end();
			++iter)
	{
		this->handle_quit_league(*iter, GameEnum::LEAGUE_LOG_MEMBER_QUITED);
	}

	this->__lstore.reset();

	this->member_map_.clear();
}

void League::caculate_league_force()
{
	this->league_force_ = 0;
	for (MemberMap::iterator iter = this->member_map_.begin();
			iter != this->member_map_.end(); ++iter)
	{
		iter->second.role_force_ = iter->second.new_role_force_;
		this->league_force_ += iter->second.role_force_;
	}
}

void League::handle_join_league(long role_index, int league_pos)
{
	JUDGE_RETURN(this->validate_member(role_index) == false, ;);

	LeagueMember& league_member = this->member_map_[role_index];
	league_member.role_index_ = role_index;
	league_member.league_pos_ = league_pos;
	league_member.join_tick_ = ::time(NULL);
	league_member.offline_tick_ = ::time(NULL);

	LogicPlayer* player = NULL;
	if (LOGIC_MONITOR->find_player(role_index, player) == 0)
	{
		// online
		league_member.role_name_ = player->name();
		league_member.role_sex_ = player->role_detail().__sex;
		league_member.role_career_ = player->role_detail().__career;
		league_member.role_force_ = player->role_detail().__fight_force;
		league_member.new_role_force_ = league_member.role_force_;
		league_member.role_lvl_ = player->role_detail().__level;
		league_member.vip_type_ = player->vip_detail().__vip_level;
		league_member.cur_contribute_ = player->leaguer_info().cur_contri_;

		player->handle_join_league(this->league_index_);
		player->send_map_league_info();
		player->send_map_skill_info();
		player->send_map_flag_info();
	}
	else if (this->validate_applier(role_index) == true)
	{
		// applier
		LeagueApplier& applier = this->applier_map_[role_index];

		league_member.role_name_ = applier.role_name_;
		league_member.role_sex_ = applier.role_sex_;
		league_member.role_career_ = applier.role_career_;
		league_member.role_force_ = applier.role_force_;
		league_member.new_role_force_ = league_member.role_force_;
		league_member.role_lvl_ = applier.role_lvl_;
		league_member.vip_type_ = applier.vip_type_;
	}
	else
	{
		// db, no use
		MMORole::load_league_member(league_member);
	}

	this->caculate_league_force();
	this->add_league_member_log(GameEnum::LEAGUE_LOG_MEMBER_JOIN, league_member.role_name_);
	GameCommon::request_save_role_long(role_index, Role::LEAGUE_ID, this->league_index_);

	SERIAL_RECORD->record_other_serial(this->leader_index_, 0, 0,
			LEAGUE_MAIN_SERIAL, SUB_LEAGUE_MEMBER, this->member_map_.size());

	LEAGUE_SYSTEM->erase_all_apply(role_index);
}

void League::handle_quit_league(long role_index, int leave_type)
{
	LeagueMember* league_member = this->league_member(role_index);
	JUDGE_RETURN(league_member != NULL, ;);

	this->add_league_member_log(leave_type, league_member->role_name_);
	GameCommon::request_save_role_long(role_index, Role::LEAGUE_ID, 0);

	this->member_map_.erase(role_index);
	this->caculate_league_force();

	LogicPlayer* player = NULL;
	if (LOGIC_MONITOR->find_player(role_index, player) == 0)
	{
		player->handle_quit_league(leave_type);
		player->notify_quit_league(this->league_index_, player->name());
		player->send_map_league_info();
		player->send_map_skill_info();
		player->send_map_flag_info();
	}
	else
	{
		MMOLeague::save_quit_info(role_index, leave_type);
	}

	SERIAL_RECORD->record_other_serial(this->leader_index_, 0, 0,
			LEAGUE_MAIN_SERIAL, SUB_LEAGUE_MEMBER, this->member_map_.size());
}

bool League::can_view_apply(long role_index)
{
	JUDGE_RETURN(this->validate_member(role_index) == true, false);

	LeagueMember& league_member = this->member_map_[role_index];
	JUDGE_RETURN(league_member.league_pos_ >= GameEnum::LEAGUE_POS_ELDER, false);

	return true;
}

bool League::can_operate_league(long role_index)
{
	JUDGE_RETURN(this->validate_member(role_index) == true, false);

	LeagueMember& league_member = this->member_map_[role_index];
	JUDGE_RETURN(league_member.league_pos_ >= GameEnum::LEAGUE_POS_DEPUTY, false);

	return true;
}

bool League::online_flag(Int64 role_index)
{
	LogicPlayer* player = NULL;
	JUDGE_RETURN(LOGIC_MONITOR->find_player(role_index, player) == 0, false);
	return true;
}

bool League::have_region_reward()
{
	JUDGE_RETURN(this->region_rank_ > 0, false);
	return ::time(NULL) < this->region_tick_;
}

bool League::auto_accept()
{
	return this->auto_accept_;
}

bool League::arrive_auto_accept(int level)
{
	JUDGE_RETURN(this->auto_accept() == true, false);
	//	return fight_force >= this->accept_force_;
	int accept_level = CONFIG_INSTANCE->league("accept_level").asInt();
	return level >= accept_level;
}

bool League::arrive_auto_accept(Int64 role_id)
{
	JUDGE_RETURN(this->validate_applier(role_id) == true, false);
	return this->arrive_auto_accept(this->applier_map_[role_id].role_lvl_);
}

int League::validate_join(long role_index)
{
	JUDGE_RETURN(this->validate_applier(role_index) == true,
			ERROR_LEAGUE_APPLY_CANCEL);

	JUDGE_RETURN(LEAGUE_SYSTEM->player_have_league(role_index) == false,
			ERROR_HAVE_LEAGUE);

	return 0;
}

int League::validate_member(long role_index)
{
	return this->member_map_.count(role_index) > 0;
}

int League::member_pos(long role_index)
{
	MemberMap::iterator iter = this->member_map_.find(role_index);

	if (iter != this->member_map_.end())
	{
		return iter->second.league_pos_;
	}
	else
	{
		return GameEnum::LEAGUE_POS_NONE;
	}
}

int League::transfer_leader(long role_index)
{
	JUDGE_RETURN(this->leader_index_ != role_index, -1);

	LeagueMember* league_member = this->league_member(role_index);
	JUDGE_RETURN(league_member != NULL, -1);

	LeagueMember* leader_member = this->league_member(this->leader_index_);
	if (leader_member != NULL)
	{
		leader_member->league_pos_ = GameEnum::LEAGUE_POS_NONE;
		this->add_league_member_log(GameEnum::LEAGEU_LOG_LEADER_TRANSFER,
				leader_member->role_name_, league_member->role_name_);
	}

	league_member->league_pos_ = GameEnum::LEAGUE_POS_LEADER;
	this->leader_index_ = role_index;

	LEAGUE_SYSTEM->sync_leader_to_map(this);

	return 0;
}


long League::fetch_next_leader()
{
	int max_contribute = -1;
	long next_leader_index = 0;

	for (MemberMap::iterator iter = this->member_map_.begin();
			iter != this->member_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(this->leader_index_ != iter->first);

		if (iter->second.league_pos_ == GameEnum::LEAGUE_POS_DEPUTY)
		{
			return iter->first;
		}

		if (iter->second.total_contribute_ > max_contribute)
		{
			next_leader_index = iter->first;
			max_contribute = iter->second.total_contribute_;
		}
	}

	return next_leader_index;
}

const Json::Value& League::set_up()
{
	return LEAGUE_SYSTEM->league_set_up(this->league_lvl_);
}

int League::league_pos_count(int league_pos)
{
	int pos_count = 0;

	for (MemberMap::iterator iter = this->member_map_.begin();
			iter != this->member_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second.league_pos_ == league_pos);
		pos_count += 1;
	}

	return pos_count;
}

int League::fetch_max_contri(int source)
{
	switch(source)
	{
	case CONTRI_FROM_DONATE:
	{
		return this->member_map_.size() * 2000;
	}
	}

	return INT_MAX;
}

LeagueMember* League::league_member(long role_index)
{
	MemberMap::iterator iter = this->member_map_.find(role_index);

	if (iter != this->member_map_.end())
	{
		return &iter->second;
	}
	else
	{
		return NULL;
	}
}

void League::push_applier(long role_index)
{
	JUDGE_RETURN(this->validate_applier(role_index) == false, ;);

	LogicPlayer* player = NULL;
	JUDGE_RETURN(LOGIC_MONITOR->find_player(role_index, player) == 0, ;);

	LeagueApplier& league_applier = this->applier_map_[role_index];
	league_applier.role_index_ = role_index;
	league_applier.apply_tick_ = ::time(NULL);
	league_applier.role_name_ = player->name();

	league_applier.vip_type_ = player->vip_detail().__vip_level;
	league_applier.role_sex_ = player->role_detail().__sex;
	league_applier.role_lvl_ = player->role_detail().__level;
	league_applier.role_career_ = player->role_detail().__career;
	league_applier.role_force_ = player->role_detail().__fight_force;
}

void League::erase_applier(long role_index)
{
	this->applier_map_.erase(role_index);
}

bool League::validate_applier(long role_index)
{
	return this->applier_map_.count(role_index) > 0;
}

void League::sync_map_league_info()
{
//	Proto30400437 proto_league;
//	proto_league.set_league_index(this->league_index_);
//
//	ProtoSyncLeague* sync_league = proto_league.mutable_league_info();
//	sync_league->set_league_level(this->league_lvl_);
//
//	LOGIC_MONITOR->dispatch_to_scene(GameEnum::LBONFIRE_SCENE_ID, &proto_league);
}

void League::add_league_resource(long role_index, int contribute, int source)
{
	JUDGE_RETURN(contribute > 0, ;);

	this->league_resource_ += contribute;
	this->resource_map_[source] += contribute;
	this->check_and_upgrade_league();
}

void League::fetch_sort_member(PairObjVec& role_set)
{
	FourObjVec online_set, offline_set;
	online_set.reserve(this->member_map_.size());
	offline_set.reserve(this->member_map_.size());

	LongMap offline_time_map;

	// fetch
	for (MemberMap::iterator iter = this->member_map_.begin();
			iter != this->member_map_.end(); ++iter)
	{
		FourObj obj;
		obj.id_ = iter->first;
		obj.tick_ = iter->second.join_tick_;

		obj.first_value_ = iter->second.league_pos_;
		obj.second_value_ = iter->second.role_force_;

		if (this->online_flag(iter->first) == true)
		{
			online_set.push_back(obj);
		}
		else
		{
			offline_set.push_back(obj);
			offline_time_map[iter->first] = iter->second.left_time();
		}
	}

	// sort
	std::sort(online_set.begin(), online_set.end(), GameCommon::four_comp_by_desc);
	std::sort(offline_set.begin(), offline_set.end(), GameCommon::four_comp_by_desc);

	// online
	role_set.reserve(this->member_map_.size());
	for (FourObjVec::iterator iter = online_set.begin(); iter != online_set.end();
			++iter)
	{
		role_set.push_back(PairObj(iter->id_, 0));
	}

	// offline
	for (FourObjVec::iterator iter = offline_set.begin(); iter != offline_set.end();
			++iter)
	{
		role_set.push_back(PairObj(iter->id_, offline_time_map[iter->id_]));
	}
}

void League::fetch_sort_applier(ThreeObjVec& applier_set)
{
	applier_set.reserve(this->applier_map_.size());

	for (ApplierMap::iterator iter = this->applier_map_.begin();
			iter != this->applier_map_.end(); ++iter)
	{
		ThreeObj obj;
		obj.id_ = iter->first;

		obj.tick_ = iter->second.apply_tick_;
		obj.value_ = iter->second.role_force_;

		applier_set.push_back(obj);
	}

	std::sort(applier_set.begin(), applier_set.end(), GameCommon::three_comp_by_desc);
}

void League::make_up_league_list_info(long role_index, ProtoLeagueItem* league_item)
{
	league_item->set_league_index(this->league_index_);
	league_item->set_league_name(this->league_name_);
	league_item->set_rank_index(this->league_rank_);

	league_item->set_league_lvl(this->league_lvl_);
	league_item->set_current_count(this->member_map_.size());
	league_item->set_league_force(this->league_force_);

	int max_member = LEAGUE_SYSTEM->max_member(this->league_lvl_);
	league_item->set_max_role(max_member);

	if (this->league_full() == true)
	{
		league_item->set_apply_flag(GameEnum::LEAGUE_FULL);
	}
	else
	{
		league_item->set_apply_flag(this->validate_applier(role_index));
	}

	LeagueMember* leader = this->league_member(this->leader_index_);
	JUDGE_RETURN(leader != NULL, ;);

	league_item->set_league_leader(leader->role_name_);
	league_item->set_leader_id(leader->role_index_);
}

void League::make_up_league_list_info(ProtoLeagueItem* league_item)
{
	league_item->set_league_index(this->league_index_);
	league_item->set_league_name(this->league_name_);
	league_item->set_rank_index(this->league_rank_);
	league_item->set_league_lvl(this->league_lvl_);
	league_item->set_league_force(this->league_force_);
}

void League::sort_rank()
{
	this->today_contri_rank_.clear();
	this->total_contri_rank_.clear();
	for (MemberMap::iterator iter = this->member_map_.begin();
			iter != this->member_map_.end(); ++iter)
	{
		LeagueMember &member = iter->second;

		if (member.today_contribute_ > 0)
		{
			ThreeObj today_obj;
			today_obj.id_ = member.role_index_;
			today_obj.tick_ = member.vip_type_;
			today_obj.value_ = member.today_contribute_;
			this->today_contri_rank_.push_back(today_obj);

			this->league_daily_total_contribution += member.today_contribute_;
		}
		if (member.total_contribute_ > 0)
		{
			ThreeObj total_obj;
			total_obj.id_ = member.role_index_;
			total_obj.tick_ = member.vip_type_;
			total_obj.value_ = member.total_contribute_;
			this->total_contri_rank_.push_back(total_obj);
		}
	}
	std::sort(this->today_contri_rank_.begin(), this->today_contri_rank_.end(),
			GameCommon::three_comp_by_desc);
	std::sort(this->total_contri_rank_.begin(), this->total_contri_rank_.end(),
			GameCommon::three_comp_by_desc);

	int rank = 1;
	for (ThreeObjVec::iterator iter = this->today_contri_rank_.begin();
			iter != this->today_contri_rank_.end(); ++iter)
	{
		LeagueMember &member = this->member_map_[iter->id_];
		member.today_rank_ = rank;
		++rank;
	}
	rank = 1;
	for (ThreeObjVec::iterator iter = this->total_contri_rank_.begin();
			iter != this->total_contri_rank_.end(); ++iter)
	{
		LeagueMember &member = this->member_map_[iter->id_];
		member.total_rank_ = rank;
		++rank;
	}
}

void League::add_league_log(const LeagueLogItem& log_item)
{
	this->league_log_set_.push_front(log_item);
	JUDGE_RETURN(this->league_log_set_.size() > GameEnum::LEAGUE_LOG_MAX_COUNT, ;);

	this->league_log_set_.pop_back();
}

void League::add_league_member_log(int log_type, const string& role_name)
{
	const Json::Value &league_conf = CONFIG_INSTANCE->league_log(log_type);
	JUDGE_RETURN(league_conf != Json::Value::null, ;);

	char log_content[GameEnum::DEFAULT_MAX_CONTENT_LEGNTH + 1] = {0};
	::snprintf(log_content, GameEnum::DEFAULT_MAX_CONTENT_LEGNTH, league_conf["content"].asCString(), role_name.c_str());
	log_content[GameEnum::DEFAULT_MAX_CONTENT_LEGNTH] = '\0';

	LeagueLogItem log_item;
	log_item.log_tick_ = ::time(NULL);
	log_item.log_conent_ = log_content;

	this->add_league_log(log_item);
}

void League::add_league_member_log(int log_type, const string& first_name,
		const string& second_name)
{
	const Json::Value &league_conf = CONFIG_INSTANCE->league_log(log_type);
	JUDGE_RETURN(league_conf != Json::Value::null, ;);

	char log_content[GameEnum::DEFAULT_MAX_CONTENT_LEGNTH + 1] = {0};
	::snprintf(log_content, GameEnum::DEFAULT_MAX_CONTENT_LEGNTH,
			league_conf["content"].asCString(), first_name.c_str(), second_name.c_str());

	LeagueLogItem log_item;
	log_item.log_tick_ = ::time(NULL);
	log_item.log_conent_ = log_content;

	this->add_league_log(log_item);
}

void League::add_league_donate_log(int log_type, int donate_value, const string& role_name)
{
	const Json::Value &league_conf = CONFIG_INSTANCE->league_log(log_type);
	JUDGE_RETURN(league_conf != Json::Value::null, ;);

	char log_content[GameEnum::DEFAULT_MAX_CONTENT_LEGNTH + 1] = {0};
	::snprintf(log_content, GameEnum::DEFAULT_MAX_CONTENT_LEGNTH, league_conf["content"].asCString(),
			role_name.c_str(), donate_value);
	log_content[GameEnum::DEFAULT_MAX_CONTENT_LEGNTH] = '\0';

	LeagueLogItem log_item;
	log_item.log_tick_ = ::time(NULL);
	log_item.log_conent_ = log_content;

	this->add_league_log(log_item);
}

void League::change_name_log(int log_type, const string& role_name)
{
	const Json::Value &league_conf = CONFIG_INSTANCE->league_log(log_type);
	JUDGE_RETURN(league_conf != Json::Value::null, ;);

	char log_content[GameEnum::DEFAULT_MAX_CONTENT_LEGNTH + 1] = {0};
	::snprintf(log_content, GameEnum::DEFAULT_MAX_CONTENT_LEGNTH, league_conf["content"].asCString(),
			role_name.c_str(), this->league_name_.c_str());
	log_content[GameEnum::DEFAULT_MAX_CONTENT_LEGNTH] = '\0';

	LeagueLogItem log_item;
	log_item.log_tick_ = ::time(NULL);
	log_item.log_conent_ = log_content;

	this->add_league_log(log_item);
}

void League::check_leader_offline()
{
	JUDGE_RETURN(this->online_flag(this->leader_index_) == false, ;);

	LeagueMember* leader = this->league_member(this->leader_index_);
	JUDGE_RETURN(leader != NULL, ;);

	static int transfer_tick = CONFIG_INSTANCE->league("transfer_tick").asInt();
	JUDGE_RETURN(GameCommon::validate_time_span(leader->offline_tick_,
			transfer_tick) == true, ;);

	long leader_index = this->fetch_next_leader();
	this->transfer_leader(leader_index);
}

void League::check_leader_validate()
{
	JUDGE_RETURN(this->validate_member(this->leader_index_) == false, ;);
	JUDGE_RETURN(this->member_map_.empty() == false, ;);

	long leader_index = this->fetch_next_leader();
	this->transfer_leader(leader_index);
}

void League::check_and_upgrade_league()
{
	const Json::Value &league_item = LEAGUE_SYSTEM->league_set_up(this->league_lvl_);
	JUDGE_RETURN(league_item != Json::Value::null, ;);

	int need_resource = league_item["upgrade_resource"].asInt();
	JUDGE_RETURN(need_resource > 0 && this->league_resource_ >= need_resource, ;);

	this->league_resource_ -= need_resource;
	this->league_lvl_ += 1;

	SERIAL_RECORD->record_other_serial(this->leader_index_, 0, 0,
			LEAGUE_MAIN_SERIAL, SUB_LEAGUE_LEVEL, this->league_lvl_,
			this->league_resource_, this->member_map_.size());

	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_int(para_vec, this->league_lvl_);
	int shout_id = CONFIG_INSTANCE->league("uplevel_shout").asInt();
	GameCommon::announce(this->league_index_, shout_id, &para_vec);

	// 检测技能升级红点
	for (League::MemberMap::iterator iter = this->member_map_.begin();
			iter != this->member_map_.end(); ++iter)
	{
		LogicPlayer *member_player = NULL;
		if(LOGIC_MONITOR->find_player(iter->first, member_player) == 0)
		{
			member_player->check_skill_red_point();
		}
	}

	// check and upgrade next
	this->check_and_upgrade_league();
}

void League::impeach_leader(const Int64 role_id)
{
	LeagueImpeach &league_impeach = this->league_impeach_;
	league_impeach.impeach_role_ = role_id;
	int last_tick = CONFIG_INSTANCE->league("transfer_last").asInt();
	league_impeach.impeach_tick_ = GameCommon::today_zero() + last_tick;
	league_impeach.voter_map_[role_id] = true;

	LeagueImpeach::ImpeachTimer &impeach_timer = league_impeach.impeach_timer_;
	impeach_timer.cancel_timer();
	impeach_timer.schedule_timer(1);
}

void League::impeach_voter(const Int64 role_id, int vote_type)
{
	LeagueImpeach &league_impeach = this->league_impeach_;
	JUDGE_RETURN(league_impeach.impeach_role_ > 0 && league_impeach.impeach_tick_ > 0, ;);
	JUDGE_RETURN(league_impeach.impeach_timer_.is_registered() == true, ;);

	if (vote_type == 0)
	{
		//为了存数据库
		vote_type = 2;
	}
	league_impeach.voter_map_[role_id] = vote_type;
	JUDGE_RETURN(this->cal_impeach_enough() == true, ;);

	LeagueImpeach::ImpeachTimer &impeach_timer = league_impeach.impeach_timer_;
	impeach_timer.cancel_timer();
}

void League::handle_start_impeach()
{
	LeagueImpeach &league_impeach = this->league_impeach_;
	if (league_impeach.impeach_role_ > 0 && league_impeach.impeach_tick_ > 0)
	{
		Int64 cur_tick = ::time(NULL);
		int left_tick = league_impeach.impeach_tick_ - cur_tick;
		if (left_tick <= 0)
		{
			this->handle_timeout_impeach();
		}
		else
		{
			LeagueImpeach::ImpeachTimer &impeach_timer = league_impeach.impeach_timer_;
			impeach_timer.cancel_timer();
			impeach_timer.schedule_timer(1);
		}
	}
	else
	{
		league_impeach.reset();
	}
}

int League::handle_timeout_impeach()
{
	LeagueImpeach &league_impeach = this->league_impeach_;
	LeagueImpeach::ImpeachTimer &impeach_timer = league_impeach.impeach_timer_;
	impeach_timer.cancel_timer();

	this->cal_impeach_enough();

	return 0;
}

bool League::check_impeach_role_leave()
{
	LeagueImpeach &league_impeach = this->league_impeach_;
	Int64 impeach_role = league_impeach.impeach_role_;
	JUDGE_RETURN(impeach_role > 0, false);

	LeagueMember *member = this->league_member(impeach_role);
	if (member == NULL)
	{
		league_impeach.reset();
		return false;
	}
	return true;
}

bool League::cal_impeach_enough()
{
	LeagueImpeach &league_impeach = this->league_impeach_;
	int apply_num = this->impeach_now_num();
	int need_num = this->impeach_need_num();
	JUDGE_RETURN(apply_num >= need_num, false);

	this->transfer_leader(league_impeach.impeach_role_);
	LogicPlayer *player = NULL;
	if(LOGIC_MONITOR->find_player(league_impeach.impeach_role_, player) == 0)
	{
		player->impeach_timeout();
	}

	Proto10100614 proto_info;
	for (League::MemberMap::iterator iter = this->member_map_.begin();
			iter != this->member_map_.end(); ++iter)
	{
		LogicPlayer *member_player = NULL;
		if(LOGIC_MONITOR->find_player(iter->first, member_player) == 0)
		{
			member_player->fetch_league_info();
			member_player->fetch_league_member_list(&proto_info);
		}
	}

	league_impeach.reset();

	return true;
}

bool League::is_in_impeach()
{
	LeagueImpeach &league_impeach = this->league_impeach_;
	if (league_impeach.impeach_timer_.is_registered() == true)
		return true;
	return false;
}

bool League::is_vote(const Int64 role_id)
{
	LeagueImpeach &league_impeach = this->league_impeach_;
	if (league_impeach.voter_map_.count(role_id) > 0)
		return true;
	return false;
}

int League::impeach_need_num()
{
	double impeach_precent = CONFIG_INSTANCE->league("impeach_num").asInt();
	double member_num = this->member_map_.size();
	int need_num = std::ceil(member_num * impeach_precent / 100);
	return need_num;
}

int League::impeach_now_num()
{
	LeagueImpeach &league_impeach = this->league_impeach_;
	int apply_num = 0;
	for (LongMap::iterator iter = league_impeach.voter_map_.begin();
			iter != league_impeach.voter_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second == 1);
		++apply_num;
	}
	return apply_num;
}

int League::fetch_wave_player(int wave)
{
	int amount = 0;
	for (LFbPlayerMap::iterator iter = this->lfb_player_map_.begin();
			iter != this->lfb_player_map_.end(); ++iter)
	{
		LFbPlayer &lfb_player = iter->second;
		JUDGE_CONTINUE(lfb_player.wave_ >= wave);

		++amount;
	}

	return amount;
}

LFbPlayer *League::lfb_player(Int64 role_id)
{
	JUDGE_RETURN(this->lfb_player_map_.count(role_id) > 0, NULL);
	return &(this->lfb_player_map_[role_id]);
}

void League::sort_lfb_player()
{
	this->lfb_rank_vec_.clear();
	for (LFbPlayerMap::iterator iter = this->lfb_player_map_.begin();
			iter != this->lfb_player_map_.end(); ++iter)
	{
		LFbPlayer &lfb_player = iter->second;
		if (lfb_player.wave_ > 0)
		{
			ThreeObj obj;
			obj.id_ = lfb_player.role_id_;
			obj.tick_ = lfb_player.tick_;
			obj.value_ = lfb_player.wave_;
			this->lfb_rank_vec_.push_back(obj);
		}
	}
	std::sort(this->lfb_rank_vec_.begin(), this->lfb_rank_vec_.end(),
			GameCommon::three_comp_by_desc);
}

void League::set_max_last_wave_player()
{
	this->last_wave_player_ = 0;

	ThreeObjVec rank_vec;
	for (LFbPlayerMap::iterator iter = this->lfb_player_map_.begin();
			iter != this->lfb_player_map_.end(); ++iter)
	{
		LFbPlayer &lfb_player = iter->second;
		if (lfb_player.last_wave_ > 0)
		{
			ThreeObj obj;
			obj.id_ = lfb_player.role_id_;
			obj.tick_ = lfb_player.tick_;
			obj.value_ = lfb_player.wave_;
			rank_vec.push_back(obj);
		}
	}
	JUDGE_RETURN(rank_vec.size() > 0, ;);

	std::sort(rank_vec.begin(), rank_vec.end(), GameCommon::three_comp_by_desc);

	this->last_wave_player_ = rank_vec[0].id_;
}

void League::notify_store()
{
	LongSet& view_list = this->__lstore.view_list;
	for(LongSet::iterator iter = view_list.begin();iter != view_list.end();)
	{
		LogicPlayer* player = NULL;
		if(LOGIC_MONITOR->find_player(*iter,player) != 0)
		{
			view_list.erase(iter++);
			continue;
		}
		player->fetch_store();
		++iter;
	}
}

int LeagueImpeach::ImpeachTimer::type(void)
{
	return GTT_LOGIC_ONE_SEC;
}

int LeagueImpeach::ImpeachTimer::handle_timeout(const Time_Value &tv)
{
	bool is_member = this->league_->check_impeach_role_leave();
	Int64 cur_tick = ::time(NULL);
	int impeach_tick = this->league_->league_impeach_.impeach_tick_;
	if (is_member == true && cur_tick >= impeach_tick)
	{
		this->league_->handle_timeout_impeach();
		this->league_->league_impeach_.reset();
	}
	return 0;
}

LeagueImpeach::LeagueImpeach()
{
	this->impeach_role_ = 0;
	this->impeach_tick_ = 0;
}

void LeagueImpeach::reset(void)
{
	this->impeach_role_ = 0;
	this->impeach_tick_ = 0;
	this->voter_map_.clear();
	this->impeach_timer_.cancel_timer();
}


ApplyInfo::ApplyInfo(void)
{
	ApplyInfo::reset();
}

void ApplyInfo::reset(void)
{
	this->item_id = 0;
	this->item_num = 0;
	this->role_id = 0;
	this->item_unique_id = 0;
	this->role_name.clear();
}

ApplyHistory::ApplyHistory(void)
{
	ApplyHistory::reset();
}

void ApplyHistory::reset(void)
{
	this->tick = 0;
	this->item_id = 0;
	this->item_num = 0;
	this->opt = 0;
	this->role_name = "";
	this->checker_name = "";
}

LeagueStore::LeagueStore(void)
{
	LeagueStore::reset();
}

void LeagueStore::reset(void)
{
	this->apply_map.clear();
	this->apply_vec.clear();
	this->package.reset();
	this->lock_map.clear();
	this->apply_history.clear();
	this->view_list.clear();
	this->item_role_map.clear();
	this->next_refresh_tick = 0;
}

int LeagueStore::search_empty_index_i(void)
{
	JUDGE_RETURN(this->package.left_capacity() > 0, ERROR_NO_ENOUGH_SPACE);

	Int64 now = ::time(NULL);
	for (int i = 0; i < this->package.pack_size_; ++i)
	{
		Int64& pack_index = this->lock_map[i];
		if(pack_index > 0 && pack_index < now)
		{
			pack_index = 0;
		}
		if(pack_index == 0)
		{
			if(this->package.item_list_map_.count(i) > 0)
			{
				pack_index = -1;
				continue;
			}
			return i;
		}
	}
	return ERROR_SERVER_INNER;
}


void LeagueStore::luck_block(int index)
{
	this->lock_map[index] = ::time(NULL) + 3;
}

void LeagueStore::apply_his_timeout()
{
	const Int64 timeout = ::time(0)- 5 * 24 * 3600;
	while(!this->apply_history.empty() && this->apply_history.front().tick < timeout)
	{
		this->apply_history.pop_front();
	}
}

CheerRecord::CheerRecord()
{
	CheerRecord::reset();
}

void CheerRecord::reset()
{
	this->role_id_ = 0;
	this->type_ = 0;
	this->is_active_ = 0;
	this->time_ = 0;
	this->name_.clear();
}

void CheerRecord::serialize(ProtoCheerRecord *record)
{
	record->set_role_id(this->role_id_);
	record->set_type(this->type_);
	record->set_is_active(this->is_active_);
	record->set_time(this->time_);
	record->set_role_name(this->name_);
}

LFbPlayer::LFbPlayer()
{
	LFbPlayer::reset();
}

void LFbPlayer::reset()
{
	this->role_id_ 		= 0;
	this->tick_ 		= 0;
	this->sex_ 			= 0;
	this->wave_ 		= 0;
	this->last_wave_ 	= 0;
	this->cheer_ 		= 0;
	this->encourage_ 	= 0;
	this->be_cheer_ 	= 0;
	this->be_encourage_ = 0;
	this->name_.clear();

	this->record_vec_.clear();
}

void LFbPlayer::reset_every_day()
{
	this->last_wave_ = this->wave_;

	this->tick_ 		= 0;
	this->wave_ 		= 0;
	this->cheer_ 		= 0;
	this->encourage_ 	= 0;
	this->be_cheer_ 	= 0;
	this->be_encourage_ = 0;
	this->record_vec_.clear();
}

int LFbPlayer::fetch_wave()
{
	return this->wave_ >= this->last_wave_ ? this->wave_ : this->last_wave_;
}

int LFbPlayer::fetch_buff_num()
{
	return this->be_cheer_ >= this->be_encourage_ ? this->be_cheer_ : this->be_encourage_;
}

int LFbPlayer::fetch_in_record(Int64 role_id, int type, int is_active)
{
	for (CheerRecordVec::iterator iter = this->record_vec_.begin();
			iter != this->record_vec_.end(); ++iter)
	{
		JUDGE_CONTINUE(role_id == iter->role_id_ && type == iter->type_
				&& is_active == iter->is_active_);

		return true;
	}
	return false;
}

CheerRecord LFbPlayer::create_record(Int64 role_id, int type, int is_active, string name)
{
	CheerRecord record;
	record.role_id_ = role_id;
	record.name_ = name;
	record.type_ = type;
	record.is_active_ = is_active;
	record.time_ = ::time(NULL) - GameCommon::today_zero();
	this->record_vec_.push_back(record);

	return record;
}
