/*
 * InvestRechargeSys.h
 *
 *  Created on: 2016年11月9日
 *      Author: lzy
 */

#ifndef InvestRechargeSys_H_
#define InvestRechargeSys_H_

#include "LogicStruct.h"

class InvestRechargeSys
{
public:
	enum DailyInfo
	{
		TYPE_NUM_1 = 10,
		TYPE_NUM_2 = 99,

		INFO_END
	};


	class MailTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

	};

public:
	InvestRechargeSys();
	virtual ~InvestRechargeSys();

	void init(void);
	void fina(void);

public:
	InvestRechargeDetail &player_detail(Int64 role_id);

	int send_mail_to_player();
	int send_mail_to_player(Int64 role_id);
	int check_invest_reward(Int64 role_id);

	InvestMap &invest_map();
	LongMap &serial_daily();
	LongMap &serial_rebate();
	LongMap &serial_invest();

	int update_serial_info(Message* msg);

	int update_system();
	int every_day_serial_work();

private:
	InvestMap invest_map_;

	LongMap serial_daily_map_;
	LongMap serial_rebate_map_;
	LongMap serial_invest_map_;
};

typedef Singleton<InvestRechargeSys> InvestRechargeSysSingle;
#define BackIR_SYS (InvestRechargeSysSingle::instance())



#endif /* InvestRechargeSys_H_ */
