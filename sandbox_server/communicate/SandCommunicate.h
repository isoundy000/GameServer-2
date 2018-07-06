/*
 * SandCommunicate.h
 *
 * Created on: 2013-01-19 15:14
 *     Author: glendy
 */

#ifndef _SANDCOMMUNICATE_H_
#define _SANDCOMMUNICATE_H_

#include "Acceptor.h"
#include "Connector.h"
#include "Svc.h"
#include "Receiver.h"
#include "Sender.h"
#include "Stream_Packer.h"

class SandClientAcceptor : public Acceptor
{
public:
    virtual int accept_svc(int connfd);
};

class SandClientService : public Svc
{
public:
    virtual Block_Buffer *pop_block(int cid);
    virtual int push_block(int cid, Block_Buffer *block);
    virtual int register_recv_handler(void);
    virtual int unregister_recv_handler(void);
    virtual int register_send_handler(void);
    virtual int unregister_send_handler(void);
    virtual int recv_handler(int cid);
    virtual int close_handler(int cid);
};

class SandClientReceiver : public Receiver
{
public:
    virtual int drop_handler(int cid);
    virtual Svc *find_svc(int cid);
};

class SandClientSender : public Sender
{
public:
    virtual Block_Buffer *pop_block(int cid);
    virtual int push_block(int cid, Block_Buffer *buf);
    virtual int drop_handler(int cid);
    virtual Svc *find_svc(int cid);
};

class SandClientPacker : public Stream_Packer
{
public:
    virtual Svc *find_svc(int cid);
    virtual Block_Buffer *pop_block(int cid);
    virtual int push_block(int cid, Block_Buffer *block);
    virtual int packed_data_handler(Block_Vector &block_vec);
    virtual int drop_handler(int cid);  
};

#endif //_SANDCOMMUNICATE_H_
