/*
 * AuthCommunicate.cpp
 *
 * Created on: 2013-04-12 20:43
 *     Author: lyz
 */

#include "PubStruct.h"
#include "GameTimerHandler.h"
#include "PoolMonitor.h"
#include "AuthCommunicate.h"
#include "AuthMonitor.h"
#include "LogClientMonitor.h"

///////////client communicate///////////////////
// {{{
AuthClientService::Monitor *AuthClientService::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

Block_Buffer *AuthClientService::pop_block(int cid)
{
    return AUTH_MONITOR->pop_block(cid);
}

int AuthClientService::push_block(int cid, Block_Buffer *block)
{
    return AUTH_MONITOR->push_block(block, cid);
}

int AuthClientService::register_recv_handler(void)
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

int AuthClientService::unregister_recv_handler(void)
{
    if (this->get_reg_recv() == true)
    {
        this->monitor()->receiver()->unregister_svc(this);
        this->set_reg_recv(false);
    }
    return 0;
}

int AuthClientService::register_send_handler(void)
{
    if (this->get_reg_send() == false)
    {
        this->monitor()->sender(this->get_cid())->register_svc(this);
        this->set_reg_send(true);
    }
    return 0;
}

int AuthClientService::unregister_send_handler(void)
{
    if (this->get_reg_send() == true)
    {
        this->monitor()->sender(this->get_cid())->unregister_svc(this);
        this->set_reg_send(false);
    }
    return 0;
}

int AuthClientService::recv_handler(int cid)
{
    return this->monitor()->packer()->push_packing_cid(cid);
}

int AuthClientService::close_handler(int cid)
{
    return this->monitor()->receiver()->push_drop(cid);
}

void AuthClientService::set_msg_sequence(const uint64_t val)
{
    this->msg_sequence_ = val;
}

uint64_t AuthClientService::msg_sequence(void)
{
    return this->msg_sequence_;
}

int AuthClientPacker::process_block(Block_Buffer *buff)
{
    ProtoClientHead client_head;
    UnitMessage msg;
    client_head.reset();
    msg.reset();

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
    AuthClientService *svc = dynamic_cast<AuthClientService *>(this->find_svc(msg.__sid));
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

    AuthClientService *svc = dynamic_cast<AuthClientService *>(this->find_svc(msg.__sid));
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
    if (msg.__msg_head.__recogn != CLIENT_SERVER_KEEP_ALIVE && (msg.__msg_head.__recogn < 10700000 || 10800000 <= msg.__msg_head.__recogn))
    {
        return -1;
    }

#ifndef NO_USE_PROTO
    Message *proto_msg = parse_message(msg.__msg_head.__recogn, msg_buff);
    msg.set_msg_proto(proto_msg);
#else
    msg.set_data_buff(msg_buff);
#endif
    if (AUTH_MONITOR->logic_unit()->push_request(msg) != 0)
        return -1;

    if (msg.__type != UnitMessage::TYPE_BLOCK_BUFF)
        this->push_block(msg.__sid, msg_buff);

    return 0;
}

int AuthClientPacker::process_close_handler(int cid)
{
    return 0;
}
// }}}

////////////inner communicate////////////////////////
// {{{
AuthInnerService::Monitor *AuthInnerService::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

Block_Buffer *AuthInnerService::pop_block(int cid)
{
    return AUTH_MONITOR->pop_block(cid);
}

int AuthInnerService::push_block(int cid, Block_Buffer *block)
{
    return AUTH_MONITOR->push_block(block, cid);
}

int AuthInnerService::register_recv_handler(void)
{
    if (this->get_reg_recv() == false)
    {
        this->monitor()->receiver()->register_svc(this);
        this->set_reg_recv(true);
    }
    return 0;
}

int AuthInnerService::unregister_recv_handler(void)
{
    if (this->get_reg_recv() == true)
    {
        this->monitor()->receiver()->unregister_svc(this);
        this->set_reg_recv(false);
    }
    return 0;
}

