/*
 * LogicLuckyTable.h
 *
 *  Created on: Mar 23, 2016
 *      Author: zhangshaopeng
 */

#ifndef LOGIC_SERVER_LUCKYTABLE_MOUDLE_LOGICLUCKYTABLE_H_
#define LOGIC_SERVER_LUCKYTABLE_MOUDLE_LOGICLUCKYTABLE_H_

#include "BaseLogicPlayer.h"
struct LuckyTableDetial
{
	int left_times[2];//0消费转盘/1充值转盘
	int exec_times[2];
	int gold[2];
	Int64 activity_key;

	void reset();
};

class LogicLuckyTable : virtual public BaseLogicPlayer
{
public:
	typedef std::vector<ItemRate> RateSet;
	LogicLuckyTable();
	virtual ~LogicLuckyTable();
	void reset();
	LuckyTableDetial& fetch_ltable_detail();

public:
	int open_ltable(Message *msg);
	int close_ltable();
	int exec_ltable(Message *msg);

	int exec_ltable_after(Message *msg);
	int notify_ltable_state();
	int add_ltable_times(Message *msg);
	void check_and_reset_ltable();

private:
	int calc_rate_set(int type,RateSet& rate_set);
	int gen_reward_item(const RateSet& rate_set);
	void record_and_send_reward(Message *msg);
private:
	LuckyTableDetial detail_;
};


#endif /* LOGIC_SERVER_LUCKYTABLE_MOUDLE_LOGICLUCKYTABLE_H_ */
