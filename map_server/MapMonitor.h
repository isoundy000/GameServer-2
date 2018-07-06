/*
 * MapMonitor.h
 *
 * Created on: 2013-01-17 20:02
 *     Author: glendy
 */

#ifndef _MAPMONITOR_H_
#define _MAPMONITOR_H_

#include "PoolPackage.h"
#include "DoubleKeyMap.h"
#include "PubStruct.h"
#include "ServerDefine.h"

class MapClientService;
class MapInnerService;
class MapConnectService;
class MapClientPacker;
class MapInnerPacker;
class InnerRouteHead;
class EntityCommunicate;
class BaseUnit;
class MapUnit;
class MapLogicUnit;
class MapTimerHandler;
class Scene;
class UnitMessage;
class MapPlayerEx;
class FighterSkill;
class BasicStatus;
class HistoryStatus;
class StatusQueueNode;
class MapLogicPlayer;
class MailBox;
class MailInformation;
class TaskInfo;
class TaskConditon;
class TaskImplement;
class TaskRoutineImp;
class TaskTrialImp;
class DelaySkillInfo;
class ScriptPlayerRel;
class ScriptTeamDetail;
class SceneAIRecord;
class ScriptAI;
class ActiCodeDetail;
class MapBeast;
class BaseScript;
class ScriptFactory;
class SceneLineManager;
class PlayerAssistTip;
class SceneFactory;
class MapOneSecTimer;
class MapIntHourTimer;
class MapMidNightTimer;
class MLIntMinTimer;
class MLMidNightTimer;
class TrvlKeepAliveTimer;
class PassiveSkillInfo;
class SceneBlock;
class SkillCoolInfo;


class MapMonitor : public ServerMonitor<MapClientService, MapInnerService, MapConnectService>
{
public:
    typedef ServerMonitor<MapClientService, MapInnerService, MapConnectService> SUPPER;
    typedef ObjectPoolEx<UnitMessage> UnitMessagePool;
    typedef ObjectPoolEx<MapPlayerEx> MapPlayerPool;
    typedef ObjectPoolEx<FighterSkill> SkillPool;
    typedef ObjectPoolEx<BasicStatus> StatusPool;
    typedef ObjectPoolEx<HistoryStatus> HistoryStatusPool;
    typedef ObjectPoolEx<StatusQueueNode> StatusQueueNodePool;
    typedef ObjectPoolEx<MapLogicPlayer> LogicPlayerPool;
    typedef ObjectPoolEx<MailBox> MailBoxPool;
    typedef ObjectPoolEx<MailInformation> MailInfoPool;
    typedef ObjectPoolEx<TaskInfo> TaskInfoPool;
    typedef ObjectPoolEx<TaskConditon> TaskConditionPool;
    typedef ObjectPoolEx<TaskImplement> TaskImplementPool;
    typedef ObjectPoolEx<TaskRoutineImp> TaskRoutineImpPool;
    typedef ObjectPoolEx<TaskTrialImp> TaskTrialImpPool;
    typedef ObjectPoolEx<DelaySkillInfo> DelaySkillPool;
    typedef ObjectPoolEx<ScriptPlayerRel> ScriptPlayerRelPool;
    typedef ObjectPoolEx<ScriptTeamDetail> ScriptTeamDetailPool;
    typedef ObjectPoolEx<SceneAIRecord> SceneAIRecordPool;
    typedef ObjectPoolEx<ScriptAI> ScriptAIPool;
    typedef ObjectPoolEx<ActiCodeDetail> ActiCodeDetailPool;
    typedef PoolPackage<MapBeast, Int64> MapBeastPackage;
    typedef PoolPackage<SkillCoolInfo, Int64> SkillCoolPackage;
    typedef PoolPackage<PlayerAssistTip, Int64> MLPlayerAssistPackage;
    typedef ObjectPoolEx<PassiveSkillInfo> PassiveSkillQNPool;
    typedef ObjectPoolEx<SceneBlock> SceneBlockPool;

