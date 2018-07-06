/*
 * ServiceAcceptor.h
 *
 * Created on: 2013-04-08 17:33
 *     Author: lyz
 */

#ifndef _SERVICEACCEPTOR_H_
#define _SERVICEACCEPTOR_H_

#include "Acceptor.h"
#include "Log.h"

template<class Service>
class ServiceMonitor;

template <class Service>
class ServiceAcceptor : public Acceptor
{
public:
    typedef ServiceMonitor<Service> Monitor;
public:
    ServiceAcceptor(void);
    virtual int accept_svc(int connfd);
    virtual Monitor *monitor(Monitor *monitor = 0);
    virtual void set_limit_connect(const int amount);

protected:
    Monitor *monitor_;
    int limit_connect_;
};

template <class Service>
ServiceAcceptor<Service>::ServiceAcceptor(void) :
    monitor_(0), limit_connect_(10000)
{ /*NULL*/ }

template <class Service>
inline typename ServiceAcceptor<Service>::Monitor *ServiceAcceptor<Service>::monitor(typename ServiceAcceptor<Service>::Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

template <class Service>
inline int ServiceAcceptor<Service>::accept_svc(int connfd)
{
    int connect_amount = this->monitor()->connect_amount();
    if (connect_amount > this->limit_connect_)
    {
        ::close(connfd);
        MSG_USER("ERROR coonect too more %d %d", connect_amount, this->limit_connect_);
        return -1;
    }

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
    if (this->monitor()->sender(svc->get_cid()) != 0)
    	svc->register_send_handler();
    if (this->monitor()->receiver() != 0)
    	svc->register_recv_handler();

    std::string address;
    int port = 0;
    svc->get_peer_addr(address, port);
    MSG_USER("accept %s:%d[%d]", address.c_str(), port, svc->get_cid());
    return 0;
}

template <class Service>
inline void ServiceAcceptor<Service>::set_limit_connect(const int amount)
{
    this->limit_connect_ = amount;
}

#endif //_SERVICEACCEPTOR_H_
