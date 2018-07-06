/*
 * LeagueReginFightSystem.cpp
 *
 *  Created on: Mar 28, 2017
 *      Author: root
 */

#include "LeagueReginFightSystem.h"
#include "LeagueRegionFightScene.h"
#include "MapMonitor.h"
#include "MapPlayerEx.h"
#include "ProtoDefine.h"
#include "MMOLeague.h"
#include "LeagueMonitor.h"
#include "LeagueWarSystem.h"

LRFLeagueInfo::LRFLeagueInfo()
{
	LRFLeagueInfo::reset();
}

void LRFLeagueInfo::reset()
{
	this->id_ = 0;
	this->force_ = 0;
	this->flag_lvl_ = 1;

	this->camp_ = 0;
	this->rank_ = -1;
	this->space_ = -1;

	this->result_ = 0;
	this->fight_score_ = 0;
	this->flag_monster_ = 0;
	this->killed_tower_ = 0;
}

LRFBetSupport::LRFBetSupport()
{
	this->role_id = 0;
	this->support_league_id = 0;
	this->result_ = 0;
}

void LRFLeagueInfo::copy(int space, LeagueWarInfo::LeagueWarRank& info)
{
	this->space_ 	= space;
	this->rank_ 	= info.rank_;
	this->id_ 		= info.league_index_;
	this->name_ 	= info.league_name_;
	this->force_ 	= info.force_;
	this->leader_ 	= info.leader_;
	this->flag_lvl_ = info.flag_lvl_;
	this->result_ 	= 0;
}

void LRFLeagueInfo::copy_result(int rank, LRFLeagueInfo* info)
{
	this->rank_ = rank;
	this->id_ = info->id_;
	this->name_ = info->name_;
	this->force_ = info->force_;
	this->leader_ = info->leader_;
}

LeagueRegionResult::LeagueRegionResult()
{
	LeagueRegionResult::reset();
}

int LeagueRegionResult::fetch_result(Int64 id)
{
	JUDGE_RETURN(this->history_result_.count(id) > 0, 0);
	return this->history_result_[id];
}

void LeagueRegionResult::reset()
{
	this->tick_ = 0;
	this->finish_ = true;
}

void LeagueRegionResult::reset_every_times()
{
	this->finish_ = false;
	this->atted_rank_map_.clear();
	this->attend_id_vec_.clear();
	this->attend_league_.clear();
	this->bet_support_.clear();
}

int LeagueReginFightSystem::ActivityTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int LeagueReginFightSystem::ActivityTimer::handle_timeout(const Time_Value &tv)
{
	JUDGE_RETURN(this->lrf_system_ != NULL, -1);
	return this->lrf_system_->handle_activity_timeout();
}

void LeagueReginFightSystem::ActivityTimer::init(LeagueReginFightSystem* parent)
{
	JUDGE_RETURN(NULL != parent, ;);
	this->lrf_system_ = parent;
}

int LeagueReginFightSystem::TipsTimer::type()
{
	return GTT_MAP_ONE_SECOND;
}

int LeagueReginFightSystem::TipsTimer::handle_timeout(const Time_Value &tv)
{
	return LRF_MONITOR->handle_tips_timeout();
}

LeagueReginFightSystem::LeagueReginFightSystem()
{
	this->scene_id_ = 0;
	this->activity_id_ = 0;

	this->max_space_ = 0;
	this->max_rank_ = 0;
	this->open_days_ = 0;

	for (int i = 0; i < TOTAL_SIZE; ++i)
	{
		this->support_reward_[i] = 0;
		this->support_mail_[i] = 0;
	}

	this->check_cur_attend_ = false;
	this->lrf_scene_package_ = new PoolPackage<LeagueRegionFightScene>;
}

LeagueReginFightSystem::~LeagueReginFightSystem()
{
}

const Json::Value& LeagueReginFightSystem::conf()
{
	return CONFIG_INSTANCE->scene(this->scene_id_);
}

int LeagueReginFightSystem::is_arrive_open_day()
{
	return CONFIG_INSTANCE->client_open_days() >= this->open_days_;
}

