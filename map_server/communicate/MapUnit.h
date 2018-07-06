/*
 * MapUnit.h
 *
 * Created on: 2013-01-18 10:20
 *     Author: glendy
 */

#ifndef _MAPUNIT_H_
#define _MAPUNIT_H_

#include "BaseUnit.h"

class MapUnit : public BaseUnit
{
public:
    virtual int type(void);

protected:
    virtual UnitMessage *pop_unit_message(void);
    virtual int push_unit_message(UnitMessage *msg);

    virtual int process_block(UnitMessage *unit_msg);
    virtual int process_stop(void);

    int process_wedding_cruise_begin(Message *msg);
};

#endif //_MAPUNIT_H_
