/*
 * LeagueWarSystem.cpp
 *
 *  Created on: 2016年9月19日
 *      Author: lyw
 */

#include "LeagueWarSystem.h"
#include "ProtoDefine.h"
#include "MapPlayerEx.h"
#include "MapMonitor.h"
#include "MMOLeague.h"
#include "MMORole.h"
#include "LeagueWarScene.h"
#include "LeagueMonitor.h"

LeagueWarSysInfo::LeagueWarSysInfo()
{
	LeagueWarSysInfo::reset();
}

void LeagueWarSysInfo::reset(void)
{
	this->__activity_id = 0;
	this->__enter_level = 0;
}

FirstSpaceWinLeague::FirstSpaceWinLeague()
{
	FirstSpaceWinLeague::reset();
}

void FirstSpaceWinLeague::reset(void)
{
	this->league_index_ = 0;
	this->league_name_.clear();
	this->leader_name_.clear();
}

LeagueWarSystem::LeagueWarSystem()
{
	// TODO Auto-generated constructor stub
	this->lwar_scene_package_ = new PoolPackage<LeagueWarScene>;
	this->player_map_ = new PoolPackage<LWarRoleInfo, Int64>;
	this->lwar_check_timer_.init(this);

	LeagueWarSystem::reset();
}

LeagueWarSystem::~LeagueWarSystem()
{
	// TODO Auto-generated destructor stub
}

void LeagueWarSystem::reset(void)
{
	this->real_scene_ = 0;
	this->is_test_ = false;
	this->lwar_check_timer_.cancel_timer();

	this->time_info_.reset();
	this->lwar_detail_.reset();
	this->first_win_.reset();
	this->lwar_info_.reset();
}

void LeagueWarSystem::recycle_lwar(LeagueWarScene* scene)
{
	this->lwar_scene_package_->unbind_and_push(scene->space_id(), scene);
}

LeagueWarSystem::CheckStartTimer::CheckStartTimer(void)
{
	this->lwar_system_ = NULL;
}

void LeagueWarSystem::CheckStartTimer::init(LeagueWarSystem* parent)
{
	JUDGE_RETURN(NULL != parent, ;);
	this->lwar_system_ = parent;
}

int LeagueWarSystem::CheckStartTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int LeagueWarSystem::CheckStartTimer::handle_timeout(const Time_Value &tv)
{
	JUDGE_RETURN(this->lwar_system_ != NULL, -1);
	return this->lwar_system_->handle_lwar_timeout();
}

int LeagueWarSystem::client_scene(void)
{
	return GameEnum::LWAR_SCENE_ID;
}

int LeagueWarSystem::real_scene(void)
{
	return this->real_scene_;
}

void LeagueWarSystem::start_league_war(int real_scene)
{
	this->real_scene_ = real_scene;

	int activity_id = CONFIG_INSTANCE->league_war("activity_id").asInt();
	int enter_level = CONFIG_INSTANCE->league_war("enter_level").asInt();

	this->lwar_detail_.__activity_id = activity_id;
	this->lwar_detail_.__enter_level = enter_level;

	const Json::Value& activity_conf = CONFIG_INSTANCE->common_activity(activity_id);
	JUDGE_RETURN(activity_conf.empty() == false, ;);

	MMOLeague::load_league_war(this->lwar_info_);
	GameCommon::cal_activity_info(this->time_info_, activity_conf);

	this->lwar_check_timer_.schedule_timer(this->time_info_.refresh_time_);
	MSG_USER("league_war %d %d", this->time_info_.cur_state_, this->time_info_.refresh_time_);
	JUDGE_RETURN(this->time_info_.cur_state_ == GameEnum::ACTIVITY_STATE_START, ;);

	this->start_lwar_event();

}

void LeagueWarSystem::stop_league_war()
{

}