int LeagueReginFightSystem::load_league_war_info_begin(int check)
{
	if (check == true)
	{
		MSG_USER("LRF load begin %d", this->validate_cur_lwar());
		JUDGE_RETURN(this->validate_cur_lwar() == true, -1);
	}

	DBShopMode* shop_mode = GameCommon::pop_shop_mode();
	JUDGE_RETURN(shop_mode != NULL, -1);

	shop_mode->recogn_ = TRANS_LOAD_LFR_WAR_INFO;
	return MAP_MONITOR->db_map_load_mode_begin(shop_mode);
}

int LeagueReginFightSystem::load_league_war_info_done(DBShopMode* shop_mode)
{
	BSONObj* p_res = shop_mode->output_argv_.bson_obj_;

	this->lwar_info_.reset();
	MMOLeague::load_league_war(this->lwar_info_, p_res);

	this->history_info_.finish_ = false;
	this->init_region_info();

	MMOLeague::updateLeagueRegionFightInfo(this->history_info_);
	MSG_USER("LRF load done %d", this->lwar_info_.lwar_rank_map_.size());
	return 0;
}


int LeagueReginFightSystem::request_enter_lrf(int sid, Proto30400051* request)
{
	JUDGE_RETURN(this->is_activity_time() == true, ERROR_GUARD_TIME);
	JUDGE_RETURN(this->history_info_.attend_league_.count(request->league_index()) > 0,
			ERROR_LEAGUE_NO_QUALITY);

	LRFLeagueInfo& lrf_info = this->history_info_.attend_league_[request->league_index()];
	LeagueRegionFightScene* scene = this->find_lrf_scene(lrf_info.space_);
	JUDGE_RETURN(scene != NULL && scene->is_lrf_finish() == false, ERROR_ACTIVITY_ENDED);

	string pos_name;
	if (lrf_info.camp_ == GameEnum::CAMP_TWO)
	{
		pos_name = "right_birth_point";
	}
	else
	{
		pos_name = "left_birth_point";
	}

	Proto30400052 enter_info;
	enter_info.set_space_id(lrf_info.space_);
	enter_info.set_scene_mode(SCENE_MODE_REGION_FIGHT);

	const Json::Value&  cfg = this->conf()[pos_name];
	enter_info.set_pos_x(cfg[0u].asInt());
	enter_info.set_pos_y(cfg[1u].asInt());
	return MAP_MONITOR->respond_enter_scene_begin(sid, request, &enter_info);
}

int LeagueReginFightSystem::apply_lrf_operate(int sid, Int64 role, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400057*, request, -1);

	this->check_and_update_cur_attend_info();
	switch (request->type())
	{
	case 0:
	{
		return this->fetch_lrf_rank_info(sid, role, msg);
	}
	case 1:
	{
		return this->fetch_lrf_support_info(sid, role, msg);
	}
	case 2:
	{
		return this->apply_lrf_support_league(sid, role, msg);
	}
	case 3:
	{
		return this->fetch_lwar_rank_info(sid, role, msg);
	}
	}
	return 0;
}

int LeagueReginFightSystem::fetch_lrf_rank_info(int sid, Int64 role, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400057*, request, -1);

	Proto50404001 respond;
	respond.set_is_activity_time(this->is_activity_time());

	Int64 next_time = this->fetch_next_fight_time();
	respond.set_next_league_fight_time(next_time);

	Int64 league_id = request->league_id();
	for (LeagueRegionResult::HistoryMap::iterator
			iter = this->history_info_.history_league_.begin();
			iter != this->history_info_.history_league_.end(); ++iter)
	{
		const LRFLeagueInfo& region_result = iter->second;

		ProtoLeagueItem* proto_item = respond.add_league_list_info();
		proto_item->set_rank_index(iter->first);
		proto_item->set_league_name(region_result.name_);
		proto_item->set_league_leader(region_result.leader_);
		JUDGE_CONTINUE(region_result.id_ == league_id);

		respond.set_self_league_rank(iter->first);
	}

	for (LongMap::iterator iter = this->history_info_.atted_rank_map_.begin();
			iter != this->history_info_.atted_rank_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(this->history_info_.attend_league_.count(iter->second) > 0);

		LRFLeagueInfo& attend = this->history_info_.attend_league_[iter->second];
		ProtoLeagueItem* proto_item = respond.add_cur_league_list();

		proto_item->set_rank_index(iter->first);
		proto_item->set_league_name(attend.name_);
	}

	IntStrPair pair = this->fetch_cur_lrf_info(league_id);
	respond.set_next_region_land_id(pair.first);
	respond.set_enemy_league_name(pair.second);

	return MAP_MONITOR->dispatch_to_client_from_gate(sid, role, &respond);
}

