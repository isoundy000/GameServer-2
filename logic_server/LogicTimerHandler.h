/*
 * LogicTimerHandler.h
 *
 * Created on: 2013-01-21 11:11
 *     Author: glendy
 */

#ifndef _LOGICTIMERHANDLER_H_
#define _LOGICTIMERHANDLER_H_

#include "GameTimerHandler.h"

class LogicTimerHandler : public GameTimerHandler
{
protected:
    virtual BaseUnit *logic_unit(void);
};

#endif //_LOGICTIMERHANDLER_H_
