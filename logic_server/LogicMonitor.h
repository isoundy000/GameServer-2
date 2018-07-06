/*
 * LogicMonitor.h
 *
 * Created on: 2013-01-07 16:30
 *     Author: glendy
 */

#ifndef _LOGICMONITOR_H_
#define _LOGICMONITOR_H_

#include "PubStruct.h"
#include "ServerDefine.h"

class CenterUnit;
class BackUnit;
class LogicUnit;
class LogicClientService;
class LogicInnerService;
class LogicConnectService;
class LogicPhpService;
class LogicClientPacker;
class LogicInnerPacker;
class LogicConnectPacker;
class PhpReceiver;
class PhpSender;
class LogicPhpPacker;
class PhpServiceMonitor;
class BaseLogicPlayer;
class LogicPlayer;
class BoxRecord;
class RpmRecomandInfo;
class BackstageBrocastRecord;
class LogicTimerHandler;
class LogicMidNightTimer;
class LogicOneSecTimer;
class LogicTenSecTimer;
class LogicIntMinTimer;
class LogicOneMinTimer;
class LogicOneHourTimer;

class RechargeRankItem;
class BossDetail;
class WeddingDetail;
class StrengthInfo;
class JYBackActivityItem;
class ServerItemRecord;
class SpecialBoxItem;

class LogicMonitor : public ServerMonitor<LogicClientService, LogicInnerService, LogicConnectService>
{
public:
    typedef ServerMonitor<LogicClientService, LogicInnerService, LogicConnectService> SUPPER;
    typedef ServiceReceiver<LogicConnectService> ConnectReceiver;
    typedef ServiceReceiver<LogicPhpService> PhpReceiver;
    typedef ServiceSender<LogicPhpService> PhpSender;
    typedef ServiceMonitor<LogicPhpService> PhpServiceMonitor;
    typedef ServiceAcceptor<LogicPhpService> PhpServiceAcceptor;

    typedef ObjectPoolEx<UnitMessage> UnitMessagePool;
    typedef ObjectPoolEx<LogicPlayer> LogicPlayerPool;
	typedef ObjectPoolEx<BoxRecord> BoxRecordPool;
	typedef ObjectPoolEx<RpmRecomandInfo> RpmRecomandInfoPool;
    typedef PoolPackage<BackstageBrocastRecord, int> BackBrocastRecPool;
    typedef ObjectPoolEx<RechargeRankItem> RechargeRankItemPool;
    typedef ObjectPoolEx<WeddingDetail> WeddingDetailPool;
    typedef ObjectPoolEx<JYBackActivityItem> JYBackActivityItemPool;


    typedef std::vector<LogicTimerHandler> TimerHandlerList;

    typedef HashMap<int64_t, LogicPlayer *, NULL_MUTEX> PlayerMap;
    typedef HashMap<std::string, LogicPlayer *, NULL_MUTEX> NamePlayerMap;
    typedef HashMap<std::string, int64_t, NULL_MUTEX> GlobalKeyMap;
    typedef std::map<int, BossDetail> BossMap;
    typedef std::vector<BossDetail *> BossVec;
    typedef std::map<int, BossVec> BossVecMap;
    typedef std::vector<ServerItemRecord> ServerItemSet;

    struct LoigcMonitorTimer : public GameTimer
    {
    	LoigcMonitorTimer(void);
    	virtual ~LoigcMonitorTimer(void);

    	virtual int type(void);
    	virtual int handle_timeout(const Time_Value &nowtime);

    	LogicMonitor *monitor_;
    };

public:
    LogicMonitor(void);
    ~LogicMonitor(void);

    int combine_first();	//是否是合服第一执行，0表示否，1表示是
    int is_need_day_reset(Int64 last_tick);

    virtual int init(void);
    virtual int start(void);
    virtual int start_game(void);
    virtual int stop(void);
    virtual void fina(void);

    virtual int start_logic_monitor_timer(void);

    virtual BaseUnit *logic_unit(void);
    CenterUnit *center_unit(void);
    BackUnit *back_unit(void);
    PhpServiceAcceptor *php_acceptor(void);
    PhpReceiver *php_receiver(void);
    PhpSender *php_sender(void);
    LogicPhpPacker *php_packer(void);