//竞猜信息
int LeagueReginFightSystem::fetch_lrf_support_info(int sid, Int64 role, Message* msg)
{
	Proto50404002 respond;

	IntPair pair = this->fetch_support_state();
	respond.set_state(pair.first);
	respond.set_left_time(pair.second);
	respond.set_win_reward(this->support_reward_[0]);
	respond.set_attend_reward(this->support_reward_[1]);
	respond.set_league_iswin(this->history_info_.finish_);

	if (this->history_info_.bet_support_.count(role) > 0)
	{
		LRFBetSupport& bet_support = this->history_info_.bet_support_[role];
		respond.set_support_league_id(bet_support.support_league_id);
	}

	for (LongMap::iterator iter = this->history_info_.atted_rank_map_.begin();
			iter != this->history_info_.atted_rank_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(this->history_info_.attend_league_.count(iter->second) > 0);

		LRFLeagueInfo& attend = this->history_info_.attend_league_[iter->second];
		ProtoLeagueItem* proto_item = respond.add_bet_league_list_info();

		proto_item->set_league_index(attend.id_);
		proto_item->set_league_name(attend.name_);
		proto_item->set_league_force(attend.force_);
		proto_item->set_apply_flag(attend.result_);
	}

	return MAP_MONITOR->dispatch_to_client_from_gate(sid, role, &respond);
}

//竞猜支持
int LeagueReginFightSystem::apply_lrf_support_league(int sid, Int64 role, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400057*, request, -1);

	IntPair pair = this->fetch_support_state();
	if (pair.first != 1)
	{
		return MAP_MONITOR->dispatch_to_client_from_gate(sid, role,
				RETURN_GET_LRF_BATSUPPORT, ERROR_CLIENT_OPERATE);
	}

	if (this->history_info_.attend_league_.count(request->league_id()) == 0)
	{
		return MAP_MONITOR->dispatch_to_client_from_gate(sid, role,
				RETURN_GET_LRF_BATSUPPORT, ERROR_CLIENT_OPERATE);
	}

	if (this->history_info_.bet_support_.count(role) > 0)
	{
		return MAP_MONITOR->dispatch_to_client_from_gate(sid, role,
				RETURN_GET_LRF_BATSUPPORT, ERROR_CLIENT_OPERATE);
	}

	LRFBetSupport& bet_support = this->history_info_.bet_support_[role];
	bet_support.role_id = role;
	bet_support.support_league_id = request->league_id();
	return this->fetch_lrf_support_info(sid, role, NULL);
}

int LeagueReginFightSystem::fetch_lwar_rank_info(int sid, Int64 role, Message* msg)
{
	Proto50404007 respond;
	for (LeagueWarInfo::LWarRankMap::iterator
			iter = this->lwar_info_.lwar_rank_map_.begin();
			iter != this->lwar_info_.lwar_rank_map_.end(); ++iter)
	{
		LeagueWarInfo::LeagueWarRank& war_rank = iter->second;
		ProtoLeagueItem* proto_item = respond.add_war_list();
		proto_item->set_rank_index(iter->first);
		proto_item->set_league_name(war_rank.league_name_);
		proto_item->set_league_force(war_rank.score_);
	}

	return MAP_MONITOR->dispatch_to_client_from_gate(sid, role, &respond);
}

