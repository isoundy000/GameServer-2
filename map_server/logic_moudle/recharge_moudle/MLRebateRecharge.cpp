/*
 * MLRebateRecharge.cpp
 *
 *  Created on: 2016年11月9日
 *      Author: lzy
 */

#include "MapLogicPlayer.h"
#include "MLRebateRecharge.h"
#include "ProtoDefine.h"
//#include "MLBackRebateRechargeSys.h"
#include "MapMonitor.h"
#include "SerialRecord.h"

void MLRebateRecharge::ShowTimer::set_parent(MLRebateRecharge* parent)
{
	this->parent_ = parent;
}

int MLRebateRecharge::ShowTimer::type(void)
{
	return GTT_ML_ONE_SECOND;
}

int MLRebateRecharge::ShowTimer::handle_timeout(const Time_Value &tv)
{
	return parent_->notify_rebate_recharge_info();
}


MLRebateRecharge::MLRebateRecharge()
{
	// xTODO Auto-generated constructor stub
	this->show_timer_.set_parent(this);
}

MLRebateRecharge::~MLRebateRecharge()
{
	// xTODO Auto-generated destructor stub
}

void MLRebateRecharge::reset(void)
{
	this->show_timer_.cancel_timer();
	this->rebate_recharge_dtail_.reset();
}

void MLRebateRecharge::init_rebate_recharge_rewards()
{

}

RebateRechargeDetail& MLRebateRecharge::rebate_recharge_dtail()
{
	return this->rebate_recharge_dtail_;
}

int MLRebateRecharge::fetch_rebate_recharge_info()
{
	return this->notify_rebate_recharge_info();
}

int MLRebateRecharge::notify_rebate_recharge_info(int show_icon)
{
	RebateRechargeDetail &detail = this->rebate_recharge_dtail();

	const Json::Value &rr_json = CONFIG_INSTANCE->rebate_recharge_json();
	Int64 now = GameCommon::fetch_cur_day_sec();
	Int64 end = rr_json["end_time"][1u].asInt() * Time_Value::HOUR +
			rr_json["end_time"][2u].asInt() * Time_Value::MINUTE + rr_json["end_time"][3u].asInt();
//	int open_day = CONFIG_INSTANCE->open_server_days();
	int day = rr_json["end_time"][0u].asInt() - CONFIG_INSTANCE->open_server_days() + 1;

	Int64 left_time = day * Time_Value::DAY + end - now;
	Proto81401806 msg;
	msg.set_buy_time(rr_json["act_times"].asInt() - detail.__rebate_times);
	msg.set_left_min(left_time);

	if (left_time > 0)
	{
		show_icon = 1;
		this->show_timer_.cancel_timer();
		this->show_timer_.schedule_timer((int)left_time);
	}

	if (msg.buy_time() == 0) show_icon = -1;

	msg.set_show_icon(show_icon);
	return this->respond_to_client(ACTIVE_NOTIFY_REBATE_RECHARGE_INFO, &msg);
}

int MLRebateRecharge::fetch_rebate_recharge_rewards()
{
//	DYNAMIC_CAST_NOTIFY(Proto11401809*, request, msg, ERROR_CLIENT_OPERATE);
	const Json::Value &rr_json = CONFIG_INSTANCE->rebate_recharge_json();
	CONDITION_NOTIFY_RETURN(this->rebate_recharge_dtail_.__rebate_times < rr_json["act_times"].asInt(),
			RETURN_FETCH_REBATE_RECHARGE_REWARDS, ERROR_CLIENT_OPERATE);

	Money money;
	money.__gold = rr_json["need_money"].asInt();
	int ret = this->pack_money_sub(money, SerialObj(SUB_MONEY_REBATE_RECHARGE));
	CONDITION_NOTIFY_RETURN(ret == 0,RETURN_FETCH_REBATE_RECHARGE_REWARDS, ret);

	this->rebate_recharge_dtail_.__rebate_times ++;
	this->rebate_recharge_dtail_.__last_buy_time = ::time(0);

	int reward_id = rr_json["reward_id"].asInt();
	this->add_reward(reward_id, SerialObj(ADD_FROM_ITEM_REBATE_RECHARGE));
	this->act_serial_info_update(SERIAL_ACT_REBATE_RECHARGE, this->map_logic_player()->role_id(), 1);
	this->notify_rebate_recharge_info();

	//传闻
	int shout_id = rr_json["shout_id"].asInt();
	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_string(para_vec, this->role_name());
	GameCommon::announce(shout_id, &para_vec);

	return this->respond_to_client(RETURN_FETCH_REBATE_RECHARGE_REWARDS);
}

int MLRebateRecharge::sync_transfer_rebate_recharge(int scene_id)
{
	Proto31400162 info;
	info.set_rebate_times(this->rebate_recharge_dtail_.__rebate_times);
	info.set_last_buy_time(this->rebate_recharge_dtail_.__last_buy_time);

	return this->send_to_other_logic_thread(scene_id, info);
}

int MLRebateRecharge::read_transfer_rebate_recharge(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400162*, request, -1);

	this->rebate_recharge_dtail_.__last_buy_time = request->last_buy_time();
	this->rebate_recharge_dtail_.__rebate_times = request->rebate_times();

	return 0;
}
