/*
 * DailyActSys.h
 *
 *  Created on: 2017年3月21日
 *      Author: lyw
 */

#ifndef DAILYACTSYS_H_
#define DAILYACTSYS_H_

#include "LogicStruct.h"

class DailyActSys : public DailyActivity
{
public:
	class BackActivityTimer : public GameTimer
	{
	public:
		virtual int type();
		virtual int handle_timeout(const Time_Value &tv);
	};

public:
	DailyActSys();
	virtual ~DailyActSys();
	void reset();

	void start(void);
	void stop(void);

	int request_load_activity();
	int after_load_activity_done(DBShopMode* shop_mode);
	int check_update_activity();
	int check_delete_activity(IntMap &type_map);
	int update_activity_list();

	int midnight_handle_timeout();

	IntMap& fetch_all_activity();
	void save_activity();

	int total_double_update_flag();
	int check_total_double_is_update(Message* msg);

protected:
	void update_total_double();	//检测全民双倍开关
	int fetch_script_type(int value);

private:
	int has_update_;

	IntMap act_map_; //当前的活动
	BackActivityTimer act_timer_;

};

typedef Singleton<DailyActSys> DailyActSysSingle;
#define DAILY_ACT_SYSTEM 	DailyActSysSingle::instance()

#endif /* DAILYACTSYS_H_ */
