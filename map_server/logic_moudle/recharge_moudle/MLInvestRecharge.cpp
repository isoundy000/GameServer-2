/*
 * MLInvestRecharge.cpp
 *
 *  Created on: 2016年11月11日
 *      Author: lzy
 */

#include "MapLogicPlayer.h"
#include "MLInvestRecharge.h"
#include "ProtoDefine.h"
#include "MapMonitor.h"
#include "SerialRecord.h"

MLInvestRecharge::MLInvestRecharge()
{
	// xTODO Auto-generated constructor stub
}

MLInvestRecharge::~MLInvestRecharge()
{
	// xTODO Auto-generated destructor stub
}

void MLInvestRecharge::reset(void)
{
	this->invest_detail_.reset();
}

void MLInvestRecharge::set_invest_test_time(int days)
{
	this->invest_detail_.__buy_time.set_tick(::time(NULL)
			- (days - 1) * Time_Value::DAY);
}

int MLInvestRecharge::reset_invest_everyday()
{
	this->check_invest_normal_reward();
	this->check_invest_vip_reward();
	return 0;
}

int MLInvestRecharge::fetch_left_invest_time()
{
	const Json::Value &rr_json = CONFIG_INSTANCE->invest_recharge_json();
	static int end = rr_json["end_time"][0u].asInt() * Time_Value::DAY
			+ rr_json["end_time"][1u].asInt() * Time_Value::HOUR
			+ rr_json["end_time"][2u].asInt() * Time_Value::MINUTE
			+ rr_json["end_time"][3u].asInt();

	int now_sec = GameCommon::fetch_cur_day_sec();
	int open_days = CONFIG_INSTANCE->open_server_days() - 1;

	return std::max<int>(end - open_days * Time_Value::DAY - now_sec, 0);
}

int MLInvestRecharge::is_have_invest_reward()
{
	InvestRechargeDetail &detail = this->invest_recharge_detail();
	JUDGE_RETURN(detail.__is_buy == true, false);

	int cur_max_days = detail.cur_max_days();
	for (int i = DAILY_1; i <= cur_max_days; ++i)
	{
		if (detail.__invest_rewards[i] < REWARD_GONE)
		{
			//有普通奖励
			return true;
		}

		if (detail.__vip_rewards[i] == REWARD_HAVE)
		{
			//有VIP6奖励
			return true;
		}
	}

	return detail.__buy_time.passed_days() <= DAILY_7;
}

int MLInvestRecharge::is_show_invest_icon()
{
	if (this->fetch_left_invest_time() > 0)
	{
		//有剩余时间
		return true;
	}
	else
	{
		//有奖励未领取
		return this->is_have_invest_reward();
	}
}

InvestRechargeDetail& MLInvestRecharge::invest_recharge_detail()
{
	return this->invest_detail_;
}

int MLInvestRecharge::fetch_invest_recharge_info()
{
	return this->notify_invest_recharge_info();
}

int MLInvestRecharge::check_invest_normal_reward()
{
	InvestRechargeDetail& detail = this->invest_recharge_detail();
	JUDGE_RETURN(detail.__is_buy == true, -1);

	int cur_max_days = detail.cur_max_days();
	for (int i = DAILY_1; i <= cur_max_days; ++i)
	{
		JUDGE_CONTINUE(detail.__invest_rewards[i] == REWARD_NONE);
		detail.__invest_rewards[i] = REWARD_HAVE;
	}

	return 0;
}

int MLInvestRecharge::check_invest_vip_reward()
{
	JUDGE_RETURN(this->is_have_invest_reward() == true, -1);

	InvestRechargeDetail& detail = this->invest_recharge_detail();
	JUDGE_RETURN(detail.__is_buy == true, -1);

	detail.__vip_level = this->vip_detail().__vip_level;

	int vip_limit = CONFIG_INSTANCE->invest_recharge_json()["vip_level"].asInt();
	JUDGE_RETURN(detail.__vip_level >= vip_limit, -1);

	int cur_max_days = detail.cur_max_days();
	for (int i = DAILY_1; i <= cur_max_days; ++i)
	{
		JUDGE_CONTINUE(detail.__vip_rewards[i] == REWARD_NONE);
		detail.__vip_rewards[i] = REWARD_HAVE;
	}

	return 0;
}

int MLInvestRecharge::check_and_notify_invest_icon()
{
	if (this->is_show_invest_icon() == true)
	{
		return this->notify_invest_recharge_info();
	}
	else
	{
		return this->notify_cancel_invest_icon();
	}
}

int MLInvestRecharge::notify_cancel_invest_icon()
{
	Proto81401807 msg;
	msg.set_show_icon(0);
	FINER_PROCESS_RETURN(ACTIVE_NOTIFY_INVEST_RECHARGE_INFO, &msg);
}

