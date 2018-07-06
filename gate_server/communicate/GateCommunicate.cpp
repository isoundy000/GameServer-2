/*
 * GateCommunicate.cpp
 *
 * Created on: 2013-04-12 16:46
 *     Author: lyz
 */

#include "PubStruct.h"
#include "GameTimerHandler.h"
#include "PoolMonitor.h"
#include "GateCommunicate.h"
#include "GateMonitor.h"
#include "LogClientMonitor.h"

///////////client communicate///////////////////
// {{{
GateClientService::GateClientService()
{
	this->msg_sequence_ = 0;
	this->monitor_ = NULL;
}

GateClientService::Monitor *GateClientService::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

Block_Buffer *GateClientService::pop_block(int cid)
{
    return GATE_MONITOR->pop_block(cid);
}

int GateClientService::push_block(int cid, Block_Buffer *block)
{
    return GATE_MONITOR->push_block(block, cid);
}

int GateClientService::register_recv_handler(void)
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

int GateClientService::unregister_recv_handler(void)
{
    if (this->get_reg_recv() == true)
    {
        this->monitor()->receiver()->unregister_svc(this);
        this->set_reg_recv(false);
    }
    return 0;
}

int GateClientService::register_send_handler(void)
{
    if (this->get_reg_send() == false)
    {
        this->monitor()->sender(this->get_cid())->register_svc(this);
        this->set_reg_send(true);
    }
    return 0;
}

int GateClientService::unregister_send_handler(void)
{
    if (this->get_reg_send() == true)
    {
        this->monitor()->sender(this->get_cid())->unregister_svc(this);
        this->set_reg_send(false);
    }
    return 0;
}

int GateClientService::recv_handler(int cid)
{
    return this->monitor()->packer()->push_packing_cid(cid);
}

int GateClientService::close_handler(int cid)
{
    return this->monitor()->receiver()->push_drop(cid);
}

void GateClientService::reset()
{
	Svc::reset();
	this->restore_ = true;
}

void GateClientService::set_msg_sequence(const uint64_t val)
{
    this->msg_sequence_ = val;
}

uint64_t GateClientService::msg_sequence(void)
{
    return this->msg_sequence_;
}

int GateClientPacker::process_block(Block_Buffer *buff)
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
    GateClientService *svc = dynamic_cast<GateClientService *>(this->find_svc(msg.__sid));
    if (svc == 0)
    {
        MSG_USER("ERROR client disconnected %d", msg.__sid);
        return -1;
    }

//    int svc_seq = svc->msg_sequence();
//	if (msg_seq <= svc_seq)
//	{
//		MSG_USER("ERROR data msg sequence %d %ld %ld", msg.__sid, msg_seq, svc_seq);
////		svc->handle_close();
//		return -1;
//	}
//	svc->set_msg_sequence(msg_seq);
#else
    if (msg_buff->readable_bytes() < (sizeof(ProtoClientHead) + sizeof(uint64_t)))
    {
        MSG_USER("ERROR recv error data len %d %d", msg.__sid, msg_buff->readable_bytes());
        return -1;
    }

    uint64_t msg_seq = 0;
    msg_buff->read_uint64(msg_seq);
    msg_seq = des_decrypt(msg_seq, MSG_SEQUENCE_KEY);

    GateClientService *svc = dynamic_cast<GateClientService *>(this->find_svc(msg.__sid));
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
    if (msg.__msg_head.__recogn != CLIENT_SERVER_KEEP_ALIVE &&
    		(msg.__msg_head.__recogn < 10000000 || 20000000 <= msg.__msg_head.__recogn ||
            msg.__msg_head.__recogn == CLIENT_MAP_CONNECT))
    {
        return -1;
    }

    msg.__route_head.__broad_type = BT_DIRECT_TARGET_SCENE;
    msg.__route_head.__recogn = msg.__msg_head.__recogn;
    msg.__route_head.__scene_id = 0;
    msg.__msg_head.__scene_id = 0;
    int route_type = msg.__msg_head.__recogn / 100000;
    switch (route_type)
    {
        case MSG_TYPE_CLIENT_TO_LOGIC:
        {
            msg.__route_head.__scene_id = SCENE_LOGIC;
            msg.__msg_head.__scene_id = SCENE_LOGIC;
            break;
        }
        case MSG_TYPE_CLIENT_TO_GATE:
        {
            msg.__route_head.__scene_id = GATE_MONITOR->gate_scene();
            msg.__msg_head.__scene_id = GATE_MONITOR->gate_scene();

#ifndef NO_USE_PROTO
            Message *proto_msg = parse_message(msg.__msg_head.__recogn, msg_buff);
            msg.set_msg_proto(proto_msg);
#else
            int recogn = msg.__msg_head.__recogn;
            if (recogn == CLIENT_GATE_SESSION || recogn == CLIENT_CREATE_ROLE || recogn == CLIENT_START_GAME)
            {
                msg.set_data_buff(msg_buff); 
            }
            else
            {
                Message *proto_msg = parse_message(msg.__msg_head.__recogn, msg_buff);
                msg.set_msg_proto(proto_msg);
            }
#endif
            if (GATE_MONITOR->logic_unit()->push_request(msg) != 0)
                return -1;

            if (msg.__type != UnitMessage::TYPE_BLOCK_BUFF)
                this->push_block(msg.__sid, msg_buff);
            return 0;
            break;
        }
        default:
            break;
    }

    msg.set_data_buff(msg_buff);
    if (GATE_MONITOR->logic_unit()->push_request(msg) != 0)
        return -1;

    return 0;
}

