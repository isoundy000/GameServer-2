/*
 * MLTotalRecharge.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: jinxing
 */

#include "MLTotalRecharge.h"
#include "ProtoDefine.h"
#include "MapLogicPlayer.h"
//#include "MapMonitor.h"

MLTotalRecharge::MLTotalRecharge()
{
	/**/
}

MLTotalRecharge::~MLTotalRecharge()
{
	/**/
}

void MLTotalRecharge::reset()
{
	detail_.reset();
	this->init_total_recharge_states();
}

void MLTotalRecharge::init_total_recharge_states()
{
	JUDGE_RETURN(detail_.__reward_states.size() == 0, );

	const  Json::Value &rewards_json = CONFIG_INSTANCE->total_recharge()["rewards"];
	detail_.__reward_states.reserve(rewards_json.size());
	for(size_t i=0; i<rewards_json.size(); i++)
	{
		detail_.__reward_states.push_back(REWARD_NONE);
	}
}

int MLTotalRecharge::get_total_recharge_reward(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto11401805*, req, msg, -1);

	size_t reward_idx = req->reward_index();
	CONDITION_NOTIFY_RETURN(
			reward_idx >= 0 && reward_idx < detail_.__reward_states.size(),
			RETURN_FETCH_TOTAL_RECHARGE_REWARDS, ERROR_CLIENT_OPERATE);

	int  state = detail_.__reward_states[reward_idx];
	if(state == REWARD_NONE)
		return this->respond_to_client_error(RETURN_FETCH_TOTAL_RECHARGE_REWARDS, ERROR_NO_REWARD);
	if(state == REWARD_GONE)
		return this->respond_to_client_error(RETURN_FETCH_TOTAL_RECHARGE_REWARDS, ERROR_REWARD_DRAWED);

	if(state == REWARD_HAVE)
	{
		// 插入奖励物品到背包
		const  Json::Value &rewards_json = CONFIG_INSTANCE->total_recharge()["rewards"];
		CONDITION_NOTIFY_RETURN(rewards_json.size() > reward_idx,
				RETURN_FETCH_DAILY_RECHARGE_REWARDS, ERROR_CONFIG_ERROR);
		const Json::Value &reward_json = rewards_json[reward_idx];
		const Json::Value* items_json = NULL;
		if(reward_json.isMember("unisex"))
		{
			items_json = &(reward_json["unisex"]);
		}
		else if(this->role_detail().__sex == GameEnum::SEX_FEMALE)
		{
			items_json = &(reward_json["female"]);
		}
		else
		{
			items_json = &(reward_json["male"]);
		}

		ItemObjVec award_set;
		award_set.reserve(items_json->size());
		for(Json::Value::iterator it = items_json->begin(); it != items_json->end(); it++)
		{
			ItemObj tmp_item;
			tmp_item.id_ = (*it)[0u].asInt();
			tmp_item.amount_ = (*it)[1u].asInt();
			tmp_item.bind_ = (*it)[2u].asInt();
			award_set.push_back(tmp_item);
		}

		int ret = insert_package(SerialObj(ITEM_TOTAL_RECHARGE_REWARD, 0,
						reward_json["target"].asInt()), award_set);
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_FETCH_TOTAL_RECHARGE_REWARDS, ret);

		detail_.__reward_states[reward_idx] = REWARD_GONE;
		this->cache_tick().update_cache(MapLogicPlayer::CACHE_RECHARGE_REWARDS, true);
		total_recharge_broadcast(reward_idx);
	}
	else
	{
		return this->respond_to_client_error(RETURN_FETCH_TOTAL_RECHARGE_REWARDS, ERROR_SERVER_INNER);
	}

	return notify_total_recharge_info();
}