int LeagueWarSystem::handle_lwar_timeout()
{
	int last_state = this->time_info_.cur_state_;
	this->time_info_.set_next_stage();
	return this->handle_lwar_i(last_state);
}

int LeagueWarSystem::handle_lwar_i(int state)
{
	switch (state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
		this->ahead_lwar_event();
		break;
	}
	case GameEnum::ACTIVITY_STATE_AHEAD:
	{
		this->start_lwar_event();
		break;
	}
	case GameEnum::ACTIVITY_STATE_START:
	{
		this->stop_lwar_event();
		break;
	}
	}

	this->lwar_check_timer_.cancel_timer();
	this->lwar_check_timer_.schedule_timer(this->time_info_.refresh_time_);

	MSG_USER("league_war %d, %d, %d %d", this->time_info_.time_index_,
			this->time_info_.cur_state_, this->time_info_.refresh_time_,
			this->is_today_activity());
	return 0;
}

int LeagueWarSystem::is_test()
{
	return this->is_test_;
}

int LeagueWarSystem::lwar_left_time()
{
	return this->lwar_check_timer_.left_second();
}

int LeagueWarSystem::is_today_activity()
{
	if (this->is_test_ == true)
	{
		return true;
	}

	if (CONFIG_INSTANCE->client_open_days() <= 1)
	{
		return true;
	}
	else
	{
		return this->time_info_.today_activity();
	}
}

int LeagueWarSystem::is_activity_time()
{
	switch (this->time_info_.cur_state_)
	{
	case GameEnum::ACTIVITY_STATE_AHEAD:
	case GameEnum::ACTIVITY_STATE_START:
	{
		return true;
	}
	}

	return false;
}

int LeagueWarSystem::validate_player_level(int role_level)
{
	return role_level >= this->lwar_detail_.__enter_level;
}

int LeagueWarSystem::request_enter_lwar(int sid, Proto30400051* request)
{
	JUDGE_RETURN(this->is_activity_time() == true, ERROR_GUARD_TIME);
	JUDGE_RETURN(this->validate_player_level(request->role_level()) == true, ERROR_PLAYER_LEVEL);
	JUDGE_RETURN(request->league_index() > 0, ERROR_LEAGUE_NO_EXIST);

	Int64 role_id = request->role_id();
	LeagueWarScene* lwar_scene = this->fetch_lwar_scene(role_id);
	JUDGE_RETURN(lwar_scene != NULL, ERROR_CLIENT_OPERATE);

	LWarRoleInfo* lwar_role = this->find_lwar(role_id);
	if (lwar_role == NULL)
	{
		lwar_role = this->player_map_->pop_object();
		this->player_map_->bind_object(role_id, lwar_role);

		lwar_role->id_ 			= role_id;
		lwar_role->league_index_= request->league_index();
		lwar_role->league_name_ = request->league_name();
		lwar_role->space_ 		= lwar_scene->space_id();

		//TODO：不能直接读取数据库
		MapLeagueInfo* l_info = LEAGUE_MONITOR->find_map_league(request->league_index());
		if (l_info != NULL)
		{
			lwar_role->leader_name_ = l_info->leader_;
			lwar_role->leader_index_ = l_info->leader_id_;
		}
		else
		{
			Int64 leader_id = MMOLeague::fetch_leader_id(request->league_index());
			lwar_role->leader_index_ = leader_id;
			lwar_role->leader_name_ = MMORole::fetch_player_name(leader_id);
		}
	}
	lwar_role->camp_index_ = lwar_scene->fetch_camp_index(request->league_index());

	lwar_scene->register_player(lwar_role);

	this->init_league_map(role_id, request->league_index(), request->league_name(), lwar_role->space_);

	Proto30400052 enter_info;
	enter_info.set_space_id(lwar_scene->space_id());
	enter_info.set_scene_mode(SCENE_MODE_LEAGUE_WAR);

	const Json::Value& enter_point = lwar_role->camp_index_ == 0 ?
			CONFIG_INSTANCE->league_war("attack_pos") :
			CONFIG_INSTANCE->league_war("defend_pos");

	enter_info.set_pos_x(enter_point[0u].asInt());
	enter_info.set_pos_y(enter_point[1u].asInt());
	return MAP_MONITOR->respond_enter_scene_begin(sid, request, &enter_info);
}

