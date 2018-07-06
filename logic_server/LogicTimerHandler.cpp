/*
 * LogicTimerHandler.cpp
 *
 * Created on: 2013-01-21 11:12
 *     Author: glendy
 */

#include "LogicTimerHandler.h"
#include "LogicMonitor.h"

BaseUnit *LogicTimerHandler::logic_unit(void)
{
    return LOGIC_MONITOR->logic_unit();
}

