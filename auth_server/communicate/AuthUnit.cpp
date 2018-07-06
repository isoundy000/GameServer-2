/*
 * AuthUnit.cpp
 *
 * Created on: 2013-04-15 10:49
 *     Author: lyz
 */

#include "AuthUnit.h"
#include "AuthMonitor.h"

#include "DaemonServer.h"

AuthUnit::AuthUnit(void) : interval_amount_(0)
{ /*NULL*/ }

int AuthUnit::type(void)
{
    return AUTH_UNIT;
}

UnitMessage *AuthUnit::pop_unit_message(void)
{
    return AUTH_MONITOR->unit_msg_pool()->pop();
}

int AuthUnit::push_unit_message(UnitMessage *msg)
{
    return AUTH_MONITOR->unit_msg_pool()->push(msg);
}

int AuthUnit::process_block(UnitMessage *unit_msg)
{
    int sid = unit_msg->__sid, recogn = unit_msg->__msg_head.__recogn;

    Block_Buffer *msg = unit_msg->data_buff();
    Message *msg_proto = unit_msg->proto_msg();
    
    switch(recogn)
    {
        case INNER_UPDATE_CONFIG:
            return DAEMON_SERVER->request_update_config();
        case INNER_INIT_ROLEAMOUNT:
            return AUTH_MONITOR->refresh_gate_roleamount(msg);
        case INNER_GATE_CLOSE_ROLE:
            return AUTH_MONITOR->close_gate_role(msg);
#ifndef NO_USE_PROTO
        case CLIENT_AUTH_SESSION:
            return AUTH_MONITOR->validate_auth_session(sid, msg_proto);
#else
        case CLIENT_AUTH_SESSION:
            return AUTH_MONITOR->validate_auth_session(sid, msg);
#endif
        case INNER_SYNC_SEESION:
        	return AUTH_MONITOR->synced_gate_session(msg);
        default:
            MSG_USER("ERROR can't recognize recogn %d %d",sid, recogn);
    }

    return 0;
}

int AuthUnit::process_stop(void)
{
    return BaseUnit::process_stop();
}

int AuthUnit::interval_run(void)
{
    ++this->interval_amount_;
    if (this->interval_amount_ < 5000)
        return 0;
    this->interval_amount_ = 0;

    Time_Value nowtime = Time_Value::gettimeofday();
    if (this->interval_tick_ > nowtime)
        return 0; 
    this->interval_tick_ = nowtime + Time_Value(10);

    return AUTH_MONITOR->dispatch_to_all_gate(INNER_GATE_ROLEAMOUNT);
}


