/*
 * MapTimerHandler.h
 *
 * Created on: 2013-01-21 11:14
 *     Author: glendy
 */

#ifndef _MAPTIMERHANDLER_H_
#define _MAPTIMERHANDLER_H_

#include "GameTimerHandler.h"

class MapTimerHandler : public GameTimerHandler
{
protected:
    virtual BaseUnit *logic_unit(void);
};

#endif //_MAPTIMERHANDLER_H_
