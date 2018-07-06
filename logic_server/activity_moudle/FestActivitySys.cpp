/*
 * FestActivitySys.cpp
 *
 *  Created on: Jan 10, 2017
 *      Author: peizhibi
 */

#include "FestActivitySys.h"
#include "MMOOpenActivity.h"
#include "LogicMonitor.h"
#include "ProtoDefine.h"
#include "LogicPlayer.h"

int FestActivitySys::LoginActTimer::type()
{
	return GTT_LOGIC_ONE_SEC;
}

int FestActivitySys::LoginActTimer::handle_timeout(const Time_Value &tv)
{
	return FEST_ACTIVITY_SYS->handle_login_timer();
}

int FestActivitySys::BossActTimer::type()
{
	return GTT_LOGIC_ONE_SEC;
}

int FestActivitySys::BossActTimer::handle_timeout(const Time_Value &tv)
{
	return FEST_ACTIVITY_SYS->handle_boss_timer();
}

int FestActivitySys::MapSyncTimer::type()
{
	return GTT_LOGIC_ONE_MINUTE;
}

int FestActivitySys::MapSyncTimer::handle_timeout(const Time_Value &tv)
{
	return FEST_ACTIVITY_SYS->handle_map_sync_timer();
}

FestActivitySys::FestActivitySys()
{
	// TODO Auto-generated constructor stub
	this->boss_index_ = 0;
	this->last_boss_index_ = 0;
}

FestActivitySys::~FestActivitySys()
{
	// TODO Auto-generated destructor stub
}

int FestActivitySys::after_load_activity(DBShopMode* shop_mode)
{
	return 0;
}

BackSetActDetail::ActTypeItem* FestActivitySys::find_fest_activity(int type)
{
	return this->find_item_by_day(type, this->fest_info_.icon_type_);
}

int FestActivitySys::start(void)
{
	this->login_time_.set_freq_type(GameEnum::DAILY_ACTIVITY);
	this->boss_time_.set_freq_type(GameEnum::DAILY_ACTIVITY);

	this->init_festival_activity();
	MMOOpenActivity::load_fetst_act_time();

	MSG_USER("festival %d, %d, %d, %ld, %ld", this->fest_info_.icon_type_,
			this->fest_info_.act_state_, this->act_list_.size(),
			this->fest_info_.start_tick_, this->fest_info_.end_tick_);

	return 0;
}

int FestActivitySys::stop(void)
{
	this->boss_timer.cancel_timer();
	this->login_timer_.cancel_timer();
	this->map_sync_timer_.cancel_timer();

	return 0;
}

int FestActivitySys::midnight_handle_timeout(int test_day)
{
	this->finish_cur_day_act();
	this->update_activity_sys();
	return 0;
}

int FestActivitySys::handle_login_timer()
{
	int last_state = this->login_time_.cur_state_;

	this->login_timer_.cancel_timer();
	this->login_time_.set_next_stage();
	this->login_timer_.schedule_timer(this->login_time_.refresh_time_);

	switch (last_state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
		LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
		for(LogicMonitor::PlayerMap::iterator iter = player_map.begin();
				iter != player_map.end(); ++iter)
		{
			LogicPlayer* player = iter->second;
			JUDGE_CONTINUE(player != NULL);
			player->update_fest_activity_login();
		}
		break;
	}
	case GameEnum::ACTIVITY_STATE_START:
	{
		break;
	}
	}

	return 0;
}

int FestActivitySys::handle_boss_timer()
{
	int last_state = this->boss_time_.cur_state_;

	this->boss_timer.cancel_timer();
	this->boss_time_.set_next_stage();
	this->boss_timer.schedule_timer(this->boss_time_.refresh_time_);

	switch (last_state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
		this->start_boss();
		this->shout_boss();
		break;
	}
	case GameEnum::ACTIVITY_STATE_START:
	{
		this->recycle_boss();
		break;
	}
	}

	return 0;
}

int FestActivitySys::handle_map_sync_timer()
{
	Proto30400041 inner;
	inner.set_icon_type(this->fest_info_.icon_type_);
	inner.set_drop_act(this->fest_info_.act_state_);
	inner.set_start_tick(this->fest_info_.start_tick_);
	inner.set_end_tick(this->fest_info_.end_tick_);
	return LOGIC_MONITOR->dispatch_to_all_map(&inner);
}

int FestActivitySys::icon_type()
{
	return this->fest_info_.icon_type_;
}

int FestActivitySys::left_activity_time()
{
	return GameCommon::left_time(this->fest_info_.end_tick_);
}

int FestActivitySys::is_activity_time()
{
	return this->fest_info_.act_state_;
}

int FestActivitySys::is_login_timer_state()
{
	return this->login_timer_.left_second() > 0;
}

IntMap& FestActivitySys::fetch_act_list()
{
	return this->act_list_;
}

