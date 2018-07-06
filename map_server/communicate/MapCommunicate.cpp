/*
 * MapCommunicate.cpp
 *
 * Created on: 2013-01-17 20:27
 *     Author: glendy
 */

#include "PubStruct.h"
#include "PoolMonitor.h"
#include "GameDefine.h"
#include "MapCommunicate.h"
#include "BaseUnit.h"
#include "MapMonitor.h"
#include "LogClientMonitor.h"
#include <google/protobuf/message.h>

////////outer communicate/////////////////////
// {{{
MapClientService::Monitor *MapClientService::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

Block_Buffer *MapClientService::pop_block(int cid)
{
    return MAP_MONITOR->pop_block(cid);
}

int MapClientService::push_block(int cid, Block_Buffer *block)
{
    return MAP_MONITOR->push_block(block, cid);
}

int MapClientService::register_recv_handler(void)
{
    if (this->get_reg_recv() == false)
    {
        this->monitor()->receiver()->register_svc(this);
        this->set_reg_recv(true);
        this->set_flash_policy(true);
    }
    return 0;
}

int MapClientService::unregister_recv_handler(void)
{
    if (this->get_reg_recv() == true)
    {
        this->monitor()->receiver()->unregister_svc(this);
        this->set_reg_recv(false);
    }
    return 0;
}

int MapClientService::register_send_handler(void)
{
    if (this->get_reg_send() == false)
    {
        this->monitor()->sender(this->get_cid())->register_svc(this);
        this->set_reg_send(true);
    }
    return 0;
}

int MapClientService::unregister_send_handler(void)
{
    if (this->get_reg_send() == true)
    {
        this->monitor()->sender(this->get_cid())->unregister_svc(this);
        this->set_reg_send(false);
    }
    return 0;
}

int MapClientService::recv_handler(int cid)
{
    return this->monitor()->packer()->push_packing_cid(cid);
}

int MapClientService::close_handler(int cid)
{
    return this->monitor()->receiver()->push_drop(cid);
}

int MapClientPacker::process_block(Block_Buffer *buff)
{
    ProtoClientHead client_head;
    UnitMessage msg;

    Block_Buffer *msg_buff = buff;
    msg_buff->read_int32(msg.__sid);
    msg_buff->read_uint32(msg.__len);
    if (msg_buff->readable_bytes() < (sizeof(ProtoClientHead) + sizeof(uint32_t)))
    {
        MSG_USER("ERROR recv error data len %d %d", msg.__sid, msg_buff->readable_bytes());
        return -1;
    }

    int32_t sequence = 0;
    msg_buff->read_int32(sequence);

    msg_buff->read((char *)&client_head, sizeof(ProtoClientHead));
    msg.__msg_head.__recogn = client_head.__recogn;
    if (msg.__msg_head.__recogn != CLIENT_MAP_CONNECT)
    {
        return -1;
    }

    Message *proto_msg = parse_message(msg.__msg_head.__recogn, msg_buff);
    msg.set_msg_proto(proto_msg);

    if (MAP_MONITOR->map_unit()->push_request(msg) != 0)
    {
        return -1;
    }

    if (msg.__type != UnitMessage::TYPE_BLOCK_BUFF)
    {
        this->push_block(msg.__sid, msg_buff);
    }
    return 0;
}

int MapClientPacker::process_close_handler(int cid)
{
    UnitMessage unit_msg;
    unit_msg.reset();
    unit_msg.__type = UnitMessage::TYPE_PROTO_MSG;
    unit_msg.__sid = cid;
    unit_msg.__len = sizeof(ProtoHead) + 2;
    unit_msg.__msg_head.__recogn = INNER_MAP_CLOSE;
    return MAP_MONITOR->map_unit()->push_request(unit_msg);
}
// }}}

////////inner accept communicate/////////////////////
// {{{
MapInnerService::Monitor *MapInnerService::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

Block_Buffer *MapInnerService::pop_block(int cid)
{
    return MAP_MONITOR->pop_block(cid);
}