void LeagueReginFightSystem::init()
{
	this->scene_id_ 	= GameEnum::LEAGUE_REGION_FIGHT_ID;
	this->null_info_.name_ = CONFIG_INSTANCE->const_set_conf(
			"null_legion_league")["remark"].asString();

	const Json::Value& conf = this->conf();
	this->activity_id_ 	= conf["activity_id"].asInt();
	this->max_space_ 	= conf["init_tower_space"].asInt();
	this->max_rank_ 	= this->max_space_ * TOTAL_SIZE;

	for (int i = 0; i < TOTAL_SIZE; ++i)
	{
		this->support_reward_[i] = conf["guess_reward"][i].asInt();
		this->support_mail_[i] = conf["guess_mail"][i].asInt();
	}

	const Json::Value& activity_conf = CONFIG_INSTANCE->common_activity(this->activity_id_);
	JUDGE_RETURN(activity_conf.empty() == false, ;);

	this->open_days_ = activity_conf["open_day"].asInt();
	GameCommon::cal_activity_info(this->time_info_, activity_conf);
	GameCommon::cal_one_activity_time(this->tips_time_, activity_conf);

	this->activity_timer.init(this);
	this->activity_timer.schedule_timer(this->time_info_.refresh_time_);
	this->tips_timer_.schedule_timer(this->tips_time_.refresh_time_);
	MSG_USER("LRF init %d %d %d", this->time_info_.cur_state_,
			this->time_info_.refresh_time_, this->tips_time_.refresh_time_);

	//帮派争霸
	MMOLeague::load_league_war(this->lwar_info_);
	//城池战
	MMOLeague::loadLeagueRegionFightInfo(this->history_info_);
	//初实化
	this->init_region_info();

	JUDGE_RETURN(this->time_info_.cur_state_ == GameEnum::ACTIVITY_STATE_START, ;);
	this->start_region_event();
}

void LeagueReginFightSystem::stop()
{
	MMOLeague::updateLeagueRegionFightInfo(this->history_info_);
}

//初始化城池
void LeagueReginFightSystem::init_region_info()
{
	this->adjust_history_info();
	this->adjust_cur_info();

	LeagueWarInfo::LWarRankMap& lwar_rank = this->lwar_info_.lwar_rank_map_;
	LeagueRegionResult::LeagueMap& attend_map = this->history_info_.attend_league_;

	for (LeagueWarInfo::LWarRankMap::iterator iter = lwar_rank.begin();
			iter != lwar_rank.end(); ++iter)
	{
		this->history_info_.atted_rank_map_[iter->first] = iter->second.league_index_;
	}

	int total = std::min<int>(lwar_rank.size() / 2, this->max_space_);
	for (int i = 1; i <= total; ++i)
	{
		LeagueWarInfo::LeagueWarRank& first_rank = lwar_rank[i * 2 - 1];
		LeagueWarInfo::LeagueWarRank& second_rank = lwar_rank[i * 2];

		LRFLeagueInfo& first_league = attend_map[first_rank.league_index_];
		LRFLeagueInfo& second_league = attend_map[second_rank.league_index_];

		first_league.copy(i, first_rank);
		second_league.copy(i, second_rank);

		first_league.camp_ = GameEnum::CAMP_ONE;
		second_league.camp_ = GameEnum::CAMP_TWO;

		first_league.rival_name_ = second_league.name_;
		second_league.rival_name_ = first_league.name_;

		LongPair pair(first_league.id_, second_league.id_);
		this->history_info_.attend_id_vec_.push_back(pair);
		JUDGE_CONTINUE(this->history_info_.finish_ == true);

		first_league.result_ = this->history_info_.fetch_result(first_league.id_);
		second_league.result_ = this->history_info_.fetch_result(second_league.id_);
	}

	this->last_league_ = 0;
	uint last_index = total * 2 + 1;

	while (last_index <= lwar_rank.size() && last_index < this->max_rank_)
	{
		LeagueWarInfo::LeagueWarRank& last_rank = lwar_rank[last_index];
		LRFLeagueInfo& last_league = attend_map[last_rank.league_index_];

		last_league.copy(total + 1, last_rank);
		last_league.camp_ = GameEnum::CAMP_ONE;

		this->last_league_ = last_rank.league_index_;
		JUDGE_BREAK(this->history_info_.finish_ == true);

		last_league.result_ = 1;
		break;
	}
}

void LeagueReginFightSystem::adjust_history_info()
{
	JUDGE_RETURN(this->history_info_.finish_ == true, ;);

	if (GameCommon::is_continue_day(this->lwar_info_.tick_,
			this->fetch_next_fight_time()) == false)
	{
		//非下次比赛，清除
		this->lwar_info_.reset();
		MSG_USER("LRF no cur reset");
	}
	else
	{
		this->history_info_.finish_ = false;
		this->adjust_cur_info();
		MSG_USER("LRF cur reset");
	}
}

