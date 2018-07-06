/*
 * TrvlPeakMonitor.cpp
 *
 *  Created on: 2017年5月22日
 *      Author: lyw
 */

#include "TrvlPeakMonitor.h"
#include "TrvlPeakPreScene.h"
#include "TrvlPeakScene.h"
#include "ProtoDefine.h"
#include "MapMonitor.h"
#include "GameCommon.h"
#include "MapPlayerEx.h"
#include "MMOTravTeam.h"
#include "MongoDataMap.h"
#include "MMORole.h"

TrvlPeakSysInfo::TrvlPeakSysInfo(void)
{
	TrvlPeakSysInfo::reset();
}

void TrvlPeakSysInfo::reset(void)
{
	this->__activity_id = 0;
	this->__enter_level = 0;
}

int TrvlPeakMonitor::StartTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int TrvlPeakMonitor::StartTimer::handle_timeout(const Time_Value &tv)
{
	if (this->timer_type_ == 1)
	{
		TRVL_PEAK_MONITOR->handle_peak_quality_timeout();
	}
	else if (this->timer_type_ == 2)
	{
		TRVL_PEAK_MONITOR->handle_peak_knockout_timeout();
	}
	else if (this->timer_type_ == 3)
	{
		TRVL_PEAK_MONITOR->handle_peak_signup_timeout();
	}
	return 0;
}

void TrvlPeakMonitor::StartTimer::set_timer_type(int timer_type)
{
	this->timer_type_ = timer_type;
}

int TrvlPeakMonitor::ArrangeTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int TrvlPeakMonitor::ArrangeTimer::handle_timeout(const Time_Value &tv)
{
	TRVL_PEAK_MONITOR->arrange_rival_timeout();
	TRVL_PEAK_MONITOR->active_add_player_exp();
	TRVL_PEAK_MONITOR->sort_quality_score_rank();
	return 0;
}

int TrvlPeakMonitor::SaveTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int TrvlPeakMonitor::SaveTimer::handle_timeout(const Time_Value &tv)
{
	TRVL_PEAK_MONITOR->save_peak_act_info();
	return 0;
}

int TrvlPeakMonitor::DayTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int TrvlPeakMonitor::DayTimer::handle_timeout(const Time_Value &tv)
{
	TRVL_PEAK_MONITOR->reset_day_info();
	return 0;
}

int TrvlPeakMonitor::TransferTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int TrvlPeakMonitor::TransferTimer::handle_timeout(const Time_Value &tv)
{
	TRVL_PEAK_MONITOR->set_fight_type(0);
	TRVL_PEAK_MONITOR->peak_pre_scene_->notify_all_player_exit();
	return this->cancel_timer();
}

TrvlPeakMonitor::TrvlPeakMonitor() {
	// TODO Auto-generated constructor stub
	this->scene_package_ = new PoolPackage<TrvlPeakScene>;
	this->trav_peak_team_map_ = new TravPeakTeamMap();
	this->trav_role_team_map_ = new TravRoleTeamMap();
	this->trav_peak_team_pool_ = new TravPeakTeamPool();
	this->peak_pre_scene_ = new TrvlPeakPreScene();

	TrvlPeakMonitor::reset();
}

TrvlPeakMonitor::~TrvlPeakMonitor() {
	// TODO Auto-generated destructor stub
}

void TrvlPeakMonitor::reset(void)
{
	this->signup_time_.reset();
	this->quality_time_.reset();
	this->knockout_time_.reset();
	this->signup_act_.reset();
	this->quality_act_.reset();
	this->knockout_act_.reset();

	this->quality_info_.reset();
	this->knockout_info_.reset();

	this->quality_timer_.cancel_timer();
	this->knockout_timer_.cancel_timer();
	this->signup_timer_.cancel_timer();
	this->arrange_timer_.cancel_timer();
	this->save_timer_.cancel_timer();
	this->day_timer_.cancel_timer();
	this->transfer_timer_.cancel_timer();

	this->arrange_vec_.clear();
	this->fight_list_.clear();
	this->sign_map_.clear();
	this->fight_map_.clear();

	this->monitor_init_ = 0;
	this->arena_id_ = 0;
	this->arrange_time_ = 0;
	this->transfer_pre_time_ = 0;
	this->left_prep_time_ = 0;
	this->fight_type_ = 0;
	this->add_exp_time_ = 0;

	this->is_update_quality_state_ = false;
	this->is_update_knockout_state_ = false;
	this->is_update_quality_rank_ = false;

	this->prep_waiting_time_ = 0;
	this->wait_tick_ = 0;
	this->fighting_time_ = 0;
	this->quit_time_ = 0;
	this->prep_time_ = 0;
	this->wait_time_ = 0;
	this->exp_time_  = 0;
	this->transfer_out_time_ = 0;

	this->is_test_ = 0;
}

void TrvlPeakMonitor::start(void)
{
	this->monitor_init_ = true;

	this->init_conf_info();
	this->init_signup_act_info();
	this->init_quality_act_info();
	this->init_knockout_act_info();
	this->init_pre_scene();
	this->load_trvl_peak();
	this->load_peak_act_info();
	this->reset_day_info();

	this->save_timer_.refresh_check_tick(Time_Value::HOUR);
}

void TrvlPeakMonitor::stop(void)
{
	this->quality_timer_.cancel_timer();
	this->knockout_timer_.cancel_timer();
	this->signup_timer_.cancel_timer();
	this->arrange_timer_.cancel_timer();
	this->save_timer_.cancel_timer();
	this->day_timer_.cancel_timer();
	this->transfer_timer_.cancel_timer();

	this->save_trvl_peak();
	this->save_peak_act_info();
}

Int64 TrvlPeakMonitor::find_team_id(const Int64 role_id)
{
	TravPeakTeam* trvl_team = this->find_travel_team_by_role_id(role_id);
	JUDGE_RETURN(trvl_team != NULL, 0);

	return trvl_team->__team_id;
}

TravPeakTeam *TrvlPeakMonitor::find_travel_team(const Int64 team_id)
{
	TravPeakTeam *team = NULL;
	if (this->trav_peak_team_map_->find(team_id, team) == 0)
	    return team;
	return NULL;
}

TravPeakTeam *TrvlPeakMonitor::find_travel_team_by_role_id(const Int64 role_id)
{
    Int64 team_id = 0;
    if (this->trav_role_team_map_->find(role_id, team_id) == 0)
    {
    	return this->find_travel_team(team_id);
    }
    return NULL;
}

