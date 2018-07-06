/*
 * GateCommunicate.h
 *
 * Created on: 2013-04-12 16:39
 *     Author: lyz
 */

#ifndef _GATECOMMUNICATE_H_
#define _GATECOMMUNICATE_H_

#include "Svc.h"
#include "ServicePacker.h"

////////outer communicate/////////////////////
// {{{
class GateClientService : public Svc
{
public:
    typedef ServiceMonitor<GateClientService> Monitor;
public:
    GateClientService();

    Monitor *monitor(Monitor *monitor = 0);
    virtual Block_Buffer *pop_block(int cid);
    virtual int push_block(int cid, Block_Buffer *block);
    virtual int register_recv_handler(void);
    virtual int unregister_recv_handler(void);
    virtual int register_send_handler(void);
    virtual int unregister_send_handler(void);
    virtual int recv_handler(int cid);
    virtual int close_handler(int cid);

    void reset();
    virtual void set_msg_sequence(const uint64_t val);
    virtual uint64_t msg_sequence(void);

protected:
    Monitor *monitor_;
    uint64_t msg_sequence_;
};

class GateClientPacker : public ServicePacker<GateClientService>
{
protected:
    virtual int process_block(Block_Buffer *buff);
    virtual int process_close_handler(int cid);
};
// }}}

////////php communicate////////////////////////
// {{{
class GateInnerService : public Svc
{
public:
    typedef ServiceMonitor<GateInnerService> Monitor;
public:
    Monitor *monitor(Monitor *monitor = 0);
    virtual Block_Buffer *pop_block(int cid);
    virtual int push_block(int cid, Block_Buffer *block);
    virtual int register_recv_handler(void);
    virtual int unregister_recv_handler(void);
    virtual int register_send_handler(void);
    virtual int unregister_send_handler(void);
    virtual int recv_handler(int cid);
    virtual int close_handler(int cid);

protected:
    Monitor *monitor_;
};

class GateInnerPacker : public ServicePacker<GateInnerService>
{
protected:
    virtual int process_block(Block_Buffer *buff);
    virtual int process_close_handler(int cid);
};
// }}}

/////////connect communicate/////////////////////
// {{{
class GateConnectService : public Svc
{
public:
    typedef ServiceMonitor<GateConnectService> Monitor;
public:
    Monitor *monitor(Monitor *monitor = 0);
    virtual Block_Buffer *pop_block(int cid);
    virtual int push_block(int cid, Block_Buffer *block);
    virtual int register_recv_handler(void);
    virtual int unregister_recv_handler(void);
    virtual int register_send_handler(void);
    virtual int unregister_send_handler(void);
    virtual int recv_handler(int cid);
    virtual int close_handler(int cid);
protected:
    Monitor *monitor_;
};

class GateConnectPacker : public ServicePacker<GateConnectService>
{
protected:
    virtual int process_block(Block_Buffer *buff);
    virtual int process_close_handler(int cid);
};
// }}}

#endif //_GATECOMMUNICATE_H_
