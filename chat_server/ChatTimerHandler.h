/*
 * ChatTimerHandler.h
 *
 * Created on: 2013-01-21 16:08
 *     Author: glendy
 */

#ifndef _CHATTIMERHANDLER_H_
#define _CHATTIMERHANDLER_H_

#include "GameTimerHandler.h"

class ChatTimerHandler : public GameTimerHandler
{
protected:
    virtual BaseUnit *logic_unit(void);
};

#endif //_CHATTIMERHANDLER_H_

