/*
 * LogicOnline.cpp
 *
 * Created on: 2013-07-05 19:07
 *     Author: lyz
 */

#include "LogicOnline.h"
#include "PubStruct.h"

void LogicOnline::OnlineDetail::reset(void)
{
	this->__is_online = 0;
    this->__sign_in_tick = 0;
    this->__sign_out_tick = 0;

    this->__total_online_tick = 0;
    this->__day_online_tick = 0;
    this->__week_online_tick = 0;
    this->__month_online_tick = 0;
    this->__year_online_tick = 0;

    this->__day_refresh_tick = Time_Value::zero;
    this->__week_refresh_tick = Time_Value::zero;
    this->__month_refresh_tick = Time_Value::zero;
    this->__year_refresh_tick = Time_Value::zero;
}

void LogicOnline::reset(void)
{
    this->online_detail_.reset();
}

int LogicOnline::sign_in(void)
{
//    if (this->online_detail_.__sign_out_tick < this->online_detail_.__sign_in_tick)
//    {
//        this->online_detail_.__sign_out_tick = 0;
//    }

    this->online_detail_.__is_online = 1;
    this->online_detail_.__sign_in_tick = Time_Value::gettimeofday().sec();

    return 0;
}

int LogicOnline::sign_out(void)
{
	this->online_detail_.__is_online = 0;
    this->online_detail_.__sign_out_tick = Time_Value::gettimeofday().sec();

    this->refresh_day_online();
    this->refresh_week_online();
    this->refresh_month_online();
    this->refresh_year_online();

    int64_t online_second = this->online_second_from_login();
    this->online_detail_.__total_online_tick += online_second;
    this->online_detail_.__day_online_tick += online_second;
    this->online_detail_.__week_online_tick += online_second;
    this->online_detail_.__month_online_tick += online_second;
    this->online_detail_.__year_online_tick += online_second;

    return 0;
}

LogicOnline::OnlineDetail &LogicOnline::online_detail(void)
{
    return this->online_detail_;
}

int64_t LogicOnline::offline_second_from_logout(void)
{
    if (this->online_detail_.__sign_out_tick == 0)
        return 0;

    int sec = this->online_detail_.__sign_in_tick - this->online_detail_.__sign_out_tick;
    if (sec < 0)
        sec = 0;
    return sec;
}

int64_t LogicOnline::offline_day_from_logout(void)
{
	int64_t offline_time = this->offline_second_from_logout();
	if (offline_time == 0)
		return 0;

	const time_t logout_tick = this->online_detail_.__sign_out_tick;
	const time_t login_tick = this->online_detail_.__sign_in_tick;

	tm day_tm;
	localtime_r(&logout_tick, &day_tm);
	day_tm.tm_hour = 0;
	day_tm.tm_min  = 0;
	day_tm.tm_sec  = 0;
	time_t day_zero = ::mktime(&day_tm);

	time_t front_time = login_tick - day_zero;
	int last_day = front_time % 86400 > 0 ? 1 : 0;
	int total_day = front_time / 86400 + last_day - 1;
    if (total_day < 0)
    	total_day = 0;

	return total_day;
}

int64_t LogicOnline::online_second_from_login(void)
{
    int nowtime = time(0);
    nowtime -= this->online_detail_.__sign_in_tick;
    if (nowtime < 0)
        nowtime = 0;
    return nowtime;
}

int64_t LogicOnline::online_minute_from_login(void)
{
    return this->online_second_from_login() / 60;
}

int64_t LogicOnline::online_hour_from_login(void)
{
    return this->online_second_from_login() / 3600;
}

int64_t LogicOnline::total_online_second(void)
{
    return (this->online_detail_.__total_online_tick + this->online_second_from_login());
}

int64_t LogicOnline::total_online_minute(void)
{
    return this->total_online_second() / 60;
}

int64_t LogicOnline::total_online_hour(void)
{
    return this->total_online_second() / 3600;
}

int64_t LogicOnline::day_online_second(void)
{
    this->refresh_day_online();
    return (this->online_detail_.__day_online_tick + this->online_second_from_login());
}

int64_t LogicOnline::day_online_minute(void)
{
    return this->day_online_second() / 60;
}

int64_t LogicOnline::day_online_hour(void)
{
    return this->day_online_second() / 3600;
}

int64_t LogicOnline::week_online_second(void)
{
    this->refresh_week_online();
    return (this->online_detail_.__week_online_tick + this->online_second_from_login());
}

int64_t LogicOnline::week_online_minute(void)
{
    return this->week_online_second() / 60;
}

int64_t LogicOnline::week_online_hour(void)
{
    return this->week_online_second() / 3600;
}

int64_t LogicOnline::week_online_day(void)
{
    return this->week_online_second() / 86400;
}

int64_t LogicOnline::month_online_second(void)
{
    this->refresh_month_online();
    return (this->online_detail_.__month_online_tick + this->online_second_from_login());
}

int64_t LogicOnline::month_online_minute(void)
{
    return this->month_online_second() / 60;
}

int64_t LogicOnline::month_online_hour(void)
{
    return this->month_online_second() / 3600;
}

int64_t LogicOnline::month_online_day(void)
{
    return this->month_online_second() / 86400;
}

int64_t LogicOnline::year_online_second(void)
{
    this->refresh_year_online();
    return (this->online_detail_.__year_online_tick + this->online_second_from_login());
}

int64_t LogicOnline::year_online_minute(void)
{
    return this->year_online_second() / 60;
}

int64_t LogicOnline::year_online_hour(void)
{
    return this->year_online_second() / 3600;
}

int64_t LogicOnline::year_online_day(void)
{
    return this->year_online_second() / 86400;
}

void LogicOnline::refresh_day_online(void)
{
    Time_Value nowtime = Time_Value::gettimeofday();
    if (this->online_detail_.__day_refresh_tick < nowtime)
    {
        if (this->online_detail_.__sign_in_tick < this->online_detail_.__day_refresh_tick.sec())
        {
            int online_second = this->online_detail_.__day_refresh_tick.sec() - this->online_detail_.__sign_in_tick;
            this->online_detail_.__total_online_tick += online_second;
            this->online_detail_.__day_online_tick += online_second;
            this->online_detail_.__week_online_tick += online_second;
            this->online_detail_.__month_online_tick += online_second;
            this->online_detail_.__year_online_tick += online_second;
            this->online_detail_.__sign_in_tick = this->online_detail_.__day_refresh_tick.sec();
        }
        this->online_detail_.__day_refresh_tick = next_day(0, 0, nowtime);
        this->online_detail_.__day_online_tick = 0;
    }
}

void LogicOnline::refresh_week_online(void)
{
    Time_Value nowtime = Time_Value::gettimeofday();
    if (this->online_detail_.__week_refresh_tick < nowtime)
    {
        this->online_detail_.__week_refresh_tick = next_week(7, 0, 0, nowtime);
        this->online_detail_.__week_online_tick = 0;
    }
}

void LogicOnline::refresh_month_online(void)
{
    Time_Value nowtime = Time_Value::gettimeofday();
    if (this->online_detail_.__month_refresh_tick < nowtime)
    {
        this->online_detail_.__month_refresh_tick = next_month(1, 0, 0, nowtime);
        this->online_detail_.__month_online_tick = 0;
    }
}

void LogicOnline::refresh_year_online(void)
{
    Time_Value nowtime = Time_Value::gettimeofday();
    if (this->online_detail_.__year_refresh_tick < nowtime)
    {
        this->online_detail_.__year_refresh_tick = next_year(1, 1, nowtime);
        this->online_detail_.__year_online_tick = 0;
    }
}

