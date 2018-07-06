/*
 * LogServerUnit.h
 *
 * Created on: 2013-01-09 14:43
 *     Author: glendy
 */

#ifndef _LOGSERVERUNIT_H_
#define _LOGSERVERUNIT_H_

#include "BaseUnit.h"
#include "MysqlConnector.h"
#include "GameHeader.h"

class SqlTableCache;

class LogUnit : public BaseUnit
{
public:
    virtual int type(void);

protected:
    virtual UnitMessage *pop_unit_message(void);
    virtual int push_unit_message(UnitMessage *msg);

    virtual int process_block(UnitMessage *unit_msg);
};

class MysqlUnit : public BaseUnit
{
public:
    enum {
        LOG_INTERVAL    = 10
    };

    typedef boost::unordered_map<std::string, SqlTableCache *> SqlTableCacheMap;

public:
    MysqlConnector &mysql_connector(void);
    virtual int type(void);

protected:
    virtual UnitMessage *pop_unit_message(void);
    virtual int push_unit_message(UnitMessage *msg);

    virtual int process_block(UnitMessage *unit_msg);

    virtual int prev_loop_init(void);
    virtual int interval_run(void);

    virtual int push_insert_op(const ::google::protobuf::Message *message);
    virtual int push_insert_op_with_table_name(const ::google::protobuf::Message *message);
    virtual int process_sql_cache(void);

    virtual int process_stop(void);

protected:
    int interval_amount_;
    Time_Value interval_tick_;
    SqlTableCacheMap sql_cache_map_;
    MysqlConnector connector_;
};

#endif //_LOGSERVERUNIT_H_
