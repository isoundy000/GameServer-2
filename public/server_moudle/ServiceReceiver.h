/*
 * ServiceReceiver.h
 *
 * Created on: 2013-04-08 17:39
 *     Author: lyz
 */

#ifndef _SERVICERECEIVER_H_
#define _SERVICERECEIVER_H_

#include "Receiver.h"

template<class Service>
class ServiceMonitor;
class Svc;

template <class Service>
class ServiceReceiver : public Receiver
{
public:
    typedef ServiceMonitor<Service> Monitor;
public:
    ServiceReceiver(void);
    virtual Monitor *monitor(Monitor *monitor = 0);

    virtual int drop_handler(int cid);
    virtual Svc *find_svc(int cid);

protected:
    Monitor *monitor_;
};

template <class Service>
ServiceReceiver<Service>::ServiceReceiver(void) :
    monitor_(0)
{ /*NULL*/ }

template <class Service>
inline typename ServiceReceiver<Service>::Monitor *ServiceReceiver<Service>::monitor(typename ServiceReceiver<Service>::Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

template <class Service>
inline int ServiceReceiver<Service>::drop_handler(int cid)
{
    return this->monitor()->sender(cid)->push_drop(cid);
}

template <class Service>
inline Svc *ServiceReceiver<Service>::find_svc(int cid)
{
    Svc *svc = 0;
    if (this->monitor()->find_service(cid, svc) != 0)
        return 0;
    return svc;
}

#endif //_SERVICERECEIVER_H_
