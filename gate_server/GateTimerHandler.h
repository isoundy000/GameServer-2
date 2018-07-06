/*
 * GateTimerHandler.h
 *
 * Created on: 2013-04-25 16:27
 *     Author: lyz
 */

#ifndef _GATETIMERHANDLER_H_
#define _GATETIMERHANDLER_H_

#include "GameTimerHandler.h"

class GateTimerHandler : public GameTimerHandler
{
protected:
    virtual BaseUnit *logic_unit(void);
};

#endif //_GATETIMERHANDLER_H_
