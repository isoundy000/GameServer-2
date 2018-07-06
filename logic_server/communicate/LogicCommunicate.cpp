/*
 * LogicCommunicate.cpp
 *
 * Created on: 2013-01-07 17:12
 *     Author: glendy
 */

#include "PubStruct.h"
#include "PoolMonitor.h"
#include "LogicCommunicate.h"
#include "BaseUnit.h"
#include "LogicMonitor.h"
#include "Log.h"
#include "LogClientMonitor.h"
#include "LogClientMonitor.h"
#include <google/protobuf/message.h>

////////outer communicate/////////////////////
// {{{
LogicClientService::Monitor *LogicClientService::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

Block_Buffer *LogicClientService::pop_block(int cid)
{
    return LOGIC_MONITOR->pop_block(cid);
}

int LogicClientService::push_block(int cid, Block_Buffer *block)
{
    return LOGIC_MONITOR->push_block(block, cid);
}

int LogicClientService::register_recv_handler(void)
{
    if (this->get_reg_recv() == false)
    {
        this->monitor()->receiver()->register_svc(this);
        this->set_reg_recv(true);
    }
    return 0;
}

int LogicClientService::unregister_recv_handler(void)
{
    if (this->get_reg_recv() == true)
    {
        this->monitor()->receiver()->unregister_svc(this);
        this->set_reg_recv(false);
    }
    return 0;
}

int LogicClientService::register_send_handler(void)
{
    if (this->get_reg_send() == false)
    {
        this->monitor()->sender(this->get_cid())->register_svc(this);
        this->set_reg_send(true);
    }
    return 0;
}

int LogicClientService::unregister_send_handler(void)
{
    if (this->get_reg_send() == true)
    {
        this->monitor()->sender(this->get_cid())->unregister_svc(this);
        this->set_reg_send(false);
    }
    return 0;
}

int LogicClientService::recv_handler(int cid)
{
    return this->monitor()->packer()->push_packing_cid(cid);
}

int LogicClientService::close_handler(int cid)
{
    return this->monitor()->receiver()->push_drop(cid);
}

int LogicClientPacker::process_block(Block_Buffer *buff)
{
    return -1;
}

int LogicClientPacker::process_close_handler(int cid)
{
    return 0;
}
// }}}

////////inner communicate//////////////////////
// {{{
LogicInnerService::Monitor *LogicInnerService::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

Block_Buffer *LogicInnerService::pop_block(int cid)
{
    return LOGIC_MONITOR->pop_block(cid);
}

int LogicInnerService::push_block(int cid, Block_Buffer *block)
{
    return LOGIC_MONITOR->push_block(block, cid);
}

int LogicInnerService::register_recv_handler(void)
{
    if (this->get_reg_recv() == false)
    {
        this->monitor()->receiver()->register_svc(this);
        this->set_reg_recv(true);
    }
    return 0;
}

int LogicInnerService::unregister_recv_handler(void)
{
    if (this->get_reg_recv() == true)
    {
        this->monitor()->receiver()->unregister_svc(this);
        this->set_reg_recv(false);
    }
    return 0;
}

int LogicInnerService::register_send_handler(void)
{
    if (this->get_reg_send() == false)
    {
        this->monitor()->sender(this->get_cid())->register_svc(this);
        this->set_reg_send(true);
    }
    return 0;
}

int LogicInnerService::unregister_send_handler(void)
{
    if (this->get_reg_send() == true)
    {
        this->monitor()->sender(this->get_cid())->unregister_svc(this);
        this->set_reg_send(false);
    }
    return 0;
}

int LogicInnerService::recv_handler(int cid)
{
    return this->monitor()->packer()->push_packing_cid(cid);
}

int LogicInnerService::close_handler(int cid)
{
    return this->monitor()->receiver()->push_drop(cid);
}

