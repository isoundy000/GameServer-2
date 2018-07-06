/*
 * MLCollectChests.cpp
 *
 *  Created on: 2016年8月3日
 *      Author: lzy0927
 */

#include <google/protobuf/message.h>
#include "MLCollectChests.h"
#include "ProtoDefine.h"
#include "GameHeader.h"
#include "GameConfig.h"
#include "GameCommon.h"
#include "MapLogicPlayer.h"
#include "MapMonitor.h"
#include "time.h"
#include "stdio.h"

using google::protobuf::Message;

void MLCollectChests::CheckStartTimer::set_act_type(int type)
{
	this->act_type_ = type;
}

int MLCollectChests::CheckStartTimer::type(void)
{
	return GTT_ML_ONE_SECOND;
}

int MLCollectChests::CheckStartTimer::handle_timeout(const Time_Value &tv)
{
	return this->parent_->handle_chests_timeout(this->act_type_);
}

Collect_Chests::Collect_Chests()
{
	Collect_Chests::reset();
}

void Collect_Chests::reset()
{
	this->cycle_id = GameEnum::FIRST_COLLECT_CHESTS_ID;
	this->collect_num = 0;
	this->reset_tick = 0;
}

MLCollectChests::~MLCollectChests(void)
{
	// TODO Auto-generated constructor stub
    //	test();
}

int MLCollectChests::login_collect_chests()
{
	JUDGE_RETURN(true == validate_collect_chests_level_limit(), 0);
	time_t lt;
	lt =time(0);
	struct tm *local = localtime(&lt);

	this->__collect_chests.cur_year = local->tm_year + 1900;
	this->__collect_chests.cur_mouth = local->tm_mon + 1;
	this->__collect_chests.cur_day = local->tm_mday;

//	this->__activity_start_timer.cancel_timer();
//	this->__activity_start_timer.schedule_timer(this->next_start_times());
	return 0;
}

MLCollectChests::MLCollectChests(void)
{
	this->check_start_timer_map[GameEnum::FIRST_COLLECT_CHESTS_ID].parent_ = this;
	this->check_start_timer_map[GameEnum::FIRST_COLLECT_CHESTS_ID].set_act_type(
			GameEnum::FIRST_COLLECT_CHESTS_ID);

	this->check_start_timer_map[GameEnum::SECOND_COLLECT_CHESTS_ID].parent_ = this;
	this->check_start_timer_map[GameEnum::SECOND_COLLECT_CHESTS_ID].set_act_type(
			GameEnum::SECOND_COLLECT_CHESTS_ID);

	this->__collect_chests.reset();

	//第一场宝箱
	const Json::Value& first_activity_conf = CONFIG_INSTANCE->common_activity(GameEnum::FIRST_COLLECT_CHESTS_ID);
	JUDGE_RETURN(first_activity_conf.empty() == false, ;);

    GameCommon::cal_activity_info(this->time_info_map[GameEnum::FIRST_COLLECT_CHESTS_ID], first_activity_conf);

	//第二场宝箱
	const Json::Value& second_activity_conf = CONFIG_INSTANCE->common_activity(GameEnum::SECOND_COLLECT_CHESTS_ID);
	JUDGE_RETURN(second_activity_conf.empty() == false, ;);

    GameCommon::cal_activity_info(this->time_info_map[GameEnum::SECOND_COLLECT_CHESTS_ID], second_activity_conf);

    this->check_start_timer_map[GameEnum::FIRST_COLLECT_CHESTS_ID].cancel_timer();
    this->check_start_timer_map[GameEnum::FIRST_COLLECT_CHESTS_ID].schedule_timer(
        		this->time_info_map[GameEnum::FIRST_COLLECT_CHESTS_ID].refresh_time_);

    this->check_start_timer_map[GameEnum::SECOND_COLLECT_CHESTS_ID].cancel_timer();
    this->check_start_timer_map[GameEnum::SECOND_COLLECT_CHESTS_ID].schedule_timer(
        		this->time_info_map[GameEnum::SECOND_COLLECT_CHESTS_ID].refresh_time_);

    if (this->time_info_map[GameEnum::FIRST_COLLECT_CHESTS_ID].cur_state_ == GameEnum::ACTIVITY_STATE_START)
    {
        this->__collect_chests.cycle_id  = GameEnum::FIRST_COLLECT_CHESTS_ID;
        this->new_start();
    }

    if (this->time_info_map[GameEnum::SECOND_COLLECT_CHESTS_ID].cur_state_ == GameEnum::ACTIVITY_STATE_START)
    {
        this->__collect_chests.cycle_id  = GameEnum::SECOND_COLLECT_CHESTS_ID;
        this->new_start();
    }
}

