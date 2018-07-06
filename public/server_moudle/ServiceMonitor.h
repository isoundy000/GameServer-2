/*
 * ServiceMonitor.h
 *
 * Created on: 2013-04-08 17:00
 *     Author: lyz
 */

#ifndef _SERVICEMONITOR_H_
#define _SERVICEMONITOR_H_

#include "Svc_Static_List.h"

template <class Service>
class ServiceMonitor
{
public:
    typedef ObjectPoolEx<Service> ServicePool;
    typedef Svc_Static_List<Service *, Thread_Mutex> ServiceList;

    typedef ServiceAcceptor<Service> ServerAcceptor;
    typedef ServiceConnector<Service> ServerConnector;
    typedef ServiceReceiver<Service> ServerReceiver;
    typedef ServiceSender<Service> ServerSender;
    typedef ServicePacker<Service> ServerPacker;

public:
    ServiceMonitor(void);
    virtual ~ServiceMonitor(void);
    void fini(void);

    virtual ServerAcceptor *acceptor(void);
    virtual ServerConnector *connector(void);
    virtual ServerReceiver *receiver(ServerReceiver *receiver = 0);
    virtual void set_senders(ServerSender *sender, const int size = 1);
    virtual ServerSender *sender(const int index = 0);
    virtual ServerPacker *packer(ServerPacker *packer = 0);

    virtual ServicePool *service_pool(void);
    
    virtual int bind_service(Svc *svc);
    virtual int unbind_service(const int sid);
    virtual int find_service(const int sid, Svc *&svc);

    virtual int connect_amount(void);

    void set_svc_max_list_size(const int size);
    void set_svc_max_recv_size(const int size);
    void set_svc_max_send_size(const int size);
    void set_svc_max_pack_size(const int size);
    int svc_max_recv_size(void);
    int svc_max_send_size(void);
    int svc_max_pack_size(void);
   
protected:
    ServerAcceptor acceptor_;
    ServerConnector connector_;
    ServerReceiver *receiver_;
    int sender_size_;
    ServerSender *sender_;
    ServerPacker *packer_;

    ServicePool service_pool_;
    ServiceList service_list_;

    int client_amount_;
    int svc_max_recv_size_;
    int svc_max_send_size_;
    int svc_max_pack_size_;
};

template <class Service>
ServiceMonitor<Service>::ServiceMonitor(void)
{
    this->receiver_ = 0;
    this->sender_size_ = 0;
    this->sender_ = 0;
    this->packer_ = 0;
    this->acceptor_.monitor(this);
    this->connector_.monitor(this);
    this->client_amount_ = 0;
    this->svc_max_recv_size_ = 1000;
    this->svc_max_send_size_ = 1000;
    this->svc_max_pack_size_ = 1024 * 1024;
}

template <class Service>
ServiceMonitor<Service>::~ServiceMonitor(void)
{
    this->fini();
}

template <class Service>
void ServiceMonitor<Service>::fini(void)
{
    this->acceptor_.fini();
    std::vector<Service *> used_list = this->service_list_.erase_all_used();
    for (typename std::vector<Service *>::iterator iter = used_list.begin();
            iter != used_list.end(); ++iter)
    {
//        (*iter)->handle_close();
        (*iter)->close_fd();
        (*iter)->reset();
        this->service_pool_.push(*iter);
    }
    this->service_pool_.clear();
}

template <class Service>
inline typename ServiceMonitor<Service>::ServerAcceptor *ServiceMonitor<Service>::acceptor(void)
{
    return &(this->acceptor_);
}

template <class Service>
inline typename ServiceMonitor<Service>::ServerConnector *ServiceMonitor<Service>::connector(void)
{
    return &(this->connector_);
}

template <class Service>
inline typename ServiceMonitor<Service>::ServerReceiver *ServiceMonitor<Service>::receiver(ServiceMonitor<Service>::ServerReceiver *receiver)
{
    if (receiver != 0)
        this->receiver_ = receiver;
    return this->receiver_;
}

template <class Service>
void ServiceMonitor<Service>::set_senders(ServerSender *sender, const int size)
{
    this->sender_ = sender;
    this->sender_size_ = size;
}

template <class Service>
inline typename ServiceMonitor<Service>::ServerSender *ServiceMonitor<Service>::sender(const int index)
{
    int idx = index % this->sender_size_;
    return (this->sender_ + idx);
}

template <class Service>
inline typename ServiceMonitor<Service>::ServerPacker *ServiceMonitor<Service>::packer(ServiceMonitor<Service>::ServerPacker *packer)
{
    if (packer != 0)
        this->packer_ = packer;
    return this->packer_;
}

template <class Service>
inline typename ServiceMonitor<Service>::ServicePool *ServiceMonitor<Service>::service_pool(void)
{
    return &(this->service_pool_);
}

template <class Service>
inline int ServiceMonitor<Service>::bind_service(Svc *svc)
{
    Service *service = dynamic_cast<Service *>(svc);
    if (service == 0)
        return -1;

    int sid = this->service_list_.record_svc(service);
    if (sid == -1)
    {
        MSG_USER("cid == -1");
        return -1;
    }

    ++this->client_amount_;
    svc->set_cid(sid);
    return 0;
}

template <class Service>
inline int ServiceMonitor<Service>::unbind_service(const int sid)
{
    int ret = this->service_list_.erase_svc(sid);
    if (ret == 0)
        --this->client_amount_;
    return ret;
}

template <class Service>
inline int ServiceMonitor<Service>::find_service(const int sid, Svc *&svc)
{
    Service *service = 0;
    if (this->service_list_.get_used_svc(sid, service) == true)
    {
        svc = service;
        return 0;
    }
    return -1;
}

template <class Service>
inline int ServiceMonitor<Service>::connect_amount(void)
{
    return this->client_amount_;
}

template <class Service>
inline void ServiceMonitor<Service>::set_svc_max_list_size(const int size)
{
    this->svc_max_recv_size_ = size;
    this->svc_max_send_size_ = size;
}

template <class Service>
inline void ServiceMonitor<Service>::set_svc_max_recv_size(const int size)
{
    this->svc_max_recv_size_ = size;
}

template <class Service>
inline void ServiceMonitor<Service>::set_svc_max_send_size(const int size)
{
    this->svc_max_send_size_ = size;
}

template <class Service>
inline void ServiceMonitor<Service>::set_svc_max_pack_size(const int size)
{
    this->svc_max_pack_size_ = size;
}

template <class Service>
inline int ServiceMonitor<Service>::svc_max_recv_size(void)
{
    return this->svc_max_recv_size_;
}

template <class Service>
inline int ServiceMonitor<Service>::svc_max_send_size(void)
{
    return this->svc_max_send_size_;
}

template <class Service>
inline int ServiceMonitor<Service>::svc_max_pack_size(void)
{
    return this->svc_max_pack_size_;
}

#endif //_SERVICEMONITOR_H_