void MLTotalRecharge::process_rechare_event()
{
	this->init_total_recharge_states();

	const  Json::Value &rewards_json = CONFIG_INSTANCE->total_recharge()["rewards"];
	Int64 total_recharge = this->total_recharge_gold();
	bool new_reward = false;

	for(size_t i=0; i<rewards_json.size(); i++)
	{
		if(i >= detail_.__reward_states.size())
			detail_.__reward_states.push_back(REWARD_NONE);

		if(detail_.__reward_states[i] == REWARD_NONE &&
				total_recharge >= rewards_json[i]["target"].asInt())
		{
			detail_.__reward_states[i] = REWARD_HAVE;
			new_reward = true;
		}
	}

	// 总冲值有变化就需要通知
	notify_total_recharge_info();
	if(new_reward)
	{
		this->cache_tick().update_cache(MapLogicPlayer::CACHE_RECHARGE_REWARDS, true);
	}
}

int MLTotalRecharge::notify_total_recharge_info()
{
	this->init_total_recharge_states();

	const  Json::Value &conf_json = CONFIG_INSTANCE->total_recharge();
	const  Json::Value &limit_json = conf_json["level_limit"];
	bool show_icon = (limit_json[0u].asInt() <= this->role_level() &&
			this->role_level() <= limit_json[1u].asInt());

	Proto81401804 notify_msg;
	IntVec& states = detail_.__reward_states;
	size_t target_idx = 0;
	while(target_idx < states.size() && states[target_idx] != REWARD_NONE)
		target_idx++;
	if(target_idx >= states.size())
	{
		target_idx--;
		// 所有奖励已达成，且没有未领取的奖励
		bool no_reward = (std::find(states.begin(), states.end(), REWARD_HAVE) == states.end());
		show_icon = no_reward ? false : true;
	}

	if(target_idx >= conf_json["rewards"].size())
	{
		MSG_USER("ERROR: target_idx %d", target_idx);
		return ERROR_CONFIG_ERROR;
	}
	Int64 target = conf_json["rewards"][target_idx]["target"].asInt();

	notify_msg.set_total(this->total_recharge_gold());
	notify_msg.set_target(target);

	for(size_t i=0; i<states.size(); i++)
	{
		notify_msg.add_reward_states(states[i]);
	}

	notify_msg.set_show_icon(show_icon);
//	MSG_DEBUG("Proto81401804: %s", notify_msg.Utf8DebugString().c_str());
	return this->respond_to_client(ACTIVE_NOTIFY_TOTAL_RECHARGE_INFO, &notify_msg);
}


TotalRechargeDetail& MLTotalRecharge::total_recharge_detail()
{
	return detail_;
}

int MLTotalRecharge::sync_transfer_total_recharge(int scene_id)
{
	Proto31400135 sync_msg;

	IntVec& states = detail_.__reward_states;
	for(size_t i=0; i<states.size(); i++)
	{
		sync_msg.add_reward_states(states[i]);
	}

	return this->send_to_other_logic_thread(scene_id, sync_msg);
}

int MLTotalRecharge::read_transfer_total_recharge(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400135*, sync_msg, -1);

	IntVec& states = detail_.__reward_states;
	states.clear();
	states.reserve(sync_msg->reward_states_size());
	for(int i=0; i<sync_msg->reward_states_size(); i++)
	{
		states.push_back(sync_msg->reward_states(i));
	}

	return 0;
}

int MLTotalRecharge::total_recharge_broadcast(int step_idx)
{
//	IntVec& states = detail_.__reward_states;
//	JUDGE_RETURN(step_idx >= 0 && step_idx < (int)states.size(), ERROR_SERVER_INNER);
//
//	const  Json::Value &rewards_json = CONFIG_INSTANCE->total_recharge()["rewards"];
//	JUDGE_RETURN(step_idx < (int)rewards_json.size(), ERROR_CONFIG_ERROR);
//
//	int target_value = rewards_json[step_idx]["target"].asInt();
//
//	BrocastParaVec para_vec;
//	GameCommon::push_brocast_para_role_detail(para_vec,
//			this->role_id(), this->name(), false);
//	GameCommon::push_brocast_para_int(para_vec, target_value);
//
//	return this->monitor()->announce_world(SHOUT_ALL_TOTAL_RECHARGE, para_vec);
	return 0;
}
