/*
 * SandMonitor.h
 *
 * Created on: 2013-01-19 15:22
 *     Author: glendy
 */

#ifndef _SANDMONITOR_H_
#define _SANDMONITOR_H_

#include "Singleton.h"
#include "ObjectPoolEx.h"
#include "SandCommunicate.h"
#include "Svc_Static_List.h"

class SandMonitor
{
public:
    typedef ObjectPoolEx<SandClientService> ClientServicePool;
    typedef Svc_Static_List<SandClientService *, Thread_Mutex> ClientServiceList;

public:
    int init(void);
    int start(void);
    int stop(void);

    SandClientAcceptor &client_acceptor(void);
    SandClientReceiver &client_receiver(void);
    SandClientSender &client_sender(void);
    SandClientPacker &client_packer(void);
    
    Block_Buffer *pop_block(int cid = 0);
    int push_block(Block_Buffer *buff, int cid = 0);

    ClientServicePool *client_service_pool(void);

    int bind_client_service(SandClientService *svc);
    int unbind_client_service(const int sid);
    int find_client_service(const int sid, SandClientService *&svc);

    void set_server_config_index(const int index);

protected:
    int server_config_index_;

    ClientServicePool client_service_pool_;
    ClientServiceList client_service_list_;

    SandClientAcceptor client_acceptor_;
    SandClientReceiver client_receiver_;
    SandClientSender client_sender_;
    SandClientPacker client_packer_;
};

typedef Singleton<SandMonitor> SandMonitorSingle;
#define SAND_MONITOR    (SandMonitorSingle::instance())

#endif //_SANDMONITOR_H_
