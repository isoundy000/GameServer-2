/*
 * MLCollectChests.h
 *
 *  Created on: 2016年8月3日
 *      Author: lzy0927
 */

#ifndef MLCOLLECTCHESTS_H_
#define MLCOLLECTCHESTS_H_

#include "MLPacker.h"
#include "MapStruct.h"

struct Collect_Chests
{
	int cycle_id;
	int collect_num;
	int reset_tick;

	int cur_year;
	int cur_mouth;
	int cur_day;

	void reset(void);
	Collect_Chests(void);
};



class MLCollectChests: virtual public MLPacker
{
public:
	class CheckStartTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		void set_act_type(int type);

		int act_type_;
		MLCollectChests* parent_;
	};
	typedef std::map<int, CheckStartTimer>CheckStartTimerMap;
public:
	MLCollectChests();
	virtual ~MLCollectChests();
	void reset();
	Collect_Chests& collect_chests();
	IntSet& player_chests_list();
	IntVec& chests_award_list();
	int is_on_activity(int cycle_id, int year, int mouth, int day);
	int login_collect_chests();
	int logout_collect_chests();

	int reset_player_chests_info(Message* msg);

	int fetch_collect_chests_info(Message* msg);
	int activity_start_timeout();

	int notify_collect_info();
	int notify_get_extra_award();

	void new_start();
	void new_stop();
	int handle_chests_timeout(int type);
	int handle_chests_i(int state);
	int ahead_chests_event();
	ActivityTimeInfo &get_chests_time_info();
	int get_chests_cycle_id();
	//for test
//	void reset_today();

	int sync_transfer_collect_chests(int scene_id);
	int read_transfer_collect_chests(Message* msg);

	bool validate_collect_chests_level_limit(void);

private:
	int activity_start_timer_start(int interval);
	int collect_chests_timer_start(int interval);

	int activity_start_timer_stop();

	int next_start_times();
	int next_chests_times();


private:
	Collect_Chests  __collect_chests;
	CheckStartTimerMap check_start_timer_map;
	TimeInfoMap time_info_map;
//	ActivityStartTimer __activity_start_timer;
};
#endif /* MLCOLLECTCHESTS_H_ */
