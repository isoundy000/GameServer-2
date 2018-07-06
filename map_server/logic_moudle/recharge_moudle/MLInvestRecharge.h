/*
 * MLInvestRecharge.h
 *
 *  Created on: 2016年11月11日
 *      Author: lzy
 */

#ifndef MLINVESTRECHARGE_H_
#define MLINVESTRECHARGE_H_

#include "MLPacker.h"

class MLInvestRecharge: public virtual MLPacker
{
public:
	MLInvestRecharge();
	virtual ~MLInvestRecharge();

	void reset();
	void set_invest_test_time(int days);
	int reset_invest_everyday();
	int fetch_left_invest_time();
	int is_have_invest_reward();
	int is_show_invest_icon();

	int fetch_invest_recharge_info();
	int notify_invest_recharge_info();
	int notify_cancel_invest_icon();
	int check_and_notify_invest_icon();
	int fetch_invest_recharge_rewards(Message *msg);

	int check_invest_normal_reward();
	int check_invest_vip_reward();
	int buy_invest_recharge();
	int fetch_invest_rewards(int index, int type);

	InvestRechargeDetail& invest_recharge_detail();
	int sync_transfer_invest_recharge(int scene_id);
	int read_transfer_invest_recharge(Message *msg);

private:
	InvestRechargeDetail invest_detail_;
};



#endif /* MLINVESTRECHARGE_H_ */