int LeagueWarSystem::request_exit_lwar(Int64 league_index, Int64 role_id)
{
	JUDGE_RETURN(this->league_map_.count(league_index) > 0, -1);

	LWarLeagueInfo& league_info = this->league_map_[league_index];
	league_info.enter_lwar_player_.erase(role_id);

	return 0;
}

void LeagueWarSystem::test_lwar(int id, int set_time)
{
	this->time_info_.cur_state_ = (id + 1) % this->time_info_.time_span_;
	this->time_info_.refresh_time_ = set_time;

	this->is_test_ = true;
	this->handle_lwar_i(id);
}

void LeagueWarSystem::test_open_space()
{
	JUDGE_RETURN(this->is_activity_time() == true, ;);

	if (this->lwar_scene_set_.size() <= 1)
	{
		this->pop_lwar_scene(this->lwar_scene_set_.size());
	}
	for (uint i = 0; i < this->lwar_scene_set_.size(); ++i)
	{
		LeagueWarScene* lwar_scene = this->find_lwar_scene((int)i);
		lwar_scene->test_set_has_enter();
	}
}

int LeagueWarSystem::ahead_lwar_event()
{
	JUDGE_RETURN(this->is_today_activity() == true, -1);
	GameCommon::map_sync_activity_tips_ahead(this->lwar_detail_.__activity_id,
			this->time_info_.refresh_time_);
	MSG_USER("league_war ahead %d %d", this->time_info_.time_index_,
			this->time_info_.refresh_time_);
	return 0;
}

int LeagueWarSystem::start_lwar_event()
{
	JUDGE_RETURN(this->is_today_activity() == true, -1);
	JUDGE_RETURN(this->lwar_scene_set_.empty() == true, -1);

	this->player_map_->clear();
	this->league_map_.clear();
	this->lwar_scene_set_.clear();
	this->lwar_scene_package_->clear();

	int space_count = CONFIG_INSTANCE->league_war("init_space").asInt();
	for (int space_index = 0; space_index < space_count; ++space_index)
	{
		this->pop_lwar_scene(space_index);
	}

	this->lwar_info_.lwar_rank_map_.clear();
	this->notify_all_player(1);

	int shout_start_id = CONFIG_INSTANCE->league_war("shout_start").asInt();
	GameCommon::announce(shout_start_id);
	GameCommon::map_sync_activity_tips_start(this->lwar_detail_.__activity_id,
			this->time_info_.refresh_time_);
	MSG_USER("帮派争霸开始");
	return 0;
}