TrvlPeakPreScene *TrvlPeakMonitor::fetch_trvl_peak_pre_scene()
{
	return this->peak_pre_scene_;
}

TrvlPeakScene* TrvlPeakMonitor::find_scene(int space_id)
{
	return this->scene_package_->find_object(space_id);
}

void TrvlPeakMonitor::recycle_scene(TrvlPeakScene* scene)
{
	this->scene_package_->unbind_and_push(scene->space_id(), scene);
}

void TrvlPeakMonitor::push_scene(TrvlPeakScene* scene)
{
	this->scene_package_->push_object(scene);
}

void TrvlPeakMonitor::notify_recogn(Int64 role, int recogn)
{
	MapPlayerEx* player = this->peak_pre_scene_->find_player(role);
	JUDGE_RETURN(player != NULL, ;);

	player->respond_to_client(recogn);
}

void TrvlPeakMonitor::arrange_rival_timeout()
{
	if (this->left_prep_time_ > 0)
	{
		this->left_prep_time_ -= 1;
	}

	this->arrange_rival();
	this->start_transfer();
}

void TrvlPeakMonitor::arrange_rival()
{
	if (this->is_quality_act_time() == true)
	{
		JUDGE_RETURN(this->quality_timer_.left_second() > 10, ;);

		this->arrange_time_ += 1;
		int curr_wait = this->fetch_arrange_wait();
		JUDGE_RETURN(this->arrange_time_ >= curr_wait, ;);

		this->fight_list_.clear();
		this->fight_map_.clear();

		this->arrange_vec_.clear();
		this->arrange_vec_.reserve(this->sign_map_.size());

		//报名列表排序
		for (LongMap::iterator iter = this->sign_map_.begin();
				iter != this->sign_map_.end(); ++iter)
		{
			TravPeakTeam* team = this->find_travel_team(iter->first);
			JUDGE_CONTINUE(team != NULL);

			FourObj obj;
			obj.id_ 			= iter->first;	//队伍ID
			obj.first_value_ 	= team->total_force();
			obj.second_value_	= team->total_level();
			this->arrange_vec_.push_back(obj);
		}

		std::sort(this->arrange_vec_.begin(), this->arrange_vec_.end(),
				GameCommon::four_comp_by_desc);

		int total_count = this->arrange_vec_.size();
		for (int i = 0; i < total_count; ++i)
		{
			JUDGE_CONTINUE(this->arrange_vec_[i].tick_ == false);

			TravPeakTeam* first = this->find_travel_team(this->arrange_vec_[i].id_);
			JUDGE_CONTINUE(first != NULL);

			//0:不合法, 1:找到对手
			IntPair pair = this->find_rival(i);
			JUDGE_CONTINUE(pair.first > 0);

			FourObj obj;
			obj.id_ = pair.first;	//对手类型
			obj.first_value_ = i;
			obj.second_value_= pair.second;

			TravPeakTeam* second = this->find_travel_team(this->arrange_vec_[pair.second].id_);
			obj.first_id_  = first->__team_id;
			obj.second_id_ = second->__team_id;

			first->__state = TravPeakTeam::STATE_ATTEND;
			second->__state = TravPeakTeam::STATE_ATTEND;

			this->arrange_vec_[pair.second].tick_ = true;
			this->arrange_vec_[i].tick_ = true;

			this->fight_list_.push_back(obj);
		}

		for (FourObjList::iterator iter = this->fight_list_.begin();
				iter != this->fight_list_.end(); ++iter)
		{
			this->sign_map_.erase(iter->first_id_);
			this->sign_map_.erase(iter->second_id_);
			this->arrange_peak(*iter);
		}

		this->arrange_time_	= 0;
		this->transfer_pre_time_= 5;
	}
}

void TrvlPeakMonitor::start_transfer()
{
	this->transfer_pre_time_ -= 1;
	JUDGE_RETURN(this->transfer_pre_time_ == 0, ;);

	for (LongMap::iterator iter = this->fight_map_.begin(); iter != this->fight_map_.end(); ++iter)
	{
		TravPeakTeam *travel_team = this->find_travel_team(iter->first);
		JUDGE_CONTINUE(travel_team != NULL);

		TrvlPeakScene *scene = this->find_scene(travel_team->__space_id);
		JUDGE_CONTINUE(scene != NULL);

		for (TravPeakTeam::TeamerMap::iterator teamer_iter = travel_team->__teamer_map.begin();
				teamer_iter != travel_team->__teamer_map.end(); ++teamer_iter)
		{
			TravPeakTeam::TravPeakTeamer &teamer_info = teamer_iter->second;
			MapPlayerEx* player = this->peak_pre_scene_->find_player(teamer_iter->first);
			if (player == NULL )
			{
				player = teamer_info.__offline_player;
				if (teamer_info.__offline_player != NULL)
				{
					// 传送离线玩家到战斗场景
					player->init_mover_scene(scene);
					MoverCoord& coord = this->enter_pos_[travel_team->__camp_id];
					player->mover_detail().__location = coord;
					player->mover_detail().__scene_mode = SCENE_MODE_TRVL_PEAK;
					player->fight_detail().__blood = player->fight_detail().__blood_total(player);
					player->fight_detail().__magic = player->fight_detail().__magic_total(player);
					player->set_camp_id(travel_team->__camp_id);
					player->clear_status(true);
					player->MapOfflineHook::enter_scene();
				}
			}
			else
			{
				player->set_camp_id(travel_team->__camp_id);
				MoverCoord& coord = this->enter_pos_[travel_team->__camp_id];
				player->transfer_dispatcher(this->peak_scene_id(), coord,
						SCENE_MODE_TRVL_PEAK, travel_team->__space_id);
			}
		}
	}

	this->fight_map_.clear();
}

bool TrvlPeakMonitor::is_activity_time()
{
	JUDGE_RETURN(this->monitor_init_ == true, false);
	JUDGE_RETURN(this->is_quality_act_time() || this->is_knockout_act_time(), false);
	return true;
}

int TrvlPeakMonitor::left_prep_time()
{
	return this->left_prep_time_;
}

IntPair TrvlPeakMonitor::fetch_act_left_tick()
{
	IntPair pair;
	JUDGE_RETURN(this->monitor_init_ == true, pair);

	if (this->is_fighting_time() == true)
	{
		if (this->is_quality_act_time())
		{
			pair.first = 1;
			pair.second = this->quality_timer_.left_second();
		}
		else if (this->is_knockout_act_time())
		{
			pair.first = 2;
			pair.second = this->quality_timer_.left_second();
		}
	}
	else
	{
		pair.first = 0;
		pair.second = this->left_prep_time_;
	}

	return pair;
}

