/*
 * LogicCommunicate.h
 *
 * Created on: 2013-01-07 16:38
 *     Author: glendy
 */

#ifndef _LOGICCOMMUNICATE_H_
#define _LOGICCOMMUNICATE_H_

#include "Svc.h"
#include "ServicePacker.h"

////////outer communicate/////////////////////
// {{{
class LogicClientService : public Svc
{
public:
    typedef ServiceMonitor<LogicClientService> Monitor;
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

class LogicClientPacker : public ServicePacker<LogicClientService>
{
protected:
    virtual int process_block(Block_Buffer *buff);
    virtual int process_close_handler(int cid);
};
// }}}

////////php communicate////////////////////////
// {{{
class LogicInnerService : public Svc
{
public:
    typedef ServiceMonitor<LogicInnerService> Monitor;
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

class LogicInnerPacker : public ServicePacker<LogicInnerService>
{
protected:
    virtual int process_block(Block_Buffer *buff);
    virtual int process_close_handler(int cid);
};
// }}}

////////connect communicate////////////////////
// {{{
class LogicConnectService : public Svc
{
public:
    typedef ServiceMonitor<LogicConnectService> Monitor;
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

class LogicConnectPacker : public ServicePacker<LogicConnectService>
{
protected:
    virtual int process_block(Block_Buffer *buff);
    virtual int process_close_handler(int cid);
};

// }}}

////////////php communicate////////////////////
// {{{
class LogicPhpService : public Svc
{
public:
    typedef ServiceMonitor<LogicPhpService> Monitor;
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

class LogicPhpPacker : public ServicePacker<LogicPhpService>
{
protected:
    virtual int process_block(Block_Buffer *buff);
    virtual int process_close_handler(int cid);
};

// }}}

#endif //_LOGICCOMMUNICATE_H_
