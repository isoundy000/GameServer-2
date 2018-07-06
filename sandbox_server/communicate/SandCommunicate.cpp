/*
 * SandCommunicate.cpp
 *
 * Created on: 2013-01-19 15:17
 *     Author: glendy
 */

#include "SandCommunicate.h"
#include "SandMonitor.h"

int SandClientAcceptor::accept_svc(int connfd)
{
    SandClientService *svc = SAND_MONITOR->client_service_pool()->pop();
    if (!svc)
    {
        ::close(connfd);
        return -1;
    }

    if (SAND_MONITOR->bind_client_service(svc) != 0)
    {
        SAND_MONITOR->client_service_pool()->push(svc);
        ::close(connfd);
        return -1;
    }

    svc->set_fd(connfd);
    svc->register_send_handler();
    svc->register_recv_handler();
    return 0;
}

Block_Buffer *SandClientService::pop_block(int cid)
{
    return SAND_MONITOR->pop_block(cid);
}

int SandClientService::push_block(int cid, Block_Buffer *block)
{
    return SAND_MONITOR->push_block(block, cid);
}

int SandClientService::register_recv_handler(void)
{
    if (this->get_reg_recv() == false)
    {
        SAND_MONITOR->client_receiver().register_svc(this);
        this->set_reg_recv(true);
    }
    return 0;
}

int SandClientService::unregister_recv_handler(void)
{
    if (this->get_reg_recv() == true)
    {
        SAND_MONITOR->client_receiver().unregister_svc(this);
        this->set_reg_recv(false);
    }
    return 0;
}

int SandClientService::register_send_handler(void)
{
    if (this->get_reg_send() == false)
    {
        SAND_MONITOR->client_sender().register_svc(this);
        this->set_reg_send(true);
    }
    return 0;
}

int SandClientService::unregister_send_handler(void)
{
    if (this->get_reg_send() == true)
    {
        SAND_MONITOR->client_sender().unregister_svc(this);
        this->set_reg_send(false);
    }
    return 0;
}

int SandClientService::recv_handler(int cid)
{
    return SAND_MONITOR->client_packer().push_packing_cid(cid);
}

int SandClientService::close_handler(int cid)
{
    return SAND_MONITOR->client_receiver().push_drop(cid);
}

int SandClientReceiver::drop_handler(int cid)
{
    return SAND_MONITOR->client_sender().push_drop(cid);
}

Svc *SandClientReceiver::find_svc(int cid)
{
    SandClientService *svc = 0;
    if (SAND_MONITOR->find_client_service(cid, svc) != 0)
        return 0;
    return svc;
}

Block_Buffer *SandClientSender::pop_block(int cid)
{
    return SAND_MONITOR->pop_block(cid);
}

int SandClientSender::push_block(int cid, Block_Buffer *buf)
{
    return SAND_MONITOR->push_block(buf, cid);
}

int SandClientSender::drop_handler(int cid)
{
    return SAND_MONITOR->client_packer().push_drop(cid);
}

Svc *SandClientSender::find_svc(int cid)
{
    SandClientService *svc = 0;
    if (SAND_MONITOR->find_client_service(cid, svc) != 0)
        return 0;
    return svc;
}

Svc *SandClientPacker::find_svc(int cid)
{
    SandClientService *svc = 0;
    if (SAND_MONITOR->find_client_service(cid, svc) != 0)
        return 0;
    return svc;
}

Block_Buffer *SandClientPacker::pop_block(int cid)
{
    return SAND_MONITOR->pop_block(cid);
}

int SandClientPacker::push_block(int cid, Block_Buffer *block)
{
    return SAND_MONITOR->push_block(block, cid);
}

int SandClientPacker::packed_data_handler(Block_Vector &block_vec)
{
    static const char req_flash_sand_box[] = "<policy-file-request/>";
    static const char res_flash_sand_box[] =
        "<?xml version=\"1.0\"?><cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"*\"/></cross-domain-policy>\0";

    Block_Buffer *msg_buff = 0;
    int32_t sid = 0;
    uint32_t len = 0;
    Svc *svc = 0;
    for (Block_Vector::iterator iter = block_vec.begin();
            iter != block_vec.end(); ++iter)
    {
        msg_buff = *iter;
        msg_buff->read_int32(sid);
        msg_buff->read_uint32(len);

        if ((svc = this->find_svc(sid)) == 0)
            continue;

        if (msg_buff->readable_bytes() > sizeof(req_flash_sand_box))
        {
            this->push_block(sid, msg_buff);
            svc->handle_close();
            continue;
        }

        if (0 != ::strncmp(req_flash_sand_box, msg_buff->get_read_ptr(), sizeof(req_flash_sand_box)))
        {
            this->push_block(sid, msg_buff);
            svc->handle_close();
            continue;
        }

        msg_buff->reset();
        msg_buff->ensure_writable_bytes(sizeof(res_flash_sand_box) + sizeof(uint32_t) * 4);
        msg_buff->write_int32(sid);
        msg_buff->copy(res_flash_sand_box, sizeof(res_flash_sand_box));
        if (SAND_MONITOR->client_sender().push_pool_block_with_len(msg_buff) != 0)
            this->push_block(sid, msg_buff);
    }

    return 0;
}

int SandClientPacker::drop_handler(int cid)
{
    SandClientService *svc = dynamic_cast<SandClientService *>(this->find_svc(cid));
    if (svc)
    {
        // close socket;
        svc->close_fd();
        SAND_MONITOR->unbind_client_service(cid);
        SAND_MONITOR->client_service_pool()->push(svc);
    }
    return 0;
}

