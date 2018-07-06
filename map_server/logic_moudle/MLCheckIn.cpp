/*
 * MLCheckIn.cpp
 *
 *  Created on: 2014年1月22日
 *      Author: xing
 */

#include <google/protobuf/message.h>
#include <MLCheckIn.h>
#include "ProtoDefine.h"
#include "GameHeader.h"
#include "Log.h"
#include "GameConfig.h"
#include "GameCommon.h"

#include "MapLogicPlayer.h"

#include "MapMonitor.h"

using google::protobuf::Message;


MLCheckIn::MLCheckIn()
{
	// TODO Auto-generated constructor stub
//	test();
}

MLCheckIn::~MLCheckIn()
{
	// TODO Auto-generated destructor stub
}

CheckInDetail &MLCheckIn::check_in_detail()
{
	return this->check_in_detail_;
}

int MLCheckIn::has_charge_money()
{
	MapLogicPlayer* play = MAP_MONITOR->find_logic_player(this->role_id());
	if (this->check_in_detail_.__charge_money > 0) return 0;

	if (is_same_day(Time_Value(play->recharge_detail_.__last_recharge_time), Time_Value::gettimeofday()))
		this->check_in_detail_.__charge_money = 0;
	else this->check_in_detail_.__charge_money = -1;

	return 0;
}

bool MLCheckIn::has_check_in_award(void)
{

//	if (GameCommon::check_welfare_open_condition("sign", this->role_level(), this->role_create_days()) != 0)
//		return false;
	const Json::Value &model_json = CONFIG_INSTANCE->checkmodel_json(1);
	const Json::Value &sum_json = model_json["day_sum"];

	int awards_len = sum_json.asInt();
//	MSG_USER("check test1  awards_len %d %d", awards_len, this->check_in_detail_.__award_index);
	if(this->check_in_detail_.__award_index >= awards_len)
	{
		return false;
	}
	return !is_same_day(Time_Value(this->check_in_detail_.__last_time), Time_Value::gettimeofday());
}

bool MLCheckIn::has_check_in_award_again(void)
{
	this->has_charge_money();
//	if (GameCommon::check_welfare_open_condition("sign", this->role_level(), this->role_create_days()) != 0)
//		return false;

	if (is_same_day(Time_Value(this->check_in_detail_.__last_time), Time_Value::gettimeofday())
			&& this->check_in_detail_.__charge_money == 0) return true;
	else return false;
//	int award_id = award_json[1u].asInt();
//	int award_num = award_json[2u].asInt();

}

bool MLCheckIn::has_total_check_in_award(void)
{

//	if (GameCommon::check_welfare_open_condition("sign", this->role_level(), this->role_create_days()) != 0)
//		return false;

	const Json::Value &model_json = CONFIG_INSTANCE->checkmodel_json(1);
	const Json::Value &sum_json = model_json["total_sum"];

	int total_num = sum_json.asInt();

	if (this->check_in_detail_.__check_total_index >= total_num) return false;

	const Json::Value &totalcheck_json = CONFIG_INSTANCE->totalcheck_json(this->check_in_detail_.__check_total_index + 1);
	const Json::Value &awardtotal_json = totalcheck_json["total_num"];
	int award_sum = awardtotal_json.asInt();
//	MSG_USER("check test2  awards_len %d %d", total_num, this->check_in_detail_.__check_total_index);
//
//	MSG_USER("test total check  award_sum %d", award_sum);

	if (this->check_in_detail_.__award_index < award_sum) return false;

	return true;
//	int award_id = award_json[1u].asInt();
//	int award_num = award_json[2u].asInt();
}

int MLCheckIn::check_need_popup()
{
	if(!GameCommon::is_normal_scene(this->scene_id()))
		return 0;

	int check_in = (this->has_check_in_award() ? 1 : 0);
	int popup = ((this->check_in_detail_.__popup && (check_in)) ? 1 : 0);

	if (this->has_check_in_award_again()) popup = 1;
	this->check_in_detail_.__popup = 0;

	return popup;
}

int MLCheckIn::total_check_need_popup()
{
	if(!GameCommon::is_normal_scene(this->scene_id()))
		return 0;

	int total_check_in = (this->has_total_check_in_award() ? 1 : 0);
	int total_popup = ((this->check_in_detail_.__total_popup && (total_check_in)) ? 1 : 0);

	this->check_in_detail_.__total_popup = 0;

	return total_popup;
}