int LogicInnerPacker::process_block(Block_Buffer *buff)
{
    Block_Buffer *msg_buff = buff;
    UnitMessage msg;

    msg_buff->read_int32(msg.__sid);
    msg_buff->read_uint32(msg.__len);

    if (msg_buff->readable_bytes() < sizeof(ProtoHead))
    {
        MSG_USER("ERROR recv error data len %d %d", msg.__sid, msg_buff->readable_bytes());
        return -1;
    }

    msg_buff->read((char *)&(msg.__msg_head), sizeof(ProtoHead));

#ifndef LOCAL_DEBUG
#endif


    Message *msg_proto = parse_message(msg.__msg_head.__recogn, msg_buff);
    msg.__type = UnitMessage::TYPE_PROTO_MSG;
    msg.__data.__proto_msg = msg_proto;

    if (LOGIC_MONITOR->logic_unit()->push_request(msg) != 0)
    {
        if (msg_proto != 0)
            delete msg_proto;
        return -1;
    }

    if (msg.__type != UnitMessage::TYPE_BLOCK_BUFF)
        this->push_block(msg.__sid, msg_buff);
    return 0;
}

int LogicInnerPacker::process_close_handler(int cid)
{
    return 0;
}
// }}}

////////connect communicate////////////////////
// {{{
LogicConnectService::Monitor *LogicConnectService::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

Block_Buffer *LogicConnectService::pop_block(int cid)
{
    return LOGIC_MONITOR->pop_block(cid);
}

int LogicConnectService::push_block(int cid, Block_Buffer *block)
{
    return LOGIC_MONITOR->push_block(block, cid);
}

int LogicConnectService::register_recv_handler(void)
{
    if (this->get_reg_recv() == false)
    {
        this->monitor()->receiver()->register_svc(this);
        this->set_reg_recv(true);
    }
    return 0;
}

int LogicConnectService::unregister_recv_handler(void)
{
    if (this->get_reg_recv() == true)
    {
    	this->monitor()->receiver()->unregister_svc(this);
        this->set_reg_recv(false);
    }
    return 0;
}

int LogicConnectService::register_send_handler(void)
{
    if (this->get_reg_send() == false)
    {
    	this->monitor()->sender()->register_svc(this);
        this->set_reg_send(true);
    }
    return 0;
}

int LogicConnectService::unregister_send_handler(void)
{
    if (this->get_reg_send() == true)
    {
    	this->monitor()->sender()->unregister_svc(this);
        this->set_reg_send(false);
    }
    return 0;
}

int LogicConnectService::recv_handler(int cid)
{
    return this->monitor()->packer()->push_packing_cid(cid);
}

int LogicConnectService::close_handler(int cid)
{
	if (LOG_CLIENT_MONITOR->get_log_sid() == cid)
		LOG_CLIENT_MONITOR->set_log_sid(-1);

    if (this->monitor()->receiver() == 0)
        return this->monitor()->sender()->push_drop(cid);
    return this->monitor()->receiver()->push_drop(cid);
}

int LogicConnectPacker::process_block(Block_Buffer *buff)
{
    UnitMessage msg;

    Block_Buffer *msg_buff = buff;
    msg_buff->read_int32(msg.__sid);
    msg_buff->read_uint32(msg.__len);
    msg_buff->read((char *)&(msg.__msg_head), sizeof(ProtoHead));

    Message *msg_proto = parse_message(msg.__msg_head.__recogn, msg_buff);
    if (msg_proto == 0)
    {
        msg.__type = UnitMessage::TYPE_BLOCK_BUFF;
        msg.__data.__buf = msg_buff;
    }
    else
    {
        msg.__type = UnitMessage::TYPE_PROTO_MSG;
        msg.__data.__proto_msg = msg_proto;
    }

    if (LOGIC_MONITOR->logic_unit()->push_request(msg) != 0)
    {
        if (msg_proto != 0)
            delete msg_proto;
        return -1;
    }

    if (msg.__type != UnitMessage::TYPE_BLOCK_BUFF)
        this->push_block(msg.__sid, msg_buff);
    return 0;
}