    Block_Buffer *pop_block(int cid = 0);
    int push_block(Block_Buffer *buff, int cid = 0);

    UnitMessagePool *unit_msg_pool(void);
    LogicPlayerPool *player_pool(void);
    BoxRecordPool *box_record_pool(void);
    RpmRecomandInfoPool *rpm_recomand_info_pool(void);
    BackBrocastRecPool *backstage_brocast_record_pool(void);
    RechargeRankItemPool *recharge_rank_item_pool(void);
    WeddingDetailPool *wedding_detail_pool(void);
    JYBackActivityItemPool *jyback_activity_item_pool(void);

    void report_pool_info(const bool report = true);

    PlayerMap &player_map(void);

    int gate_sid_list(std::vector<int> &gate_sid_vc);
    int first_gate_sid(void);

    int dispatch_to_scene(const int gate_sid, const InnerRouteHead &route_head, const ProtoHead &proto_head, Block_Buffer *msg_buff);
    int dispatch_to_scene(const int gate_sid, const InnerRouteHead *route_head, const ProtoHead *proto_head, const Message *msg_proto);
    int dispatch_to_scene(const int gate_sid, InnerRouteHead *route_head, ProtoHead *proto_head, const Message *msg_proto);
    int dispatch_to_chat(BaseLogicPlayer *player, const int recogn);
    int dispatch_to_chat(BaseLogicPlayer *player, const Message *msg_proto);
    int dispatch_to_chat(const int chat_scene, const Message *msg_proto);

    int dispatch_to_scene(const int gate_sid, const int64_t role_id, const int scene_id, const int recogn);
    int dispatch_to_scene(const int gate_sid, const int64_t role_id, const int scene_id, const Message *msg_proto);
    int dispatch_to_special_line_scene(const int gate_sid, const int64_t role_id, const int line_id, const int scene_id, const int recogn);
    int dispatch_to_special_line_scene(const int gate_sid, const int64_t role_id, const int line_id, const int scene_id, const Message *msg_proto);

    int dispatch_to_scene(BaseLogicPlayer *player, const Message *msg_proto);
    int dispatch_to_scene_with_back(BaseLogicPlayer *player, const Message *msg_proto);
    int dispatch_to_scene(BaseLogicPlayer *player, const int scene_id, const Message *msg_proto);
    int dispatch_to_client(const int gate_sid, const int64_t role_id, const int recogn, const int error = 0);
    int dispatch_to_client(const int gate_sid, const int64_t role_id, const int error, const Message *msg_proto);
    int dispatch_to_client(BaseLogicPlayer *player, const int recogn, const std::string& msg_body);
    int dispatch_to_php(const int sid, const Message *proto_msg);
    int process_inner_center_thread(Message &request, int64_t role_id=0);
    int process_inner_logic_request(const int64_t role_id, Message &msg_proto);
    int process_inner_back_thread(const int recogn, int64_t role_id = 0);
    int process_inner_back_thread(Message &request, int64_t role_id = 0);

    // send to scene by no player
    int dispatch_to_scene(int scene_id, const Message* msg_proto);
    int dispatch_to_aim_sid(const ProtoHead& src_head, const Message* msg_proto);
    int dispatch_to_scene_by_noplayer(BaseLogicPlayer *player, const Message *msg_proto);

    /*
     * 发送消息到所有地图进程
     *
     * 逻辑进程启动时网关进程还未启动，所以该接口无法在游戏启动时使用
     * ps: 在地图进程启动时可能需要主动获取一次数据，否则地图进程重启后数据会不同步
     * */
    int dispatch_to_all_map(const Message* msg_proto);

    int request_login_player(const int gate_sid, const int64_t role_id, Message *request);
    int after_load_player(Transaction *transaction);
    int bind_player(const int64_t role_id, LogicPlayer *player);
    int unbind_player(const int64_t role_id);
    int find_player(const int64_t role_id, LogicPlayer *&player);