int LeagueWarSystem::stop_lwar_event()
{
	JUDGE_RETURN(this->is_today_activity() == true, -1);
	GameCommon::map_sync_activity_tips_stop(this->lwar_detail_.__activity_id);

	for (LeagueWarSceneSet::iterator iter = this->lwar_scene_set_.begin();
			iter != this->lwar_scene_set_.end(); ++iter)
	{
		LeagueWarScene* scene = *iter;
		scene->stop_lwar();
	}

	this->lwar_info_.total_num_ += 1;
	this->lwar_info_.tick_ = ::time(0);
	const Json::Value score_rank_awards = CONFIG_INSTANCE->league_war("score_rank_awards");
	for (AllLeagueInfoMap::iterator iter = this->league_map_.begin();
			iter != this->league_map_.end(); ++iter)
	{
		// 保存帮派排名
		LWarLeagueInfo &lwar_league = iter->second;
		LeagueWarInfo::LeagueWarRank &rank_info = this->lwar_info_.lwar_rank_map_[lwar_league.rank_];
		rank_info.league_index_ = lwar_league.league_index_;
		rank_info.league_name_ = lwar_league.league_name_;
		rank_info.rank_ = lwar_league.rank_;
		rank_info.leader_ = lwar_league.leader_name_;
		rank_info.score_ = lwar_league.score_;
		this->init_force_info(rank_info);

		MSG_USER("stop_lwar_event, league_name: %s, score: %d",
				lwar_league.leader_name_.c_str(), lwar_league.score_);

		//计算帮派积分排名奖励
		int add_exp = 0;
		int rank = lwar_league.rank_;
		if (rank < (int)score_rank_awards.size())
		{
			//上榜的帮派
			for (uint i = 0; i < score_rank_awards.size(); ++i)
			{
				int need_rank = score_rank_awards[i][0u].asInt();
				int get_award = score_rank_awards[i][1u].asInt();
				JUDGE_CONTINUE(rank == need_rank);

				int player_num = lwar_league.enter_lwar_player_.size();
				add_exp = player_num * get_award;
				break;
			}
		}
		else
		{
			//未上榜的帮派
			uint last_size = score_rank_awards.size()-1;
			int get_award = score_rank_awards[last_size][1u].asInt();
			int player_num = lwar_league.enter_lwar_player_.size();
			add_exp = player_num * get_award;
		}
		Proto30100230 inner_req;
		inner_req.set_league_index(iter->first);
		inner_req.set_add_flag_exp(add_exp);
		MAP_MONITOR->dispatch_to_logic(&inner_req);

		JUDGE_CONTINUE(rank == 1);

		int shout_id = CONFIG_INSTANCE->league_war("win_shout").asInt();
		BrocastParaVec para_vec;
		GameCommon::push_brocast_para_string(para_vec, lwar_league.league_name_);
		GameCommon::announce(shout_id, &para_vec);
	}

	MMOLeague::save_league_war(this->lwar_info_);
	MAP_MONITOR->dispatch_to_scene_by_noplayer(GameEnum::LEAGUE_REGION_FIGHT_ID,
			INNER_MAP_LOAD_LEAGUE_REGION);

	this->lwar_scene_set_.clear();
	this->notify_all_player(0);

	int shout_end_id = CONFIG_INSTANCE->league_war("shout_finish").asInt();
	GameCommon::announce(shout_end_id);
	MSG_USER("帮派争霸结束");
	return 0;
}

void LeagueWarSystem::update_leauge_score(LWarLeagueInfo& lwar_league)
{
	Int64 league_index = lwar_league.league_index_;
	JUDGE_RETURN(league_index > 0, ;);

	bool rank_flag = false;
	if (this->league_map_.count(league_index) == 0)
	{
		LWarLeagueInfo& league_info = this->league_map_[league_index];
		league_info.league_index_ = lwar_league.league_index_;
		league_info.league_name_ = lwar_league.league_name_;
		league_info.score_ = lwar_league.score_;
		league_info.tick_ = ::time(0);
		league_info.space_id_ = lwar_league.space_id_;

		rank_flag = true;
	}
	else
	{
		LWarLeagueInfo& league_info = this->league_map_[league_index];
		if (lwar_league.score_ > league_info.score_)
		{
			league_info.score_ = lwar_league.score_;
			league_info.tick_ = ::time(0);

			rank_flag = true;
		}

		MSG_USER("update_leauge_score, league_name: %s, score: %d",
				league_info.leader_name_.c_str(), league_info.score_);
	}
	JUDGE_RETURN(rank_flag == true, ;);

	this->league_score_rank_.clear();
	for (AllLeagueInfoMap::iterator iter = this->league_map_.begin();
			iter != this->league_map_.end(); ++iter)
	{
		LWarLeagueInfo &all_league = iter->second;
		ThreeObj obj;
		obj.id_ = all_league.league_index_;
		obj.tick_ = all_league.tick_;
		obj.value_ = all_league.score_;

		this->league_score_rank_.push_back(obj);
	}
	int rank = 1;
	std::sort(this->league_score_rank_.begin(), this->league_score_rank_.end(),
			GameCommon::three_comp_by_desc);

	for (ThreeObjVec::iterator iter = this->league_score_rank_.begin();
			iter != this->league_score_rank_.end(); ++iter)
	{
		LWarLeagueInfo &all_league = this->league_map_[iter->id_];
		all_league.rank_ = rank;
		++rank;
	}
}

