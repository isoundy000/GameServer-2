/*
 * MapMapStruct.cpp
 *
 *  Created on: Feb 15, 2014
 *      Author: peizhibi
 */

#include "MapMapStruct.h"
#include "ProtoDefine.h"
#include "MapPlayerEx.h"
#include "Scene.h"

void MapRoleDetail::reset(void)
{
	BaseRoleInfo::reset();

	this->__league_name.clear();
	this->__league_pos = 0;
	this->__trvl_server_flag.clear();

	this->drop_act_ = 0;
	this->is_big_act_time_ = 0;
	this->server_tick_ = 0;
	this->combine_tick_ = 0;
	this->escort_times = 0;
	this->protect_times = 0;
	this->rob_times = 0;
	this->fb_flag_ = 0;
	this->team_flag_ = 0;
	this->prev_force_ = 0;
	this->client_type_ = 0;
	this->sign_trvl_team_ = 0;
	this->notify_trvl_chat_ = 0;
	this->save_trvl_scene_ = 0;
	this->prev_force_map_.clear();

	this->__refresh_free_relive_tick = Time_Value::zero;
	this->__used_free_relive = 0;
	this->__wedding_giftbox_tick = Time_Value::zero;
	this->__wedding_giftbox_times = 0;
	this->__collect_chest_amount = 0;      // 活动期间内采集宝箱数
	this->hickty_id_ = 0;
	this->health_ = 1;

	this->__save_scene.reset();		//保存场景信息
	this->__scene_history.clear();
}

void LWarRoleInfo::reset()
{
	this->id_ = 0;
	this->sex_ = 0;
	this->name_.clear();
	this->league_index_ = 0;
	this->leader_index_ = 0;
	this->league_name_.clear();
	this->leader_name_.clear();
	this->tick_ = 0;
	this->state_ = 0;
	this->space_ = 0;
	this->camp_index_ = 0;
	this->score_ = 0;
	this->reward_map_.clear();
	this->record_map_.clear();
}

int LWarRoleInfo::camp_id()
{
	return this->camp_index_ + 1;
}

void LWarLeagueInfo::reset()
{
	this->league_index_ = 0;
	this->leader_index_ = 0;
	this->league_name_.clear();
	this->leader_name_.clear();
	this->tick_ = 0;
	this->score_ = 0;
	this->rank_ = 0;
	this->space_id_ = 0;

	this->lwar_player_.clear();
	this->enter_lwar_player_.clear();
}

void LWarLeagueInfo::serialize(ProtoLeagueRankInfo *proto)
{
	proto->set_league_index(this->league_index_);
	proto->set_league_name(this->league_name_);
	proto->set_score(this->score_);
	proto->set_rank(this->rank_);
	proto->set_space_id(this->space_id_);
}

void LWarLeagueHurt::reset(void)
{
	this->league_index_ = 0;
	this->league_name_.clear();
	this->tick_ = 0;
	this->hurt_value_ = 0;
	this->rank_ = 0;
}

LeagueWarInfo::LeagueWarInfo()
{
	LeagueWarInfo::reset();
}

void LeagueWarInfo::reset()
{
	this->tick_ = 0;
	this->total_num_ = 0;
	this->lwar_rank_map_.clear();
}

Int64 LeagueWarInfo::find_camp_league(int rank_index)
{
	LWarRankMap::iterator iter = this->lwar_rank_map_.find(rank_index);
	JUDGE_RETURN(iter != this->lwar_rank_map_.end(), 0);

	return iter->second.league_index_;
}

LeagueWarInfo::LeagueWarRank::LeagueWarRank()
{
	LeagueWarRank::reset();
}

void LeagueWarInfo::LeagueWarRank::reset()
{
	this->league_index_ = 0;
	this->league_name_.clear();
	this->leader_.clear();

	this->rank_ = 0;
	this->force_ = 0;
	this->score_ = 0;
	this->flag_lvl_ = 1;
}

int MAttackInfo::camp_id()
{
	return this->camp_index_ + 1;
}

