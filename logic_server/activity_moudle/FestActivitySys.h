/*
 * FestActivitySys.h
 *
 *  Created on: Jan 10, 2017
 *      Author: peizhibi
 */

#ifndef FESTACTIVITYSYS_H_
#define FESTACTIVITYSYS_H_

#include "ActivityStruct.h"

/*
 * 节日活动
 * */
class FestActivitySys : public BackSetActDetail
{
public:
	class LoginActTimer : public GameTimer
	{
	public:
		virtual int type();
		virtual int handle_timeout(const Time_Value &tv);
	};

	class BossActTimer : public GameTimer
	{
	public:
		virtual int type();
		virtual int handle_timeout(const Time_Value &tv);
	};

	class MapSyncTimer : public GameTimer
	{
	public:
		virtual int type();
		virtual int handle_timeout(const Time_Value &tv);
	};

public:
	FestActivitySys();
	virtual ~FestActivitySys();
	virtual int after_load_activity(DBShopMode* shop_mode);

	BackSetActDetail::ActTypeItem* find_fest_activity(int type);

	int start(void);
	int stop(void);
	int handle_login_timer();
	int handle_boss_timer();
	int handle_map_sync_timer();
	int midnight_handle_timeout(int test_day = -1);

	int icon_type();
	int left_activity_time();

	int is_activity_time();
	int is_login_timer_state();
	IntMap& fetch_act_list();

	int request_festival_time_begin();
	int request_festival_time_done(DBShopMode* shop_mode);
	int handle_scene_boss_info(Message* msg);
	int record_player_hurt_map(Message* msg);
	int handle_boss_kill(Message* msg);

	void init_festival_activity();
	void init_cur_day_act();
	void start_login_timer();
	void start_boss_timer();
	void start_sync_timer();

	void start_boss();
	void shout_boss();
	void recycle_boss();

	void finish_cur_day_act();
	void update_activity_sys();
	void notify_activity_state(LogicPlayer* player = NULL);

private:
	int boss_index_;
	int last_boss_index_;
	IntMap act_list_;		//当前的活动

	FestivalInfo fest_info_;
	MapSyncTimer map_sync_timer_;

	ActivityTimeInfo login_time_;
	ActivityTimeInfo boss_time_;

	LoginActTimer login_timer_;
	BossActTimer boss_timer;
};

typedef Singleton<FestActivitySys> FestActivitySysSingle;
#define FEST_ACTIVITY_SYS	FestActivitySysSingle::instance()

#endif /* FESTACTIVITYSYS_H_ */
