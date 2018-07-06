/*
 * MLOnlineRewards.h
 *
 *  Created on: 2014-1-16
 *      Author: root
 */

#ifndef MLONLINEREWARDS_H_
#define MLONLINEREWARDS_H_

#include "MLPacker.h"
class MLOnlineRewards :virtual public MLPacker
{
public:
	class OnlineRewardsTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		void set_parent(MLOnlineRewards* online_rewards);
	private:
		MLOnlineRewards* online_rewards_;
	};

	class GetAwardTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		void set_parent(MLOnlineRewards* online_rewards);
	private:
		MLOnlineRewards* online_rewards_;
	};

public:
	MLOnlineRewards();
	virtual ~MLOnlineRewards();
	void reset();

	int login_online_rewards();
	int logout_online_rewards();
	int fetch_online_rewards();
	int get_online_rewards_info();		//可领取的物品 领取的倒数时间
	int notify_online_rewards();		//可领取奖励数量
	int check_rewards_timeout();
	int get_award_timeout();

	int check_online_pa_event();
	OnlineRewardsDetail& online_rewards_detail();

	IntVec& get_award_list();
	//for test
	void reset_today_stage();
	int test_online(void);
	int sync_transfer_online_rewards(int scene_id);
	int read_transfer_online_rewards(Message* msg);

private:
	int online_rewards_timer_start(int interval);
	int online_rewards_timer_stop();
	int next_stage_times();
	int now_rewards_stage();
	int pre_rewards_stage();
	int reset_stage();
	int add_stage();

	bool validate_online_rewards_level_limit(void);


private:
	OnlineRewardsTimer online_rewards_timer_;
	OnlineRewardsDetail online_rewards_detail_;
	GetAwardTimer get_award_timer_;
};

#endif /* MLONLINEREWARDS_H_ */
