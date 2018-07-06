/*
 * MongoUnit.h
 *
 * Created on: 2013-01-25 16:45
 *     Author: glendy
 */

#ifndef _MONGOUNIT_H_
#define _MONGOUNIT_H_

#include "BaseUnit.h"

class MongoConnector;

class MongoUnit : public BaseUnit
{
public:
	MongoUnit(void);
    virtual int type(void);

protected:
    virtual UnitMessage *pop_unit_message(void);
    virtual int push_unit_message(UnitMessage *msg);

    virtual int process_block(UnitMessage *unit_msg);
    virtual int process_stop(void);

    virtual int interval_run(void);

    int update_player_name(Transaction *trans);

protected:
    MongoConnector *conn_;
};

#endif //_MONGOUNIT_H_
