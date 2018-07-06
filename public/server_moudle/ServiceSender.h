/*
 * ServiceSender.h
 *
 * Created on: 2013-04-08 17:45
 *     Author: lyz
 */

#ifndef _SERVICESENDER_H_
#define _SERVICESENDER_H_

#include "Sender.h"

template<class Service>
class ServiceMonitor;
class Block_Buffer;
class Svc;

template <class Service>
class ServiceSender : public Sender
{
public:
    typedef ServiceMonitor<Service> Monitor;
public:
    ServiceSender(void);
    virtual Monitor *monitor(Monitor *monitor = 0);

    virtual Block_Buffer *pop_block(int cid);
    virtual int push_block(int cid, Block_Buffer *buf);
    virtual int drop_handler(int cid);
    virtual Svc *find_svc(int cid);

protected:
    Monitor *monitor_;
};

template <class Service>
ServiceSender<Service>::ServiceSender(void) :
    monitor_(0)
{ /*NULL*/ }

template <class Service>
inline typename ServiceSender<Service>::Monitor *ServiceSender<Service>::monitor(typename ServiceSender<Service>::Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

template <class Service>
inline Block_Buffer *ServiceSender<Service>::pop_block(int cid)
{
    return POOL_MONITOR->pop_buf_block(cid);
}

template <class Service>
inline int ServiceSender<Service>::push_block(int cid, Block_Buffer *buf)
{
    return POOL_MONITOR->push_buf_block(buf, cid);
}

template <class Service>
inline int ServiceSender<Service>::drop_handler(int cid)
{
    if (this->monitor()->packer() != 0)
    {
        return this->monitor()->packer()->push_drop(cid);
    }

    Svc *svc = this->find_svc(cid);
    if (svc)
    {
        svc->close_fd();
        this->monitor()->unbind_service(cid);
        this->monitor()->service_pool()->push(dynamic_cast<Service *>(svc));
    }
    else
    {
    	LOG_USER_INFO("ERROR 1 drop close cid %d", cid);
    }
    return 0;
}

template <class Service>
inline Svc *ServiceSender<Service>::find_svc(int cid)
{
    Svc *svc = 0;
    if (this->monitor()->find_service(cid, svc) != 0)
        return 0;
    return svc;
}

#endif //_SERVICESENDER_H_