    typedef std::vector<MapTimerHandler> TimerHandlerList;
    typedef DoubleKeyMap<int, int, Scene *, NULL_MUTEX> SceneMap;
    typedef DoubleKeyMap<int, int64_t, ScriptPlayerRel *, NULL_MUTEX> ScriptPlayerRelMap;
    typedef HashMap<int, BaseScript *, NULL_MUTEX> ScriptMap;
    typedef HashMap<int, ScriptTeamDetail *, NULL_MUTEX> ScriptTeamRelMap;
    typedef HashMap<int, Int64, NULL_MUTEX> ScriptProgressIDMap;
    typedef std::vector<MapLogicPlayer *> LogicPlayerSet;

public:
    MapMonitor(void);
    ~MapMonitor(void);

    virtual int init(const int config_index);
    virtual int start(void);
    virtual int start_game(void);
    virtual int stop(void);
    virtual void fina(void);

    virtual BaseUnit *map_unit(void);
    virtual BaseUnit *logic_unit(void);

    Block_Buffer *pop_block(int cid = 0);
    int push_block(Block_Buffer *buff, int cid = 0);
    UnitMessagePool *unit_msg_pool(void);
    MapPlayerPool *player_pool(void);

    FighterSkill* pop_skill(int skill_id, int level = 1);
    void update_skill_level(FighterSkill* skill, int level = 0);
    SkillPool *skill_pool(void);
    StatusPool *status_pool(void);
    HistoryStatusPool *history_status_pool(void);
    StatusQueueNodePool *status_queue_node_pool(void);
    LogicPlayerPool *logic_player_pool(void);
    MailBoxPool *mail_box_pool(void);
    TaskInfoPool *task_info_pool(void);
    TaskConditionPool *task_condition_pool(void);
    TaskImplementPool *task_imp_pool(void);
    TaskRoutineImpPool *task_routine_imp_pool(void);
    TaskTrialImpPool *task_trial_imp_pool(void);
    DelaySkillPool *delay_skill_pool(void);
    ScriptPlayerRelPool *script_player_rel_pool(void);
    ScriptTeamDetailPool *script_team_detail_pool(void);
    SceneAIRecordPool *scene_ai_record_pool(void);
    ScriptAIPool *script_ai_pool(void);
    ActiCodeDetailPool *acti_code_detail_pool(void);
    PassiveSkillQNPool *passive_skill_qn_pool(void);
    SceneBlockPool *team_scene_block_pool(void);

    MapBeastPackage *map_beast_package(void);
    MLPlayerAssistPackage *ml_player_assist_package(void);

    void reset_everyday();
    void report_pool_info(const bool report = true);

    int chat_scene(void);
    virtual int chat_scene(const int map_scene);
    std::string &chat_ip(void);
    int chat_port(void);

    int line_id();
    int average_level();

    int find_client_service(const int sid, MapClientService *&svc);
    int add_player_label(Int64 role_id, int label_id);

    int gate_sid_list(IntVec &gate_sid_vc);
    int first_gate_sid(void);
    int dispatch_to_scene(const int gate_sid, const InnerRouteHead *route_head, Block_Buffer *msg_buff);
    int dispatch_to_scene(const int gate_sid, const InnerRouteHead *route_head, const ProtoHead *proto_head, const Message *msg_proto = 0);
    int dispatch_to_scene(const int gate_sid, const InnerRouteHead &route_head, const ProtoHead &proto_head, Block_Buffer *msg_buff);

    int dispatch_to_chat(const int64_t role_id, const int recogn, const int scene_id, const Message *msg_proto);
    int dispatch_to_chat(EntityCommunicate* entity, const int recogn);
    int dispatch_to_chat(EntityCommunicate* entity, const Message* msg_proto);
    int dispatch_to_chat(const Message *msg_proto);

    int dispatch_to_logic(EntityCommunicate* entity, const int recogn);
    int dispatch_to_logic(EntityCommunicate* entity, const Message* msg_proto);