int MLCollectChests::next_start_times()
{

	//当天上午活动之前
	int id = GameEnum::FIRST_COLLECT_CHESTS_ID;


	int interval = this->is_on_activity(id, this->__collect_chests.cur_year,
			this->__collect_chests.cur_mouth, this->__collect_chests.cur_day);
	if (interval >= 0)
	{
//		MSG_USER("活动开启 From the COLLECT CHESTS activity start %d second", interval);
		this->__collect_chests.cycle_id = id;

		return interval;
	}

	//当天下午活动之前
	id = GameEnum::SECOND_COLLECT_CHESTS_ID;
	interval = this->is_on_activity(id, this->__collect_chests.cur_year,
			this->__collect_chests.cur_mouth, this->__collect_chests.cur_day);
	if (interval >= 0)
	{
//		MSG_USER("活动开启 From the COLLECT CHESTS activity start %d second", interval);
		this->__collect_chests.cycle_id = id;

		return interval;
	}
	//下次开启时间为第二天上午活动开启时间
	id = GameEnum::FIRST_COLLECT_CHESTS_ID;
	Time_Value start_time;
	int start_h = CONFIG_INSTANCE->collect_chests_json(id)["start_time"][0u].asInt();
	int start_m = CONFIG_INSTANCE->collect_chests_json(id)["start_time"][1u].asInt();
	GameCommon::make_time_value(start_time, this->__collect_chests.cur_year,
			this->__collect_chests.cur_mouth, this->__collect_chests.cur_day, start_h, start_m);

	this->__collect_chests.cycle_id = id;
	int time_temp = Time_Value::gettimeofday().get_tv().tv_sec - start_time.get_tv().tv_sec;
	return 24*60*60 - time_temp;
}


void MLCollectChests::reset()
{
	this->__collect_chests.reset();
//	this->__activity_start_timer.set_parent(this);

//	this->__activity_start_timer.cancel_timer();
//	this->__activity_start_timer.schedule_timer(this->next_start_times());
}

int MLCollectChests::is_on_activity(int cycle_id, int year, int mouth, int day)
{
	int start_h = CONFIG_INSTANCE->collect_chests_json(cycle_id)["start_time"][0u].asInt();
	int start_m = CONFIG_INSTANCE->collect_chests_json(cycle_id)["start_time"][1u].asInt();

	int end_h = CONFIG_INSTANCE->collect_chests_json(cycle_id)["end_time"][0u].asInt();
	int end_m = CONFIG_INSTANCE->collect_chests_json(cycle_id)["end_time"][1u].asInt();

	Time_Value start_time;
	Time_Value end_time;

	GameCommon::make_time_value(start_time, year, mouth, day, start_h, start_m);
	GameCommon::make_time_value(end_time, year, mouth, day, end_h, end_m);

	if (start_time > Time_Value::gettimeofday()) return start_time.get_tv().tv_sec - Time_Value::gettimeofday().get_tv().tv_sec;

	return -1;
}



int MLCollectChests::logout_collect_chests()
{
	MLCollectChests::reset();
//	this->__collect_chest_timer.cancel_timer();
//	this->__activity_start_timer.cancel_timer();
	return 0;
}

Collect_Chests& MLCollectChests::collect_chests()
{
	return this->__collect_chests;
}

int MLCollectChests::activity_start_timeout()
{
	this->__collect_chests.collect_num = 0;

	return 0;
}

