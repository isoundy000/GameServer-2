/*
 * ServicePacker.h
 *
 * Created on: 2013-04-08 18:56
 *     Author: lyz
 */

#ifndef _SERVICEPACKER_H_
#define _SERVICEPACKER_H_

#include "Stream_Packer.h"
#include "PoolMonitor.h"

template<class Service> class ServiceMonitor;
class Block_Buffer;
class Svc;

template <class Service>
class ServicePacker : public Stream_Packer
{
public:
    typedef ServiceMonitor<Service> Monitor;
public:
    ServicePacker(void);
    virtual ~ServicePacker(void);

    Monitor *monitor(Monitor *monitor = 0);
    virtual Svc *find_svc(int cid);
    virtual Block_Buffer *pop_block(int cid);
    virtual int push_block(int cid, Block_Buffer *block);
    virtual int packed_data_handler(Block_Vector &block_vec);
    virtual int drop_handler(int cid);

protected:
    virtual int process_block(Block_Buffer *buff) = 0;
    virtual int process_close_handler(int cid);

protected:
    Monitor *monitor_;
};

template <class Service>
ServicePacker<Service>::ServicePacker(void) :
    monitor_(0)
{ /*NULL*/ }

template <class Service>
ServicePacker<Service>::~ServicePacker(void)
{ /*NULL*/ }

template <class Service>
inline typename ServicePacker<Service>::Monitor *ServicePacker<Service>::monitor(typename ServicePacker<Service>::Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

template <class Service>
inline Svc *ServicePacker<Service>::find_svc(int cid)
{
    Svc *svc = 0;
    if (this->monitor()->find_service(cid, svc) != 0)
        return 0;
    return svc;
}

template <class Service>
inline Block_Buffer *ServicePacker<Service>::pop_block(int cid)
{
    return POOL_MONITOR->pop_buf_block(cid);
}

template <class Service>
inline int ServicePacker<Service>::push_block(int cid, Block_Buffer *block)
{
    return POOL_MONITOR->push_buf_block(block, cid);
}

template <class Service>
inline int ServicePacker<Service>::packed_data_handler(Block_Vector &block_vec)
{
    for (Block_Vector::iterator iter = block_vec.begin();
            iter != block_vec.end(); ++iter)
    {
        Block_Buffer *buff = *iter;
        int32_t sid = 0;
        buff->peek_int32(sid);
        if (this->process_block(*iter) != 0)
            this->push_block(sid, *iter);
    }

    return 0;
}

template <class Service>
inline int ServicePacker<Service>::drop_handler(int cid)
{
	Svc *svc = this->find_svc(cid);
    if (svc)
    {
        this->process_close_handler(cid);

        svc->close_fd();
        this->monitor()->unbind_service(cid);
        this->monitor()->service_pool()->push(dynamic_cast<Service *>(svc));
    }
    else
    {
    	LOG_USER_INFO("ERROR 2 drop close cid %d", cid);
    }
    return 0;
}

template <class Service>
inline int ServicePacker<Service>::process_close_handler(int cid)
{
    return 0;
}

#endif //_SERVICEPACKER_H_