MAttackInfo::MAttackInfo(void)
{
	MAttackInfo::reset();
}

void MAttackInfo::reset(void)
{
	this->id_ = 0;
	this->tick_ = 0;
	this->sex_ = 0;
	this->name_.clear();
	this->score_ = 0;
	this->state_ = 0;
	this->camp_index_ = 0;
	this->reward_map_.clear();
}

MAttackLabelRecord::MAttackLabelRecord()
{
	MAttackLabelRecord::reset();
}

void MAttackLabelRecord::reset(void)
{
	this->label_id_ = 0;
	this->role_id_ = 0;
	this->role_sex_ = 0;
	this->role_name_.clear();
}

void WorldBossInfo::reset(void)
{
	this->scene_id_ = 0;
	this->status_ = 0;
	this->die_tick_ = 0;
	this->cur_blood_ = 0;
	this->killer_ = 0;
	this->killer_name_.clear();
}

TravPeakTeam::TravPeakTeamer::TravPeakTeamer(void)
{
	TravPeakTeamer::reset();
}

void TravPeakTeam::TravPeakTeamer::reset(void)
{
    this->__teamer_id = 0;
    this->__teamer_name.clear();
    this->__teamer_sex = 0;
    this->__teamer_career = 0;
    this->__teamer_level = 0;
    this->__teamer_force = 0;
    this->__offline_teamer_id = 0;
    this->__offline_player = NULL;
}

void TravPeakTeam::TravPeakTeamer::serialize(ProtoTeamer *msg)
{
    msg->set_role_id(this->__teamer_id);
    msg->set_role_name(this->__teamer_name);
    msg->set_role_sex(this->__teamer_sex);
    msg->set_role_career(this->__teamer_career);
    msg->set_role_level(this->__teamer_level);
    msg->set_role_force(this->__teamer_force);
}

void TravPeakTeam::TravPeakTeamer::unserialize(const ProtoTeamer &msg)
{
	this->__teamer_id 	  = msg.role_id();
	this->__teamer_name   = msg.role_name();
	this->__teamer_sex 	  = msg.role_sex();
	this->__teamer_career = msg.role_career();
	this->__teamer_level  = msg.role_level();
	this->__teamer_force  = msg.role_force();
}

void TravPeakTeam::reset(void)
{
	BaseServerInfo::reset();

	this->__team_id = 0;
	this->__team_name.clear();
	this->__leader_id = 0;
	this->__quality_times = 0;
	this->__score = 0;
	this->__rank = 0;
	this->__continue_win = 0;
	this->__update_tick = 0;

	this->__space_id = 0;
	this->__camp_id = 0;
	this->__state = 0;
	this->__last_rival = 0;
	this->__start_tick = 0;
	this->__last_add_score = 0;
	this->__last_reward_id = 0;

	this->__teamer_map.clear();
}

void TravPeakTeam::serialize(ProtoTravelTeam *msg)
{
	msg->set_team_id(this->__team_id);
	msg->set_team_name(this->__team_name);
	msg->set_leader_id(this->__leader_id);
	msg->set_teamer_amount(this->__teamer_map.size());

	TravPeakTeamer *teamer_info = this->find_teamer(this->__leader_id);
	if (teamer_info != NULL)
	{
	    msg->set_leader_name(teamer_info->__teamer_name);
	}
}

void TravPeakTeam::serialize_rank(ProtoQualityRank *msg)
{
	msg->set_team_id(this->__team_id);
	msg->set_team_name(this->__team_name);
	msg->set_team_score(this->__score);
	msg->set_rank(this->__rank);
}

TravPeakTeam::TravPeakTeamer *TravPeakTeam::find_teamer(const Int64 src_role_id)
{
    TeamerMap::iterator teamer_iter = this->__teamer_map.find(src_role_id);
    if (teamer_iter != this->__teamer_map.end())
        return &(teamer_iter->second);

    return NULL;
}

