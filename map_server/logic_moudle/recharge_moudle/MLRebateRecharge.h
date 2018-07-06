/*
 * MLRebateRecharge.h
 *
 *  Created on: 2016年11月9日
 *      Author: lzy
 */

#ifndef MLREBATERECHARGE_H_
#define MLREBATERECHARGE_H_

#include "MLPacker.h"

class MLRebateRecharge: public virtual MLPacker
{
public:
	class ShowTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		void set_parent(MLRebateRecharge* parent);
	private:
		MLRebateRecharge* parent_;
	};

	MLRebateRecharge();
	virtual ~MLRebateRecharge();
	void reset();
	void init_rebate_recharge_rewards();

public:
	int fetch_rebate_recharge_info();
	int notify_rebate_recharge_info(int show_icon = -1);
	int fetch_rebate_recharge_rewards();

	RebateRechargeDetail& rebate_recharge_dtail();
	int sync_transfer_rebate_recharge(int scene_id);
	int read_transfer_rebate_recharge(Message *msg);

private:
	RebateRechargeDetail rebate_recharge_dtail_;
	ShowTimer show_timer_;
};


#endif /* MLREBATERECHARGE_H_ */
