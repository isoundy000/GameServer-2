/*
 * InvestRechargeSys.cpp
 *
 *  Created on: 2016年11月9日
 *      Author: lzy
 */

#include "InvestRechargeSys.h"
#include "PoolMonitor.h"
//#include "TransactionMonitor.h"
#include "LogicMonitor.h"
#include "MongoDataMap.h"
//#include "Transaction.h"
#include "MongoData.h"
#include "MMOInvestSystem.h"
#include "ProtoDefine.h"
#include "SerialRecord.h"

InvestRechargeSys::InvestRechargeSys()
{
	// TODO Auto-generated constructor stub
}

InvestRechargeSys::~InvestRechargeSys()
{
	// TODO Auto-generated destructor stub
}


int InvestRechargeSys::MailTimer::type(void)
{
	return GTT_LOGIC_ONE_SEC;
}

int InvestRechargeSys::MailTimer::handle_timeout(const Time_Value &tv)
{
	return BackIR_SYS->send_mail_to_player();
}

InvestRechargeDetail& InvestRechargeSys::player_detail(Int64 role_id)
{
	return this->invest_map_[role_id];
}

int InvestRechargeSys::every_day_serial_work()
{
	int daily_total_1 = 0;
	int daily_total_2 = 0;

	LongMap::iterator it = this->serial_daily_map_.begin();
	for (; it != this->serial_daily_map_.end(); ++it)
	{
		if (it->second >= TYPE_NUM_1) daily_total_1++;
		if (it->second >= TYPE_NUM_2) daily_total_2++;
	}
	SERIAL_RECORD->record_activity(SERIAL_ACT_DAILY_RECHARGE, this->serial_daily_map_.size(),
			daily_total_1, daily_total_2);

	SERIAL_RECORD->record_activity(SERIAL_ACT_REBATE_RECHARGE, this->serial_rebate_map_.size());
	SERIAL_RECORD->record_activity(SERIAL_ACT_INVEST_RECHARGE, this->serial_invest_map_.size());

	this->serial_daily_map_.clear();
	this->serial_invest_map_.clear();
	this->serial_rebate_map_.clear();

	return 0;
}

int InvestRechargeSys::update_system()
{
//	this->mongo_timer_.cancel_timer();
//	this->mongo_timer_.schedule_timer(2 * Time_Value::MINUTE);

	MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	MMOInvestSystem::update_data(data_map);
	if (TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_SAVE_INVEST_SYSTEM, data_map) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}
	return 0;
}

InvestMap &InvestRechargeSys::invest_map()
{
	return this->invest_map_;
}

LongMap &InvestRechargeSys::serial_daily()
{
	return this->serial_daily_map_;
}

LongMap &InvestRechargeSys::serial_rebate()
{
	return this->serial_rebate_map_;
}

LongMap &InvestRechargeSys::serial_invest()
{
	return this->serial_invest_map_;
}

int InvestRechargeSys::update_serial_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30103102*, request, -1);

	Int64 id = request->id();
	Int64 value = request->value();
	switch (request->type())
	{
	case SERIAL_ACT_DAILY_RECHARGE:
	{
		this->serial_daily_map_[id] = value;
		break;
	}
	case SERIAL_ACT_REBATE_RECHARGE:
	{
		this->serial_rebate_map_[id] = value;
		break;
	}
	case SERIAL_ACT_INVEST_RECHARGE:
	{
		this->serial_invest_map_[id] = value;
		break;
	}
	}

	return 0;
}

int InvestRechargeSys::check_invest_reward(Int64 role_id)
{
//	InvestRechargeDetail &detail = this->player_detail(role_id);
//
//	const Json::Value &ir_json = CONFIG_INSTANCE->invest_recharge_json();
//	int vip_limit = ir_json["vip_level"].asInt();
//
//	for (int i = DAILY_1; i <= detail.__invest_index; ++i)
//	{
//		if (detail.__invest_rewards[i] == REWARD_NONE)
//		{
//			detail.__invest_rewards[i] = REWARD_HAVE;
//		}
//		if (detail.__vip_rewards[i] == REWARD_NONE &&
//				detail.__vip_level >= vip_limit)
//		{
//			detail.__vip_rewards[i] = REWARD_HAVE;
//		}
//	}
	return 0;
}