int LogicConnectPacker::process_close_handler(int cid)
{
    return 0;
}
// }}}

////////////////php Communicate///////////////////
// {{{
LogicPhpService::Monitor *LogicPhpService::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

Block_Buffer *LogicPhpService::pop_block(int cid)
{
    return LOGIC_MONITOR->pop_block(cid);
}

int LogicPhpService::push_block(int cid, Block_Buffer *block)
{
    return LOGIC_MONITOR->push_block(block, cid);
}

int LogicPhpService::register_recv_handler(void)
{
    if (this->get_reg_recv() == false)
    {
        this->monitor()->receiver()->register_svc(this);
        this->set_reg_recv(true);
    }
    return 0;
}

int LogicPhpService::unregister_recv_handler(void)
{
    if (this->get_reg_recv() == true)
    {
        this->monitor()->receiver()->unregister_svc(this);
        this->set_reg_recv(false);
    }
    return 0;
}

int LogicPhpService::register_send_handler(void)
{
    if (this->get_reg_send() == false)
    {
        this->monitor()->sender()->register_svc(this);
        this->set_reg_send(true);
    }
    return 0;
}

int LogicPhpService::unregister_send_handler(void)
{
    if (this->get_reg_send() == true)
    {
        this->monitor()->sender()->unregister_svc(this);
        this->set_reg_send(false);
    }
    return 0;
}

int LogicPhpService::recv_handler(int cid)
{
    return this->monitor()->packer()->push_packing_cid(cid);
}

int LogicPhpService::close_handler(int cid)
{
    return this->monitor()->receiver()->push_drop(cid);
}

int LogicPhpPacker::process_block(Block_Buffer *buff)
{
    ProtoClientHead client_head;
    UnitMessage msg;
    client_head.reset();
    msg.reset();

    Block_Buffer *msg_buff = buff;
    msg_buff->read_int32(msg.__sid);
    msg_buff->read_uint32(msg.__len);
    if (msg_buff->readable_bytes() < sizeof(ProtoClientHead))
    {
        MSG_USER("ERROR recv error data len %d %d", msg.__sid, msg_buff->readable_bytes());
        return -1;
    }

    msg_buff->read((char *)&client_head, sizeof(ProtoClientHead));
    msg.__msg_head.__recogn = client_head.__recogn;
    /* 308XXXXX or 318XXXXX 表示 PHP 的请求*/
    if( client_head.__recogn/100000 != 301 && client_head.__recogn/100000 != 308)
    {
    	MSG_USER("recogn error: %d", client_head.__recogn);
		return -1;
    }

    Message *proto_msg = 0;
    int ivalue = 0;
    /* php 消息使用第4位标识消息(UnitMessage)的类型
     * 0: BLOCK_BUFF, 1: PROTO_MSG, 2: IVALUE */
    switch( (client_head.__recogn%1000)/100 )
    {
    	case 0: // BLOCK_BUFF
    		msg.set_data_buff(msg_buff);
    		break;
    	case 1: // PROTO_MSG
    		proto_msg = parse_message(msg.__msg_head.__recogn, msg_buff);
    		msg.set_msg_proto(proto_msg);
    		break;
    	case 2: // PROTO_MSG
    		msg_buff->read_int32(ivalue);
    		msg.set_ivalue(ivalue);
			break;
    	default:
    		MSG_USER("recogn error: %d", client_head.__recogn);
    		return -1;
    }

    if (LOGIC_MONITOR->logic_unit()->push_request(msg) != 0)
    {
        return -1;
    }

    if (msg.__type != UnitMessage::TYPE_BLOCK_BUFF)
        this->push_block(msg.__sid, msg_buff);
    return 0;
}

int LogicPhpPacker::process_close_handler(int cid)
{
    return 0;
}

// }}}

