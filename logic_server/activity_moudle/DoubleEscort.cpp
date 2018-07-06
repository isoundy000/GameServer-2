/*
 * DoubleEscort.cpp
 *
 *  Created on: 2016年10月25日
 *      Author: lzy
 */

#include "DoubleEscort.h"
#include "ProtoDefine.h"
#include "SerialRecord.h"
#include "LogicMonitor.h"
#include "LogicPlayer.h"

void DoubleEscort::CheckStartTimer::set_act_type(int type)
{
	this->act_type = type;
}

int DoubleEscort::CheckStartTimer::type(void)
{
	return GTT_LOGIC_ONE_SEC;
}

int DoubleEscort::CheckStartTimer::handle_timeout(const Time_Value &tv)
{
	return DOUBLE_ESCORT->handle_timeout(this->act_type);
}

int DoubleEscort::RefreshTimer::type(void)
{
	return GTT_LOGIC_ONE_SEC;
}

int DoubleEscort::RefreshTimer::handle_timeout(const Time_Value &tv)
{
	return DOUBLE_ESCORT->every_day_serial_work();
}

DoubleEscort::DoubleEscort()
{

}

DoubleEscort::~DoubleEscort()
{

}

int DoubleEscort::get_cycle_id()
{
	return this->escort_detail_.cycle_id;
}

int DoubleEscort::get_open()
{
	return this->escort_detail_.open;
}

void DoubleEscort::init_escort()
{
	this->refresh_timer_.cancel_timer();
	Int64 left_time = ::next_day(0,0).sec() - Time_Value::gettimeofday().sec();
	this->refresh_timer_.schedule_timer((int)left_time);

	MSG_USER("DOUBLE_ESCORT %d %d %d", this->refresh_timer_.left_second(),this->check_start_timer_map.size());
	this->check_start_timer_map[GameEnum::ESCORT_ACTIVITY_FIRST_ID].set_act_type(GameEnum::ESCORT_ACTIVITY_FIRST_ID);
	this->check_start_timer_map[GameEnum::ESCORT_ACTIVITY_SECOND_ID].set_act_type(GameEnum::ESCORT_ACTIVITY_SECOND_ID);

	const Json::Value& activity_conf_1 = CONFIG_INSTANCE->common_activity(GameEnum::ESCORT_ACTIVITY_FIRST_ID);
	JUDGE_RETURN(activity_conf_1.empty() == false, ;);
	GameCommon::cal_activity_info(this->time_info_map[GameEnum::ESCORT_ACTIVITY_FIRST_ID], activity_conf_1);
	this->check_start_timer_map[GameEnum::ESCORT_ACTIVITY_FIRST_ID].cancel_timer();
	this->check_start_timer_map[GameEnum::ESCORT_ACTIVITY_FIRST_ID].schedule_timer(
			this->time_info_map[GameEnum::ESCORT_ACTIVITY_FIRST_ID].refresh_time_);

	const Json::Value& activity_conf_2 = CONFIG_INSTANCE->common_activity(GameEnum::ESCORT_ACTIVITY_SECOND_ID);
	JUDGE_RETURN(activity_conf_2.empty() == false, ;);
	GameCommon::cal_activity_info(this->time_info_map[GameEnum::ESCORT_ACTIVITY_SECOND_ID], activity_conf_2);
	this->check_start_timer_map[GameEnum::ESCORT_ACTIVITY_SECOND_ID].cancel_timer();
	this->check_start_timer_map[GameEnum::ESCORT_ACTIVITY_SECOND_ID].schedule_timer(
			this->time_info_map[GameEnum::ESCORT_ACTIVITY_SECOND_ID].refresh_time_);

	if (this->time_info_map[GameEnum::ESCORT_ACTIVITY_FIRST_ID].cur_state_ == GameEnum::ACTIVITY_STATE_START)
	{
		this->escort_detail_.cycle_id = GameEnum::ESCORT_ACTIVITY_FIRST_ID;
		this->new_start();
	}


	if (this->time_info_map[GameEnum::ESCORT_ACTIVITY_SECOND_ID].cur_state_ == GameEnum::ACTIVITY_STATE_START)
	{
		this->escort_detail_.cycle_id = GameEnum::ESCORT_ACTIVITY_SECOND_ID;
		this->new_start();
	}

	this->escort_detail_.cycle_id = (this->time_info_map[GameEnum::ESCORT_ACTIVITY_FIRST_ID].fetch_left_time() <
						this->time_info_map[GameEnum::ESCORT_ACTIVITY_SECOND_ID].fetch_left_time()) ?
						GameEnum::ESCORT_ACTIVITY_FIRST_ID : GameEnum::ESCORT_ACTIVITY_SECOND_ID;

	MSG_USER("DOUBLE_ESCORT %d %d %d", this->refresh_timer_.left_second(),
			this->check_start_timer_map.size());
}

ActivityTimeInfo &DoubleEscort::get_time_info()
{
	return this->time_info_map[this->escort_detail_.cycle_id];
}