int TrvlPeakMonitor::is_fighting_time()
{
	JUDGE_RETURN(this->monitor_init_ == true, false);
	JUDGE_RETURN(this->is_activity_time() == true, false);
	return this->left_prep_time_ <= 0;
}

int TrvlPeakMonitor::check_teamer_can_enter(Int64 team_id)
{
	if (this->is_quality_act_time())
	{
		JUDGE_RETURN(this->quality_info_.__signup_team_set.count(team_id) > 0, ERROR_NO_SIGN_TRAVPEAK);
		return 0;
	}

	return ERROR_TRAVEL_PEAK_NOT_OPEN;
}

void TrvlPeakMonitor::handle_peak_signup_timeout()
{
	int state = this->signup_time_.cur_state_;
	this->signup_time_.set_next_stage();
	this->handle_peak_signup_i(state);
}

void TrvlPeakMonitor::handle_peak_signup_i(int state)
{
	switch (state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
		this->start_event(TIMER_SIGNUP);
		break;
	}
	case GameEnum::ACTIVITY_STATE_START:
	{
		this->stop_event(TIMER_SIGNUP);
		break;
	}
	}

	this->signup_timer_.cancel_timer();
	this->signup_timer_.schedule_timer(this->signup_time_.refresh_time_);

	MSG_USER("trvl peak knockout %d, %d, %d", this->signup_time_.time_index_,
			this->signup_time_.cur_state_, this->signup_time_.refresh_time_);
}

void TrvlPeakMonitor::handle_peak_quality_timeout()
{
	int state = this->quality_time_.cur_state_;
	this->quality_time_.set_next_stage();
	this->handle_peak_quality_i(state);
}

void TrvlPeakMonitor::handle_peak_quality_i(int state)
{
	switch (state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
#ifdef TEST_COMMAND
		this->left_prep_time_ = 30;
#else
		this->left_prep_time_ = this->prep_waiting_time_;
#endif
		this->start_event(TIMER_QUALITY);
		break;
	}
	case GameEnum::ACTIVITY_STATE_START:
	{
		this->stop_event(TIMER_QUALITY);
		break;
	}
	}

	this->quality_timer_.cancel_timer();
	this->quality_timer_.schedule_timer(this->quality_time_.refresh_time_);

	MSG_USER("trvl peak quality %d, %d, %d", this->quality_time_.time_index_,
			this->quality_time_.cur_state_, this->quality_time_.refresh_time_);
}

void TrvlPeakMonitor::handle_peak_knockout_timeout()
{
	int state = this->knockout_time_.cur_state_;
	this->knockout_time_.set_next_stage();
	this->handle_peak_knockout_i(state);
}

void TrvlPeakMonitor::handle_peak_knockout_i(int state)
{
	switch (state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
		this->start_event(TIMER_KNOCKOUT);
		break;
	}
	case GameEnum::ACTIVITY_STATE_START:
	{
		this->stop_event(TIMER_KNOCKOUT);
		break;
	}
	}

	this->knockout_timer_.cancel_timer();
	this->knockout_timer_.schedule_timer(this->knockout_time_.refresh_time_);

	MSG_USER("trvl peak knockout %d, %d, %d", this->knockout_time_.time_index_,
			this->knockout_time_.cur_state_, this->knockout_time_.refresh_time_);
}

void TrvlPeakMonitor::init_signup_act_info()
{
	const Json::Value& conf = this->scene_conf();
	this->signup_act_.__activity_id = conf["signup_act_id"].asInt();

	const Json::Value& activity_conf = CONFIG_INSTANCE->common_activity(this->signup_act_.__activity_id);
	JUDGE_RETURN(activity_conf.empty() == false, ;);

	GameCommon::cal_activity_info(this->signup_time_, activity_conf);

	this->signup_timer_.schedule_timer(this->signup_time_.refresh_time_);
	this->signup_timer_.set_timer_type(TIMER_SIGNUP);
	JUDGE_RETURN(this->signup_time_.cur_state_ == GameEnum::ACTIVITY_STATE_START, ;);

	this->start_event(TIMER_SIGNUP);
}

void TrvlPeakMonitor::init_quality_act_info()
{
	const Json::Value& conf = this->scene_conf();
	this->quality_act_.__activity_id = conf["quality_act_id"].asInt();
	this->quality_act_.__enter_level = conf["enter_level"].asInt();

	const Json::Value& activity_conf = CONFIG_INSTANCE->common_activity(this->quality_act_.__activity_id);
	JUDGE_RETURN(activity_conf.empty() == false, ;);

	GameCommon::cal_activity_info(this->quality_time_, activity_conf);

	this->quality_timer_.schedule_timer(this->quality_time_.refresh_time_);
	this->quality_timer_.set_timer_type(TIMER_QUALITY);
	JUDGE_RETURN(this->quality_time_.cur_state_ == GameEnum::ACTIVITY_STATE_START, ;);

	this->start_event(TIMER_QUALITY);
}

void TrvlPeakMonitor::init_knockout_act_info()
{
	const Json::Value& conf = this->scene_conf();
	this->knockout_act_.__activity_id = conf["knockout_act_id"].asInt();
	this->knockout_act_.__enter_level = conf["enter_level"].asInt();

	const Json::Value& activity_conf = CONFIG_INSTANCE->common_activity(this->knockout_act_.__activity_id);
	JUDGE_RETURN(activity_conf.empty() == false, ;);

	GameCommon::cal_activity_info(this->knockout_time_, activity_conf);

	this->knockout_timer_.schedule_timer(this->knockout_time_.refresh_time_);
	this->knockout_timer_.set_timer_type(TIMER_KNOCKOUT);
	JUDGE_RETURN(this->knockout_time_.cur_state_ == GameEnum::ACTIVITY_STATE_START, ;);

	this->start_event(TIMER_KNOCKOUT);
}

bool TrvlPeakMonitor::is_quality_act_time()
{
	JUDGE_RETURN(this->quality_time_.cur_state_ == GameEnum::ACTIVITY_STATE_START, false);
	return true;
}

TravPeakQualityInfo &TrvlPeakMonitor::quality_info()
{
	return this->quality_info_;
}

bool TrvlPeakMonitor::is_knockout_act_time()
{
	JUDGE_RETURN(this->knockout_time_.cur_state_ == GameEnum::ACTIVITY_STATE_START, false);
	return true;
}

