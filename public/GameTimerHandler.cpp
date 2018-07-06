/*
 * GameTimerHandler.cpp
 *
 * Created on: 2013-01-21 10:20
 *     Author: glendy
 */

#include "GameTimerHandler.h"
#include "PubStruct.h"
#include "BaseUnit.h"
#include "PoolMonitor.h"
#include "Epoll_Watcher.h"

GameTimerHandler::GameTimerHandler(void) :
    handler_type_(0)
{ /*NULL*/ }

GameTimerHandler::~GameTimerHandler(void)
{ /*NULL*/ }

void GameTimerHandler::set_type(const int type)
{
    this->handler_type_ = type;
}

int GameTimerHandler::type(void)
{
    return this->handler_type_;
}

void GameTimerHandler::set_interval(const Time_Value &interval)
{
	this->interval_ = interval;
}

Time_Value &GameTimerHandler::interval(void)
{
	return this->interval_;
}

int GameTimerHandler::handle_timeout(const Time_Value &tv)
{
	if (POOL_MONITOR->timer_flag(this->type()) == 0)
		return 0;

    BaseUnit *unit = this->logic_unit();

    UnitMessage unit_msg;
    unit_msg.__type = UnitMessage::TYPE_IVALUE;
    unit_msg.__msg_head.__recogn = INNER_TIMER_TIMEOUT;
    unit_msg.__data.__i_val = this->type();

    unit->push_request(unit_msg);

    return 0;
}
int GameTimerHandler::schedule_timer(Time_Value &interval)
{
    return POOL_MONITOR->global_timer_watcher()->add(this, Epoll_Watcher::EVENT_TIMEOUT, &interval);
}

int GameTimerHandler::cancel_timer(void)
{
    return POOL_MONITOR->global_timer_watcher()->remove(this);
}

