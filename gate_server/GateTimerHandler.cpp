/*
 * GateTimerHandler.cpp
 *
 * Created on: 2013-04-25 16:28
 *     Author: lyz
 */

#include "PubStruct.h"
#include "GateTimerHandler.h"
#include "GateMonitor.h"

BaseUnit *GateTimerHandler::logic_unit(void)
{
    return GATE_MONITOR->logic_unit();
}

