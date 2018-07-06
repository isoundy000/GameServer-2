/*
 * GateMonitor.h
 *
 * Created on: 2013-04-12 16:37
 *     Author: lyz
 */

#ifndef _GATEMONITOR_H_
#define _GATEMONITOR_H_

#include "Singleton.h"
#include "ObjectPoolEx.h"
#include "ServerDefine.h"
#include "GatePlayer.h"
#include "GateCommunicate.h"
#include "GateUnit.h"
#include "BroadUnit.h"
#include "GateTimerHandler.h"

class SessionManager;
class ConnectUnit;

class GateMonitor : public ServerMonitor<GateClientService, GateInnerService, GateConnectService>
{
    friend class ConnectUnit;

public:
    typedef ServerMonitor<GateClientService, GateInnerService, GateConnectService> SUPPER;
    typedef ServiceReceiver<GateConnectService> ConnectReceiver;

    typedef ObjectPoolEx<UnitMessage> UnitMessagePool;
    typedef ObjectPoolEx<GatePlayer> GatePlayerPool;

    typedef HashMap<std::string, int64_t, NULL_MUTEX> GlobalKeyMap;
    typedef HashMap<int64_t, GatePlayer *, NULL_MUTEX> PlayerMap;
    typedef HashMap<int, std::string, NULL_MUTEX> SidAccountMap;
    typedef std::vector<GateTimerHandler> TimerHandlerList;
    typedef boost::unordered_map<std::string, int> AccountSidMap;
    typedef HashMap<int64_t, int, RW_MUTEX> PlayerSidMap;

    typedef HashMap<int, int, NULL_MUTEX> TravelSceneLineMap;
    typedef HashMap<std::string, GatePlayer *, NULL_MUTEX> AccountPlayerMap;
    typedef HashMap<std::string, int64_t, NULL_MUTEX> AccountLoginTickMap;

    enum
    {
    	ACCOUNT_RELOGIN		= 0,	//帐号重登
    	SOCK_DISCOUNT		= 101,	//Sock断开
    	SAME_ACCCCOUNT		= 102,	//相同帐号
    	STOP_SERVER			= 103,	//关服
    	STOP_SAME_SID		= 104,	//SID相同
    	END
    };

    struct GateOneSecTimer : public GameTimer
    {
    	GateOneSecTimer(void);
        virtual ~GateOneSecTimer(void);

        virtual int type(void);
        virtual int handle_timeout(const Time_Value &nowtime);
    };

    struct GateTenSecTimer : public GameTimer
    {
    	GateTenSecTimer(void);
        virtual ~GateTenSecTimer(void);

        virtual int type(void);
        virtual int handle_timeout(const Time_Value &nowtime);
    };

public:
    GateMonitor(void);

    virtual int init(const int config_index);
    virtual int start(void);
    virtual int start_game(void);
    virtual int stop(void);
    virtual void fina(void);
    int logout_all_player(void);

    virtual BaseUnit *logic_unit(void);
    BaseUnit *broad_unit(const int index = 0);
    BaseUnit *connect_unit(void);

    SessionManager *session_manager(void);

    Block_Buffer *pop_block(int cid = 0);
    int push_block(Block_Buffer *buff, int cid = 0);

    UnitMessagePool *unit_msg_pool(void);
    GatePlayerPool *player_pool(void);

    virtual int check_all_connect_set(void);
    virtual int fetch_sid_of_scene(const int scene_id, const bool for_log = false, const int64_t line_id = 0);
    int make_up_client_block(Block_Buffer *buff, const int recogn, const int error = 0, Block_Buffer *body = 0);
    int dispatch_to_scene(ProtoHead *head, Block_Buffer *body);
    int dispatch_to_scene(ProtoHead *head, const Message *msg_proto);
    int dispatch_to_scene_by_sid(int sid, ProtoHead *head, Block_Buffer *body);
    int dispatch_to_scene_by_sid(int sid, ProtoHead *head, const Message *msg_proto);
    int dispatch_to_server(UnitMessage *unit_msg);
    int dispatch_to_client(const int sid, const Message *msg_proto);
    int dispatch_to_client(const int sid, const int recogn, const int error = 0, const Message *msg_proto = 0);
    int dispatch_to_auth(const int auth_sid, const int recogn, Block_Buffer *body);
    int dispatch_to_logic(const int recogn);

    int set_role_scene_info(const int client_sid, int64_t &role_id, int &scene_id);
    int fetch_line_id_by_scene(const Int64 role_id, const int scene_id);

    int bind_player_by_role_id(const Int64 role_id, GatePlayer *player);
    int unbind_player_by_role_id(const Int64 role_id);
    int find_player(const int64_t role_id, GatePlayer *&player);

    int bind_sid_by_role_id(const Int64 role_id, const int sid);
    int unbind_sid_by_role_id(const Int64 role_id);
    int find_sid_by_role_id(const int64_t role_id);

