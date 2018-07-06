/*
 * ChatCommunicate.cpp
 *
 * Created on: 2013-01-18 14:23
 *     Author: glendy
 */

#include "PubStruct.h"
#include "GameTimerHandler.h"
#include "PoolMonitor.h"
#include "ChatCommunicate.h"
#include "ChatMonitor.h"
#include "LogClientMonitor.h"

////////outer communicate/////////////////////
// {{{
ChatClientService::Monitor *ChatClientService::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

Block_Buffer *ChatClientService::pop_block(int cid)
{
    return CHAT_MONITOR->pop_block(cid);
}

int ChatClientService::push_block(int cid, Block_Buffer *block)
{
    return CHAT_MONITOR->push_block(block, cid);
}

int ChatClientService::register_recv_handler(void)
{
    if (this->get_reg_recv() == false)
    {
    	this->msg_sequence_ = 0;
        this->monitor()->receiver()->register_svc(this);
        this->set_reg_recv(true);
        this->set_flash_policy(true);
    }
    return 0;
}

int ChatClientService::unregister_recv_handler(void)
{
    if (this->get_reg_recv() == true)
    {
        this->monitor()->receiver()->unregister_svc(this);
        this->set_reg_recv(false);
    }
    return 0;
}

int ChatClientService::register_send_handler(void)
{
    if (this->get_reg_send() == false)
    {
        this->monitor()->sender(this->get_cid())->register_svc(this);
        this->set_reg_send(true);
    }
    return 0;
}

int ChatClientService::unregister_send_handler(void)
{
    if (this->get_reg_send() == true)
    {
        this->monitor()->sender(this->get_cid())->unregister_svc(this);
        this->set_reg_send(false);
    }
    return 0;
}

int ChatClientService::recv_handler(int cid)
{
    return this->monitor()->packer()->push_packing_cid(cid);
}

int ChatClientService::close_handler(int cid)
{
    return this->monitor()->receiver()->push_drop(cid);
}

void ChatClientService::reset(void)
{
	Svc::reset();
//	this->restore_ = true;
}

void ChatClientService::set_msg_sequence(const uint64_t val)
{
    this->msg_sequence_ = val;
}

uint64_t ChatClientService::msg_sequence(void)
{
    return this->msg_sequence_;
}

