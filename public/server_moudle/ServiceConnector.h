/*
 * ServiceConnector.h
 *
 * Created on: 2013-04-09 10:25
 *     Author: lyz
 */

#ifndef _SERVICECONNECTOR_H_
#define _SERVICECONNECTOR_H_

#include "Connector.h"

template<class Service>
class ServiceMonitor;

template <class Service>
class ServiceConnector : public Connector
{
public:
    typedef ServiceMonitor<Service> Monitor;
public:
    ServiceConnector(void);
    Monitor *monitor(Monitor *monitor = 0);

    virtual int connect_svc(int connfd);

protected:
    Monitor *monitor_;
};

template <class Service>
ServiceConnector<Service>::ServiceConnector(void) :
    monitor_(0)
{ /*NULL*/ }

template <class Service>
inline typename ServiceConnector<Service>::Monitor *ServiceConnector<Service>::monitor(typename ServiceConnector<Service>::Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

template <class Service>
inline int ServiceConnector<Service>::connect_svc(int connfd)
{
    Service *svc = this->monitor()->service_pool()->pop();
    if (!svc)
    {
        ::close(connfd);
        return -1;
    }

    svc->monitor(this->monitor());
    if (this->monitor()->bind_service(svc) != 0)
    {
        this->monitor()->service_pool()->push(svc);
        ::close(connfd);
        return -1;
    }

    svc->set_fd(connfd);
    svc->set_peer_addr();
    svc->set_max_recv_list_size(this->monitor()->svc_max_recv_size());
    svc->set_max_send_list_size(this->monitor()->svc_max_send_size());
    svc->set_max_pack_size(this->monitor()->svc_max_pack_size());
    if (this->monitor()->sender() != 0)
    	svc->register_send_handler();
    if (this->monitor()->receiver() != 0)
    	svc->register_recv_handler();
    return svc->get_cid();
}

#endif //_SERVICECONNECTOR_H_