void LeagueReginFightSystem::adjust_cur_info()
{
	JUDGE_RETURN(this->history_info_.finish_ == false, ;);
	JUDGE_RETURN(this->history_info_.tick_ != this->lwar_info_.tick_, ;);

	this->history_info_.reset_every_times();
	this->history_info_.tick_ = this->lwar_info_.tick_;
}

void LeagueReginFightSystem::recycle_league_region(LeagueRegionFightScene* scene)
{
	this->lrf_scene_package_->unbind_and_push(scene->space_id(), scene);
}

void LeagueReginFightSystem::test_league_region(int id, int last)
{
	this->time_info_.cur_state_ = id + 1;
	this->time_info_.refresh_time_ = last;
	this->handle_activity_timeout_i(id);
}

void LeagueReginFightSystem::check_and_handle_all_scene()
{
	BIntMap index_map = this->lrf_scene_package_->fetch_index_map();
	JUDGE_RETURN(index_map.empty() == false, ;);

	int is_finish_all = true;
	for (BIntMap::iterator iter = index_map.begin(); iter != index_map.end(); ++iter)
	{
		LeagueRegionFightScene* scene = this->find_lrf_scene(iter->first);
		JUDGE_CONTINUE(scene->is_lrf_finish() == false);

		is_finish_all = false;
		break;
	}

	JUDGE_RETURN(is_finish_all == true, ;);
}

//某个城池结束
void LeagueReginFightSystem::set_result_info(LRFLeagueInfo* win, LRFLeagueInfo* lose)
{
	if (lose->id_ > 0)
	{
		int first_rank = win->space_ * 2 - 1;
		int second_rank = win->space_ * 2;

		this->set_bet_result_info(win, 1);
		this->set_bet_result_info(lose, 2);

		LRFLeagueInfo& first_region = this->history_info_.history_league_[first_rank];
		LRFLeagueInfo& second_region = this->history_info_.history_league_[second_rank];

		first_region.copy_result(first_rank, win);
		second_region.copy_result(second_rank, lose);
	}
	else
	{
		int first_rank = win->space_ * 2 - 1;
		this->set_bet_result_info(win, 1);

		LRFLeagueInfo& league_region = this->history_info_.history_league_[first_rank];
		league_region.copy_result(first_rank, win);
	}
}

void LeagueReginFightSystem::set_bet_result_info(LRFLeagueInfo* league, int result)
{
	for (LeagueRegionResult::SupportMap::iterator
			iter = this->history_info_.bet_support_.begin();
			iter != this->history_info_.bet_support_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second.support_league_id == league->id_);

		iter->second.result_ = league->result_;
		this->send_bet_mail(iter->second.role_id, league->result_);
	}
}

void LeagueReginFightSystem::send_bet_mail(Int64 role, int result)
{
	JUDGE_RETURN(result > 0 && result <= TOTAL_SIZE, ;);

	int mail = this->support_mail_[result - 1];
	int reward = this->support_reward_[result - 1];

	MailInformation *mail_info = GameCommon::create_sys_mail(mail);
	mail_info->add_goods(reward);
	GameCommon::request_save_mail(role, mail_info);
}

//校正数据
void LeagueReginFightSystem::check_and_update_cur_attend_info()
{
	JUDGE_RETURN(this->check_cur_attend_ == false, ;);

	for (LeagueRegionResult::LeagueMap::iterator
			iter = this->history_info_.attend_league_.begin();
			iter != this->history_info_.attend_league_.end(); ++iter)
	{
		LRFLeagueInfo& league = iter->second;
		JUDGE_CONTINUE(league.force_ == 0 || league.leader_.empty() == true);

		MapLeagueInfo* info = LEAGUE_MONITOR->find_map_league(iter->first);
		JUDGE_CONTINUE(info != NULL);

		league.force_ = info->force_;
		league.leader_ = info->leader_;
		league.flag_lvl_ = info->flag_lvl_;
	}

	this->check_cur_attend_ = true;
}

