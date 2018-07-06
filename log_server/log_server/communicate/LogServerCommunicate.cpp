/*
 * LogServerCommunicate.cpp
 *
 * Created on: 2013-01-09 14:43
 *     Author: glendy
 */

#include "LogServerCommunicate.h"
#include "LogServerMonitor.h"
#include <google/protobuf/message.h>

int LogServerAcceptor::accept_svc(int connfd)
{
    LogServerService *svc = LOG_SERVER_MONITOR->server_service_pool()->pop();
    if (!svc)
    {
        ::close(connfd);
        return -1;
    }

    if (LOG_SERVER_MONITOR->bind_server_service(svc) != 0)
    {
        LOG_SERVER_MONITOR->server_service_pool()->push(svc);
        ::close(connfd);
        return -1;
    }

    svc->set_fd(connfd);
    svc->set_max_list_size(1000);
    svc->set_max_pack_size(1024 * 1024 * 3);
    svc->register_recv_handler();
    return 0;
}

Block_Buffer *LogServerService::pop_block(int cid)
{
    return LOG_SERVER_MONITOR->pop_block(cid);
}

int LogServerService::push_block(int cid, Block_Buffer *block)
{
    return LOG_SERVER_MONITOR->push_block(block, cid);
}

int LogServerService::register_recv_handler(void)
{
    if (this->get_reg_recv() == false)
    {
        LOG_SERVER_MONITOR->server_receiver().register_svc(this);
        this->set_reg_recv(true);
    }
    return 0;
}

int LogServerService::unregister_recv_handler(void)
{
    if (this->get_reg_recv() == true)
    {
        LOG_SERVER_MONITOR->server_receiver().unregister_svc(this);
        this->set_reg_recv(false);
    }
    return 0;
}

int LogServerService::recv_handler(int cid)
{
    return LOG_SERVER_MONITOR->server_packer().push_packing_cid(cid);
}

int LogServerService::close_handler(int cid)
{
    return LOG_SERVER_MONITOR->server_receiver().push_drop(cid);
}

int LogServerReceiver::drop_handler(int cid)
{
    return LOG_SERVER_MONITOR->server_packer().push_drop(cid);
}

Svc *LogServerReceiver::find_svc(int cid)
{
    LogServerService *svc = 0;
    if (LOG_SERVER_MONITOR->find_server_service(cid, svc) != 0)
        return 0;
    return svc;
}

Svc *LogServerPacker::find_svc(int cid)
{
    LogServerService *svc = 0;
    if (LOG_SERVER_MONITOR->find_server_service(cid, svc) != 0)
        return 0;
    return svc;
}

Block_Buffer *LogServerPacker::pop_block(int cid)
{
    return LOG_SERVER_MONITOR->pop_block(cid);
}

int LogServerPacker::push_block(int cid, Block_Buffer *block)
{
    return LOG_SERVER_MONITOR->push_block(block, cid);
}

int LogServerPacker::packed_data_handler(Block_Vector &block_vec)
{
    UnitMessage msg;
    Block_Buffer *msg_buff = 0;
    std::string type_name;
    char sz_recogn[32];
    for (Block_Vector::iterator iter = block_vec.begin();
            iter != block_vec.end(); ++iter)
    {
    	::google::protobuf::Message *proto_msg = 0;
        msg.reset();
        msg_buff = *iter;
        msg.__type = UnitMessage::TYPE_PROTO_MSG;
        msg_buff->read_int32(msg.__sid);
        msg_buff->read_uint32(msg.__len);
        msg_buff->read((char *)&(msg.__msg_head), sizeof(ProtoHead));

        if (msg.__msg_head.__recogn == INNER_MYSQL_INSERT
        		|| msg.__msg_head.__recogn == INNER_MYSQL_INSERT_WITH_TABLE_NAME)
        {
            msg_buff->read_string(type_name);
            proto_msg = parse_message(type_name, msg_buff);
            msg.__data.__proto_msg = proto_msg;
        }
        else
        {
            type_name = "Proto";
            ::sprintf(sz_recogn, "%d", msg.__msg_head.__recogn);
            type_name += sz_recogn;
            proto_msg = parse_message(type_name, msg_buff);
            msg.__data.__proto_msg = proto_msg;
        }

        int ret = 0;
        if ((msg.__msg_head.__recogn / 10000) == MSG_TYPE_INNER_TO_LOG_LOG)
            ret = LOG_SERVER_MONITOR->log_unit().push_request(msg);
        else 
            ret = LOG_SERVER_MONITOR->mysql_unit().push_request(msg);
        if (ret != 0)
        {
            this->push_block(msg.__sid, msg_buff);
            if (proto_msg != 0)
                delete proto_msg;
            continue;
        }

        if (msg.__type != UnitMessage::TYPE_BLOCK_BUFF)
        {
            this->push_block(msg.__sid, msg_buff);
            msg_buff = 0;
        }
    }
    return 0;
}

int LogServerPacker::drop_handler(int cid)
{
    LogServerService *svc = dynamic_cast<LogServerService *>(this->find_svc(cid));
    if (svc)
    {
        svc->close_fd();
        LOG_SERVER_MONITOR->unbind_server_service(cid);
        LOG_SERVER_MONITOR->server_service_pool()->push(svc);
    }
    return 0;
}

