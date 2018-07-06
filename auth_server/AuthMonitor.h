/*
 * AuthMonitor.h
 *
 * Created on: 2013-04-12 21:42
 *     Author: lyz
 */

#ifndef _AUTHMONITOR_H_
#define _AUTHMONITOR_H_

#include "Singleton.h"
#include "AuthCommunicate.h"
#include "AuthUnit.h"
#include "ServerDefine.h"

class AuthMonitor : public ServerMonitor<AuthClientService, AuthInnerService, AuthConnectService>
{
public:
	enum
	{
		AUTH_SESSION_TIMEOUT = 1800
	};

    struct HistoryTick
    {
        Int64 __tick_one;
        Int64 __tick_two;

        HistoryTick(void);
    };

    typedef ServerMonitor<AuthClientService, AuthInnerService, AuthConnectService> SUPPER;
    typedef ServiceReceiver<AuthConnectService> ConnectReceiver;
    typedef ObjectPoolEx<UnitMessage> UnitMessagePool;

    typedef HashMap<int, int, NULL_MUTEX> SceneToAmountMap;
    typedef boost::unordered_map<std::string, HistoryTick> HistoryTickMap;

public:
    virtual int init(void);
    virtual int start(void);
    virtual int start_game(void);
    virtual void fina(void);

    virtual BaseUnit *logic_unit(void);

    Block_Buffer *pop_block(int cid = 0);
    int push_block(Block_Buffer *buff, int cid = 0);

    UnitMessagePool *unit_msg_pool(void);

    virtual int dispatch_to_all_gate(const int recogn, Block_Buffer *body = 0);

    int refresh_gate_roleamount(Block_Buffer *msg);
    int close_gate_role(Block_Buffer *msg);
#ifndef NO_USE_PROTO
    int validate_auth_session(const int sid, Message *msg);
#else
    int validate_auth_session(const int sid, Block_Buffer *msg);
#endif
    int synced_gate_session(Block_Buffer *msg);

    int dispatch_to_client(const int sid, const int recogn, Block_Buffer *body);
    int dispatch_to_client(const int sid, const int recogn, const int error = 0, Message *body = 0);
    int dispatch_to_gate(const int scene_id, const int recogn, Block_Buffer *body = 0);

    int update_server_config(void);

protected:
    virtual int process_init_scene(const int scene_id, const int config_index, const int space_id = 0);

    virtual int init_game_timer_handler(void);
    virtual int start_game_timer_handler(void);

protected:
    AuthClientPacker client_packer_;
    AuthInnerPacker inner_packer_;

    ConnectReceiver connect_receiver_;
    AuthConnectPacker connect_packer_;

    AuthUnit logic_unit_;

    SceneToAmountMap gate_to_amount_map_;

    HistoryTickMap history_tick_map_;
};

typedef Singleton<AuthMonitor> AuthMonitorSingle;
#define AUTH_MONITOR    (AuthMonitorSingle::instance())

#endif //_AUTHMONITOR_H_
