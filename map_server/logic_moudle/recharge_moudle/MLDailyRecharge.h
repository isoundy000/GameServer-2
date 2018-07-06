/*
 * MLDailyRecharge.h
 *
 *  Created on: Nov 3, 2014
 *      Author: jinxing
 */

#ifndef MLDAILYRECHARGE_H_
#define MLDAILYRECHARGE_H_

#include "MLPacker.h"

class MLDailyRecharge: public virtual MLPacker
{
public:
	enum RewardState{
		REWARD_NONE = 0,
		REWARD_HAVE = 1,
		REWARD_GONE = 2	// 已领取
	};
	enum RewardType{
		DAILY_FIRST		= 0, //每日首充
		DAILY_TOTAL		= 1, //每日累充
		TOTAL_EXT		= 2, //每日累充额外奖励1
		TYPE_NUM
	};

	enum LabelNum{
		ACT_REACHARGE_NUM = 10000,
		ACT_REACHARGE_TIMES = 3,
		ACT_OPEN_DAY = 7,
		ACT_MAIL_ID = 20011,
		ACT_FIRST_LIMIT = 10,
		ACT_TOTAL_LIMIT = 99,
		ACT_CAST_ID	= 101006,
		ACT_LABEL_ID = 1030015,
		ACT_NEW_RECHARGE_NUM = 30000,
	};

public:
	MLDailyRecharge();
	virtual ~MLDailyRecharge();
	void reset();
	void init_daily_recharge_rewards();
	void daily_recharge_clean();

public:
	int handle_recharge_event(size_t gold);
	int check_get_label_event();

	int fetch_daily_recharge_info();
	int notify_daily_recharge_info(int show_icon = -1);
	int update_daily_recharge_to_sys();
	int fetch_daily_recharge_rewards(Message *msg);

	bool is_daily_recharge_during();
	DailyRechargeDetail& daily_recharge_dtail();
	int sync_transfer_daily_recharge(int scene_id);
	int read_transfer_daily_recharge(Message *msg);

	int get_json_id();
	int get_reward_id(int type);
	int get_gold_num(int type);
	int get_index_num();
private:
	DailyRechargeDetail daily_recharge_dtail_;
};

#endif /* MLDAILYRECHARGE_H_ */
