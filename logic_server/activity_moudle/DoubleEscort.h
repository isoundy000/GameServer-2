/*
 * DoubleEscort.h
 *
 *  Created on: 2016年10月25日
 *      Author: lzy
 */

#ifndef DOUBLEESCORT_H_
#define DOUBLEESCORT_H_


#include "ActivityTipsSystem.h"

struct EscortDetail
{
	int cycle_id;
	int open;
	Int64 start_time;
	Int64 stop_time;
	LongSet player_list;
	LongMap gold_list;
	LongMap item_list;

	Int64 daily_gold;
	EscortDetail(){ EscortDetail::reset();}
	void reset()
	{
		this->cycle_id = GameEnum::ESCORT_ACTIVITY_FIRST_ID ;
		this->start_time = 0;
		this->stop_time = 0;
		this->open = 0;
		this->player_list.clear();
		this->gold_list.clear();
		this->item_list.clear();
		this->daily_gold = 0;
	}
};

class DoubleEscort
{
public:
	class CheckStartTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		void set_act_type(int type);
		int act_type;
	};
	typedef std::map<int, CheckStartTimer>CheckStartTimerMap;

	class RefreshTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

	};

public:
	DoubleEscort();
	virtual ~DoubleEscort();

	ActivityTimeInfo &get_time_info();
	void new_start();
	void init_escort();
	void new_stop();

	int handle_timeout(int type);
	int handle_escort_i(int state);
	int ahead_escort_event();

	int get_open();
	int get_cycle_id();

	void test_escort(int id, int set_time);

	int update_serial_info(Message* msg);
	int update_daily_gold(Int64 role_id, Int64 gold, Int64 item_id, Int64 item_amount);
	int every_day_serial_work();
	int escort_serial_work(Int64 role_id);

private:
	EscortDetail escort_detail_;
	TimeInfoMap time_info_map;
	CheckStartTimerMap check_start_timer_map;
	RefreshTimer refresh_timer_;

};

typedef Singleton<DoubleEscort> DoubleEscortSingleton;
#define DOUBLE_ESCORT  DoubleEscortSingleton::instance()


#endif /* DOUBLEESCORT_H_ */
