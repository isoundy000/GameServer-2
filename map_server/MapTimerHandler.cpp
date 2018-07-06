/*
 * MapTimerHandler.cpp
 *
 * Created on: 2013-01-21 16:00
 *     Author: glendy
 */

#include "MapTimerHandler.h"
#include "MapMonitor.h"

BaseUnit *MapTimerHandler::logic_unit(void)
{
	switch (this->type())
	{
	case GTT_ML_ONE_SECOND:
	case GTT_ML_ONE_MINUTE:
	{
		return MAP_MONITOR->logic_unit();
	}

	default:
	{
		return MAP_MONITOR->map_unit();
	}
	}
}