int LeagueReginFightSystem::handle_activity_timeout()
{
	int last_state = this->time_info_.cur_state_;
	this->time_info_.set_next_stage();
	return this->handle_activity_timeout_i(last_state);
}

int LeagueReginFightSystem::handle_activity_timeout_i(int state)
{
	switch (state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
		this->ahead_region_event();
		break;
	}
	case GameEnum::ACTIVITY_STATE_AHEAD:
	{
		this->start_region_event();
		break;
	}
	case GameEnum::ACTIVITY_STATE_START:
	{
		this->stop_region_event();
		GameCommon::map_sync_activity_tips_stop(this->activity_id_);
		break;
	}
	}

	this->activity_timer.cancel_timer();
	this->activity_timer.schedule_timer(this->time_info_.refresh_time_);

	MSG_USER("LRF timeout %d, %d, %d, %d", this->time_info_.cur_state_,
			this->time_info_.refresh_time_, CONFIG_INSTANCE->client_open_days(),
			this->open_days_);
	return 0;
}

int LeagueReginFightSystem::handle_tips_timeout()
{
	this->tips_timer_.cancel_timer();

	this->tips_time_.set_next_stage();
	this->tips_timer_.schedule_timer(this->tips_time_.refresh_time_);

	MSG_USER("LRF tips time %d", this->tips_time_.refresh_time_);
	return 0;
}

int LeagueReginFightSystem::ahead_region_event()
{
	JUDGE_RETURN(this->is_arrive_open_day() == true, -1);

	if (this->history_info_.finish_ == true)
	{
		this->load_league_war_info_begin();
	}

	MSG_USER("LRF ahead time %d %d", this->time_info_.refresh_time_,
			this->history_info_.finish_);

	return GameCommon::map_sync_activity_tips_ahead(
			this->activity_id_, this->time_info_.refresh_time_);
}

int LeagueReginFightSystem::start_region_event()
{
	JUDGE_RETURN(this->is_arrive_open_day() == true, -1);

	this->history_info_.history_league_.clear();
	this->history_info_.history_result_.clear();
	this->init_region_fight_map();

	GameCommon::announce(this->conf()["brocast_activity_begin"].asInt());
	GameCommon::map_sync_activity_tips_start(this->activity_id_,
			this->time_info_.refresh_time_);

	MSG_USER("LRF start %d", this->time_info_.refresh_time_);
	return 0;
}

int LeagueReginFightSystem::stop_region_event()
{
	JUDGE_RETURN(this->is_arrive_open_day() == true, -1);

	BIntMap index_map = this->lrf_scene_package_->fetch_index_map();
	for (BIntMap::iterator iter = index_map.begin(); iter != index_map.end(); ++iter)
	{
		LeagueRegionFightScene* scene = this->find_lrf_scene(iter->first);
		JUDGE_CONTINUE(scene != NULL);
		scene->stop_region_war();
	}

//	for (LeagueRegionResult::LeagueMap::iterator
//			iter = this->history_info_.attend_league_.begin();
//			iter != this->history_info_.attend_league_.end(); ++iter)
//	{
//		LRFLeagueInfo& league = iter->second;
//		JUDGE_CONTINUE(league.result_ == 0);
//
//		LRFLeagueInfo& league_region = this->history_info_.history_league_[league.rank_];
//		league_region.copy_result(league.rank_, &league);
//
//		league.result_ = 1;
//		this->set_bet_result_info(&league, 1);
//		break;
//	}

	string league_list;
	string city_list;

	Proto30100111 inner;
	Int64 tick = this->fetch_next_fight_time();
	for (LeagueRegionResult::HistoryMap::iterator
			iter = this->history_info_.history_league_.begin();
			iter != this->history_info_.history_league_.end(); ++iter)
	{
		//传闻
		if (league_list.empty() == false)
		{
			league_list += string(",");
		}

		if (city_list.empty() == false)
		{
			city_list += string(",");
		}

		league_list += iter->second.name_;
		city_list += this->fetch_city_name(iter->first);

		//逻辑服
		ProtoItem* item = inner.add_result_list();
		item->set_unique_id(iter->second.id_);
		item->set_id(iter->first);
		item->set_use_tick(tick);
	}

	MAP_MONITOR->dispatch_to_logic(&inner);

	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_string(para_vec, league_list);
	GameCommon::push_brocast_para_string(para_vec, city_list);

	int shout_id = this->conf()["brocast_activity_end"].asInt();
	GameCommon::announce(shout_id, &para_vec);

	this->history_info_.finish_ = true;
	MMOLeague::updateLeagueRegionFightInfo(this->history_info_);
	return 0;
}