int TravPeakTeam::total_force()
{
	int force = 0;
	for (TeamerMap::iterator iter = this->__teamer_map.begin();
			iter != this->__teamer_map.end(); ++iter)
	{
		force += iter->second.__teamer_force;
	}
	return force;
}

int TravPeakTeam::total_level()
{
	int level = 0;
	for (TeamerMap::iterator iter = this->__teamer_map.begin();
			iter != this->__teamer_map.end(); ++iter)
	{
		level += iter->second.__teamer_level;
	}
	return level;
}

void TravPeakQualityInfo::reset()
{
	this->__signup_team_set.clear();
	this->__team_score_vec.clear();

	this->__signup_start_tick = Time_Value::zero;
	this->__signup_end_tick = Time_Value::zero;
	this->__quality_start_tick = Time_Value::zero;
	this->__quality_end_tick = Time_Value::zero;
}

void TravPeakKnockoutInfo::reset(void)
{
	this->__knockout_start_tick = Time_Value::zero;
	this->__knockout_end_tick = Time_Value::zero;
}

int SMBattlerInfo::camp_id()
{
	return this->camp_index_ + 1;
}

int SMBattlerInfo::con_score(int per, int max)
{
	JUDGE_RETURN(this->con_kill_num_ > 1, 0);

	int cur = (this->con_kill_num_ - 1) * per;
	return std::min<int>(cur, max);
}

void SMBattlerInfo::reset()
{
	this->id_ = 0;
	this->sex_ = 0;
	this->state_ = 0;
	this->tick_ = 0;

	this->name_.clear();
	this->reward_map_.clear();

	this->camp_index_ = 0;
	this->force_ = 0;
	this->score_ = 0;
	this->score_tick_ = ::time(NULL);

	this->total_kill_num_ = 0;
	this->con_kill_num_ = 0;
	this->max_con_kill_num_ = 0;
}

void SMBattlerInfo::make_up_rank(int rank, ProtoSMBattleRankRec* proto)
{
	proto->set_rank(rank);
	proto->set_role_id(this->id_);
	proto->set_role_name(this->name_);
	proto->set_sex(this->sex_);

	proto->set_score(this->score_);
	proto->set_camp_id(this->camp_id());
	proto->set_kill(this->total_kill_num_);
	proto->set_max_kill(this->max_con_kill_num_);
}

void SMCampInfo::reset()
{
	this->rank_ = 1;
	this->score_ = 0;
	this->tick_ = 0;
	this->sm_player_.clear();
}

int SMCampInfo::total_force()
{
	int force = 0;
	for (LongMap::iterator iter = this->sm_player_.begin();
			iter != this->sm_player_.end(); ++iter)
	{
		force += iter->second;
	}

	return force;
}

int SMCampInfo::total_player()
{
	return this->sm_player_.size();
}

OfflineRoleDetail::OfflineRoleDetail()
{
	OfflineRoleDetail::reset();
}

void OfflineRoleDetail::reset(void)
{
	this->role_id_ = 0;
	this->role_info_.reset();
	this->magic_weapon_info_.reset();

	this->skill_map_.clear();
	this->shape_map_.clear();

	this->attack_lower_ = 0.0;
	this->attack_upper_ = 0.0;
	this->defence_lower_ = 0.0;
	this->defence_upper_ = 0.0;

	this->total_hit_ = 0;
	this->total_avoid_ = 0;
	this->total_lucky_ = 0.0;

	this->total_crit_ = 0;
	this->total_toughness_ = 0;

	this->total_blood_ = 0;
	this->total_magic_ = 0;
	this->total_speed_ = 0;
	this->cur_label_ = 0;
	this->equie_refine_lvl_ = 0;

	this->total_damage_ = 0;
	this->total_reduction_ = 0;
}

OfflineBeastDetail::OfflineBeastDetail(void) :
		beast_id_(0), beast_sort_(0)
{ /*NULL*/
}

