/*
 * BroadUnit.h
 *
 * Created on: 2014-09-26 10:49
 *     Author: lyz
 */

#ifndef _BROADUNIT_H_
#define _BROADUNIT_H_

#include "BaseUnit.h"

class BroadUnit : public BaseUnit
{
public:
    virtual int type(void);

protected:
    virtual UnitMessage *pop_unit_message(void);
    virtual int push_unit_message(UnitMessage *msg);

    virtual int process_block(UnitMessage *unit_msg);

    bool dispatch_by_broad_in_gate(UnitMessage *unit_msg);
    bool dispatch_by_broad_client(UnitMessage *unit_msg);

};

#endif //_BROADUNIT_H_
