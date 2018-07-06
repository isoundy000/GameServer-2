/*
 * GameTimer.cpp
 *
 * Created on: 2013-01-21 12:17
 *     Author: glendy
 */

#include "GameTimer.h"
#include "PubStruct.h"
#include "PoolMonitor.h"
#include "Log.h"

GameTimer::GameTimer(void) : 
    is_registered_(false)
{ /*NULL*/ }

GameTimer::~GameTimer(void)
{ /*NULL*/ }

int GameTimer::timeout(const Time_Value &nowtime)
{
    if (this->check_tick_ <= nowtime)
    {
        this->handle_timeout(nowtime);

        int loop = 0;
        while (this->check_tick_ <= nowtime && (++loop) <= 10)
        {
//			long check_tick = this->check_tick_.sec() * 1000000 + this->check_tick_.usec(),
//					now_tick = nowtime.sec() * 1000000 + nowtime.usec(),
//					interval_tick = this->interval_tick_.sec() * 1000000 + this->interval_tick_.usec();
//			long next_check = interval_tick;
//			if (interval_tick > 0)
//				 next_check -= ((now_tick - check_tick) % interval_tick);
			this->check_tick_ += this->interval_tick_;
        }
    }
    return 0;
}

int GameTimer::schedule_timer(int interval)
{
	return this->schedule_timer(Time_Value(interval));
}

int GameTimer::schedule_timer(double interval)
{
	Time_Value tv = Time_Value::gettime(interval);
	return this->schedule_timer(tv);
}

int GameTimer::schedule_timer(const Time_Value &interval)
{
    if (interval == Time_Value::zero)
    {
        MSG_USER("ERROR interval zero");
        return -1;
    }
//    if (this->is_registered_ == true)
//    {
//        MSG_USER("ERROR timer reg flag TRUE %d", this->type());
//        return -1;
//    }

//    this->check_tick_ = Time_Value::gettimeofday() + interval;
    this->interval_tick_ = interval;

    this->is_registered_ = true;
    if (this->register_timer_set() != 0)
    {
        MSG_USER("ERROR timer set repeat %d", this->type());
        return -1;
    }

    return 0;
}

int GameTimer::cancel_timer(void)
{
    if (this->is_registered_ == false)
    {
//        MSG_DEBUG("ERROR timer reg flag FALSE %d", this->type());
        return -1;
    }

    this->unregister_timer_set();
    this->is_registered_ = false;
    return 0;
}

int GameTimer::left_second(void)
{
	JUDGE_RETURN(this->is_registered() == true, 0);

	Time_Value diff = this->check_tick_  - Time_Value::gettimeofday();
	if (diff.sec() > 0)
	{
		return diff.sec();
	}
	else
	{
		return 0;
	}
}

int GameTimer::refresh_tick(const Time_Value &nowtime, const Time_Value &interval)
{
    this->interval_tick_ = interval;
    return 0;
}

int GameTimer::refresh_check_tick(double check_time)
{
	this->check_tick_ = Time_Value::gettimeofday() + Time_Value::gettime(check_time);
	return 0;
}

bool GameTimer::is_registered(void)
{
    return this->is_registered_;
}

const Time_Value &GameTimer::check_tick(void) const
{
	return this->check_tick_;
}

void GameTimer::set_check_tick(const Time_Value &tick)
{
    this->check_tick_ = tick;
}

const Time_Value &GameTimer::interval_tick(void) const
{
	return this->interval_tick_;
}

int GameTimer::register_timer_set(void)
{
    return POOL_MONITOR->register_game_timer(this);
}

int GameTimer::unregister_timer_set(void)
{
    return POOL_MONITOR->unregister_game_timer(this);
}

bool GameTimerCmp::operator()(GameTimer *&left, GameTimer *&right)
{
    return left->check_tick() < right->check_tick();
}


