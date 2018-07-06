/*
 * BaseUnit.h
 *
 * Created on: 2012-12-29 14:26
 *     Author: glendy
 */

#ifndef _BASEUNIT_H_
#define _BASEUNIT_H_

#include "PubStruct.h"
#include "DoubleQueue.h"
#include "ObjectPoolEx.h"

class BaseUnit : public Thread
{
public:
    enum {
        AUTH_UNIT   = 1,
        CHAT_UNIT   = 2,
        GATE_UNIT   = 3,
        LOGIC_UNIT  = 4,
        MAP_UNIT    = 5,
        MAP_LOGIC_UNIT  = 6,
        LOG_UNIT    = 7,
        MYSQL_UNIT  = 8,
        MONGO_UNIT  = 9,
        CENTER_UNIT = 10,
        GATE_BROAD_UNIT = 11,
        GATE_CONNECT_UNIT = 12,
        BACK_UNIT = 13
    };

    struct MessagePerformInfo
    {
        int __recogn;
        Int64 __process_num;

        void reset(void);
    };

    typedef ObjectPoolEx<MessagePerformInfo> MsgPerfPool;
    typedef boost::unordered_map<int, MessagePerformInfo *> RecognMsgPerfMap;
    typedef boost::unordered_set<MessagePerformInfo *> MsgPerfSet;

public:
    BaseUnit(void);
    virtual void run_handler(void);

    virtual int push_request(const UnitMessage &msg);
    virtual bool is_running(void);
    virtual void stop_wait(void);
    virtual int type(void) = 0;

protected:
    virtual UnitMessage *pop_unit_message(void) = 0;
    virtual int push_unit_message(UnitMessage *msg) = 0;
    // recycle the msg->__data
    virtual int push_message_data(UnitMessage &msg);

    virtual int process_block(UnitMessage *unit_msg) = 0;
    virtual int prev_loop_init(void);
    virtual int process_request(void);
    virtual int interval_run(void);
    virtual int process_stop(void);
    int report_recogn_number(void);

protected:
    DoubleQueue unit_msg_queue_;
    bool is_running_;
    bool is_ready_stop_;

    MsgPerfPool msg_perf_pool_;
    RecognMsgPerfMap recogn_msgperf_map_;
    MsgPerfSet msg_perf_set_;
    MessagePerformInfo *less_info_;
    int total_msg_num_;
};

#endif //_BASEUNIT_H_