    int logout_all_player(void);
    int notify_all_player(Message* msg);

    int bind_player(const std::string &name, LogicPlayer *player);
    int unbind_player(const std::string &name);
    int find_player(const std::string &name, LogicPlayer *&player);
    int find_player_name(const std::string &name, StringVec &player_set);

    int find_global_value(const std::string &key, int64_t &value);
    int bind_global_value(const std::string &key, int64_t &value);
    int fetch_global_value(const std::string &key, int64_t &value);

    /*
     * logic thread
     * */
    int db_load_mode_begin(int trans_recogn, Int64 role_id = 0);
    int db_load_mode_begin(DBShopMode* shop_mode, Int64 role_id = 0);
    int db_load_mode_done(Transaction* trans);



    int back_stage_push_system_announce(const std::string &content, int type = BANNER_BROCAST_JUST_TOP);
    int notify_all_player(const LongMap& player_set, int recogn, Message* msg = NULL);
    int notify_all_player(int recogn, Message* msg = NULL);

	//
	int inner_notify_player_assist_event(Int64 role_id, int event_id, int event_value);
	//
	int parse_msg_to_all_player(Message *msg);
	int parse_msg_to_one_player(Message *msg);
    
    void reset_all_player_everyday(int test = false);
    void update_boss_lastest_attacker_name(const Int64 role_id, const std::string &name);

    int notify_player_from_map(Message* msg);
    int send_tmarena_rank_reward(Message* msg);

    int limit_team_scene(int scene_id);
    int update_limit_team_scene_set();
    LongMap& fetch_qq_49();
    int save_server_record(SpecialBoxItem& slot_info, ServerItemRecord &item_info);
    const ServerItemSet& get_server_record();

protected:
    virtual int process_init_scene(const int scene_id, const int config_index, const int space_id = 0);
    virtual int init_game_timer_handler(void);
    virtual int start_game_timer_handler(void);
    int load_global_data(void);
    int update_back_role_offline(void);

protected:
    LogicClientPacker *client_packer_;
    LogicInnerPacker *inner_packer_;

    ConnectReceiver *connect_receiver_;
    LogicConnectPacker *connect_packer_;

    PhpReceiver *php_receiver_;
    PhpSender *php_sender_;
    LogicPhpPacker *php_packer_;
    PhpServiceMonitor *php_svc_monitor_;

    LogicUnit *logic_unit_;
    CenterUnit *center_unit_;
    BackUnit *back_unit_;

    LogicPlayerPool *player_pool_;
    BoxRecordPool *box_record_pool_;
    RpmRecomandInfoPool	*rpm_recomand_info_pool_;
    BackBrocastRecPool *backstage_brocast_record_pool_;
    RechargeRankItemPool *recharge_rank_item_pool_;
    WeddingDetailPool *wedding_detail_pool_;
    JYBackActivityItemPool *jyback_activity_item_pool_;


    LoigcMonitorTimer logic_monitor_timer_;
    TimerHandlerList *timer_handler_list_;

    LogicMidNightTimer *midnight_timer_;
    LogicOneSecTimer *one_second_timer_;
    LogicTenSecTimer *ten_second_timer_;
    LogicIntMinTimer *int_minute_timer_;
    LogicOneMinTimer *one_minute_timer_;
    LogicOneHourTimer *one_hour_timer_;

    GlobalKeyMap global_key_map_;
    PlayerMap player_map_;
    NamePlayerMap name_player_map_;

    BossMap *boss_map_;
    BossVecMap boss_vc_map_;    // key: scene_id
    BossVec total_boss_vc_;
    Time_Value update_boss_tick_;
    IntSet limit_team_scene_set_;

    int combine_first_;
    LongMap qq_49_;

    ServerItemSet server_record_set_;
};

typedef Singleton<LogicMonitor> LogicMonitorSingle;
#define LOGIC_MONITOR   (assert(LogicMonitorSingle::instance()->is_inited() == true),LogicMonitorSingle::instance())

#define BBRecord_PACKAGE LOGIC_MONITOR->backstage_brocast_record_pool()

#endif //_LOGICMONITOR_H_
