/*
 * QuintupleOnline.h
 *
 *  Created on: 2016年9月8日
 *      Author: lzy
 */

#ifndef QUINTUPLEONLINE_H_
#define QUINTUPLEONLINE_H_

#include "ActivityTipsSystem.h"

struct QuintupleDetail
{
	int cycle_id;
	int open;
	Int64 start_time;
	Int64 stop_time;

	LongMap player_exp;
	QuintupleDetail(){QuintupleDetail::reset();}
	void reset()
	{
		this->cycle_id = GameEnum::QUINTUPLE_ACTIVITY_ID ;
		this->start_time = 0;
		this->stop_time = 0;
		this->open = 0;
		this->player_exp.clear();
	}
};

class QuintupleOnline
{
public:
	class CheckStartTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
	};
	typedef std::map<int, CheckStartTimer>CheckStartTimerMap;

public:
	QuintupleOnline();
	virtual ~QuintupleOnline();
	static QuintupleOnline* instance();

	ActivityTimeInfo &get_time_info();
	void new_start();
	void init_quintuple();
	void new_stop();

	int handle_timeout();
	int handle_quintuple_i(int state);
	int ahead_quintuple_event();

	int update_monster_exp_info(Int64 role_id, int exp);
	Int64 get_exp_info(Int64 role_id);
	void test_quintuple(int id, int set_time);
private:
	QuintupleDetail quintuple_detail_;
	ActivityTimeInfo time_info_;
	CheckStartTimer check_start_timer_;
};

#define QUINTUPLE_SYS		QuintupleOnline::instance()

#endif /* QUINTUPLEONLINE_H_ */