void TrvlPeakMonitor::init_pre_scene()
{
	this->peak_pre_scene_->peak_prep_init();
}

void TrvlPeakMonitor::start_event(int timer_type)
{
	this->arena_id_ = 1;
	this->arrange_time_ = 0;
	this->transfer_pre_time_ = 0;
	this->transfer_timer_.cancel_timer();

	int shout_id = 0;
	switch (timer_type)
	{
	case TIMER_QUALITY:
	{
		this->fight_type_ = timer_type;
		this->arrange_timer_.schedule_timer(1);

		GameCommon::map_sync_activity_tips_start(PairObj(this->quality_act_.__activity_id, true),
				this->quality_time_.refresh_time_);

		shout_id = this->scene_conf()["quality_start_shout"].asInt();
		MSG_USER("trvl peak quality start %d %d", this->quality_time_.time_index_, this->quality_time_.refresh_time_);
		break;
	}
	case TIMER_KNOCKOUT:
	{
		this->fight_type_ = timer_type;
		this->arrange_timer_.schedule_timer(1);

		GameCommon::map_sync_activity_tips_start(PairObj(this->knockout_act_.__activity_id, true),
				this->knockout_time_.refresh_time_);

		shout_id = this->scene_conf()["knockout_start_shout"].asInt();
		MSG_USER("trvl peak knockout start %d %d", this->knockout_time_.time_index_, this->knockout_time_.refresh_time_);
		break;
	}
	case TIMER_SIGNUP:
	{
		GameCommon::map_sync_activity_tips_start(PairObj(this->signup_act_.__activity_id, true),
				this->signup_time_.refresh_time_);

		shout_id = this->scene_conf()["signup_start_shout"].asInt();
		MSG_USER("trvl peak signup start %d %d", this->signup_time_.time_index_, this->signup_time_.refresh_time_);

		break;
	}
	default:
		break;
	}

	GameCommon::trvl_announce(shout_id);
}

void TrvlPeakMonitor::stop_event(int timer_type)
{
	int shout_id = 0;
	this->arrange_timer_.cancel_timer();

	switch (timer_type)
	{
	case TIMER_QUALITY:
	{
		GameCommon::map_sync_activity_tips_stop(PairObj(this->quality_act_.__activity_id, true),
				this->quality_time_.refresh_time_);

		if (this->is_test_ > 0)
		{
			this->is_test_ = 0;
			this->cal_activity_time();
			this->send_act_time_to_logic();
		}

		this->transfer_timer_.schedule_timer(this->transfer_out_time_);

		this->is_update_quality_state_ = true;
		this->quality_info_.__signup_team_set.clear();
		this->start_send_signup_state();

		shout_id = this->scene_conf()["quality_end_shout"].asInt();
		MSG_USER("trvl peak quality end %d %d", this->quality_time_.time_index_, this->quality_time_.refresh_time_);
		break;
	}
	case TIMER_KNOCKOUT:
	{
		GameCommon::map_sync_activity_tips_stop(PairObj(this->knockout_act_.__activity_id, true),
				this->knockout_time_.refresh_time_);

		if (this->is_test_ > 0)
		{
			this->is_test_ = 0;
			this->cal_activity_time();
			this->send_act_time_to_logic();
		}

		this->transfer_timer_.schedule_timer(this->transfer_out_time_);
		this->is_update_knockout_state_ = true;

		shout_id = this->scene_conf()["knockout_end_shout"].asInt();
		MSG_USER("trvl peak knockout end %d %d", this->knockout_time_.time_index_, this->knockout_time_.refresh_time_);
		break;
	}
	case TIMER_SIGNUP:
	{
		GameCommon::map_sync_activity_tips_stop(PairObj(this->signup_act_.__activity_id, true),
				this->signup_time_.refresh_time_);

		shout_id = this->scene_conf()["signup_end_shout"].asInt();
		MSG_USER("trvl peak signup end %d %d", this->signup_time_.time_index_, this->signup_time_.refresh_time_);

		break;
	}
	default:
		break;
	}

	GameCommon::trvl_announce(shout_id);
}

void TrvlPeakMonitor::set_fight_type(int type)
{
	this->fight_type_ = type;
}

int TrvlPeakMonitor::fight_type()
{
	return this->fight_type_;
}

int TrvlPeakMonitor::player_exp(Int64 role_id)
{
	JUDGE_RETURN(this->player_exp_map_.count(role_id) > 0, 0);
	return this->player_exp_map_[role_id];
}

void TrvlPeakMonitor::update_fight_after_team_record(TravPeakTeam *win_team, TravPeakTeam *lost_team)
{
	JUDGE_RETURN(win_team != NULL && lost_team != NULL, ;);

	win_team->__state = TravPeakTeam::STATE_NONE;
	lost_team->__state = TravPeakTeam::STATE_NONE;

	const Json::Value& conf = this->scene_conf();
	if (this->fight_type_ == TIMER_QUALITY)
	{
		++win_team->__quality_times;
		++win_team->__continue_win;
		win_team->__start_tick = ::time(NULL) + conf["quit_time"].asInt() + 1;
		win_team->__last_reward_id = this->fetch_win_or_lost_reward(win_team->__quality_times, 1, conf["quality_reward"]);
		if (win_team->__quality_times <= conf["no_score_times"].asInt())
		{
			int conti_score = this->fetch_conti_or_break_win_score(win_team->__continue_win, conf["continue_win"]);
			int break_score = this->fetch_conti_or_break_win_score(lost_team->__continue_win, conf["break_win"]);
			win_team->__last_add_score = conti_score + break_score + conf["get_score"][0u].asInt();
			win_team->__score += win_team->__last_add_score;
			win_team->__update_tick = ::time(NULL);

			this->is_update_quality_rank_ = true;
		}
		else
		{
			win_team->__last_add_score = 0;
		}

		++lost_team->__quality_times;
		lost_team->__continue_win = 0;
		lost_team->__start_tick = ::time(NULL) + conf["quit_time"].asInt() + 1;
		lost_team->__last_reward_id = this->fetch_win_or_lost_reward(lost_team->__quality_times, 2, conf["quality_reward"]);
		if (lost_team->__quality_times <= conf["no_score_times"].asInt())
		{
			lost_team->__score += conf["get_score"][1u].asInt();
			lost_team->__last_add_score = conf["get_score"][1u].asInt();
			lost_team->__update_tick = ::time(NULL);

			this->is_update_quality_rank_ = true;
		}
		else
		{
			lost_team->__last_add_score = 0;
		}
	}
}

