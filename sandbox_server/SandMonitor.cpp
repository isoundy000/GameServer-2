/*
 * SandMonitor.cpp
 *
 * Created on: 2013-01-19 15:34
 *     Author: glendy
 */

#include "SandMonitor.h"
#include "GameConfig.h"
#include "PoolMonitor.h"

int SandMonitor::init(void)
{
    {
        Time_Value recv_timeout(60, 0), send_timeout(0, 200 * 1000);
        this->client_receiver_.set(&recv_timeout);
        this->client_sender_.set(send_timeout);
    }

    this->server_config_index_ = -1;
    this->client_acceptor_.init();
    this->client_receiver_.init();
    this->client_sender_.init();

    return 0;
}

int SandMonitor::start(void)
{
    const Json::Value &server_json = CONFIG_INSTANCE->global()["server_list"][this->server_config_index_];
    int client_port = server_json["outer_port"].asInt();

    this->client_acceptor_.set(client_port);

    this->client_packer_.thr_create();
    this->client_sender_.thr_create();
    this->client_receiver_.thr_create();
    this->client_acceptor_.thr_create();

    return 0;
}

int SandMonitor::stop(void)
{
    return 0;
}

SandClientAcceptor &SandMonitor::client_acceptor(void)
{
    return this->client_acceptor_;
}

SandClientReceiver &SandMonitor::client_receiver(void)
{
    return this->client_receiver_;
}

SandClientSender &SandMonitor::client_sender(void)
{
    return this->client_sender_;
}

SandClientPacker &SandMonitor::client_packer(void)
{
    return this->client_packer_;
}

Block_Buffer *SandMonitor::pop_block(int cid)
{
    return POOL_MONITOR->pop_buf_block(cid);
}

int SandMonitor::push_block(Block_Buffer *buff, int cid)
{
    return POOL_MONITOR->push_buf_block(buff, cid);
}

SandMonitor::ClientServicePool *SandMonitor::client_service_pool(void)
{
    return &(this->client_service_pool_);
}

int SandMonitor::bind_client_service(SandClientService *svc)
{
    int sid = this->client_service_list_.record_svc(svc);
    if (sid == -1)
    {
    	LOG_USER_INFO("cid == -1");
        return -1;
    }

    svc->set_cid(sid);
    return 0;
}

int SandMonitor::unbind_client_service(const int sid)
{
    return this->client_service_list_.erase_svc(sid);
}

int SandMonitor::find_client_service(const int sid, SandClientService *&svc)
{
    if (this->client_service_list_.get_used_svc(sid, svc) == true)
        return 0;
    return -1;
}

void SandMonitor::set_server_config_index(const int index)
{
    this->server_config_index_ = index;
}

