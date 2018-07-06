/*
 * LogicOnline.h
 *
 * Created on: 2013-07-05 17:04
 *     Author: lyz
 */

#ifndef _LOGICONLINE_H_
#define _LOGICONLINE_H_

#include <stdint.h>
#include "Time_Value.h"

class LogicOnline
{
public:
    struct OnlineDetail
    {
    	int __is_online;
        int __sign_in_tick;
        int __sign_out_tick;
        
        int64_t __total_online_tick;
        int64_t __day_online_tick;
        int64_t __week_online_tick;
        int64_t __month_online_tick;
        int64_t __year_online_tick;

        Time_Value __day_refresh_tick;
        Time_Value __week_refresh_tick;
        Time_Value __month_refresh_tick;
        Time_Value __year_refresh_tick;
        void reset(void);
    };

public:
    void reset(void);
    int sign_in(void);
    int sign_out(void);

    OnlineDetail &online_detail(void);

    // 上次离线到这次登录的离线时间，异常离线的情况下没有离线时间
    int64_t offline_second_from_logout(void);
    int64_t offline_day_from_logout(void);

    // 这次登录到现在的在线时间
    int64_t online_second_from_login(void);
    int64_t online_minute_from_login(void);
    int64_t online_hour_from_login(void);

    // 从创建号开始到现在的总在线时间
    int64_t total_online_second(void);
    int64_t total_online_minute(void);
    int64_t total_online_hour(void);

    // 今天总的在线时间，每天零晨清零
    int64_t day_online_second(void);
    int64_t day_online_minute(void);
    int64_t day_online_hour(void);

    // 本周总的在线时间，每周日零晨清零
    int64_t week_online_second(void);
    int64_t week_online_minute(void);
    int64_t week_online_hour(void);
    int64_t week_online_day(void);

    // 本月总的在线时间，每月1日零晨清零
    int64_t month_online_second(void);
    int64_t month_online_minute(void);
    int64_t month_online_hour(void);
    int64_t month_online_day(void);

    // 本年总的在线时间，每年1月1日零晨清零
    int64_t year_online_second(void);
    int64_t year_online_minute(void);
    int64_t year_online_hour(void);
    int64_t year_online_day(void);

protected:
    void refresh_day_online(void);
    void refresh_week_online(void);
    void refresh_month_online(void);
    void refresh_year_online(void);

protected:
    OnlineDetail online_detail_;
};

#endif //_LOGICONLINE_H_
