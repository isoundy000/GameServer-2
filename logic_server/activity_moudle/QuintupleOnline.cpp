/*
 * QuintupleOnline.cpp
 *
 *  Created on: 2016年9月8日
 *      Author: lzy
 */

#include "QuintupleOnline.h"
#include "ProtoDefine.h"
#include "LogicMonitor.h"
#include "LogicPlayer.h"

int QuintupleOnline::CheckStartTimer::type(void)
{
	return GTT_LOGIC_ONE_SEC;
}

int QuintupleOnline::CheckStartTimer::handle_timeout(const Time_Value &tv)
{
	return QUINTUPLE_SYS->handle_timeout();
}

QuintupleOnline::QuintupleOnline()
{

}

QuintupleOnline::~QuintupleOnline()
{

}


QuintupleOnline* QuintupleOnline::instance()
{
	static QuintupleOnline* instance = NULL;

	if (instance == NULL)
	{
		instance = new QuintupleOnline;
	}

	return instance;
}

void QuintupleOnline::init_quintuple()
{
	const Json::Value& activity_conf = CONFIG_INSTANCE->common_activity(this->quintuple_detail_.cycle_id);
	JUDGE_RETURN(activity_conf.empty() == false, ;);

	MSG_USER("QUINTUPLE %d %d %d", this->quintuple_detail_.cycle_id);

	GameCommon::cal_activity_info(this->time_info_, activity_conf);
	this->check_start_timer_.cancel_timer();
	this->check_start_timer_.schedule_timer(this->time_info_.refresh_time_);
	JUDGE_RETURN(this->time_info_.cur_state_ == GameEnum::ACTIVITY_STATE_START, ;);

	MSG_USER("QUINTUPLE %d %d %d", this->time_info_.refresh_time_, this->quintuple_detail_.player_exp.size());

	this->new_start();
}

ActivityTimeInfo &QuintupleOnline::get_time_info()
{
	return this->time_info_;
}

void QuintupleOnline::new_start()
{
	this->quintuple_detail_.player_exp.clear();
	this->quintuple_detail_.open = 1;
	Proto30100601 request;
	request.set_activity_id(this->quintuple_detail_.cycle_id);
	request.set_sub_value(0);

	request.set_activity_state(GameEnum::ACTIVITY_STATE_START);
	request.set_end_time(this->time_info_.refresh_time_);

	int shout_id = GameEnum::QUINTUPLE_START_SHOUT;
	GameCommon::announce(shout_id);
	ACTIVITY_TIPS_SYSTEM->scene_sync_activity_state(&request);

	//add exp buff

	Proto30102001 request_buff;
	request_buff.set_activity_id(this->quintuple_detail_.cycle_id);
	request_buff.set_status(true);
	request_buff.set_buff_time(this->time_info_.refresh_time_);

	ACTIVITY_TIPS_SYSTEM->sync_activity_buff_state(&request_buff);
}

void QuintupleOnline::new_stop()
{
	this->quintuple_detail_.open = 0;
	this->quintuple_detail_.player_exp.clear();
	Proto30100601 request;
	request.set_activity_id(this->quintuple_detail_.cycle_id);
	request.set_sub_value(0);

	request.set_activity_state(GameEnum::ACTIVITY_STATE_END);
	request.set_left_times(this->time_info_.refresh_time_);
	ACTIVITY_TIPS_SYSTEM->remove_activity_buff(GameEnum::QUINTUPLE_ACTIVITY_ID);
	int shout_id = GameEnum::QUINTUPLE_END_SHOUT;
	GameCommon::announce(shout_id);
	ACTIVITY_TIPS_SYSTEM->scene_sync_activity_state(&request);

}

int QuintupleOnline::handle_timeout()
{
	int last_state = this->time_info_.cur_state_;
	this->time_info_.set_next_stage();
	return this->handle_quintuple_i(last_state);
}


int QuintupleOnline::handle_quintuple_i(int state)
{
	switch (state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
		this->ahead_quintuple_event();
		break;
	}

	case GameEnum::ACTIVITY_STATE_AHEAD:
	{
		this->new_start();
		break;
	}

	case GameEnum::ACTIVITY_STATE_START:
	{
		this->new_stop();
		break;
	}
	}

	this->check_start_timer_.cancel_timer();
	this->check_start_timer_.schedule_timer(this->time_info_.refresh_time_);

	return 0;
}

int QuintupleOnline::ahead_quintuple_event()
{
	Proto30100601 request;
	request.set_activity_id(this->quintuple_detail_.cycle_id);
	request.set_sub_value(0);

	request.set_activity_state(GameEnum::ACTIVITY_STATE_AHEAD);
	request.set_ahead_time(this->time_info_.refresh_time_);

	ACTIVITY_TIPS_SYSTEM->scene_sync_activity_state(&request);

	return 0;
}

Int64 QuintupleOnline::get_exp_info(Int64 role_id)
{
	return this->quintuple_detail_.player_exp[role_id];
}

int QuintupleOnline::update_monster_exp_info(Int64 role_id, int exp)
{
	JUDGE_RETURN(this->quintuple_detail_.open == 1, 0);

	if (this->quintuple_detail_.player_exp.count(role_id))
		this->quintuple_detail_.player_exp[role_id] += exp;
	else
		this->quintuple_detail_.player_exp[role_id] = exp;
	LogicPlayer* player = NULL;
	if(LOGIC_MONITOR->find_player(role_id, player) == 0 && player != NULL)
	{
		Proto80400117 respond;
		respond.set_exp(this->get_exp_info(role_id));

		player->respond_to_client(ACTIVE_MONSTER_EXP_INFO, &respond);
	}
	return 0;
}

void QuintupleOnline::test_quintuple(int id, int set_time)
{
	this->time_info_.cur_state_ = (id + 1) % this->time_info_.time_span_;
	this->time_info_.refresh_time_ = set_time;
	this->handle_quintuple_i(id);
}
