/*
 * MLBackRebateRechargeSys.h
 *
 *  Created on: 2016年11月9日
 *      Author: lzy
 */

#ifndef MLBACKREBATERECHARGESYS_H_
#define MLBACKREBATERECHARGESYS_H_
#include <ctime>
#include "Singleton.h"

class Transaction;

class MLBackRebateRechargeSys
{
public:
	MLBackRebateRechargeSys();
	virtual ~MLBackRebateRechargeSys();

public:
	void init(void);

	time_t start_tick();
	time_t end_tick();

	bool daily_recharge_opening(time_t now = 0);
	int check_and_notify_state_to_all(void);

public:
	int request_load_DR_open_time();
	int after_load_DR_open_time(Transaction* trans);
	int update_daily_recharge_open_time(time_t s, time_t e);

private:
	time_t	start_time_;
	time_t 	end_time_;
	bool activity_opening_;
};

typedef Singleton<MLBackRebateRechargeSys> MLBackRebateRechargeSysSingle;
#define BackRR_SYS (MLBackRebateRechargeSysSingle::instance())



#endif /* MLBACKREBATERECHARGESYS_H_ */