void OfflineBeastDetail::reset(void)
{
	this->beast_id_ = 0;
	this->beast_sort_ = 0;

	this->prop_map_.clear();
	this->skill_set_.clear();

	this->beast_name_.clear();
}

void OfflineDetail::reset(void)
{
	this->die_times_ = 0;
	this->last_is_move_ = false;
}

void BloodContainer::copy_to_status()
{
	this->status_.__value1 = this->cur_blood_;
	this->status_.__value2 = this->max_blood_;
}

void BloodContainer::reset(void)
{
	this->status_.set_status(BUFF_ID);

	this->max_blood_ = CONFIG_INSTANCE->blood_cont("total").asInt();
	this->interval_ = CONFIG_INSTANCE->blood_cont("interval").asInt();

	this->cur_blood_ = 0;
	this->check_flag_ = false;

	this->check_time_ = 0;
	this->non_tips_ = 0;
	this->everyday_tick_ = 0;
	this->last_tips_tick_ = 0;
}

void BloodContainer::adjust_cur_blood()
{
	if (this->cur_blood_ < 0) {
		this->cur_blood_ = 0;
	}

	if (this->cur_blood_ > this->max_blood_)
	{
		this->cur_blood_ = this->max_blood_;
	}
}

double BloodContainer::cur_left_percent()
{
	return this->cur_blood_ * 1.0 / this->max_blood_;
}

int BloodContainer::left_capacity()
{
	return this->max_blood_ - this->cur_blood_;
}

int BloodContainer::arrive_time_add()
{
	this->check_time_ += 1;
	return this->check_time_ >= this->interval_;
}

void AutoFighterDetail::reset(void)
{
	this->aim_id_ = 0;
	this->owner_id_ = 0;

	this->last_attacker_id_ = 0;
	this->attack_distance_ = 0;

	this->aim_coord_.reset();
	this->is_attack_ = 0;
	this->skill_queue_.clear();
	this->schedule_list_.clear();
	while (this->skill_sequence_.empty() == false)
	{
		this->skill_sequence_.pop();
	}

	this->group_id_ = 0;
	this->fetch_skill_mode_ = 0;
	this->rect_skill_coord_.reset();

	this->guide_tick_ = Time_Value::zero;
	this->guide_skill_ = 0;
	this->guide_range_effect_sort_ = 0;

	this->die_skill_vec_.clear();
	this->__monster_history_defender_set_.clear();
	this->skill_note_countdown_.clear();
}

LeagueFBDetail::BOSSWave::BOSSWave()
{
	this->wave_id_ = 0;
	this->wave_state_ = 0;
}

void LeagueFBDetail::reset()
{
	this->league_index_ = 0;
	this->score_map_.clear();
	this->boss_wave_map_.clear();

	this->fb_state_ = 0;
	this->league_lvl_ = 0;
	this->open_mode_ = 0;

	this->pass_monster_ = 0;
	this->finish_state_ = 0;
}

void MapEquipDetail::reset()
{
	this->weapon_lvl_ = 0;
}

DynamicMoverCoord::DynamicMoverCoord(const int pixel_x, const int pixel_y)
{
	this->pixel_x_ = pixel_x;
	this->pixel_y_ = pixel_y;
	this->pos_x_ = DynamicMoverCoord::pixel_to_dynamic_pos(pixel_x);
	this->pos_y_ = DynamicMoverCoord::pixel_to_dynamic_pos(pixel_y);
}

void DynamicMoverCoord::reset(void)
{
	::memset(this, 0, sizeof(DynamicMoverCoord));
}

int DynamicMoverCoord::dynamic_pos_x(void) const
{
	return this->pos_x_;
}

int DynamicMoverCoord::dynamic_pos_y(void) const
{
	return this->pos_y_;
}

int DynamicMoverCoord::pixel_x(void) const
{
	return this->pixel_x_;
}

int DynamicMoverCoord::pixel_y(void) const
{
	return this->pixel_y_;
}