int LeagueWarSystem::find_league_rank(Proto50400351* respond)
{
	JUDGE_RETURN(this->league_map_.size() > 0, -1);

	for (AllLeagueInfoMap::iterator iter = this->league_map_.begin();
			iter != this->league_map_.end(); ++iter)
	{
		LWarLeagueInfo &all_league = iter->second;
		JUDGE_RETURN(all_league.league_index_ > 0, -1);

		ProtoLeagueRankInfo *total_league_rank = respond->add_total_league_rank();
		all_league.serialize(total_league_rank);
	}

	return 0;
}

int LeagueWarSystem::find_all_league_rank(Proto50400353* respond)
{
	JUDGE_RETURN(this->league_map_.size() > 0, -1);

	for (AllLeagueInfoMap::iterator iter = this->league_map_.begin();
			iter != this->league_map_.end(); ++iter)
	{
		LWarLeagueInfo &all_league = iter->second;
		JUDGE_RETURN(all_league.league_index_ > 0, -1);

		ProtoLeagueRankInfo *total_league_rank = respond->add_total_league_rank();
		all_league.serialize(total_league_rank);
	}

	return 0;
}

int LeagueWarSystem::find_player_league_rank(Proto50400351* respond, Int64 league_index)
{
	JUDGE_RETURN(this->league_map_.count(league_index) > 0, -1);

	LWarLeagueInfo &my_league = this->league_map_[league_index];
	ProtoLeagueRankInfo *total_my_league = respond->mutable_total_my_league();
	total_my_league->set_league_index(my_league.league_index_);
	total_my_league->set_league_name(my_league.league_name_);
	total_my_league->set_score(my_league.score_);
	total_my_league->set_rank(my_league.rank_);

	return 0;
}

LWarRoleInfo* LeagueWarSystem::find_lwar(Int64 role_id)
{
	return this->player_map_->find_object(role_id);
}

LeagueWarScene* LeagueWarSystem::fetch_lwar_scene(Int64 role_id)
{
	LWarRoleInfo* lwar_role = this->find_lwar(role_id);
	if (lwar_role != NULL)
	{
		return this->lwar_scene_package_->find_object(lwar_role->space_);
	}

	for (LeagueWarSceneSet::iterator iter = this->lwar_scene_set_.begin();
			iter != this->lwar_scene_set_.end(); ++iter)
	{
		LeagueWarScene* scene = *iter;
		JUDGE_CONTINUE(scene->is_player_full() == false);
		return scene;
	}

	return this->pop_lwar_scene(this->lwar_scene_set_.size());
}

LeagueWarScene* LeagueWarSystem::pop_lwar_scene(int space_id)
{
	LeagueWarInfo::LeagueWarRank &war_rank = this->lwar_info_.lwar_rank_map_[1];
	Int64 league_index = this->lwar_info_.find_camp_league(1);
	string league_name = war_rank.league_name_;

	LeagueWarScene* lwar_scene = this->lwar_scene_package_->pop_object();
	JUDGE_RETURN(lwar_scene != NULL, NULL);

	lwar_scene->init_lwar_scene(this->real_scene_, space_id);
	lwar_scene->run_scene();
	lwar_scene->set_camp_info(league_index, league_name);

	this->lwar_scene_package_->bind_object(space_id, lwar_scene);
	this->lwar_scene_set_.push_back(lwar_scene);

	return lwar_scene;
}