int TrvlPeakMonitor::request_enter_travel_peak(int sid, Proto30400051* request)
{
	string main_version = request->main_version();
	JUDGE_RETURN(CONFIG_INSTANCE->is_same_main_version(main_version) == true,
			ERROR_TRVL_SAME_VERSION);

	JUDGE_RETURN(this->is_activity_time() == true, ERROR_GUARD_TIME);

	Proto30400052 enter_info;
	enter_info.set_space_id(0);
	enter_info.set_scene_mode(SCENE_MODE_TRVL_PEAK);

	MoverCoord enter_point = this->peak_pre_scene_->fetch_enter_pos();
	enter_info.set_pos_x(enter_point.pixel_x());
	enter_info.set_pos_y(enter_point.pixel_y());

	return MAP_MONITOR->respond_enter_scene_begin(sid, request, &enter_info);
}

int TrvlPeakMonitor::sync_travel_team_info(int sid, Int64 role_id, Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto30402001 *, request, msg, -1);

	Int64 team_id = request->team_id();

	int op_type = request->op_type();
	if (op_type == 1)
	{
		TravPeakTeam *travel_team = this->find_travel_team(team_id);
		if (travel_team == NULL)
		{
			travel_team = this->trav_peak_team_pool_->pop();

			travel_team->__team_id = team_id;
			this->trav_peak_team_map_->rebind(team_id, travel_team);
		}

		travel_team->BaseServerInfo::unserialize(request->server_info());

		travel_team->__team_name = request->team_name();
		travel_team->__leader_id = request->leader_id();

		// 预加载机器人
		MapPlayerEx *offline_player = NULL;
		LongMap src_role_to_offline_map;
		for (TravPeakTeam::TeamerMap::iterator teamer_iter = travel_team->__teamer_map.begin();
		        teamer_iter != travel_team->__teamer_map.end(); ++teamer_iter)
		{
		    TravPeakTeam::TravPeakTeamer &teamer_info = teamer_iter->second;
		    this->trav_role_team_map_->unbind(teamer_info.__teamer_id);

		    if (teamer_info.__offline_teamer_id > 0)
		        src_role_to_offline_map[teamer_info.__teamer_id] = teamer_info.__offline_teamer_id;
		}

		travel_team->__teamer_map.clear();
		for (int i = 0; i < request->teamer_list_size(); ++i)
		{
			const ProtoTeamer &proto_teamer = request->teamer_list(i);

			TravPeakTeam::TravPeakTeamer &teamer_info = travel_team->__teamer_map[proto_teamer.role_id()];
			teamer_info.unserialize(proto_teamer);

			this->trav_role_team_map_->rebind(proto_teamer.role_id(), travel_team->__team_id);

			LongMap::iterator map_iter = src_role_to_offline_map.find(teamer_info.__teamer_id);
			if (map_iter != src_role_to_offline_map.end())
			{
				teamer_info.__offline_teamer_id = map_iter->second;
			    if (MAP_MONITOR->find_player_with_offline(teamer_info.__offline_teamer_id, offline_player) == 0)
			    {
			     	teamer_info.__offline_player = offline_player;
			        src_role_to_offline_map.erase(map_iter);
			    }
			    else
			    {
			    	teamer_info.__offline_teamer_id = 0;
			    }
			}
		}

		for (LongMap::iterator iter = src_role_to_offline_map.begin();
				iter != src_role_to_offline_map.end(); ++iter)
		{
			if (MAP_MONITOR->find_player_with_offline(iter->second, offline_player) == 0)
		    {
		    	offline_player->offline_exit_scene();
		        offline_player->offline_sign_out();
		    }
		}

		this->save_remote_travel_team(travel_team);
	}
	else if (op_type == 2)
	{
		MMOTravTeam::remove_remote_travel_team(team_id);

		TravPeakTeam *travel_team = this->find_travel_team(team_id);
		this->trav_peak_team_map_->unbind(team_id);
		JUDGE_RETURN(travel_team != NULL, 0);

		this->quality_info_.__signup_team_set.erase(team_id);
		this->is_update_quality_state_ = true;

		for (TravPeakTeam::TeamerMap::iterator teamer_iter =travel_team->__teamer_map.begin();
				teamer_iter != travel_team->__teamer_map.end(); ++teamer_iter)
		{
			TravPeakTeam::TravPeakTeamer &teamer_info = teamer_iter->second;
			this->trav_role_team_map_->unbind(teamer_info.__teamer_id);

			if (teamer_info.__offline_player != NULL)
			{
				teamer_info.__offline_player->offline_exit_scene();
				teamer_info.__offline_player->offline_sign_out();
			}
		}
		travel_team->__teamer_map.clear();

		this->trav_peak_team_pool_->push(travel_team);
	}

	return 0;
}

int TrvlPeakMonitor::process_signup_travel_team(int sid, Int64 role_id, Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto30402006 *, request, msg, -1);

	Int64 team_id = request->team_id();
	TravPeakTeam *travel_team = this->find_travel_team(team_id);
	JUDGE_RETURN(travel_team != NULL, 0);

	this->quality_info_.__signup_team_set.insert(team_id);
	this->is_update_quality_state_ = true;

	Proto30101905 inner_req;
	inner_req.set_team_id(team_id);
	MAP_MONITOR->dispatch_to_logic(&inner_req, sid, role_id);

	return 0;
}

int TrvlPeakMonitor::process_fetch_travel_peak_tick(int sid, Int64 role_id, Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto30402009 *, request, msg, -1);

	Proto30402010 inner_req;
	inner_req.set_signup_start(this->quality_info_.__signup_start_tick.sec());
	inner_req.set_signup_end(this->quality_info_.__signup_end_tick.sec());
	inner_req.set_quality_start(this->quality_info_.__quality_start_tick.sec());
	inner_req.set_quality_end(this->quality_info_.__quality_end_tick.sec());
	inner_req.set_knockout_start(this->knockout_info_.__knockout_start_tick.sec());
	inner_req.set_knockout_end(this->knockout_info_.__knockout_end_tick.sec());
	return MAP_MONITOR->dispatch_to_logic(sid, &inner_req);
}