int FestActivitySys::request_festival_time_begin()
{
	DBShopMode* shop_mode = GameCommon::pop_shop_mode();
	JUDGE_RETURN(shop_mode != NULL, -1);

	shop_mode->recogn_ = TRANS_LOAD_FESTIVAL_TIME;
	shop_mode->input_argv_.type_int64_ = this->fest_info_.update_tick_;

	return LOGIC_MONITOR->db_load_mode_begin(shop_mode);
}

int FestActivitySys::request_festival_time_done(DBShopMode* shop_mode)
{
#ifdef LOCAL_DEBUG
	JUDGE_RETURN(this->fest_info_.icon_type_ == 0, -1);

	this->fest_info_.icon_type_ = 3;
	this->fest_info_.start_tick_ = ::time(NULL);
	this->fest_info_.end_tick_ = ::time(NULL) + Time_Value::DAY + ::rand() % Time_Value::DAY;
#else
	JUDGE_RETURN(shop_mode->sub_value_ == 1, -1);

	this->fest_info_.icon_type_ = shop_mode->output_argv_.type_int_;
	this->fest_info_.start_tick_ = shop_mode->output_argv_.long_vec_[0];
	this->fest_info_.end_tick_ = shop_mode->output_argv_.long_vec_[1];
	this->fest_info_.update_tick_ = shop_mode->output_argv_.long_vec_[2];
#endif

	this->update_activity_sys();
	return 0;
}

int FestActivitySys::handle_scene_boss_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100604*, request, -1);

	if (request->type() == 0)
	{
		return this->record_player_hurt_map(msg);
	}
	else
	{
		return this->handle_boss_kill(msg);
	}
}

int FestActivitySys::record_player_hurt_map(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100604*, request, -1);
	JUDGE_RETURN(this->is_activity_time() == true, -1);

	for (int i = 0; i < request->player_set_size(); ++i)
	{
		Int64 role_id = request->player_set(i);

		LogicPlayer* player = NULL;
		JUDGE_CONTINUE(LOGIC_MONITOR->find_player(role_id, player) == 0);

		player->update_fest_boss_hurt(this->last_boss_index_);
	}

	return 0;
}

int FestActivitySys::handle_boss_kill(Message* msg)
{
	BackSetActDetail::ActTypeItem* s_act_t_item = FEST_ACTIVITY_SYS->find_fest_activity(
			BackSetActDetail::F_ACT_FEST_BOSS);
	JUDGE_RETURN(s_act_t_item != NULL, -1);

	BackSetActDetail::ActItem& act_item = s_act_t_item->act_item_set_[this->last_boss_index_];
	act_item.sub_value_ = true;

	s_act_t_item->force_red_ = false;
	LogicActivityer::notify_all_player_red_point(s_act_t_item, false);

	return 0;
}

void FestActivitySys::init_festival_activity()
{
	GameConfig::ConfigMap& act_map = CONFIG_INSTANCE->festival_activity_map();
	GameConfig::ConfigMap::iterator iter = act_map.begin();

	for (; iter != act_map.end(); ++iter)
	{
		this->add_new_item(iter->first, *(iter->second));
	}
}

void FestActivitySys::init_cur_day_act()
{
	for (BackSetActDetail::ActTypeItemSet::iterator
			iter = this->act_type_item_set_.begin();
			iter != this->act_type_item_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->open_time_.count(this->fest_info_.icon_type_) > 0);
		this->act_list_[iter->act_index_] = true;
	}
}

void FestActivitySys::start_login_timer()
{
	this->login_time_.reset();
	this->login_timer_.cancel_timer();

	BackSetActDetail::ActTypeItem* s_act_t_item = this->find_fest_activity(
			BackSetActDetail::F_ACT_FEST_LOGIN);
	JUDGE_RETURN(s_act_t_item != NULL, ;);

	int total_size = s_act_t_item->act_item_set_.size();
	for (int i = 0; i < total_size; ++i)
	{
		BackSetActDetail::ActItem& act_item = s_act_t_item->act_item_set_[i];
		JUDGE_CONTINUE(act_item.cond_value_.empty() == false);

		int start = GameCommon::fetch_day_sec(act_item.cond_value_[0]);
		int end = GameCommon::fetch_day_sec(act_item.cond_value_[1]);

		this->login_time_.time_set_.push_back(start);
		this->login_time_.time_set_.push_back(end);
	}

	GameCommon::cal_two_activity_time(this->login_time_);

	JUDGE_RETURN(this->login_time_.cur_state_ == GameEnum::ACTIVITY_STATE_NO_START, ;);
	this->login_timer_.schedule_timer(this->login_time_.refresh_time_);
}