LeagueWarScene* LeagueWarSystem::find_lwar_scene(int space_id)
{
	return this->lwar_scene_package_->find_object(space_id);
}

void LeagueWarSystem::set_first_space_winner(LWarLeagueInfo& lwar_league)
{
	this->first_win_.league_index_ = lwar_league.league_index_;
	this->first_win_.league_name_ = lwar_league.league_name_;
	this->first_win_.leader_name_ = lwar_league.leader_name_;
}

void LeagueWarSystem::fetch_first_space_winner(Proto80400386* respond)
{
	respond->set_win_league_id(this->first_win_.league_index_);
	respond->set_win_league_name(this->first_win_.league_name_);
	respond->set_win_league_leader(this->first_win_.leader_name_);
}

void LeagueWarSystem::update_leader_info(Int64 league_id, Int64 leader_id, string leader)
{
	JUDGE_RETURN(this->league_map_.count(league_id) > 0, ;);

	LWarLeagueInfo& league_info = this->league_map_[league_id];
	league_info.leader_index_ = leader_id;
	league_info.leader_name_ = leader;

	for (LeagueWarSceneSet::iterator iter = this->lwar_scene_set_.begin();
			iter != this->lwar_scene_set_.end(); ++iter)
	{
		LeagueWarScene* scene = *iter;
		JUDGE_CONTINUE(scene != NULL);

		scene->update_league_info(league_id, leader_id, leader);
	}
}

Int64 LeagueWarSystem::fetch_first_win_league_id()
{
	return this->first_win_.league_index_;
}

int LeagueWarSystem::notify_all_player(int value)
{
	int event_id = GameEnum::PA_EVENT_LEAGUE_WAR;
	if (value >= 1)
	{
		// 向全服玩家发送红点
		Proto81401703 active_res;
		active_res.set_even_id(event_id);
		active_res.set_even_value(value);
		MAP_MONITOR->notify_all_player_info(ACTIVE_PLAYER_ASSIST_APPEAR, &active_res);
	}
	else if (value <= 0)
	{
		Proto81401705 active_res;
		active_res.set_even_id(event_id);
		MAP_MONITOR->notify_all_player_info(ACTIVE_PLAYER_ASSIST_DISAPPEAR, &active_res);
	}

	return 0;
}

int LeagueWarSystem::find_open_space_num()
{
	return this->lwar_scene_set_.size();
}

void LeagueWarSystem::find_open_space(Proto80400385* respond)
{
	for (LeagueWarSceneSet::iterator iter = this->lwar_scene_set_.begin();
			iter != this->lwar_scene_set_.end(); ++iter)
	{
		LeagueWarScene* scene = *iter;
		JUDGE_CONTINUE(scene->is_player_enter() == true);

		respond->add_total_space(scene->space_id());
	}
}

void LeagueWarSystem::init_force_info(LeagueWarInfo::LeagueWarRank& war_rank)
{
	MapLeagueInfo* info = LEAGUE_MONITOR->find_map_league(war_rank.league_index_);
	JUDGE_RETURN(info != NULL, ;);

	war_rank.force_ = info->force_;
	war_rank.leader_ = info->leader_;
	war_rank.flag_lvl_ = info->flag_lvl_;
}

void LeagueWarSystem::init_league_map(Int64 role_id, Int64 league_id, string league_name, int space_id)
{
	if (this->league_map_.count(league_id) == 0)
	{
		LWarLeagueInfo& league_info = this->league_map_[league_id];
		league_info.league_index_ = league_id;
		league_info.league_name_ = league_name;
		league_info.score_ = 0;
		league_info.tick_ = ::time(0);
		league_info.space_id_ = space_id;
		league_info.enter_lwar_player_[role_id] = league_id;
	}
	else
	{
		LWarLeagueInfo& league_info = this->league_map_[league_id];
		league_info.enter_lwar_player_[role_id] = league_id;
	}
}


