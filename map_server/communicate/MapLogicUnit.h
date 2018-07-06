/*
 * MapLogicUnit.h
 *
 * Created on: 2013-11-28 11:02
 *     Author: lyz
 */

#ifndef _MAPLOGICUNIT_H_
#define _MAPLOGICUNIT_H_

#include "BaseUnit.h"

class MapLogicUnit : public BaseUnit
{
public:
    virtual int type(void);

protected:
    virtual UnitMessage *pop_unit_message(void);
    virtual int push_unit_message(UnitMessage *msg);

    virtual int process_block(UnitMessage *unit_msg);
    virtual int process_stop(void);

    int handle_with_noplayer_online(MapLogicPlayer* player, UnitMessage *unit_msg);
};

#endif //_MAPLOGICUNIT_H_