void FestActivitySys::start_boss_timer()
{
	this->boss_index_ = 0;
	this->last_boss_index_ = 0;

	this->boss_time_.reset();
	this->boss_timer.cancel_timer();

	BackSetActDetail::ActTypeItem* s_act_t_item = this->find_fest_activity(
			BackSetActDetail::F_ACT_FEST_BOSS);
	JUDGE_RETURN(s_act_t_item != NULL, ;);

	int total_size = s_act_t_item->act_item_set_.size();
	for (int i = 0; i < total_size; ++i)
	{
		BackSetActDetail::ActItem& act_item = s_act_t_item->act_item_set_[i];
		JUDGE_CONTINUE(act_item.cond_value_.empty() == false);

		int start = GameCommon::fetch_day_sec(act_item.cond_value_[0]);
		int end = GameCommon::fetch_day_sec(act_item.cond_value_[1]);

		this->boss_time_.time_set_.push_back(start);
		this->boss_time_.time_set_.push_back(end);
	}

	GameCommon::cal_two_activity_time(this->boss_time_);

	JUDGE_RETURN(this->boss_time_.cur_state_ == GameEnum::ACTIVITY_STATE_NO_START, ;);
	this->boss_timer.schedule_timer(this->boss_time_.refresh_time_);
}

void FestActivitySys::start_sync_timer()
{
	this->map_sync_timer_.cancel_timer();
	this->map_sync_timer_.schedule_timer(1);
	this->handle_map_sync_timer();
}

void FestActivitySys::start_boss()
{
	BackSetActDetail::ActTypeItem* s_act_t_item = this->find_fest_activity(
			BackSetActDetail::F_ACT_FEST_BOSS);

	MSG_USER("festival %d, %d", s_act_t_item != NULL, this->boss_time_.time_index_);
	JUDGE_RETURN(s_act_t_item != NULL, ;);

#ifndef LOCAL_DEBUG
	this->boss_index_ = this->boss_time_.time_index_;
#endif

	this->last_boss_index_ = this->boss_index_ % s_act_t_item->act_item_set_.size();
	this->boss_index_ = (this->boss_index_ + 1) % s_act_t_item->act_item_set_.size();
	BackSetActDetail::ActItem& act_item = s_act_t_item->act_item_set_[this->last_boss_index_];

	Proto30400040 inner;
	inner.set_scene_id(act_item.cond_value_[3]);
	inner.set_boss_id(act_item.cond_value_[6]);

	ProtoCoord* coord = inner.mutable_coord();
	coord->set_pixel_x(act_item.cond_value_[4]);
	coord->set_pixel_y(act_item.cond_value_[5]);
	LOGIC_MONITOR->dispatch_to_scene(act_item.cond_value_[3], &inner);

	s_act_t_item->force_red_ = true;
	LogicActivityer::notify_all_player_red_point(s_act_t_item, true);
}

void FestActivitySys::shout_boss()
{
	int shout_id = CONFIG_INSTANCE->const_set("shout_fest_boss");
	JUDGE_RETURN(shout_id > 0, ;);

	GameCommon::announce(shout_id);
}

void FestActivitySys::recycle_boss()
{
	BackSetActDetail::ActTypeItem* s_act_t_item = this->find_fest_activity(
			BackSetActDetail::F_ACT_FEST_BOSS);
	JUDGE_RETURN(s_act_t_item != NULL, ;);

	s_act_t_item->force_red_ = false;
	LogicActivityer::notify_all_player_red_point(s_act_t_item, false);
}

void FestActivitySys::finish_cur_day_act()
{
	this->boss_timer.cancel_timer();
	this->login_timer_.cancel_timer();

	for (IntMap::iterator iter = this->act_list_.begin();
			iter != this->act_list_.end(); ++iter)
	{
		BackSetActDetail::ActTypeItem* s_act_t_item = this->find_item(iter->first);
		JUDGE_CONTINUE(s_act_t_item != NULL);
		s_act_t_item->reset_everyday();
	}
}

void FestActivitySys::update_activity_sys()
{
	this->act_list_.clear();

	Int64 now_tick = ::time(NULL);
	if (now_tick >= this->fest_info_.start_tick_ && now_tick < this->fest_info_.end_tick_)
	{
		//开启
		this->fest_info_.act_state_ = true;
		this->init_cur_day_act();
		this->start_login_timer();
		this->start_boss_timer();
		this->start_sync_timer();
		this->notify_activity_state();
	}
	else if (this->fest_info_.act_state_ == true)
	{
		//关闭
		this->boss_timer.cancel_timer();
		this->login_timer_.cancel_timer();
		this->map_sync_timer_.cancel_timer();

		this->fest_info_.act_state_ = false;
		this->notify_activity_state();
		this->handle_map_sync_timer();
	}

	MSG_USER("festival state %d, %d", this->fest_info_.icon_type_, this->fest_info_.act_state_);
}

void FestActivitySys::notify_activity_state(LogicPlayer* player)
{
	JUDGE_RETURN(this->fest_info_.icon_type_  > 0, ;);

	Proto80100111 respond;
	respond.set_type(this->fest_info_.icon_type_);
	respond.set_state(this->fest_info_.act_state_);

	if (player != NULL)
	{
		player->respond_to_client(ACTIVE_FESTIVAL_ACTIVITY_ICON, &respond);
	}
	else
	{
		LOGIC_MONITOR->notify_all_player(ACTIVE_FESTIVAL_ACTIVITY_ICON, &respond);
	}
}

