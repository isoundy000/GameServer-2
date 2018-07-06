/*
 * AuthCommunicate.h
 *
 * Created on: 2013-04-12 20:35
 *     Author: lyz
 */

#ifndef _AUTHCOMMUNICATE_H_
#define _AUTHCOMMUNICATE_H_

#include "Svc.h"
#include "ServicePacker.h"

///////////client communicate/////////////////////////
// {{{
class AuthClientService : public Svc
{
public:
    typedef ServiceMonitor<AuthClientService> Monitor;
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

    virtual void set_msg_sequence(const uint64_t val);
    virtual uint64_t msg_sequence(void);

protected:
    Monitor *monitor_;
    uint64_t msg_sequence_;
};

class AuthClientPacker : public ServicePacker<AuthClientService>
{
protected:
    virtual int process_block(Block_Buffer *buff);
    virtual int process_close_handler(int cid);
};
// }}}

///////////inner communicate/////////////////////////
// {{{
class AuthInnerService : public Svc
{
public:
    typedef ServiceMonitor<AuthInnerService> Monitor;
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

class AuthInnerPacker : public ServicePacker<AuthInnerService>
{
protected:
    virtual int process_block(Block_Buffer *buff);
    virtual int process_close_handler(int cid);
};
// }}}

///////////connect communicate/////////////////////////
// {{{
class AuthConnectService : public Svc
{
public:
    typedef ServiceMonitor<AuthConnectService> Monitor;
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

class AuthConnectPacker : public ServicePacker<AuthConnectService>
{
protected:
    virtual int process_block(Block_Buffer *buff);
    virtual int process_close_handler(int cid);
};
// }}}

#endif //_AUTHCOMMUNICATE_H_
