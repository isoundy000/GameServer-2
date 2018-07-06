/*
 * GateUnit.h
 *
 * Created on: 2013-04-15 16:09
 *     Author: lyz
 */

#ifndef _GATEUNIT_H_
#define _GATEUNIT_H_

#include "BaseUnit.h"

class GateUnit : public BaseUnit
{
public:
    virtual int type(void);

protected:
    virtual UnitMessage *pop_unit_message(void);
    virtual int push_unit_message(UnitMessage *msg);

    virtual int process_block(UnitMessage *unit_msg);
    virtual int process_stop(void);

    bool dispatch_by_noplayer(UnitMessage *unit_msg);
    bool process_back(UnitMessage* unit_msg);
};

class ConnectUnit : public BaseUnit
{
public:
    virtual int type(void);
    virtual void stop_wait(void);

    int dispatch_to_gate_unit(const Message &msg);

protected:
    virtual UnitMessage *pop_unit_message(void);
    virtual int push_unit_message(UnitMessage *msg);
    virtual int process_block(UnitMessage *unit_msg);

};

#endif //_GATEUNIT_H_