    // 跨服中使用的no player接口
    int dispatch_to_logic(const int gate_sid, const int recogn);
    int dispatch_to_logic(const int gate_sid, const Message *msg_proto);
    int dispatch_to_gate(int gate_sid, int recogn, Message *msg_proto);

    // 跨服中通过所有网关发送，如果开了两个网关，会重复发两次
    int dispatch_to_logic_in_all_server(int recogn);
    int dispatch_to_logic_in_all_server(Message *msg_proto);
    int dispatch_to_gate_in_all_server(int recogn, Message* msg_proto);

    int dispatch_to_logic(const int recogn);
    int dispatch_to_logic(const Message *msg_proto, int gate_sid = -1, Int64 role_id = 0);

    int dispatch_to_scene(const int gate_sid, const int64_t role_id, const int scene_id,
    		const int recogn, const Message *msg_proto = NULL);
    int dispatch_to_scene(const int gate_sid, const int64_t role_id, const int scene_id, const Message *msg_proto);
    int dispatch_to_scene(EntityCommunicate *entity, const int scene_id, const int recogn);
    int dispatch_to_scene(EntityCommunicate *entity, const int scene_id, const Message *msg_proto);
    int dispatch_to_scene_by_gate(int gate_sid, Int64 role_id, const Message* msg_proto);

    // no player
    int dispatch_to_scene(int scene_id, const Message* msg_proto);
    int dispatch_to_scene_by_noplayer(int scene_id, int recogn);
    int dispatch_to_scene_by_noplayer(EntityCommunicate *entity, int scene_id,
    		const Message *msg_proto);
    int dispatch_to_scene_by_noplayer(int gate_sid, Int64 role_id, int scene_id,
    		const Message* msg_proto, int recogn = 0);

// message send by client {{{
    int dispatch_to_client_from_gate(const int gate_sid, const Int64 role_id, const int recogn, const int error = 0);
    int dispatch_to_client_from_gate(const int gate_sid, const Int64 role_id, const Message *msg_proto, const int error = 0);

    // don't use this interface send to client, please use below two interface
    // msg_buff = ProtoHead + msg_body; {{{
    int dispatch_to_client_from_gate(GameMover *mover, Block_Buffer *msg_buff);
    // }}}
    int dispatch_to_client_from_gate(GameMover *mover, const int recogn, const int error = 0);
    int dispatch_to_client_from_gate(GameMover *mover, const Message *msg_proto, const int error = 0);
    int dispatch_to_client_from_gate(MapLogicPlayer *player, Block_Buffer *msg_buff);
    int dispatch_to_client_from_gate(MapLogicPlayer *player, const int recogn, const int error = 0);
    int dispatch_to_client_from_gate(MapLogicPlayer *player, const Message *msg_proto, const int error = 0);

    // dispatch from map socket
    // don't use this interface send to client, please use below two interface;
    // msg = (len + ProtoClientHead + msg_body) * N; {{{
    int dispatch_to_client_direct(GameMover *mover, Block_Buffer *msg_buff);
    // }}}
    int dispatch_to_client_direct(GameMover *mover, const int recogn, const int error = 0);
    int dispatch_to_client_direct(GameMover *mover, const Message *msg_proto, const int error = 0);
// }}}

// message inner send to logic unit {{{
    int process_inner_logic_request(const int64_t role_id, const int recogn);
    int process_inner_logic_request(const int64_t role_id, Message &msg_proto);
    int process_inner_map_request(const int64_t role_id, const int recogn);
    int process_inner_map_request(const int64_t role_id, Message &msg_proto);
// }}}

    SessionManager *session_manager(void);
    PlayerManager *player_manager(void);
    SceneLineManager *scene_line_manager(void);

    int request_map_player_login(const int gate_sid, const int64_t role_id, Message *msg);
    int after_load_player(Transaction *transaction);

    int bind_player(const int64_t role_id, MapPlayerEx *player);
    int unbind_player(const int64_t role_id);

    int find_player(const int64_t role_id, MapPlayerEx *&player);
    int find_player_with_offline(const int64_t role_id, MapPlayerEx *&player);