int GateClientPacker::process_close_handler(int cid)
{
	MSG_USER("close gate client sid %d", cid);
    // notify chat, map, logic unit to close;
    UnitMessage unit_msg;
    unit_msg.__type = UnitMessage::TYPE_PROTO_MSG;
    unit_msg.__sid = cid;
    unit_msg.__len = sizeof(ProtoHead) + 2;
    unit_msg.__msg_head.__recogn = CLIENT_STOP_GAME;
    return GATE_MONITOR->logic_unit()->push_request(unit_msg);
}
// }}}

/////////auth communicate////////////////////////////
// {{{
GateInnerService::Monitor *GateInnerService::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

Block_Buffer *GateInnerService::pop_block(int cid)
{
    return GATE_MONITOR->pop_block(cid);
}

int GateInnerService::push_block(int cid, Block_Buffer *block)
{
    return GATE_MONITOR->push_block(block, cid);
}

int GateInnerService::register_recv_handler(void)
{
    if (this->get_reg_recv() == false)
    {
        this->monitor()->receiver()->register_svc(this);
        this->set_reg_recv(true);
    }
    return 0;
}

int GateInnerService::unregister_recv_handler(void)
{
    if (this->get_reg_recv() == true)
    {
        this->monitor()->receiver()->unregister_svc(this);
        this->set_reg_recv(false);
    }
    return 0;
}

int GateInnerService::register_send_handler(void)
{
    if (this->get_reg_send() == false)
    {
        GATE_MONITOR->set_auth_sid(this->get_cid());
        this->monitor()->sender(this->get_cid())->register_svc(this);
        this->set_reg_send(true);
    }
    return 0;
}

int GateInnerService::unregister_send_handler(void)
{
    if (this->get_reg_send() == true)
    {
        GATE_MONITOR->set_auth_sid(0);
        this->monitor()->sender(this->get_cid())->unregister_svc(this);
        this->set_reg_send(false);
    }
    return 0;
}

int GateInnerService::recv_handler(int cid)
{
    return this->monitor()->packer()->push_packing_cid(cid);
}

int GateInnerService::close_handler(int cid)
{
    return this->monitor()->receiver()->push_drop(cid);
}

int GateInnerPacker::process_block(Block_Buffer *buff)
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
    msg.set_data_buff(msg_buff);
    if (GATE_MONITOR->logic_unit()->push_request(msg) != 0)
    {
        return -1;
    }

    if (msg.__type != UnitMessage::TYPE_BLOCK_BUFF)
        this->push_block(msg.__sid, msg_buff);

    return 0;
}

int GateInnerPacker::process_close_handler(int cid)
{
    return 0;
}
// }}}

//////// connect communicate///////////////////
// {{{
GateConnectService::Monitor *GateConnectService::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

Block_Buffer *GateConnectService::pop_block(int cid)
{
    return GATE_MONITOR->pop_block(cid);
}

int GateConnectService::push_block(int cid, Block_Buffer *block)
{
    return GATE_MONITOR->push_block(block, cid);
}

int GateConnectService::register_recv_handler(void)
{
    if (this->get_reg_recv() == false)
    {
        this->monitor()->receiver()->register_svc(this);
        this->set_reg_recv(true);
    }
    return 0;
}

int GateConnectService::unregister_recv_handler(void)
{
    if (this->get_reg_recv() == true)
    {
        this->monitor()->receiver()->unregister_svc(this);
        this->set_reg_recv(false);
    }
    return 0;
}

int GateConnectService::register_send_handler(void)
{
    if (this->get_reg_send() == false)
    {
        this->monitor()->sender()->register_svc(this);
        this->set_reg_send(true);
    }
    return 0;
}

int GateConnectService::unregister_send_handler(void)
{
    if (this->get_reg_send() == true)
    {
        this->monitor()->sender()->unregister_svc(this);
        this->set_reg_send(false);
    }
    return 0;
}

int GateConnectService::recv_handler(int cid)
{
    return this->monitor()->packer()->push_packing_cid(cid);
}

