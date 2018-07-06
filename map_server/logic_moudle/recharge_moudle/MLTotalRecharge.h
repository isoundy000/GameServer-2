/*
 * MLTotalRecharge.h
 *
 *  Created on: Mar 24, 2015
 *      Author: jinxing
 */

#ifndef MLTOTALRECHARGE_H_
#define MLTOTALRECHARGE_H_

#include "MLPacker.h"

class MLTotalRecharge: virtual public MLPacker
{
	enum RewardState{
		REWARD_NONE = 0,
		REWARD_HAVE = 1,
		REWARD_GONE = 2	// 已领取
	};

public:
	MLTotalRecharge();
	virtual ~MLTotalRecharge();
	void reset();
	void init_total_recharge_states();

public:
	int get_total_recharge_reward(Message *msg);

public:
	void process_rechare_event();
	int notify_total_recharge_info();

	int total_recharge_broadcast(int step_idx);
	TotalRechargeDetail& total_recharge_detail();
	int sync_transfer_total_recharge(int scene_id);
	int read_transfer_total_recharge(Message *msg);

private:
	TotalRechargeDetail	detail_;
};

#endif /* MLTOTALRECHARGE_H_ */