int ChatClientPacker::process_block(Block_Buffer *buff)
{
    ProtoClientHead client_head;
    UnitMessage msg;

    Block_Buffer *msg_buff = buff;
    msg_buff->read_int32(msg.__sid);
    msg_buff->read_uint32(msg.__len);

#ifndef SEQUENCE_VALIDATE
    if (msg_buff->readable_bytes() < (sizeof(ProtoClientHead) + sizeof(uint32_t)))
    {
        MSG_USER("ERROR recv error data len %d %d", msg.__sid, msg_buff->readable_bytes());
        return -1;
    }

    int msg_seq = 0;
    msg_buff->read_int32(msg_seq);
    ChatClientService *svc = dynamic_cast<ChatClientService *>(this->find_svc(msg.__sid));
    if (svc == 0)
    {
        MSG_USER("ERROR client disconnected %d", msg.__sid);
        return -1;
    }

#else
    if (msg_buff->readable_bytes() < (sizeof(ProtoClientHead) + sizeof(uint64_t)))
    {
        MSG_USER("ERROR recv error data len %d %d", msg.__sid, msg_buff->readable_bytes());
        return -1;
    }

    uint64_t msg_seq = 0;
    msg_buff->read_uint64(msg_seq);
    msg_seq = des_decrypt(msg_seq, MSG_SEQUENCE_KEY);

    ChatClientService *svc = dynamic_cast<ChatClientService *>(this->find_svc(msg.__sid));
    if (svc == 0)
    {
        MSG_USER("ERROR client disconnected %d", msg.__sid);
        return -1;
    }

    uint64_t svc_seq = svc->msg_sequence();
    if (msg_seq <= svc_seq)
    {
        MSG_USER("ERROR data msg sequence %d %ld %ld", msg.__sid, msg_seq, svc_seq);
        svc->handle_close();
        return -1;
    }
    svc->set_msg_sequence(msg_seq);
#endif

    msg_buff->read((char *)&client_head, sizeof(ProtoClientHead));
    msg.__msg_head.__recogn = client_head.__recogn;
    if (msg.__msg_head.__recogn != CLIENT_SERVER_KEEP_ALIVE && (msg.__msg_head.__recogn < 10200000 || 10300000 <= msg.__msg_head.__recogn))
    {
        return -1;
    }

    msg.__type = UnitMessage::TYPE_PROTO_MSG;
    Message *proto_msg = parse_message(client_head.__recogn, msg_buff);
    msg.__data.__proto_msg = proto_msg;

    if (CHAT_MONITOR->logic_unit()->push_request(msg) != 0)
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

int ChatClientPacker::process_close_handler(int cid)
{
	MSG_USER("close gate client sid %d", cid);
    // notify chat, map, logic unit to close;
    UnitMessage unit_msg;
    unit_msg.__type = UnitMessage::TYPE_PROTO_MSG;
    unit_msg.__sid = cid;
    unit_msg.__len = sizeof(ProtoHead) + 2;
    unit_msg.__msg_head.__recogn = CLIENT_CHAT_CLIENT_CLOSE;
    return CHAT_MONITOR->logic_unit()->push_request(unit_msg);
}
// }}}

////////inner accept communicate/////////////////////
// {{{
ChatInnerService::Monitor *ChatInnerService::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

Block_Buffer *ChatInnerService::pop_block(int cid)
{
    return CHAT_MONITOR->pop_block(cid);
}

int ChatInnerService::push_block(int cid, Block_Buffer *block)
{
    return CHAT_MONITOR->push_block(block, cid);
}

int ChatInnerService::register_recv_handler(void)
{
    if (this->get_reg_recv() == false)
    {
        this->monitor()->receiver()->register_svc(this);
        this->set_reg_recv(true);
    }
    return 0;
}

int ChatInnerService::unregister_recv_handler(void)
{
    if (this->get_reg_recv() == true)
    {
        this->monitor()->receiver()->unregister_svc(this);
        this->set_reg_recv(false);
    }
    return 0;
}

int ChatInnerService::register_send_handler(void)
{
    if (this->get_reg_send() == false)
    {
        this->monitor()->sender(this->get_cid())->register_svc(this);
        this->set_reg_send(true);
    }
    return 0;
}

int ChatInnerService::unregister_send_handler(void)
{
    if (this->get_reg_send() == true)
    {
        this->monitor()->sender(this->get_cid())->unregister_svc(this);
        this->set_reg_send(false);
    }
    return 0;
}

int ChatInnerService::recv_handler(int cid)
{
    return this->monitor()->packer()->push_packing_cid(cid);
}

int ChatInnerService::close_handler(int cid)
{
    return this->monitor()->receiver()->push_drop(cid);
}

int ChatInnerPacker::process_block(Block_Buffer *buff)
{
    UnitMessage msg;

    Block_Buffer *msg_buff = buff;
    msg_buff->read_int32(msg.__sid);
    msg_buff->read_uint32(msg.__len);
        
    if (msg_buff->readable_bytes() < sizeof(ProtoHead))
    {
        MSG_USER("ERROR recv error data len %d %d", msg.__sid, msg_buff->readable_bytes());
        return -1;
    }
    msg_buff->read((char *)&(msg.__msg_head), sizeof(ProtoHead));

    Message *proto_msg = parse_message(msg.__msg_head.__recogn, msg_buff);
    msg.set_msg_proto(proto_msg);

    if (CHAT_MONITOR->logic_unit()->push_request(msg) != 0)
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

int ChatInnerPacker::process_close_handler(int cid)
{
    return 0;
}
// }}}

////////inner connect communicate////////////////////
// {{{
ChatConnectService::Monitor *ChatConnectService::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

Block_Buffer *ChatConnectService::pop_block(int cid)
{
    return CHAT_MONITOR->pop_block(cid);
}

int ChatConnectService::push_block(int cid, Block_Buffer *block)
{
    return CHAT_MONITOR->push_block(block, cid);
}

int ChatConnectService::register_send_handler(void)
{
    if (this->get_reg_send() == false)
    {
        this->monitor()->sender()->register_svc(this);
        this->set_reg_send(true);
    }
    return 0;
}

int ChatConnectService::unregister_send_handler(void)
{
    if (this->get_reg_send() == true)
    {
        this->monitor()->sender()->unregister_svc(this);
        this->set_reg_send(false);
    }
    return 0;
}

int ChatConnectService::close_handler(int cid)
{
	if (LOG_CLIENT_MONITOR->get_log_sid() == cid)
		LOG_CLIENT_MONITOR->set_log_sid(-1);

    if (this->monitor()->receiver() == 0)
        return this->monitor()->sender()->push_drop(cid);
    return this->monitor()->receiver()->push_drop(cid);
}
// }}}

