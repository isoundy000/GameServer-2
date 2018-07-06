/*
 * AuthUnit.h
 *
 * Created on: 2013-04-15 10:48
 *     Author: lyz
 */

#ifndef _AUTHUNIT_H_
#define _AUTHUNIT_H_

#include "BaseUnit.h"

class AuthUnit : public BaseUnit
{
public:
    AuthUnit(void);
    virtual int type(void);

protected:
    virtual UnitMessage *pop_unit_message(void);
    virtual int push_unit_message(UnitMessage *msg);

    virtual int process_block(UnitMessage *unit_msg);
    virtual int process_stop(void);

    virtual int interval_run(void);

protected:
    int interval_amount_;
    Time_Value interval_tick_;
};

#endif //_AUTHUNIT_H_
