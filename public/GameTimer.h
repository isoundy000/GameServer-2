/*
 * GameTimer.h
 *
 * Created on: 2013-01-21 10:12
 *     Author: glendy
 */

#ifndef _GAMETIMER_H_
#define _GAMETIMER_H_

#include "Time_Value.h"
#include "Heap.h"

class GameTimer : public HeapNode
{
public:
    GameTimer(void);
    virtual ~GameTimer(void);
    int timeout(const Time_Value &tv);

    int schedule_timer(int interval);
    int schedule_timer(double interval);
    int schedule_timer(const Time_Value &interval);

    int cancel_timer(void);
    int left_second(void);
    int refresh_tick(const Time_Value &nowtime, const Time_Value &interval);
    int refresh_check_tick(double check_time);
    bool is_registered(void);

    const Time_Value &check_tick(void) const;
    const Time_Value &interval_tick(void) const;

    void set_check_tick(const Time_Value &tick);

public:
    virtual int type(void) = 0;
    virtual int handle_timeout(const Time_Value &tv) = 0;

protected:
    virtual int register_timer_set(void);
    virtual int unregister_timer_set(void);

private:
    bool is_registered_;
    Time_Value check_tick_;
    Time_Value interval_tick_;
};

class GameTimerCmp
{
public:
    bool operator()(GameTimer *&left, GameTimer *&right);
};

#endif //_GAMETIMER_H_