    int bind_sid_player(const int sid, GatePlayer *player);
    int unbind_sid_player(const int sid);
    int find_player_by_sid(const int sid, GatePlayer *&player);

    int bind_sid_account(const int sid, std::string &account);
    int unbind_sid_account(const int sid);
    int find_sid_account(const int sid, std::string &account);

    int bind_account_sid(std::string &account, const int sid);
    int unbind_account_sid(std::string &account);
    int find_account_sid(std::string &account);

    int bind_account_player(const std::string &account, GatePlayer *player);
    int unbind_account_player(const std::string &account);
    GatePlayer *find_account_player(const std::string &account);

    int sync_session(const int auth_sid, Block_Buffer *msg);
    int fetch_role_amount(const int sid, Block_Buffer *msg);
    int force_player_logout(Block_Buffer *msg);
#ifndef NO_USE_PROTO
    int validate_gate_session(const int sid, Message *msg_proto);
    int create_new_role(const int sid, Message *msg_proto);
#else
    int validate_gate_session(const int sid, Block_Buffer *msg);
    int create_new_role(const int sid, Block_Buffer *msg);
#endif
    int after_load_player(Transaction *transaction);
    int after_save_new_role(Transaction *transaction);

    void report_pool_info(const bool report = true);
    int update_relogin_tick(void);
    int relogin_logic_for_all_player(void);

    int gate_scene(void);
    void set_auth_sid(const int sid);
    int auth_sid(void);
    int get_client_ip_port(int sid, std::string &addr, int &port);

    int add_reconnect_index(int index);
    int start_reconnect_tick(int tick = 0);
    int reset_reconnect_tick();
    int check_client_accelerate(const int sid);

    int fetch_config_index_by_sid(const int sid);
    int process_connect_svc_close(Block_Buffer *buff);
    int process_connected_svc(Message *msg);
    int check_and_connect_svc(void);

    int select_random_name(const int sid, Message *msg);
    int after_select_random_name(Transaction *transaction);

    int process_travel_mongo_save(int sid, Message* msg);
    int process_travel_serial_save(int sid, Message* msg);
    int process_connect_server(Message *msg);
    int db_load_mode_done(Transaction* trans);

    int set_combine_server_check_time(Int64 now_tick);
    int request_load_combine_server(Int64 now_tick);
    int after_load_combine_server(DBShopMode* shop_mode);

    int force_reconnect_trvl(ServerDetail& server_detail);
    int check_need_reconnect_trvl(ServerDetail& server_detail);

    int init_travel_map_index();
    int check_update_trvl_url();
    int check_and_fetch_travel_area(void);
    int fetch_travel_area(int index, const ServerDetail &server_detail);
    int process_set_travel_area(int sid, Message* msg);
    int process_travel_map_keep_alive(int sid, Message* msg);

    std::string get_agent_from_account(const string &account);
    void correct_agent_market(const std::string &account_agent, std::string &agent, int &market_code);

protected:
    virtual int process_init_scene(const int scene_id, const int config_index, const int space_id = 0);
    virtual int init_game_timer_handler(void);
    virtual int start_game_timer_handler(void);

    int load_global_data(void);


protected:
    GateClientPacker client_packer_;
    GateInnerPacker inner_packer_;

    ConnectReceiver connect_receiver_;
    GateConnectPacker connect_packer_;

    GateUnit logic_unit_;

    BroadUnit *broad_unit_;
    int broad_unit_cnt_;

    ConnectUnit *connect_unit_;
    GlobalKeyMap global_key_map_;
    SessionManager *session_manager_;

    PlayerMap player_map_;      // key: role_id, value: GatePlayer
    PlayerSidMap player_sid_map_;   // key: role_id, value: client_sid;

    PlayerMap sid_player_map_;  // key: client_sid, value: GatePlayer
    SidAccountMap sid_account_map_; // key: client_sid, value: account_name;
    AccountSidMap account_sid_map_; // key: account_name, value: client_sid;
    AccountPlayerMap account_player_map_;   // key: account_name, value: GatePlayer; 用于加载完数据后判断之前的连接是否被其他连接冲掉
    AccountLoginTickMap account_login_tick_map_;    // key: account_name, value: login tick;

    GatePlayerPool player_pool_;
    TimerHandlerList timer_handler_list_;

    int auth_sid_;
    CombineServerInfo combine_server_;

    GateOneSecTimer onesec_timer_;
    GateTenSecTimer tensec_timer_;
    Time_Value relogin_all_tick_;

    IntSet reconnect_set_;		//server config index
    Time_Value reconnect_tick_;

    IntSet travel_map_index_set_;
    TravelSceneLineMap travel_scene_line_map_;

    GlobalKeyMap account_rename_tick_map_;
};

typedef Singleton<GateMonitor> GateMonitorSingle;
#define GATE_MONITOR    (GateMonitorSingle::instance())

#endif //_GATEMONITOR_H_