int AuthInnerService::register_send_handler(void)
{
    if (this->get_reg_send() == false)
    {
        this->monitor()->sender(this->get_cid())->register_svc(this);
        this->set_reg_send(true);
    }
    return 0;
}

int AuthInnerService::unregister_send_handler(void)
{
    if (this->get_reg_send() == true)
    {
        this->monitor()->sender(this->get_cid())->unregister_svc(this);
        this->set_reg_send(false);
    }
    return 0;
}

int AuthInnerService::recv_handler(int cid)
{
    return this->monitor()->packer()->push_packing_cid(cid);
}

int AuthInnerService::close_handler(int cid)
{
    return this->monitor()->receiver()->push_drop(cid);
}

int AuthInnerPacker::process_block(Block_Buffer *buff)
{
    // TODO;
    return 0;
}

int AuthInnerPacker::process_close_handler(int cid)
{
    return 0;
}
// }}}

////////////connect communicate//////////////////////
// {{{
AuthConnectService::Monitor *AuthConnectService::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

Block_Buffer *AuthConnectService::pop_block(int cid)
{
    return AUTH_MONITOR->pop_block(cid);
}

int AuthConnectService::push_block(int cid, Block_Buffer *block)
{
    return AUTH_MONITOR->push_block(block, cid);
}

int AuthConnectService::register_recv_handler(void)
{
    if (this->get_reg_recv() == false && this->monitor()->receiver() != 0)
    {
        this->monitor()->receiver()->register_svc(this);
        this->set_reg_recv(true);
    }
    return 0;
}

int AuthConnectService::unregister_recv_handler(void)
{
    if (this->get_reg_recv() == true && this->monitor()->receiver() != 0)
    {
        this->monitor()->receiver()->unregister_svc(this);
        this->set_reg_recv(false);
    }
    return 0;
}

int AuthConnectService::register_send_handler(void)
{
    if (this->get_reg_send() == false && this->monitor()->sender() != 0)
    {
        this->monitor()->sender()->register_svc(this);
        this->set_reg_send(true);
    }
    return 0;
}

int AuthConnectService::unregister_send_handler(void)
{
    if (this->get_reg_send() == true && this->monitor()->sender() != 0)
    {
        this->monitor()->sender()->unregister_svc(this);
        this->set_reg_send(false);
    }
    return 0;
}

int AuthConnectService::recv_handler(int cid)
{
    if (this->monitor()->packer() == 0)
        return -1;
    return this->monitor()->packer()->push_packing_cid(cid);
}

int AuthConnectService::close_handler(int cid)
{
	if (LOG_CLIENT_MONITOR->get_log_sid() == cid)
		LOG_CLIENT_MONITOR->set_log_sid(-1);

    if (this->monitor()->receiver() == 0)
        return this->monitor()->sender()->push_drop(cid);
    return this->monitor()->receiver()->push_drop(cid);
}

int AuthConnectPacker::process_block(Block_Buffer *buff)
{
    UnitMessage msg;
    msg.reset();

    ProtoClientHead head;
    Block_Buffer *msg_buff = buff;
    msg_buff->read_int32(msg.__sid);
    msg_buff->read_uint32(msg.__len);
    if (msg_buff->readable_bytes() < sizeof(ProtoClientHead))
    {
        MSG_USER("ERROR recv error data len %d %d", msg.__sid, msg_buff->readable_bytes());
        return -1;
    }

    msg_buff->read((char *)&(head), sizeof(ProtoClientHead));
    msg.__msg_head.__recogn = head.__recogn;

    msg.__type = UnitMessage::TYPE_BLOCK_BUFF;
    msg.__data.__buf = msg_buff;
    if (AUTH_MONITOR->logic_unit()->push_request(msg) != 0)
        return -1;

    return 0;
}

int AuthConnectPacker::process_close_handler(int cid)
{
    return 0;
}
// }}}