bool MLCheckIn::need_new_check_in_cycle(void)
{
	if (!GameCommon::is_same_month(Time_Value(this->check_in_detail_.__last_time),
			Time_Value::gettimeofday()))
	{
		return true;
	}

	const Json::Value &model_json = CONFIG_INSTANCE->checkmodel_json(1);
	const Json::Value &sum_json = model_json["day_sum"];
	int awards_len;
	if (this->check_in_detail_.__month_day == 0) awards_len = sum_json.asInt();
	else awards_len = this->check_in_detail_.__month_day;

	MSG_USER("check test3  awards_len %d %d", awards_len , this->check_in_detail_.__check_total_index);
	if(( this->check_in_detail_.__award_index >= awards_len &&
					!is_same_day(Time_Value(this->check_in_detail_.__last_time), Time_Value::gettimeofday()) ))
	{
		return true;
	}

	return false;
}

int MLCheckIn::new_check_in_cycle(void)
{

	this->check_in_detail_.__award_index = 0;
	this->check_in_detail_.__check_total_index = 0;

	this->cache_tick().update_cache(MapLogicPlayer::CACHE_WELFARE);

	return 0;
}

int MLCheckIn::fetch_check_in_info(Message *msg)
{
//	MSG_DYNAMIC_CAST_NOTIFY(Proto11401401*, request, RETURN_FETCH_CHECK_IN_INFO);
	if (this->need_new_check_in_cycle())
	{
		int ret = this->new_check_in_cycle();
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_FETCH_CHECK_IN_INFO, ret);
	}

	Proto51401401 respond;
	respond.set_award_index(this->check_in_detail_.__award_index); // 这里的 award_index 表示已领取过的奖励的数量
	respond.set_has_award(this->has_check_in_award() ? 1 : 0);
	respond.set_check_in_point(this->check_in_detail_.__check_in_point);

	respond.set_check_total_index(this->check_in_detail_.__check_total_index); // 这里的 award_index 表示已领取过的奖励的数量
	respond.set_has_total_award(this->has_total_check_in_award() ? 1 : 0);
	this->has_charge_money();
	respond.set_charge_money(this->check_in_detail_.__charge_money);

	FINER_PROCESS_RETURN(RETURN_FETCH_CHECK_IN_INFO, &respond);
}

int MLCheckIn::request_check_in(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11401402*, request, RETURN_REQUEST_CHECK_IN);
	this->check_in_detail_.__month_day = request->month_day();

	CONDITION_NOTIFY_RETURN(this->has_check_in_award(), RETURN_FETCH_CHECK_IN_INFO,
			ERROR_CLIENT_OPERATE);
//	CONDITION_NOTIFY_RETURN( request->award_index() == check_in_detail_.__award_index,
//			RETURN_FETCH_CHECK_IN_INFO, ERROR_CLIENT_OPERATE);

	const Json::Value &daycheck_json = CONFIG_INSTANCE->daycheck_json(this->check_in_detail_.__award_index + 1);
	const Json::Value &awardcheck_json = daycheck_json["award_id"];

	SerialObj obj(ADD_FROM_CHECKI_IN_AWARD , 0, 0);
	this->add_reward(awardcheck_json.asInt(), obj);
	this->check_in_detail_.__charge_money = -1;
	this->has_charge_money();

	// 签到成功，　更改相应状态，　返回
	this->check_in_detail_.__award_index += 1;
	this->check_in_detail_.__last_time = time(0);

	this->cache_tick().update_cache(MapLogicPlayer::CACHE_WELFARE);

//	MapLogicPlayer::notify_player_welfare_status(this);

	Proto51401402 respond;
	respond.set_status(1);
//	respond.set_check_in_point(this->check_in_detail_.__check_sin_point);

	this->check_in_pa_event();
	FINER_PROCESS_RETURN(RETURN_REQUEST_CHECK_IN, &respond);
}

int MLCheckIn::request_check_in_again(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11401472*, request, RETURN_REQUEST_CHECK_IN_AGAIN);
	this->has_charge_money(); //test

	if (this->has_check_in_award_again() == false)
	{
		Proto51401416 respond;
		respond.set_status(0);
		respond.set_check_in_point(this->check_in_detail_.__check_in_point);
		respond.set_charge_money(this->check_in_detail_.__charge_money);

		FINER_PROCESS_RETURN(RETURN_REQUEST_CHECK_IN_AGAIN, &respond);
	}


	this->check_in_detail_.__award_index -= 1;
	const Json::Value &daycheck_json = CONFIG_INSTANCE->daycheck_json(this->check_in_detail_.__award_index + 1);
	const Json::Value &awardcheck_json = daycheck_json["award_id"];

	SerialObj obj(ADD_FROM_CHECKI_IN_AWARD , 0, 0);
	this->add_reward(awardcheck_json.asInt(), obj);

	// 签到成功，　更改相应状态，　返回
	this->check_in_detail_.__award_index += 1;
	this->check_in_detail_.__charge_money += 1;
	this->check_in_detail_.__last_time = time(0);

	this->cache_tick().update_cache(MapLogicPlayer::CACHE_WELFARE);

	MapLogicPlayer::notify_player_welfare_status(this);

	Proto51401416 respond;
	respond.set_status(1);
	respond.set_check_in_point(this->check_in_detail_.__check_in_point);
	respond.set_charge_money(this->check_in_detail_.__charge_money);

	this->check_in_pa_event();
	FINER_PROCESS_RETURN(RETURN_REQUEST_CHECK_IN_AGAIN, &respond);
}