LeagueRegionFightScene* LeagueReginFightSystem::find_lrf_scene(int space_id)
{
	return this->lrf_scene_package_->find_object(space_id);
}

LeagueRegionResult& LeagueReginFightSystem::get_region_result()
{
	return this->history_info_;
}

LeagueWarInfo& LeagueReginFightSystem::get_league_war_rank()
{
	return this->lwar_info_;
}

int LeagueReginFightSystem::is_activity_time()
{
	return this->time_info_.cur_state_ == GameEnum::ACTIVITY_STATE_START;
}

int LeagueReginFightSystem::validate_cur_lwar()
{
	if (LEAGUE_WAR_SYSTEM->is_test() == true)
	{
		return true;
	}
	else
	{
		return GameCommon::is_continue_day(this->fetch_next_fight_time());
	}
}

int LeagueReginFightSystem::left_activity_time()
{
	JUDGE_RETURN(this->is_activity_time() == true, 0);
	return this->activity_timer.left_second();
}

int LeagueReginFightSystem::init_region_fight_map()
{
	int total = this->history_info_.attend_id_vec_.size();
	LeagueRegionResult::LeagueMap& attend_map = this->history_info_.attend_league_;

	for (int i = 0; i < total; ++i)
	{
		LeagueRegionFightScene* lrf_scene = this->lrf_scene_package_->pop_object();
		JUDGE_CONTINUE(lrf_scene != NULL);

		LongPair& pair = this->history_info_.attend_id_vec_[i];
		LRFLeagueInfo& first_info = attend_map[pair.first];
		LRFLeagueInfo& second_info = attend_map[pair.second];

		lrf_scene->init_region_scene(this->scene_id_, first_info, second_info);
		lrf_scene->run_scene();

		this->lrf_scene_package_->bind_object(i + 1, lrf_scene);
	}

	if (this->last_league_ > 0)
	{
		LeagueRegionFightScene* lrf_scene = this->lrf_scene_package_->pop_object();
		LRFLeagueInfo& last_info = attend_map[this->last_league_];

		this->null_info_.reset();
		this->null_info_.camp_ = GameEnum::CAMP_TWO;
		this->lrf_scene_package_->bind_object(total + 1, lrf_scene);

		lrf_scene->init_region_scene(this->scene_id_, last_info, this->null_info_);
		lrf_scene->run_scene();
	}

	MSG_USER("LRF init %d %d", this->lrf_scene_package_->size(), total);
	return 0;
}

int LeagueReginFightSystem::fetch_league_flag_blood(int level)
{
	const Json::Value& flag_info = CONFIG_INSTANCE->league_flag(level);
	return std::max<int>(flag_info["flag_blood"].asInt(), 1000000);
}

Int64 LeagueReginFightSystem::fetch_next_fight_time()
{
	return this->tips_timer_.check_tick().sec();
}

IntPair LeagueReginFightSystem::fetch_support_state()
{
	IntPair pair;
	if (this->is_activity_time() == true || this->history_info_.finish_ == true)
	{
		pair.first = 0;
	}
	else
	{
		pair.first = 1;
		pair.second = this->activity_timer.left_second();
	}
	return pair;
}

string LeagueReginFightSystem::fetch_city_name(int rank)
{
	return CONFIG_INSTANCE->league_region(rank)["name"].asString();
}

IntStrPair LeagueReginFightSystem::fetch_cur_lrf_info(Int64 id)
{
	IntStrPair pair;
	JUDGE_RETURN(this->is_activity_time() == false, pair);
	JUDGE_RETURN(this->history_info_.finish_ == false, pair);
	JUDGE_RETURN(this->history_info_.attend_league_.count(id) > 0, pair);

	LRFLeagueInfo& attend = this->history_info_.attend_league_[id];
	pair.first = attend.space_ * 2 -1;
	pair.second = attend.rival_name_;
	return pair;
}

