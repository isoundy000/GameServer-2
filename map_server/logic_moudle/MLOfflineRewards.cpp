/*
 * MLOfflineAward.cpp
 *
 *  Created on: 2016年8月18日
 *      Author: lzy
 */


#include "MLOfflineRewards.h"
#include "ProtoDefine.h"
#include "GameHeader.h"
#include "MapLogicPlayer.h"

MLOfflineRewards::MLOfflineRewards()
{
	this->__offline_reward.reset();
}
MLOfflineRewards::~MLOfflineRewards()
{

}
void MLOfflineRewards::reset()
{
	this->__offline_reward.reset();
}


bool MLOfflineRewards::validate_offline_rewards_level_limit(void)
{
	int use_online_rewards_level = 1;
	return this->role_level() >= use_online_rewards_level;
}

int MLOfflineRewards::notify_offline_rewards(int level)
{
	int exp_base = CONFIG_INSTANCE->role_level(0,level)["offline_exp"].asInt();
	this->__offline_reward.__exp_num = exp_base * (this->__offline_reward.__offline_sum / 60);

	if (this->__offline_reward.__exp_num > 0)
	{
		MapLogicPlayer * player = dynamic_cast<MapLogicPlayer *>(this);
		player->update_player_assist_single_event(GameEnum::PA_EVENT_OFFLINE_AWARD, 1);
	}

	return 0;
}

int MLOfflineRewards::login_offline_rewards()
{
	JUDGE_RETURN(true == validate_offline_rewards_level_limit(), 0);
	JUDGE_RETURN(this->__offline_reward.__received_mark == 1, 0);

	Int64 now = Time_Value::gettimeofday().get_tv().tv_sec;
	if (this->__offline_reward.__logout_time == 0)
		this->__offline_reward.__logout_time = now;

	this->__offline_reward.__offline_sum += now
			- this->__offline_reward.__logout_time;

	if (this->__offline_reward.__offline_sum > this->__offline_reward.__longest_time)
		this->__offline_reward.__offline_sum = this->__offline_reward.__longest_time;

	if (this->__offline_reward.__offline_sum > 60)
		this->__offline_reward.__received_mark = 1;
	else
		this->__offline_reward.__received_mark = 0;

	return notify_offline_rewards(this->role_level());
}

int MLOfflineRewards::logout_offline_rewards()
{
	this->__offline_reward.__logout_time = ::time(NULL);
	this->__offline_reward.__received_mark = 1;
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_OFFLINE_REWARDS);
	return 0;
}

int MLOfflineRewards::offline_rewards_info()
{
	Proto51401480 respond;

	respond.set_exp_num(this->__offline_reward.__exp_num);
	respond.set_longest_time(this->__offline_reward.__longest_time);
	respond.set_offline_sum(this->__offline_reward.__offline_sum);
	respond.set_received_mark(this->__offline_reward.__received_mark);

	FINER_PROCESS_RETURN(RETURN_FETCH_OFFLINE_INFO, &respond);
}

int MLOfflineRewards::fetch_offline_rewards(Message* msg)
{
	CONDITION_NOTIFY_RETURN(1 == this->__offline_reward.__received_mark,RETURN_FETCH_OFFLINE_INFO, ERROR_CLIENT_OPERATE);
	MSG_DYNAMIC_CAST_RETURN(Proto11401481*, request, -1);

	int level_limit;
	long long exp_num;
	switch(request->type_id())
	{
		case 0:
			level_limit = OFFLINE_NORMAL_NUM;
			exp_num = this->__offline_reward.__exp_num;
			break;
		case 1:
			level_limit = OFFLINE_DOUBLE_VIP_NUM;
			exp_num = this->__offline_reward.__exp_num * 2;
			break;
		case 2:
			level_limit = OFFLINE_TRIPLE_VIP_NUM;
			exp_num = this->__offline_reward.__exp_num * 3;
	}

	CONDITION_NOTIFY_RETURN(this->vip_detail().__vip_type >= level_limit,
			RETURN_FETCH_OFFLINE_AWARD, ERROR_CLIENT_OPERATE);

	this->request_add_exp(exp_num , EXP_FROM_OFFLINE_REWARDS);
	this->__offline_reward.reset();
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_OFFLINE_REWARDS);

	Proto51401481 respond;
	respond.set_status(1);
	respond.set_exp_num(exp_num);

	MapLogicPlayer * player = dynamic_cast<MapLogicPlayer *>(this);
	player->update_player_assist_single_event(GameEnum::PA_EVENT_OFFLINE_AWARD, 0);

	FINER_PROCESS_RETURN(RETURN_FETCH_OFFLINE_INFO, &respond);
}


OfflineRewardsDetail& MLOfflineRewards::offline_rewards_detail()
{
	return this->__offline_reward;
}

int MLOfflineRewards::sync_transfer_offline_rewards(int scene_id)
{
	Proto31400180 temp_info;

	temp_info.set_exp_num(this->__offline_reward.__exp_num);
	temp_info.set_longest_time(this->__offline_reward.__longest_time);
	temp_info.set_offline_sum(this->__offline_reward.__offline_sum);
	temp_info.set_received_mark(this->__offline_reward.__received_mark);

	return this->send_to_other_logic_thread(scene_id, temp_info);
}

int MLOfflineRewards::read_transfer_offline_rewards(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400180*, request, -1);
	this->__offline_reward.reset();

	this->__offline_reward.__exp_num = request->exp_num();
	this->__offline_reward.__longest_time = request->longest_time();
	this->__offline_reward.__offline_sum = request->offline_sum();
	this->__offline_reward.__received_mark = request->received_mark();

	return 0;
}
