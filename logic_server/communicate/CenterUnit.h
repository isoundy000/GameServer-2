/*
 * CenterUnit.h
 *
 * Created on: 2014-01-17 16:43
 *     Author: lyz
 */

#ifndef _CENTERUNIT_H_
#define _CENTERUNIT_H_

#include "BaseUnit.h"

class CenterUnit : public BaseUnit
{
public:
    virtual int type(void);

protected:
    virtual UnitMessage *pop_unit_message(void);
    virtual int push_unit_message(UnitMessage *msg);

    virtual int process_block(UnitMessage *unit_msg);
    virtual int process_stop(void);

    virtual int prev_loop_init(void);
    virtual int interval_run(void);

private:
    int logic_query_acti_code_ret(int64_t role_id, Message *msg);
    int logic_notify_act_mail( Message *msg);
    int logic_send_act_reward_mail(Message* msg);
    int logic_query_del_return_recharge(Message* msg);
    int logic_rank_send_mail(Message* msg);

protected:
    int interval_amount_;
    Time_Value interval_tick_;
};

#endif //_CENTERUNIT_H_