void DoubleEscort::new_start()
{
	this->escort_detail_.open = 1;
	Proto30100601 request;
	request.set_activity_id(this->escort_detail_.cycle_id);
	request.set_sub_value(0);

	request.set_activity_state(GameEnum::ACTIVITY_STATE_START);
	request.set_end_time(this->time_info_map[this->escort_detail_.cycle_id].refresh_time_);

	int shout_id = CONFIG_INSTANCE->convoy_json()["shout_start"].asInt();
	GameCommon::announce(shout_id);
	ACTIVITY_TIPS_SYSTEM->scene_sync_activity_state(&request);
}

void DoubleEscort::new_stop()
{
	this->escort_detail_.open = 0;

	Proto30100601 request;
	request.set_activity_id(this->escort_detail_.cycle_id);
	request.set_sub_value(0);

	request.set_activity_state(GameEnum::ACTIVITY_STATE_END);
	request.set_left_times(this->time_info_map[this->escort_detail_.cycle_id].refresh_time_);
	ACTIVITY_TIPS_SYSTEM->remove_activity_buff(GameEnum::QUINTUPLE_ACTIVITY_ID);
	int shout_id = CONFIG_INSTANCE->convoy_json()["shout_end"].asInt();
	GameCommon::announce(shout_id);
	ACTIVITY_TIPS_SYSTEM->scene_sync_activity_state(&request);

}

int DoubleEscort::handle_timeout(int type)
{
	this->escort_detail_.cycle_id = type;
	int last_state = this->time_info_map[this->escort_detail_.cycle_id].cur_state_;
	this->time_info_map[this->escort_detail_.cycle_id].set_next_stage();
	return this->handle_escort_i(last_state);
}


int DoubleEscort::handle_escort_i(int state)
{
	switch (state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
		this->ahead_escort_event();
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

	this->check_start_timer_map[this->escort_detail_.cycle_id].cancel_timer();
	this->check_start_timer_map[this->escort_detail_.cycle_id].schedule_timer(
			this->time_info_map[this->escort_detail_.cycle_id].refresh_time_);

	return 0;
}

int DoubleEscort::ahead_escort_event()
{
	Proto30100601 request;
	request.set_activity_id(this->escort_detail_.cycle_id);
	request.set_sub_value(0);

	request.set_activity_state(GameEnum::ACTIVITY_STATE_AHEAD);
	request.set_ahead_time(this->time_info_map[this->escort_detail_.cycle_id].refresh_time_);

	ACTIVITY_TIPS_SYSTEM->scene_sync_activity_state(&request);

	return 0;
}

void DoubleEscort::test_escort(int id, int set_time)
{
	this->escort_detail_.cycle_id = GameEnum::ESCORT_ACTIVITY_FIRST_ID;
	this->time_info_map[this->get_cycle_id()].cur_state_ = (id + 1) % this->time_info_map[this->get_cycle_id()].time_span_;
	this->time_info_map[this->get_cycle_id()].refresh_time_ = set_time;
	this->handle_escort_i(id);
}

int DoubleEscort::update_daily_gold(Int64 role_id, Int64 gold, Int64 item_id, Int64 item_amount)
{
	const Json::Value &convoy = CONFIG_INSTANCE->convoy_json();
	int item = convoy["upgrade_item_id"].asInt();
	JUDGE_RETURN(item_id == item, 0);

	if (this->escort_detail_.gold_list.count(role_id) == 0)
		this->escort_detail_.gold_list[role_id] = gold;
	else
		this->escort_detail_.gold_list[role_id] += gold;

	if (this->escort_detail_.item_list.count(role_id) == 0)
		this->escort_detail_.item_list[role_id] = item_amount;
	else
		this->escort_detail_.item_list[role_id] += item_amount;

	this->escort_detail_.daily_gold += gold;

	return 0;
}

int DoubleEscort::update_serial_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30103102*, request, -1);

	Int64 id = request->id();
//	Int64 value = request->value();

	if (this->escort_detail_.player_list.count(id) == 0)
	{
		this->escort_detail_.player_list.insert(id);
	}

	return 0;
}

int DoubleEscort::escort_serial_work(Int64 role_id)
{
	LogicPlayer* player = NULL;
	JUDGE_RETURN(LOGIC_MONITOR->find_player(role_id, player) == 0, -1);

	player->record_other_serial(ESCORT_START_SERIAL, 0,
			this->escort_detail_.gold_list[role_id],
			this->escort_detail_.item_list[role_id]);

	this->escort_detail_.gold_list.erase(role_id);
	this->escort_detail_.item_list.erase(role_id);

	return 0;
}

int DoubleEscort::every_day_serial_work()
{
	SERIAL_RECORD->record_activity(SERIAL_ACT_ESCORT, this->escort_detail_.player_list.size(), this->escort_detail_.daily_gold);

	this->escort_detail_.player_list.clear();
	this->escort_detail_.daily_gold = 0;

	this->refresh_timer_.cancel_timer();
	Int64 left_time = ::next_day(0,0).sec() - Time_Value::gettimeofday().sec();
	this->refresh_timer_.schedule_timer((int)left_time);

	return 0;
}