int TrvlPeakMonitor::process_fetch_trvl_team_detail(int sid, Int64 role_id, Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto30402005 *, request, msg, -1);

	Int64 team_id = request->team_id();
	TravPeakTeam *travel_team = this->find_travel_team(team_id);
	JUDGE_RETURN(travel_team != NULL, 0);

	Proto50101510 respond;
	respond.set_team_id(travel_team->__team_id);
	respond.set_team_name(travel_team->__team_name);

	TravPeakTeam::TravPeakTeamer *leader_info = travel_team->find_teamer(travel_team->__leader_id);
	if (leader_info != NULL)
	{
	    leader_info->serialize(respond.add_teamer_list());
	}

	for (TravPeakTeam::TeamerMap::iterator teamer_iter = travel_team->__teamer_map.begin();
	        teamer_iter != travel_team->__teamer_map.end(); ++teamer_iter)
	{
	    TravPeakTeam::TravPeakTeamer &teamer_info = teamer_iter->second;
	    JUDGE_CONTINUE(teamer_info.__teamer_id != travel_team->__leader_id);

	    teamer_info.serialize(respond.add_teamer_list());
	}

	return MAP_MONITOR->dispatch_to_client_from_gate(sid, role_id, &respond);
}

int TrvlPeakMonitor::sync_update_trvl_teamer_force_info(int sid, Int64 role_id, Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto30402014 *, request, msg, -1);

	for (int i = 0; i < request->force_info_size(); ++i)
	{
		const ProtoTeamForceInfo &force_info = request->force_info(i);

		TravPeakTeam *travel_team = this->find_travel_team(force_info.team_id());
		JUDGE_CONTINUE(travel_team != NULL);

		TravPeakTeam::TravPeakTeamer *trvl_teamer = travel_team->find_teamer(force_info.role_id());
		JUDGE_CONTINUE(trvl_teamer != NULL);

		trvl_teamer->__teamer_force = force_info.force();
		trvl_teamer->__teamer_level = force_info.level();
	}

	return 0;
}

int TrvlPeakMonitor::sync_offline_player_info(int sid, Int64 role_id, Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto30402002 *, req, msg, -1);

	Int64 src_role_id = req->src_role_id();
	TravPeakTeam *travel_team = this->find_travel_team_by_role_id(src_role_id);
	JUDGE_RETURN(travel_team != NULL, 0);

	TravPeakTeam::TeamerMap::iterator teamer_iter = travel_team->__teamer_map.find(src_role_id);
	JUDGE_RETURN(teamer_iter != travel_team->__teamer_map.end(), 0);

	TravPeakTeam::TravPeakTeamer &teamer_info = teamer_iter->second;

	Proto30400432 request;
	request.ParseFromString(req->msg_body());

	MapPlayerEx *offline_player = NULL;
	if (teamer_info.__offline_player == NULL)
	{
		Int64 machine_role_id = MAP_MONITOR->generate_role_copy_id();
		offline_player = MAP_MONITOR->player_pool()->pop();
		if (MAP_MONITOR->bind_player(machine_role_id, offline_player) != 0)
		{
			MAP_MONITOR->player_pool()->push(offline_player);
			return -1;
		}

		teamer_info.__offline_teamer_id = machine_role_id;
		teamer_info.__offline_player = offline_player;
		offline_player->set_offline_player_info(&request, src_role_id, machine_role_id);
		offline_player->MapOfflineHook::sign_in();
	}
	else
	{
		offline_player = teamer_info.__offline_player;
		offline_player->set_offline_player_info(&request, src_role_id);
	}

	if (offline_player != NULL)
	{
	    MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	    MMORole::update_copy_player(offline_player, data_map, src_role_id);
	    TRANSACTION_MONITOR->request_mongo_transaction(src_role_id, TRANS_SAVE_COPY_TRAV_TEAMER, data_map);
	}

	return 0;
}

int TrvlPeakMonitor::process_fetch_trvl_peak_rank(int sid, Int64 role_id, Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto30402015 *, req, msg, -1);

	ThreeObjVec &team_score_vec = this->quality_info_.__team_score_vec;

	PageInfo page_info;
	GameCommon::game_page_info(page_info, req->page(), team_score_vec.size(), RANK_PAGE_SIZE);

	Proto50401705 respond;
	respond.set_cur_page(page_info.cur_page_);
	respond.set_total_page(page_info.total_page_);

	TravPeakTeam *trvl_team = this->find_travel_team_by_role_id(role_id);
	if (trvl_team != NULL)
	{
		ProtoQualityRank *my_rank = respond.mutable_my_team();
		trvl_team->serialize_rank(my_rank);
	}

	for (int i = page_info.start_index_; i < ::std::min(page_info.total_count_,
			page_info.start_index_ + RANK_PAGE_SIZE); ++i)
	{
		ThreeObj &obj = team_score_vec[i];
		TravPeakTeam *trvl_team = this->find_travel_team(obj.id_);
		JUDGE_CONTINUE(trvl_team != NULL);

		ProtoQualityRank *team_list = respond.add_team_list();
		trvl_team->serialize_rank(team_list);
	}

	return MAP_MONITOR->dispatch_to_client_from_gate(sid, role_id, &respond);
}

int TrvlPeakMonitor::register_travel_peak_team(Int64 team_id)
{
	TravPeakTeam *trvl_team = this->find_travel_team(team_id);
	JUDGE_RETURN(trvl_team != NULL, 0);

	this->sign_map_[team_id] = ::time(NULL);
	return 0;
}

int TrvlPeakMonitor::unregister_travel_peak_team(Int64 team_id)
{
	this->sign_map_.erase(team_id);
	return 0;
}

void TrvlPeakMonitor::test_trvl_peak(int type, int last)
{
	JUDGE_RETURN(this->is_activity_time() == false, ;);
	JUDGE_RETURN(type > 0 && last > 0, ;);

	Int64 cur_tick = ::time(NULL);

	if (type == TIMER_SIGNUP)
	{
		this->quality_info_.__signup_start_tick.sec(cur_tick);
		this->quality_info_.__signup_end_tick.sec(cur_tick + last);

		this->signup_time_.cur_state_ = 2;
		this->signup_time_.refresh_time_ = last;
		this->handle_peak_signup_i(0);
	}
	else if (type == TIMER_QUALITY)
	{
		this->is_test_ = 1;
		this->quality_info_.__quality_start_tick.sec(cur_tick + last);
		this->quality_info_.__quality_end_tick.sec(cur_tick + last);

		this->quality_time_.cur_state_ = 2;
		this->quality_time_.refresh_time_ = last;
		this->handle_peak_quality_i(0);
	}
	else if (type == TIMER_KNOCKOUT)
	{

	}

	this->send_act_time_to_logic();
}