void DynamicMoverCoord::set_dynamic_pos(const int posX, const int posY)
{
	this->pos_x_ = posX;
	this->pos_y_ = posY;
	this->pixel_x_ = DynamicMoverCoord::dynamic_pos_to_pixel(posX);
	this->pixel_y_ = DynamicMoverCoord::dynamic_pos_to_pixel(posY);
}

void DynamicMoverCoord::set_dynamic_pixel(const int pixel_x,
		const int pixel_y)
{
	this->pixel_x_ = pixel_x;
	this->pixel_y_ = pixel_y;
	this->pos_x_ = DynamicMoverCoord::pixel_to_dynamic_pos(pixel_x);
	this->pos_y_ = DynamicMoverCoord::pixel_to_dynamic_pos(pixel_y);

}

int DynamicMoverCoord::dynamic_pos_to_pixel(const int pos, const int grid)
{
	return (pos * grid + grid / 2);
}

int DynamicMoverCoord::pixel_to_dynamic_pos(const int pixel, const int grid)
{
	return (pixel / grid);
}

bool operator==(const DynamicMoverCoord &left, const DynamicMoverCoord &right)
{
	return (left.dynamic_pos_x() == right.dynamic_pos_x()
			&& left.dynamic_pos_y() == right.dynamic_pos_y());
}

bool operator!=(const DynamicMoverCoord &left, const DynamicMoverCoord &right)
{
	return !(left == right);
}

bool operator<(const DynamicMoverCoord &left, const DynamicMoverCoord &right)
{
	if (left.dynamic_pos_x() < right.dynamic_pos_x())
	{
		return true;
	}
	else if (left.dynamic_pos_x() == right.dynamic_pos_x())
	{
		if (left.dynamic_pos_y() < right.dynamic_pos_y())
			return true;
	}
	return false;
}

PriorityCoord::PriorityCoord(void)
{
	this->__target_priority = 0;
	this->__source_priority = 0;
	this->__cur_pos.reset();
	this->__prev_pos.reset();
}

PriorityCoord::PriorityCoord(const int target_prio, const int source_prio,
		const DynamicMoverCoord &cur, const DynamicMoverCoord &prev)
{
	this->__target_priority = target_prio;
	this->__source_priority = source_prio;
	this->__cur_pos = cur;
	this->__prev_pos = prev;
}

bool operator<(const PriorityCoord &left, const PriorityCoord &right)
{
	int left_prio = left.__source_priority + left.__target_priority;
	int right_prio = right.__source_priority + right.__target_priority;
	if (left_prio > right_prio)
		return true;
	return false;
}

PriorityMptCoord::PriorityMptCoord(void)
{
	this->__target_priority = 0;
	this->__source_priority = 0;
	this->__cur_pos.reset();
	this->__prev_pos.reset();
}

PriorityMptCoord::PriorityMptCoord(const int target_prio, const int source_prio,
		const MoverCoord &cur, const MoverCoord &prev)
{
	this->__target_priority = target_prio;
	this->__source_priority = source_prio;
	this->__cur_pos = cur;
	this->__prev_pos = prev;
}

bool operator<(const PriorityMptCoord &left, const PriorityMptCoord &right)
{
	int left_prio = left.__source_priority + left.__target_priority;
	int right_prio = right.__source_priority + right.__target_priority;
	if (left_prio > right_prio)
		return true;
	return false;
}

AnswerRoleDetail::AnswerRoleDetail()
{
	AnswerRoleDetail::reset();
}

void AnswerRoleDetail::reset()
{
	this->cur_topic_num = 0;
	this->open_time = 0;
}

LEscortDetail::RankItem::RankItem()
{
	this->id_ = 0;
	this->rank_index_ = 0;
	this->name_.clear();

	this->tick_ = 0;
	this->score_ = 0;

	this->escort_type_ = 0;
	this->escort_times_ = 0;

	this->car_index_ = 0;
	this->start_tick_ = 0;
	this->total_contri_ = 0;
}

void MapMagicWeaponInfo::reset(void)
{
	this->__weapon_id = 0;
	this->__weapon_level = 0;
}
