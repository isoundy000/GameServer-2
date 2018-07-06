/*
 * MLBackDailyRechargeSys.h
 *
 *  Created on: Nov 5, 2014
 *      Author: jinxing
 */

#ifndef MLBACKDAILYRECHARGESYS_H_
#define MLBACKDAILYRECHARGESYS_H_
#include <ctime>
#include "Singleton.h"
#include "MapLogicStruct.h"
#include "GameTimer.h"

class Transaction;
typedef std::map<Int64, DailyRechargeDetail> DailyMap;

class MLBackDailyRechargeSys
{
	class MailTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

	};

public:
	MLBackDailyRechargeSys();
	virtual ~MLBackDailyRechargeSys();

public:
	void init(void);
	void stop(void);

	time_t start_tick();
	time_t end_tick();

	bool daily_recharge_opening(time_t now = 0);
	int check_and_notify_state_to_all(void);

public:
	int request_load_DR_open_time();
	int after_load_DR_open_time(Transaction* trans);
	int update_daily_recharge_open_time(time_t s, time_t e);

	int update_player_info(Message* msg);
	int send_mail_to_player();
	int send_mail_to_player(Int64 player_id);
	int get_reward_id(Int64 role_id = 0, int type = 0);
	int get_json_id(Int64 role_id);

	DailyMap &daily_map();

private:
	time_t	start_time_;
	time_t 	end_time_;
	DailyMap daily_map_;
	MailTimer mail_timer_;
	bool activity_opening_;
};

typedef Singleton<MLBackDailyRechargeSys> MLBackDailyRechargeSysSingle;
#define BackDR_SYS (MLBackDailyRechargeSysSingle::instance())

#endif /* MLBACKDAILYRECHARGESYS_H_ */