void TrvlPeakMonitor::init_conf_info()
{
	const Json::Value& conf = this->scene_conf();
	this->prep_waiting_time_ = conf["prep_waiting_time"].asInt();
	this->wait_tick_ = conf["wait_tick"].asInt();
	this->fighting_time_ = conf["fighting_time"].asInt();
	this->quit_time_ = conf["quit_time"].asInt();
	this->prep_time_ = conf["prep_time"].asInt();
	this->wait_time_ = conf["wait_time"].asInt();
	this->exp_time_  = conf["exp_time"].asInt();
	this->transfer_out_time_ = conf["transfer_time"].asInt();

	const Json::Value& enter_pos = conf["enter_pos"];
	this->enter_pos_[0].set_pixel(enter_pos[0u][0u].asInt(), enter_pos[0u][1u].asInt());
	this->enter_pos_[1].set_pixel(enter_pos[1u][0u].asInt(), enter_pos[1u][1u].asInt());
}

void TrvlPeakMonitor::reset_day_info()
{
	this->cal_activity_time();
	this->send_act_time_to_logic();

	this->day_timer_.cancel_timer();
	this->day_timer_.schedule_timer(GameCommon::next_day());
}

void TrvlPeakMonitor::cal_activity_time()
{
	if (this->is_test_ == 0)
	{
		const Json::Value& conf = this->scene_conf();
		int signup_start = conf["signup_start"].asInt();
		int signup_end = conf["signup_end"].asInt();
		int quality_start = conf["quality_start"].asInt();
		int quality_end = conf["quality_end"].asInt();
		int knockout_start = conf["knockout_start"].asInt();
		int knockout_end = conf["knockout_end"].asInt();

		Time_Value nowtime = Time_Value::gettimeofday();
		this->quality_info_.__signup_start_tick = current_week(signup_start / 10000,
				(signup_start % 10000 / 100), signup_start % 100, nowtime);

		this->quality_info_.__signup_end_tick = current_week(signup_end / 10000,
				(signup_end % 10000 / 100), signup_end % 100, nowtime);

		this->quality_info_.__quality_start_tick = current_week(quality_start / 10000,
				(quality_start % 10000 / 100), quality_start % 100, nowtime);

		this->quality_info_.__quality_end_tick = current_week(quality_end / 10000,
				(quality_end % 10000 / 100), quality_end % 100, nowtime);

		this->knockout_info_.__knockout_start_tick = current_week(knockout_start / 10000,
				(knockout_start % 10000 / 100), knockout_start % 100, nowtime);

		this->knockout_info_.__knockout_end_tick = current_week(knockout_end / 10000,
				(knockout_end % 10000 / 100), knockout_end % 100, nowtime);
	}
}

int TrvlPeakMonitor::peak_scene_id()
{
	return GameEnum::TRVL_PEAK_SCENE_ID;
}

int TrvlPeakMonitor::peak_pre_scene_id()
{
	return GameEnum::TRVL_PEAK_PRE_SCENE_ID;
}

int TrvlPeakMonitor::fetch_arrange_wait()
{
	if (this->sign_map_.size() > 10)
	{
		return this->wait_time_ - 10;
	}
	else
	{
		return this->wait_time_;
	}
}

const Json::Value& TrvlPeakMonitor::scene_conf()
{
	return CONFIG_INSTANCE->scene(this->peak_scene_id());
}

int TrvlPeakMonitor::fetch_conti_or_break_win_score(int continue_win, const Json::Value& conf)
{
	int score = 0;
	for (uint i = 0; i < conf.size(); ++i)
	{
		JUDGE_BREAK(continue_win >= conf[i][0u].asInt());
		score = conf[i][1u].asInt();
	}
	return score;
}

int TrvlPeakMonitor::fetch_win_or_lost_reward(int times, int result, const Json::Value& conf)
{
	int reward_id = 0;
	for (uint i = 0; i < conf.size(); ++i)
	{
		JUDGE_BREAK(times >= conf[i][0u].asInt());
		if (result == 1)
		{
			reward_id = conf[i][1u].asInt();
		}
		else
		{
			reward_id = conf[i][2u].asInt();
		}
	}
	return reward_id;
}

void TrvlPeakMonitor::arrange_peak(FourObj& obj)
{
	int space_id = this->arena_id_++;

	TrvlPeakScene* scene = this->scene_package_->pop_object();
	int ret = scene->init_peak_scene(space_id, obj);
	JUDGE_RETURN(ret == 0, ;);

	this->scene_package_->bind_object(space_id, scene);

	FourObj& first = this->arrange_vec_[obj.first_value_];
	this->notify_match_knock_fight_team(first.id_);

	TravPeakTeam* first_team = this->find_travel_team(first.id_);
	first_team->__space_id 	 = space_id;
	first_team->__camp_id    = 0;
	first_team->__last_rival = 0;
	first_team->__start_tick = ::time(NULL);

	this->fight_map_[first.id_] = true;

	FourObj& second = this->arrange_vec_[obj.second_value_];
	this->notify_match_knock_fight_team(second.id_);

	TravPeakTeam* second_team = this->find_travel_team(second.id_);
	second_team->__space_id   = space_id;
	second_team->__camp_id 	  = 1;
	second_team->__last_rival = first.id_;
	second_team->__start_tick = ::time(NULL);

	this->fight_map_[second.id_] = true;
}

IntPair TrvlPeakMonitor::find_rival(int start_index)
{
	IntPair pair;

	TravPeakTeam* first = this->find_travel_team(this->arrange_vec_[start_index].id_);
	JUDGE_RETURN(first != NULL, pair);

	int total_size = this->arrange_vec_.size();
	for (int i = start_index  + 1; i < total_size; ++i)
	{
		TravPeakTeam* second = this->find_travel_team(this->arrange_vec_[i].id_);
		JUDGE_CONTINUE(second != NULL);
		JUDGE_CONTINUE(first->__last_rival != second->__team_id);

		pair.first = 1;
		pair.second = i;

		return pair;
	}

	return pair;
}

int TrvlPeakMonitor::notify_match_knock_fight_team(Int64 team_id)
{
	TravPeakTeam *team = this->find_travel_team(team_id);
	JUDGE_RETURN(team != NULL, 0);

	for (TravPeakTeam::TeamerMap::iterator teamer_iter = team->__teamer_map.begin();
	        teamer_iter != team->__teamer_map.end(); ++teamer_iter)
	{
		this->notify_recogn(teamer_iter->first, ACTIVE_TRAVPEAK_MATCH_KNOCK_FIGHT);
	}
	return 0;
}

void TrvlPeakMonitor::load_trvl_peak()
{
	MMOTravTeam::load_remote_travel_team(this);
	this->sort_quality_score_rank();
}