int MLCheckIn::check_in_pa_event()
{
	MapLogicPlayer * player = this->map_logic_player();
	return player->update_player_assist_single_event(GameEnum::PA_EVENT_CHECK_IN_AWARD,
			this->check_need_popup() | this->total_check_need_popup());
}

int MLCheckIn::request_total_check_in(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11401471*, request, RETURN_REQUEST_TOTAL_CHECK_IN);
	CONDITION_NOTIFY_RETURN(this->has_total_check_in_award(), RETURN_FETCH_CHECK_IN_INFO,
			ERROR_CLIENT_OPERATE);
//	CONDITION_NOTIFY_RETURN( request->award_index() == check_in_detail_.__award_index,
//			RETURN_FETCH_CHECK_IN_INFO, ERROR_CLIENT_OPERATE);

	const Json::Value &totalcheck_json = CONFIG_INSTANCE->totalcheck_json(this->check_in_detail_.__check_total_index + 1);
	const Json::Value &awardcheck_json = totalcheck_json["award_id"];

	int award_id = awardcheck_json.asInt();
	MSG_USER("check test7 %d", award_id);

	SerialObj obj(ADD_FROM_CHECKI_IN_AWARD , 0, 0);
	this->add_reward(award_id, obj);

	// 签到成功，　更改相应状态，　返回
	this->check_in_detail_.__check_total_index += 1;
	this->check_in_detail_.__total_last_time = time(0);

	MSG_USER("day_check_in role: %ld %s, award_index: %d, last_time: %d, %s",
			this->role_id(), this->name(), this->check_in_detail_.__check_total_index,
			this->check_in_detail_.__total_last_time, awardcheck_json.toStyledString().c_str());

	this->cache_tick().update_cache(MapLogicPlayer::CACHE_WELFARE);

	MapLogicPlayer::notify_player_welfare_status(this);

	Proto51401415 respond;
	respond.set_status(1);
	respond.set_check_in_point(this->check_in_detail_.__check_in_point);
	this->check_in_pa_event();

	FINER_PROCESS_RETURN(RETURN_REQUEST_TOTAL_CHECK_IN, &respond);
}

int MLCheckIn::set_notify_msg_check_in_info(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto81401401*, respond, msg, -1);
	if (need_new_check_in_cycle())
	{
		int ret = this->new_check_in_cycle();
		JUDGE_RETURN(ret == 0, ret);
	}

	int check_in = has_check_in_award() ? 1 : 0;
	respond->set_check_in(check_in);
	respond->set_check_in_popup(false);	// MapLogicPlayer::notify_client_popup_info()
//	respond->set_show_check_in_point(this->check_in_detail_.__show_point);

	return 0;
}

int MLCheckIn::sync_transfer_check_in(int scene_id)
{
	Proto31401401 request;
	request.set_award_index(this->check_in_detail_.__award_index);
	request.set_check_in_point(this->check_in_detail_.__check_in_point);
	request.set_last_time(this->check_in_detail_.__last_time);
	request.set_show_point(this->check_in_detail_.__show_point);
	request.set_popup(this->check_in_detail_.__popup);
	request.set_check_total_index(this->check_in_detail_.__check_total_index);
	request.set_charge_money(this->check_in_detail_.__charge_money);
	request.set_total_last_time(this->check_in_detail_.__total_last_time);

	return this->send_to_other_logic_thread(scene_id, request);
}

int MLCheckIn::read_transfer_check_in(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31401401*, request, -1);

	this->check_in_detail_.__award_index = request->award_index();
	this->check_in_detail_.__check_in_point = request->check_in_point();
	this->check_in_detail_.__last_time = request->last_time();
	this->check_in_detail_.__show_point = request->show_point();
	this->check_in_detail_.__popup = request->popup();
	this->check_in_detail_.__check_total_index = request->check_total_index();
	this->check_in_detail_.__charge_money = request->charge_money();
	this->check_in_detail_.__total_last_time = request->total_last_time();
	return 0;
}

void MLCheckIn::reset(void)
{
	this->check_in_detail_.reset();
}
