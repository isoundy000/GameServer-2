/*
 * ChatUnit.h
 *
 * Created on: 2013-01-18 14:23
 *     Author: glendy
 */

#ifndef _CHATUNIT_H_
#define _CHATUNIT_H_

#include "BaseUnit.h"

class ChatUnit : public BaseUnit
{
public:
    virtual int type(void);

protected:
    virtual UnitMessage *pop_unit_message(void);
    virtual int push_unit_message(UnitMessage *msg);

    virtual int process_block(UnitMessage *unit_msg);

    virtual int process_stop(void);
};

#endif //_CHATUNIT_H_