int MapInnerService::push_block(int cid, Block_Buffer *block)
{
    return MAP_MONITOR->push_block(block, cid);
}

int MapInnerService::register_recv_handler(void)
{
    if (this->get_reg_recv() == false)
    {
        this->monitor()->receiver()->register_svc(this);
        this->set_reg_recv(true);
    }
    return 0;
}

int MapInnerService::unregister_recv_handler(void)
{
    if (this->get_reg_recv() == true)
    {
        this->monitor()->receiver()->unregister_svc(this);
        this->set_reg_recv(false);
    }
    return 0;
}

int MapInnerService::register_send_handler(void)
{
    if (this->get_reg_send() == false)
    {
        this->monitor()->sender(this->get_cid())->register_svc(this);
        this->set_reg_send(true);
    }
    return 0;
}

int MapInnerService::unregister_send_handler(void)
{
    if (this->get_reg_send() == true)
    {
        this->monitor()->sender(this->get_cid())->unregister_svc(this);
        this->set_reg_send(false);
    }
    return 0;
}

int MapInnerService::recv_handler(int cid)
{
    return this->monitor()->packer()->push_packing_cid(cid);
}

int MapInnerService::close_handler(int cid)
{
    return this->monitor()->receiver()->push_drop(cid);
}

int MapInnerPacker::process_block(Block_Buffer *buff)
{
    UnitMessage msg;

    Block_Buffer *msg_buff = buff;
    msg.__type = UnitMessage::TYPE_PROTO_MSG;
    msg_buff->read_int32(msg.__sid);
    msg_buff->read_uint32(msg.__len);

    if (msg_buff->readable_bytes() < sizeof(ProtoHead))
    {
        MSG_USER("ERROR recv error data len %d %d", msg.__sid, msg_buff->readable_bytes());
        return -1;
    }
    msg_buff->read((char *)&(msg.__msg_head), sizeof(ProtoHead));

    Message *proto_msg = parse_message(msg.__msg_head.__recogn, msg_buff);
    msg.__data.__proto_msg = proto_msg;

    int type = msg.__msg_head.__recogn / 1000000;
    type %= 10;
    BaseUnit *process_unit = 0;
    if (type == 0)
    	process_unit = MAP_MONITOR->map_unit();
    else
    	process_unit = MAP_MONITOR->logic_unit();
    if (process_unit->push_request(msg) != 0)
    {
        if (proto_msg != 0)
            delete proto_msg;
        return -1;
    }

    if (msg.__type != UnitMessage::TYPE_BLOCK_BUFF)
    {
        this->push_block(msg.__sid, msg_buff);
    }
    return 0;
}

int MapInnerPacker::process_close_handler(int cid)
{
    return 0;
}

// }}}

////////inner connect communicate////////////////////
// {{{
MapConnectService::Monitor *MapConnectService::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

Block_Buffer *MapConnectService::pop_block(int cid)
{
    return MAP_MONITOR->pop_block(cid);
}

int MapConnectService::push_block(int cid, Block_Buffer *block)
{
    return MAP_MONITOR->push_block(block, cid);
}

int MapConnectService::register_send_handler(void)
{
    if (this->get_reg_send() == false)
    {
        this->monitor()->sender()->register_svc(this);
        this->set_reg_send(true);
    }
    return 0;
}

int MapConnectService::unregister_send_handler(void)
{
    if (this->get_reg_send() == true)
    {
        this->monitor()->sender()->unregister_svc(this);
        this->set_reg_send(false);
    }
    return 0;
}

int MapConnectService::close_handler(int cid)
{
	if (LOG_CLIENT_MONITOR->get_log_sid() == cid)
		LOG_CLIENT_MONITOR->set_log_sid(-1);

    if (this->monitor()->receiver() == 0)
        return this->monitor()->sender()->push_drop(cid);
    return this->monitor()->receiver()->push_drop(cid);
}

// }}}

