/*
 * MayActivitySys.h
 *
 *  Created on: 2017年4月12日
 *      Author: lyw
 */

#ifndef MAYACTIVITYSYS_H_
#define MAYACTIVITYSYS_H_

#include "ActivityStruct.h"

class MayActivitySys : public MayActDetail
{
public:
	class BackActTimer : public GameTimer
	{
	public:
		virtual int type();
		virtual int handle_timeout(const Time_Value &tv);
	};


public:
	MayActivitySys();
	virtual ~MayActivitySys();
	void reset();
	int cur_may_day()const;

	void start(void);
	void stop(void);

	int request_load_activity();
	int after_load_activity_done(DBShopMode* shop_mode);
	void handle_sync_map_act();
	int midnight_handle_timeout(int test_day = -1);

	void init_may_activity();
	void init_cur_day_act();
	void update_activity_sys(int test_day = -1);

	int update_couple_tick(int sec);

	void init_today_red_packet_list();
	int rand_red_packet(int index);
	int rand_all_red_packet();

	IntMap& fetch_act_list();

	void save_activity();
	void load_activity();

	bool is_open_activity()const;
private:
	int cur_may_day_;		//当前五一活动过去天数

	IntMap act_list_;		//当前的活动
	BackActTimer act_timer_;

};

typedef Singleton<MayActivitySys> MayActivitySysSingle;
#define MAY_ACTIVITY_SYS	MayActivitySysSingle::instance()

#endif /* MAYACTIVITYSYS_H_ */