int GateConnectService::close_handler(int cid)
{
	if (LOG_CLIENT_MONITOR->get_log_sid() == cid)
		LOG_CLIENT_MONITOR->set_log_sid(-1);

    if (this->monitor()->receiver() == 0)
        return this->monitor()->sender()->push_drop(cid);
    return this->monitor()->receiver()->push_drop(cid);
}

int GateConnectPacker::process_block(Block_Buffer *buff)
{
    UnitMessage msg;

    Block_Buffer *msg_buff = buff;
    msg_buff->read_int32(msg.__sid);
    msg_buff->read_uint32(msg.__len);
    if (msg_buff->readable_bytes() < (sizeof(msg.__route_head) + sizeof(uint32_t) + sizeof(ProtoHead)))
    {
        MSG_USER("ERROR block len error %d %d", msg_buff->readable_bytes(), msg.__len);
        return -1;
    }
    msg_buff->read((char *)&(msg.__route_head), sizeof(InnerRouteHead));
    msg_buff->read_uint32(msg.__len);
    msg_buff->read((char *)&(msg.__msg_head), sizeof(ProtoHead));

//    msg.__route_head.__inner_req = 1;
    if (msg.__msg_head.__recogn == INNER_SYNC_ROLE_INFO ||
            msg.__msg_head.__recogn == INNER_SYNC_ROLE_NAME)
    {
        UnitMessage gate_msg;
        gate_msg = msg;
        gate_msg.__route_head.reset();
        std::string type_name = "Proto";
        char sz_recogn[32];
        ::sprintf(sz_recogn, "%d", gate_msg.__msg_head.__recogn);
        type_name += sz_recogn;

        int org_read_idx = msg_buff->get_read_idx();
        Message *proto_msg = parse_message(type_name, msg_buff);
        msg_buff->set_read_idx(org_read_idx);
        gate_msg.set_msg_proto(proto_msg);
        if (GATE_MONITOR->logic_unit()->push_request(gate_msg) != 0)
        {
            if (proto_msg != 0)
                delete proto_msg;
        }
    }

    if (msg.__route_head.__broad_type == BT_NOPLAYER_TARGET_SCENE)
    {
    	int read_index = msg_buff->get_read_idx() - sizeof(ProtoHead) - sizeof(uint32_t) - sizeof(int32_t);
    	msg_buff->set_read_idx(read_index);
    	int32_t *msg_len = ((int32_t *)(msg_buff->get_read_ptr())) + 1;
    	*msg_len = msg_buff->readable_bytes() - sizeof(int32_t) - sizeof(uint32_t);
    }
    else if (msg.__route_head.__broad_type == BT_BROAD_CLIENT)
    {
    	int read_index = msg_buff->get_read_idx() - sizeof(int32_t);
    	msg_buff->set_read_idx(read_index);
    }

    if (msg.__route_head.__broad_type == BT_DIRECT_TARGET_SCENE && msg.__route_head.__scene_id == SCENE_GATE)
    {
        std::string type_name = "Proto";
        char sz_recogn[32];
        ::sprintf(sz_recogn, "%d", msg.__msg_head.__recogn);
        type_name += sz_recogn;
    	msg.set_msg_proto(parse_message(type_name, msg_buff));
    }
    else
    {
    	msg.set_data_buff(msg_buff);
    }

    if (msg.__route_head.__broad_type == BT_BROAD_IN_GATE ||
    		msg.__route_head.__broad_type == BT_BROAD_CLIENT)
    {
        if (GATE_MONITOR->broad_unit(msg.__route_head.__scene_id)->push_request(msg) != 0)
        {
            MSG_USER("ERROR push to broad unit error");
            return -1;
        }
    }
    else
    {
        if (GATE_MONITOR->logic_unit()->push_request(msg) != 0)
        {
            MSG_USER("ERROR push to unit error");
            return -1;
        }
    }

    if (msg.__type != UnitMessage::TYPE_BLOCK_BUFF)
    {
        this->push_block(msg.__sid, msg_buff);
    }
    return 0;
}

int GateConnectPacker::process_close_handler(int cid)
{
	MSG_USER("close connect sid %d", cid);
    // notify chat, map, logic unit to close;
    UnitMessage unit_msg;
    unit_msg.__type = UnitMessage::TYPE_PROTO_MSG;
    unit_msg.__sid = cid;
    unit_msg.__len = sizeof(ProtoHead) + 2;
    unit_msg.__msg_head.__recogn = INNER_CONNECT_SVC_CLOSE;

    int config_index = GATE_MONITOR->fetch_config_index_by_sid(cid);

    Block_Buffer *buff = this->pop_block(cid);
    buff->write_int32(config_index);
    unit_msg.set_data_buff(buff);
    return GATE_MONITOR->logic_unit()->push_request(unit_msg);
}
// }}}

