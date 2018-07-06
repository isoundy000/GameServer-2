/*
 * LogicUnit.h
 *
 * Created on: 2013-01-08 11:32
 *     Author: glendy
 */

#ifndef _LOGICUNIT_H_
#define _LOGICUNIT_H_

#include "BaseUnit.h"

class LogicUnit : public BaseUnit
{
public:
    virtual int type(void);

protected:
    virtual UnitMessage *pop_unit_message(void);
    virtual int push_unit_message(UnitMessage *msg);

    virtual int process_block(UnitMessage *unit_msg);
    virtual int process_stop(void);
    virtual int interval_run(void);

    virtual int process_php_request(const int sid, const int recogn, UnitMessage *unit_msg);
};

#endif //_LOGICUNIT_H_