int MLInvestRecharge::notify_invest_recharge_info()
{
	JUDGE_RETURN(this->is_show_invest_icon() == true, 0);

	Proto81401807 msg;
	msg.set_left_time(this->fetch_left_invest_time());
	msg.set_vip_level(this->vip_detail().__vip_level);

	InvestRechargeDetail &detail = this->invest_recharge_detail();
	if (detail.__is_buy == true)
	{
		msg.set_index(detail.cur_max_days());
	}
	else
	{
		msg.set_index(0);
	}

	for (int i = DAILY_1; i < INVEST_TYPE_NUM; ++i)
	{
		ProtoPairObj* list_1 = msg.add_normal_list();
		ProtoPairObj* list_2 = msg.add_vip_list();

		list_1->set_obj_id(i);
		list_2->set_obj_id(i);

		list_1->set_obj_value(detail.__invest_rewards[i]);
		list_2->set_obj_value(detail.__vip_rewards[i]);
	}

	msg.set_show_icon(1);
	FINER_PROCESS_RETURN(ACTIVE_NOTIFY_INVEST_RECHARGE_INFO, &msg);
}

int MLInvestRecharge::buy_invest_recharge()
{
	InvestRechargeDetail& detail = this->invest_recharge_detail();
	CONDITION_NOTIFY_RETURN(detail.__is_buy == 0, RETURN_FETCH_INVEST_RECHARGE_REWARDS,
			ERROR_CLIENT_OPERATE);

	const Json::Value& ir_json = CONFIG_INSTANCE->invest_recharge_json();
	int ret = this->pack_money_sub(Money(ir_json["need_money"].asInt()),
			SUB_MONEY_INVEST_RECHARGE);
	CONDITION_NOTIFY_RETURN(ret == 0,RETURN_FETCH_INVEST_RECHARGE_REWARDS, ret);

	detail.__is_buy = 1;
	detail.__buy_time.set_tick(::time(NULL));
	this->act_serial_info_update(SERIAL_ACT_INVEST_RECHARGE, this->role_id(), 1);

	this->check_invest_normal_reward();
	this->check_invest_vip_reward();
	this->notify_invest_recharge_info();
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_INVEST);

	//传闻
	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_string(para_vec, this->role_name());
	GameCommon::announce(ir_json["shout_id"].asInt(), &para_vec);

	Proto51401811 respond;
	respond.set_index(0);
	respond.set_status(1);
	FINER_PROCESS_RETURN(RETURN_FETCH_INVEST_RECHARGE_REWARDS, &respond);
}

int MLInvestRecharge::fetch_invest_rewards(int index, int type)
{
	InvestRechargeDetail &detail = this->invest_recharge_detail();
	const Json::Value &ir_json = CONFIG_INSTANCE->invest_recharge_json();

	int is_draw = 0;
	switch (type)
	{
	case 1:
	{
		//普通奖励
		JUDGE_BREAK(detail.__invest_rewards[index] == REWARD_HAVE);

		int reward_id = ir_json["reward_id"][index - 1].asInt();
		this->add_reward(reward_id, SerialObj(ADD_FROM_ITEM_INVEST_RECHARGE));

		is_draw = true;
		detail.__invest_rewards[index] = REWARD_GONE;
		break;
	}
	case 2:
	{
		//VIP6奖励
		JUDGE_BREAK(detail.__vip_rewards[index] == REWARD_HAVE);

		int reward_id = ir_json["vip_reward"][index - 1].asInt();
		this->add_reward(reward_id, SerialObj(ADD_FROM_ITEM_INVEST_RECHARGE));

		is_draw = true;
		detail.__vip_rewards[index] = REWARD_GONE;
		break;
	}
	}

	CONDITION_NOTIFY_RETURN(is_draw > 0, RETURN_FETCH_INVEST_RECHARGE_REWARDS, ERROR_NO_REWARD);

	this->check_and_notify_invest_icon();
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_INVEST);

	Proto51401811 respond;
	respond.set_index(index);
	respond.set_status(1);
	FINER_PROCESS_RETURN(RETURN_FETCH_INVEST_RECHARGE_REWARDS, &respond);
}

int MLInvestRecharge::fetch_invest_recharge_rewards(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11401811*, request, ERROR_CLIENT_OPERATE);

	if (request->index() != 0)
	{
		return this->fetch_invest_rewards(request->index(), request->type());
	}
	else
	{
		return this->buy_invest_recharge();
	}
}

int MLInvestRecharge::sync_transfer_invest_recharge(int scene_id)
{
	InvestRechargeDetail &detail = this->invest_recharge_detail();
	JUDGE_RETURN(detail.__is_buy == true, -1);

	Proto31400163 info;
	info.set_is_buy(detail.__is_buy);
	info.set_buy_time(detail.__buy_time.create_tick_);

	for (int i = DAILY_1; i < INVEST_TYPE_NUM; ++i)
	{
		info.add_invest_rewards(detail.__invest_rewards[i] + 1);
		info.add_vip_rewards(detail.__vip_rewards[i] + 1);
	}

	return this->send_to_other_logic_thread(scene_id, info);
}

int MLInvestRecharge::read_transfer_invest_recharge(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400163*, request, -1);

	InvestRechargeDetail &detail = this->invest_recharge_detail();

	detail.__is_buy = request->is_buy();
	detail.__buy_time.set_tick(request->buy_time());
	detail.__vip_level = this->vip_detail().__vip_level;

	for (int i = DAILY_1; i < INVEST_TYPE_NUM; ++i)
	{
		detail.__invest_rewards[i] = request->invest_rewards(i - 1) - 1;
		detail.__vip_rewards[i] = request->vip_rewards(i - 1) - 1;
	}

	return 0;
}