    int player_online_flag(Int64 role_id);
    int notify_all_player_info(int recogn, Message* msg = NULL);
    int get_logic_player_set(LogicPlayerSet &logic_player_set);
    int update_map_player_sid(int sid, Int64 role_id, Message* msg);
    int update_logic_player_sid(int sid, Int64 role_id, Message* msg);

    int logout_map_all_player(void);
    int logout_logic_all_player(void);

    MapPlayerEx* find_map_player(Int64 role_id);
    MapLogicPlayer* find_logic_player(Int64 role_id);

    int64_t connect_map_broad(const int broad_sid, Message *msg_proto);
    int disconnect_map_broad(const int broad_sid);

    SceneFactory *scene_factory(void);
    Scene *pop_scene(const int scene_id);
    int push_scene(Scene *scene);
    int bind_scene(const int space_id, const int scene_id, Scene *scene);
    int unbind_scene(const int space_id, const int scene_id);
    int find_scene(const int space_id, const int scene_id, Scene *&scene);

    int init_scene(const int scene_id, const int space_id = 0);
    int check_and_run_scene_monster();

    bool is_normal_scene(const int scene_id);
    bool is_script_scene(const int scene_id);

    int generate_ai_id(void);
    int generate_drop_id(void);
    int generate_camp_id(void);
    int generate_beast_id(void);

    int generate_role_copy_id(void);
    int generate_beast_copy_id(void);
    int generate_effect_id(void);

    int request_enter_scene_begin(int sid, Int64 role_id, Message* msg);
    int respond_enter_scene_begin(int sid, Message* request, Message* enter_info);

    int check_add_item_drop_limit(Int64 role, int scene_id, const ItemObj& obj);

    /*
     * 系统提示通知
     * */
    int inner_notify_player_assist_event(MapPlayer *player, int event_id, int event_value);
    		
   	/*
     * logic thread
     * */
    int db_load_mode_begin(int trans_recogn, Int64 role_id = 0);
    int db_load_mode_begin(DBShopMode* shop_mode, Int64 role_id = 0);
    int db_load_mode_done(Transaction* trans);

    /*
     * map thread
     * */
    int db_map_load_mode_begin(DBShopMode* shop_mode, Int64 role_id = 0);
    int db_map_load_mode_done(Transaction* trans);

    int bind_script_player_rel(const int script_sort, const int64_t role_id, ScriptPlayerRel *rel);
    int unbind_script_player_rel(const int script_sort, const int64_t role_id);
    ScriptPlayerRel *find_script_player_rel(const int script_sort, const int64_t role_id);

    int bind_script_team_rel(const int team_id, ScriptTeamDetail *rel);
    int unbind_script_team_rel(const int team_id);
    ScriptTeamDetail *find_script_team_rel(const int team_id);

    int bind_script(const int script_id, BaseScript *script);
    int unbind_script(const int script_id);
    BaseScript *find_script(const int script_id);

    ScriptFactory *script_factory(void);

    int load_script_progress_id_map(void);
    Int64 script_progress_id(const int script_sort);
    void update_script_progress_id(const int script_sort, const Int64 progress);

    int fetch_lescort_rank(int sid,Int64 role_id,Message *msg);
    int back_force_close_activity(Message *msg);
    int is_has_travel_scene(void);

    int fetch_gate_sid(const string& server_flag);
    int process_fetch_travel_area(int sid, Message *msg);

    int keep_alive_msg_begin();
    int keep_alive_msg_done(int sid);
    int handle_remove_illegal_sid();
    int check_keep_alvie_timeout();

    int fetch_average_level_done(Message* msg);
    int festival_generate_boss(Message* msg);
    int set_festival_activity_info(Message* msg);
    int set_big_activity_info(Message* msg);
    int festival_icon_type();
    int is_in_big_act_time();
    int fetch_double_activity(int type);

    int map_test_command(Message *msg);

