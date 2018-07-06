/*
 * File Name: BackUnit.cpp
 * 
 * Created on: 2016-08-12 20:29:28
 * Author: glendy
 * 
 * Last Modified: 2016-08-12 21:14:46
 * Description: 
 */

#include "BackUnit.h"
#include "LogicMonitor.h"
#include "BackMailRequest.h"

int BackUnit::type(void)
{
    return BACK_UNIT;
}

UnitMessage *BackUnit::pop_unit_message(void)
{
    return LOGIC_MONITOR->unit_msg_pool()->pop();
}

int BackUnit::push_unit_message(UnitMessage *msg)
{
    return LOGIC_MONITOR->unit_msg_pool()->push(msg);
}

int BackUnit::process_block(UnitMessage *unit_msg)
{
    int recogn = unit_msg->__msg_head.__recogn;
//    Message *msg_proto = unit_msg->proto_msg();

    switch (recogn)
    {
        case TRANS_LOAD_BACK_MAIL_REQUEST:
            return this->process_back_mail_request();
        default:
            return -1;
    }
    return 0;
}

int BackUnit::process_back_mail_request(void)
{
    return BackMailRequest::process_back_mail();
}

