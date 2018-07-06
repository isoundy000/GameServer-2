/*
 * GameTimerHandler.h
 *
 * Created on: 2013-01-21 10:13
 *     Author: glendy
 */

#ifndef _GAMETIMERHANDLER_H_
#define _GAMETIMERHANDLER_H_

#include "Event_Handler.h"

class BaseUnit;

class GameTimerHandler : public Event_Handler
{
public:
    GameTimerHandler(void);
    virtual ~GameTimerHandler(void);

    virtual void set_type(const int type);
    virtual int type(void);
    virtual void set_interval(const Time_Value &interval);
    virtual Time_Value &interval(void);

    virtual int handle_timeout(const Time_Value &tv);
    virtual int schedule_timer(Time_Value &interval);
    virtual int cancel_timer(void);

protected:
    virtual BaseUnit *logic_unit(void) = 0;

private:
    int handler_type_;
    Time_Value interval_;
};

#endif //_GAMETIMERHANDLER_H_