    int add_skill_cool(GameFighter* player, FighterSkill* skill);
    Time_Value fetch_skill_cool(Int64 role, int skill);

protected:
    virtual int process_init_scene(const int scene_id, const int config_index, const int space_id = 0);
    virtual int init_game_timer_handler(void);
    virtual int start_game_timer_handler(void);

protected:

    // key: server_flag, value: gate_id
#ifdef LOCAL_DEBUG
    StrIntMap all_sid_;
#else
    BStrIntMap all_sid_;
#endif
    int check_gate_state_;
    IntMap check_gate_sid_map_;

    MapClientPacker *client_packer_;
    MapInnerPacker *inner_packer_;

    MapUnit *map_unit_;
    MapLogicUnit *logic_unit_;
    TimerHandlerList *timer_handler_list_;

    MapPlayerPool *player_pool_;
    SkillPool *skill_pool_;
    StatusPool *status_pool_;
    HistoryStatusPool *history_status_pool_;
    StatusQueueNodePool *status_queue_node_pool_;
    LogicPlayerPool *logic_player_pool_;
    MailBoxPool *mail_box_pool_;
    TaskInfoPool *task_info_pool_;
    TaskConditionPool *task_condition_pool_;
    TaskImplementPool *task_imp_pool_;
    TaskRoutineImpPool *task_routine_imp_pool_;
    TaskTrialImpPool *task_trial_imp_pool_;
    DelaySkillPool *delay_skill_pool_;
    ScriptPlayerRelPool *script_player_rel_pool_;
    ScriptTeamDetailPool *script_team_pool_;
    SceneAIRecordPool *scene_ai_record_pool_;
    ScriptAIPool *script_ai_pool_;
    ActiCodeDetailPool *acti_code_detail_pool_;
    PassiveSkillQNPool *passive_skill_qn_pool_;
    SceneBlockPool *team_scene_block_pool_;

    ThreeObjMap item_drop_limit_;	//掉落物品限制，每天晚上清除

    MapBeastPackage* map_beast_package_;
    SkillCoolPackage* skill_cool_package_;
    MLPlayerAssistPackage* ml_player_assist_package_;

    MapOneSecTimer* map_one_sec_timer_;
    MapIntHourTimer* map_int_hour_timer_;
    MapMidNightTimer* map_mid_night_timer_;
    TrvlKeepAliveTimer* trvl_keep_alive_timer_;
    MLIntMinTimer* ml_int_min_timer_;
    MLMidNightTimer* ml_mid_night_timer_;

    SessionManager *session_manager_;
    PlayerManager *player_manager_;
    SceneLineManager *scene_line_manager_;
    
    SceneFactory *scene_factory_;
    SceneMap scene_map_;

    int chat_scene_;
    string chat_ip_;
    int chat_port_;

    int generate_ai_id_;
    int generate_drop_id_;
    int generate_camp_id_;
    int generate_beast_id_;

    int generate_role_copy_id_;
    int generate_beast_copy_id_;
    int generate_effect_id_;

    int line_id_;
    int average_level_;
    int travel_scene_;

    FestivalInfo festival_info_;
    BigActInfo big_act_info_;
    ScriptPlayerRelMap script_player_rel_map_;
    ScriptMap script_map_;
    ScriptTeamRelMap script_team_rel_map_;
    ScriptFactory *script_factory_;
    ScriptProgressIDMap script_progressid_map_;

public:
    Time_Value fight_total_use_;
    Time_Value move_total_use_;
    Time_Value ai_total_use_;
};

typedef Singleton<MapMonitor> MapMonitorSingle;

#define MAP_MONITOR     		(assert(MapMonitorSingle::instance()->is_inited() == true), MapMonitorSingle::instance())

#define PLAYER_MANAGER			MAP_MONITOR->player_manager()
#define BEAST_PACKAGE			MAP_MONITOR->beast_package()
#define MAP_BEAST_PACKAGE		MAP_MONITOR->map_beast_package()

#define ML_PLAYER_ASSIST_PACKAGE     MAP_MONITOR->ml_player_assist_package()

#endif //_MAPMONITOR_H_