int InvestRechargeSys::send_mail_to_player(Int64 player_id)
{
//	const Json::Value &ir_json = CONFIG_INSTANCE->invest_recharge_json();
//	for (InvestMap::iterator it = this->invest_map_.begin(); it != this->invest_map_.end(); ++it)
//	{
//		Proto30103001 reward_info;
//
//		Int64 role_id = it->first;
//		JUDGE_CONTINUE(role_id == player_id);
//		int mail_id = ir_json["mail_id"].asInt();
//		int reward_num = 0;
//		for (int i = DAILY_1; i < TYPE_NUM; ++i)
//		{
//			if (it->second.__invest_rewards[i] == REWARD_HAVE)
//			{
//				int reward_id = ir_json["reward_id"][i - 1].asInt();
//				it->second.__invest_rewards[i] = REWARD_GONE;
//				reward_info.add_reward_list(reward_id);
//				reward_num++;
//			}
//
//			if (it->second.__vip_rewards[i] == REWARD_HAVE)
//			{
//				int reward_id = ir_json["vip_reward"][i - 1].asInt();
//				it->second.__vip_rewards[i] = REWARD_GONE;
//				reward_info.add_reward_list(reward_id);
//				reward_num++;
//			}
//		}
//
//		reward_info.set_act_type(mail_id);
//		reward_info.set_mail_id(mail_id);
//		reward_info.set_role_id(role_id);
//
//		if (reward_num > 0) SEND_ACTREWARD->send_player_act_reward(&reward_info);
//
//		if (it->second.__invest_index < DAILY_7 && it->second.__is_buy)
//			it->second.__invest_index++;
//
//		this->check_invest_reward(role_id);
//
//		MSG_USER("INVEST_SYS SEND %d %d %d", it->second.__invest_index,
//				it->second.__is_buy, it->second.__vip_level);
//
//		LogicPlayer* player = NULL;
//		if (LOGIC_MONITOR->find_player(role_id, player) == 0 && player != NULL)
//		{
//			Proto31403101 respond;
//			InvestRechargeDetail &detail = this->player_detail(role_id);
//			respond.set_role_id(role_id);
//			respond.set_buy_time(detail.__buy_time);
//			respond.set_get_reward(detail.__get_reward);
//			respond.set_invest_index(detail.__invest_index);
//			respond.set_is_buy(detail.__is_buy);
//			respond.set_reward_time(detail.__reward_time);
//			respond.set_vip_level(detail.__vip_level);
//
//			for (int i = DAILY_1; i < TYPE_NUM; ++i)
//			{
//				respond.add_invest_rewards(detail.__invest_rewards[i] + 1);
//				respond.add_vip_rewards(detail.__vip_rewards[i] + 1);
//			}
//			LOGIC_MONITOR->dispatch_to_scene(player, &respond);
//		}
//	}

//	this->mail_timer_.cancel_timer();
//	Int64 left_time = ::next_day(0,5).sec() - Time_Value::gettimeofday().sec();
//	this->mail_timer_.schedule_timer((int)left_time);
	return 0;
}

int InvestRechargeSys::send_mail_to_player(void)
{
//	const Json::Value &ir_json = CONFIG_INSTANCE->invest_recharge_json();
//	for (InvestMap::iterator it = this->invest_map_.begin(); it != this->invest_map_.end(); ++it)
//	{
//		Proto30103001 reward_info;
//
//		Int64 role_id = it->first;
//		int mail_id = ir_json["mail_id"].asInt();
//		int reward_num = 0;
//		for (int i = DAILY_1; i < TYPE_NUM; ++i)
//		{
//			if (it->second.__invest_rewards[i] == REWARD_HAVE)
//			{
//				int reward_id = ir_json["reward_id"][i - 1].asInt();
//				it->second.__invest_rewards[i] = REWARD_GONE;
//				reward_info.add_reward_list(reward_id);
//				reward_num++;
//			}
//
//			if (it->second.__vip_rewards[i] == REWARD_HAVE)
//			{
//				int reward_id = ir_json["vip_reward"][i - 1].asInt();
//				it->second.__vip_rewards[i] = REWARD_GONE;
//				reward_info.add_reward_list(reward_id);
//				reward_num++;
//			}
//		}
//
//		reward_info.set_act_type(mail_id);
//		reward_info.set_mail_id(mail_id);
//		reward_info.set_role_id(role_id);
//
//		if (reward_num > 0) SEND_ACTREWARD->send_player_act_reward(&reward_info);
//
//		if (it->second.__invest_index < DAILY_7 && it->second.__is_buy)
//			it->second.__invest_index++;
//		this->check_invest_reward(role_id);
//
//		LogicPlayer* player = NULL;
//		if (LOGIC_MONITOR->find_player(role_id, player) == 0 && player != NULL)
//		{
//			Proto31403101 respond;
//			InvestRechargeDetail &detail = this->player_detail(role_id);
//			respond.set_role_id(role_id);
//			respond.set_buy_time(detail.__buy_time);
//			respond.set_get_reward(detail.__get_reward);
//			respond.set_invest_index(detail.__invest_index);
//			respond.set_is_buy(detail.__is_buy);
//			respond.set_reward_time(detail.__reward_time);
//			respond.set_vip_level(detail.__vip_level);
//
//			for (int i = DAILY_1; i < TYPE_NUM; ++i)
//			{
//				respond.add_invest_rewards(detail.__invest_rewards[i] + 1);
//				respond.add_vip_rewards(detail.__vip_rewards[i] + 1);
//			}
//			LOGIC_MONITOR->dispatch_to_scene(player, &respond);
//		}
//	}
//
//	this->mail_timer_.cancel_timer();
//	Int64 left_time = ::next_day(0,5).sec() - Time_Value::gettimeofday().sec();
//	this->mail_timer_.schedule_timer((int)left_time);
	return 0;
}

void InvestRechargeSys::fina()
{
	this->update_system();
}

void InvestRechargeSys::init(void)
{
	MMOInvestSystem::load_data();
	MSG_USER("INVEST_RECHARGE %d %d", this->serial_daily_map_.size(),
			this->serial_rebate_map_.size());
}


