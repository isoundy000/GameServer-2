/*
 * ChatCommunicate.h
 *
 * Created on: 2013-01-18 14:22
 *     Author: glendy
 */

#ifndef _CHATCOMMUNICATE_H_
#define _CHATCOMMUNICATE_H_

#include "Svc.h"
#include "ServicePacker.h"

////////outer communicate/////////////////////
// {{{
class ChatClientService : public Svc
{
public:
    typedef ServiceMonitor<ChatClientService> Monitor;
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

    void reset(void);
    virtual void set_msg_sequence(const uint64_t val);
    virtual uint64_t msg_sequence(void);

protected:
    Monitor *monitor_;
    uint64_t msg_sequence_;
};

class ChatClientPacker : public ServicePacker<ChatClientService>
{
public:
    virtual int process_block(Block_Buffer *buff);
    virtual int process_close_handler(int cid);
};
// }}}

////////inner accept communicate/////////////////////
// {{{
class ChatInnerService : public Svc
{
public:
    typedef ServiceMonitor<ChatInnerService> Monitor;
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

class ChatInnerPacker : public ServicePacker<ChatInnerService>
{
public:
    virtual int process_block(Block_Buffer *buff);
    virtual int process_close_handler(int cid);
};
// }}}

////////inner connect communicate////////////////////
// {{{
class ChatConnectService : public Svc
{
public:
    typedef ServiceMonitor<ChatConnectService> Monitor;
public:
    Monitor *monitor(Monitor *monitor = 0);
    virtual Block_Buffer *pop_block(int cid);
    virtual int push_block(int cid, Block_Buffer *block);
    virtual int register_send_handler(void);
    virtual int unregister_send_handler(void);
    virtual int close_handler(int cid);
protected:
    Monitor *monitor_;
};
// }}}

#endif //_CHATCOMMUNICATE_H_
