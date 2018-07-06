/*
 * MapCommunicate.h
 *
 * Created on: 2013-01-17 20:05
 *     Author: glendy
 */

#ifndef _MAPCOMMUNICATE_H_
#define _MAPCOMMUNICATE_H_

#include "Svc.h"
#include "ServicePacker.h"

////////outer communicate/////////////////////
// {{{
class MapClientService : public Svc
{
public:
    typedef ServiceMonitor<MapClientService> Monitor;
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

class MapClientPacker : public ServicePacker<MapClientService>
{
protected:
    virtual int process_block(Block_Buffer *buff);
    virtual int process_close_handler(int cid);
};
// }}}

////////inner accept communicate/////////////////////
// {{{
class MapInnerService : public Svc
{
public:
    typedef ServiceMonitor<MapInnerService> Monitor;
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

class MapInnerPacker : public ServicePacker<MapInnerService>
{
protected:
    virtual int process_block(Block_Buffer *buff);
    virtual int process_close_handler(int cid);
};
// }}}

////////inner connect communicate////////////////////
// {{{
class MapConnectService : public Svc
{
public:
    typedef ServiceMonitor<MapConnectService> Monitor;
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
#endif //_MAPCOMMUNICATE_H_
