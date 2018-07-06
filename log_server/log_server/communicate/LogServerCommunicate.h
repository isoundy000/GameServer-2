/*
 * LogServerCommunicate.h
 *
 * Created on: 2013-01-09 14:37
 *     Author: glendy
 */

#ifndef _LOGSERVERCOMMUNICATE_H_
#define _LOGSERVERCOMMUNICATE_H_

#include "Acceptor.h"
#include "Receiver.h"
#include "Stream_Packer.h"

class LogServerAcceptor : public Acceptor
{
public:
    virtual int accept_svc(int connfd);
};

class LogServerService : public Svc
{
public:
    virtual Block_Buffer *pop_block(int cid);
    virtual int push_block(int cid, Block_Buffer *block);
    virtual int register_recv_handler(void);
    virtual int unregister_recv_handler(void);
    virtual int recv_handler(int cid);
    virtual int close_handler(int cid);
};

class LogServerReceiver : public Receiver
{
public:
    virtual int drop_handler(int cid);
    virtual Svc *find_svc(int cid);
};

class LogServerPacker : public Stream_Packer
{
public:
    virtual Svc *find_svc(int cid);
    virtual Block_Buffer *pop_block(int cid);
    virtual int push_block(int cid, Block_Buffer *block);
    virtual int packed_data_handler(Block_Vector &block_vec);
    virtual int drop_handler(int cid);  
};

#endif //_LOGSERVERCOMMUNICATE_H_
