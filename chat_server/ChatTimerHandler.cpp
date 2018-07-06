/*
 * ChatTimerHandler.cpp
 *
 * Created on: 2013-01-21 16:04
 *     Author: glendy
 */

#include "ChatTimerHandler.h"
#include "ChatMonitor.h"

BaseUnit *ChatTimerHandler::logic_unit(void)
{
    return CHAT_MONITOR->logic_unit();
}