int MLCollectChests::fetch_collect_chests_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11405001*, request, -1);
	CONDITION_NOTIFY_RETURN(this->role_level() >= CONFIG_INSTANCE->collect_chests_json(
			this->__collect_chests.cycle_id)["open_level"].asInt(),
			RETURN_FETCH_COLLECT_CHESTS_INFO, ERROR_PLAYER_LEVEL);

	CONDITION_NOTIFY_RETURN(GameCommon::is_normal_scene(this->scene_id()) == true,
			RETURN_FETCH_COLLECT_CHESTS_INFO, ERROR_NORMAL_SCENE);

	if (this->__collect_chests.reset_tick < ::time(NULL))
	{
		this->__collect_chests.collect_num = 0;
		this->__collect_chests.reset_tick = ::time(NULL) + Time_Value::HOUR;
	}

	JUDGE_RETURN(this->__collect_chests.collect_num < GameEnum::PLAYER_COLLECT_MAX_NUM, 0);

	Proto51405001 respond;
	respond.set_chest_sum(this->__collect_chests.collect_num);
	respond.set_activity_id(this->__collect_chests.cycle_id);
	respond.set_enter_type(request->enter_type());
	FINER_PROCESS_RETURN(RETURN_FETCH_COLLECT_CHESTS_INFO, &respond);
}

int MLCollectChests::notify_collect_info()
{
	Proto81405003 collect_chests_info;

	collect_chests_info.set_collect_num(this->__collect_chests.collect_num);

	FINER_PROCESS_RETURN(ACTIVE_NOTIFY_COLLECT_INFO, &collect_chests_info);
}

int MLCollectChests::notify_get_extra_award()
{
	Proto81405001 respond;
	int award_id = CONFIG_INSTANCE->collect_chests_json(this->__collect_chests.cycle_id)["extra_award_id"].asInt();

	SerialObj obj(ADD_FROM_COLLECT_CHESTS, 0, 0);
	this->add_reward(award_id, obj);

	respond.set_award_id(award_id);
	FINER_PROCESS_RETURN(ACTIVE_NOTIFY_GET_EX_AWARDS, &respond);
}


bool MLCollectChests::validate_collect_chests_level_limit()
{
	const Json::Value collect_chests_json = CONFIG_INSTANCE->collect_chests_json(this->__collect_chests.cycle_id);

	int collect_chests_level = collect_chests_json["open_level"].asInt();

	return this->role_level() >= collect_chests_level;
}

int MLCollectChests::sync_transfer_collect_chests(int scene_id)
{
	Proto31400161 collect_chests_info;
	collect_chests_info.set_cycle_id(this->__collect_chests.cycle_id);
	collect_chests_info.set_collect_num(this->__collect_chests.collect_num);
	collect_chests_info.set_login_time(this->__collect_chests.reset_tick);	//login_time作为下次时间

	return this->send_to_other_logic_thread(scene_id, collect_chests_info);
}

int MLCollectChests::reset_player_chests_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400190*, request, -1);

	MLCollectChests::reset();

	Proto11405001 info;
	info.set_enter_type(1);
	info.set_is_reset(true);
	return this->fetch_collect_chests_info(&info);
}

int MLCollectChests::read_transfer_collect_chests(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400161*, request, -1);

	this->__collect_chests.cycle_id = request->cycle_id();
	this->__collect_chests.collect_num = request->collect_num();
	this->__collect_chests.reset_tick = request->login_time();

	return 0;
}


int MLCollectChests::get_chests_cycle_id()
{
	return this->__collect_chests.cycle_id == 0 ? GameEnum::FIRST_COLLECT_CHESTS_ID :
			this->__collect_chests.cycle_id;
}

ActivityTimeInfo &MLCollectChests::get_chests_time_info()
{
	return this->time_info_map[this->__collect_chests.cycle_id];
}

int MLCollectChests::handle_chests_timeout(int type)
{
	this->__collect_chests.cycle_id = type;
	int last_state = this->time_info_map[this->__collect_chests.cycle_id].cur_state_;
	this->time_info_map[this->__collect_chests.cycle_id].set_next_stage();
	return this->handle_chests_i(last_state);
}

int MLCollectChests::handle_chests_i(int state)
{
	switch (state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
		this->ahead_chests_event();
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

	this->check_start_timer_map[this->__collect_chests.cycle_id].cancel_timer();
	this->check_start_timer_map[this->__collect_chests.cycle_id].schedule_timer(
			this->time_info_map[this->__collect_chests.cycle_id].refresh_time_ - 1);

	return 0;
}
void MLCollectChests::new_start()
{
	this->__collect_chests.collect_num = 0;
}

void MLCollectChests::new_stop()
{
	this->__collect_chests.collect_num = 0;
}

int MLCollectChests::ahead_chests_event()
{
	return 0;
}