void TrvlPeakMonitor::save_trvl_peak()
{
	for (TravPeakTeamMap::iterator iter = this->trav_peak_team_map_->begin();
			iter != this->trav_peak_team_map_->end(); ++iter)
	{
		TravPeakTeam *travel_team = iter->second;
		JUDGE_CONTINUE(travel_team != NULL);

		this->save_remote_travel_team(travel_team);
	}
}

void TrvlPeakMonitor::save_remote_travel_team(TravPeakTeam *travel_team)
{
	MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	MMOTravTeam::update_remote_travel_team_data(travel_team, data_map);
	TRANSACTION_MONITOR->request_mongo_transaction(travel_team->__team_id, TRANS_SAVE_REMOTE_TRAV_TEAM, data_map);
}

void TrvlPeakMonitor::load_peak_act_info()
{
	MMOTravTeam::load_travel_peak_quality_info(this);

	this->start_send_signup_state();
}

void TrvlPeakMonitor::save_peak_act_info()
{
	if (this->is_update_quality_state_ == true)
	{
		MMOTravTeam::update_travel_peak_quality_info(this);
	    this->is_update_quality_state_ = false;
	}
	if (this->is_update_knockout_state_ == true)
	{
//	    MMOTravelPeak::update_travel_peak_promot_info(this);
	    this->is_update_knockout_state_ = false;
	}
}

void TrvlPeakMonitor::start_send_signup_state()
{
	for (TravPeakTeamMap::iterator iter = this->trav_peak_team_map_->begin();
			iter != this->trav_peak_team_map_->end(); ++iter)
	{
		Int64 team_id = iter->first;
		TravPeakTeam *travel_team = iter->second;
		JUDGE_CONTINUE(travel_team != NULL);

		Proto30101909 inner;
		inner.set_team_id(team_id);
		if (this->quality_info_.__signup_team_set.count(team_id) > 0)
		{
			inner.set_is_signup(1);
		}
		else
		{
			inner.set_is_signup(0);
		}

		int sid = MAP_MONITOR->fetch_gate_sid(travel_team->fetch_flag_by_trvl());
		JUDGE_CONTINUE(sid >= 0);

		MAP_MONITOR->dispatch_to_logic(sid, &inner);
	}
}

void TrvlPeakMonitor::send_act_time_to_logic()
{
	Proto30402010 inner_req;
	inner_req.set_signup_start(this->quality_info_.__signup_start_tick.sec());
	inner_req.set_signup_end(this->quality_info_.__signup_end_tick.sec());
	inner_req.set_quality_start(this->quality_info_.__quality_start_tick.sec());
	inner_req.set_quality_end(this->quality_info_.__quality_end_tick.sec());
	inner_req.set_knockout_start(this->knockout_info_.__knockout_start_tick.sec());
	inner_req.set_knockout_end(this->knockout_info_.__knockout_end_tick.sec());
	MAP_MONITOR->dispatch_to_logic_in_all_server(&inner_req);
}

void TrvlPeakMonitor::active_add_player_exp()
{
	JUDGE_RETURN(this->is_activity_time() == true, ;);
	JUDGE_RETURN(this->add_exp_time_ <= ::time(NULL), ;);

	this->add_exp_time_ =::time(NULL) + this->exp_time_;

	TrvlPeakPreScene *pre_scene = this->fetch_trvl_peak_pre_scene();
	JUDGE_RETURN(pre_scene != NULL, ;);

	for (MoverMap::iterator iter = pre_scene->player_map().begin();
			iter != pre_scene->player_map().end(); ++iter)
	{
		MapPlayerEx* player = pre_scene->find_player(iter->first);
		JUDGE_CONTINUE(player != NULL);

		int level = player->level();
		int add_exp = CONFIG_INSTANCE->role_level(0, level)["blessing_exp"].asInt();
		player->modify_element_experience(add_exp, SerialObj(EXP_FROM_TRVL_PEAK));

		this->player_exp_map_[iter->first] += add_exp;
		player->request_trvl_peak_scene_info();
	}

	boost::unordered_map<int, int> index_map = this->scene_package_->fetch_index_map();
	for (boost::unordered_map<int, int>::iterator iter = index_map.begin();
			iter != index_map.end(); ++iter)
	{
		TrvlPeakScene *scene = this->find_scene(iter->first);
		JUDGE_CONTINUE(scene != NULL);

		for (MoverMap::iterator iter = scene->player_map().begin();
				iter != scene->player_map().end(); ++iter)
		{
			MapPlayerEx* player = scene->find_player(iter->first);
			JUDGE_CONTINUE(player != NULL);

			int level = player->level();
			int add_exp = CONFIG_INSTANCE->role_level(0, level)["blessing_exp"].asInt();
			player->modify_element_experience(add_exp, SerialObj(EXP_FROM_TRVL_PEAK));

			this->player_exp_map_[iter->first] += add_exp;
		}
	}
}

void TrvlPeakMonitor::cal_quality_rank_info()
{
	JUDGE_RETURN(this->quality_info_.__signup_team_set.size() > 0, ;);

	if (this->quality_info_.__signup_team_set.size() == 1)
	{
		// 只有一只战队报名
	}
	else
	{

	}
}

void TrvlPeakMonitor::sort_quality_score_rank()
{
	JUDGE_RETURN(this->is_update_quality_rank_ == true, ;);
	this->is_update_quality_rank_ = false;

	ThreeObjVec &team_score_vec = this->quality_info_.__team_score_vec;
	team_score_vec.clear();

	for (TravPeakTeamMap::iterator iter = this->trav_peak_team_map_->begin();
			iter != this->trav_peak_team_map_->end(); ++iter)
	{
		TravPeakTeam *travel_team = iter->second;
		JUDGE_CONTINUE(travel_team != NULL);

		ThreeObj obj;
		obj.id_ = travel_team->__team_id;
		obj.value_ = travel_team->__score;
		obj.tick_ = travel_team->__update_tick;
		team_score_vec.push_back(obj);
	}

	std::sort(team_score_vec.begin(), team_score_vec.end(), GameCommon::three_comp_by_desc);

	int rank = 1;
	for (ThreeObjVec::iterator iter = team_score_vec.begin();
			iter != team_score_vec.end(); ++iter)
	{
		TravPeakTeam *travel_team = this->find_travel_team(iter->id_);
		JUDGE_CONTINUE(travel_team != NULL);

		travel_team->__rank = rank;
		++rank;
	}
}

