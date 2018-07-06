/*
 * MapPlayer.h
 *
 * Created on: 2013-01-23 10:21
 *     Author: glendy
 */

#ifndef _MAPPLAYER_H_
#define _MAPPLAYER_H_

#include "MapSerial.h"
#include "MapMaster.h"
#include "MapMounter.h"
#include "MapSwordPool.h"
#include "MapFashion.h"
#include "LogicOnline.h"
#include "MapSkiller.h"
#include "MapVipPlayer.h"
#include "MapEquiper.h"
#include "MapTeamer.h"
#include "MapTinyPlayer.h"
#include "MapKiller.h"
#include "MapAchievementer.h"
#include "MapWedding.h"
#include "MapEscort.h"
#include "MapTransfer.h"

class ProtoRoleShape;

class MapPlayer : public MapMaster,
				  public MapMounter,
                  public MapSkiller,
                  public MapVipPlayer,
                  public MapEquiper,
                  public MapTinyPlayer,
                  public MapTeamer,
                  public MapKiller,
                  public MapAchievementer,
                  public MapWedding,
                  public MapEscort,
                  public MapSwordPool,
                  public MapFashion,
                  public MapTransfer
{
public:
    enum
    {  	// 需要保存到数据库的数据
        CACHE_BASE      = 1,
        CACHE_FIGHT     = 2,
        CACHE_SKILL     = 3,
        CACHE_STATUS    = 4,
        CACHE_SCRIPT    = 5,
        CACHE_SM_BAT	= 6,
        CACHE_CAPTURE_JEWEL = 7,
        CACHE_ESCORT	= 8,
        // 需要同步到Logic进程的数据
        CACHE_SYNC_LOGIC_FORCE	= 9,
        CACHE_END,

        // 玩家内部定时器: 在默认构造函数里设置定时器间隔
        TIMEUP_SAVE     	= 1,
        TIMEUP_FIGHT    	= 2,
        TIMEUP_SYNC_KILL    = 3,
        TIMEUP_SYNC_LOGIC	= 5,
        TIMEUP_END
    };

    struct CachedTimer : public GameTimer
    {
        CachedTimer(void);
        virtual ~CachedTimer(void);

        virtual int type(void);
        virtual int handle_timeout(const Time_Value &nowtime);

        MapPlayer *player_;
    };

//    typedef std::vector<MapPlayer*> MapPlayerVec;
//    typedef std::list<int> PassiveSkillInSignList;

public:
    MapPlayer(void);
    virtual ~MapPlayer(void);

    virtual void reset(void);

    // please don't use this interface, use below interface
    // buff = len + ProtoHead + msg_body;
    virtual int respond_to_client(Block_Buffer *buff);
    virtual int respond_to_client(const int recogn, const Message *msg_proto = 0);
    virtual int respond_to_client_error(const int recogn, const int error, const Message *msg_proto = 0);

    // please don't use this interface, use below interface
    // buff = len + ProtoClientHead + msg_body;
    // 由当前玩家的地图端口发送消息给玩家;
    virtual int respond_from_broad_client(Block_Buffer *buff);
    virtual int respond_from_broad_client(const int recogn, const int error, const Message *msg_proto = 0);

    virtual int64_t entity_id(void);
    virtual void set_cur_location(const MoverCoord& coord);
    virtual int is_lrf_change_mode(void);

    Int64 role_id(void);
    Int64 src_role_id(void);
    Int64 real_role_id(void);

    int role_id_low(void);
    int role_id_high(void);

    int copy_offline(void);
    int start_offline(void);
    int is_online_player(void);
    int load_database(void);
    int transfer_flag(void);
    Time_Value &transfer_timeout_tick(void);

    void set_is_loading_mongo(const bool flag);
    bool is_loading_mongo(void);
    void set_load_mongo_tick(const Time_Value &nowtime);
    Time_Value &load_mongo_tick(void);
    bool is_need_save_mongo(void);

    const string &role_name(void);
    virtual const char* name();

    virtual int gate_sid(void);
    virtual int client_sid(void);
    virtual int team_id(void);
    virtual Int64 league_id(void);
    virtual int fight_career(void);
    virtual int fight_sex(void);

    int agent_code();
    int market_code();
    int client_fetch_max_blood();

    void set_role_id(const int64_t role_id);
    void set_client_sid(const int client_sid);
    void set_gate_sid(const int gate_sid);

    void set_is_login(const bool flag);
    bool is_login(void);

    void base_sign_in();
    virtual void reset_everyday();

    virtual int refresh_player_info(Block_Buffer *buff);
    virtual int resign(const int gate_sid, Message* msg);
    virtual int sign_in(const int type = ENTER_SCENE_TRANSFER);
    virtual int enter_scene(const int type = ENTER_SCENE_TRANSFER);
    virtual int exit_scene(const int type = EXIT_SCENE_TRANSFER);
    virtual int sign_out(const bool is_save_player = true);
    virtual int request_sync_to_logic(const int cache_type = 0);
    virtual int request_save_player(const int recogn = TRANS_SAVE_MAP_PLAYER);
    int process_travel_save(const int recogn, MongoDataMap *data_map);

    int handle_special_scene_when_exit();
    int handle_special_scene_when_enter();

    virtual int notify_player_login(void);
    virtual int insert_skill(int skill_id, int skill_level = 1, int notify = true);
    virtual int die_process(const int64_t fighter_id = 0);
    virtual int update_fight_property(int type = -1);

    int notify_fight_property(int type);
    int handle_player_scene_skill(int type);
    int handle_player_scene_buff(int type);
    int update_glamour(Message *msg);

    MapRoleDetail &role_detail(void);
    LogicOnline &online(void);
    MapSerial &serial_record(void);
    GameCache &cache_tick(void);

    virtual int cached_timeout(const Time_Value &nowtime);
    virtual int time_up(const Time_Value &nowtime);

    int serialize_move(Message *msg);
    int unserialize_move(Message *msg);
    int serialize_fight(Message *msg);
    int unserialize_fight(Message *msg);
    int serialize_online(Proto30400105 *request);
    int unserialize_online(Proto30400105 *request);

    //登入时通知
    int login_notify_enter_info();
    //传送时通知
    int transfer_notify_enter_info();

    virtual int notify_client_enter_info();
    virtual int obtain_area_info(int request_flag = false);

    // 默认切换场景时不设置前一个场景ＩＤ和坐标，防止在异空间切换场景时出错；
    int transfer_dispatcher(int scene_id, const MoverCoord &coord,
    		int scene_mode = SCENE_MODE_NORMAL, int space_id = 0);
    int transfer_dispatcher_b(const SceneInfo& scene, const MoverCoord& coord, const SubObj& obj);

    virtual int make_up_login_info(Block_Buffer *buff, const bool send_by_gate = false);
    virtual int make_up_appear_info_base(Block_Buffer *buff, const bool send_by_gate = false);
    virtual int make_up_move_info(Block_Buffer *buff, Int64 mover_id = 0);

    int make_up_role_appear_info(Block_Buffer *buff, const bool send_by_gate = false);
    int make_up_beast_appear_info(Block_Buffer *buff, const bool send_by_gate = false);
    int makeu_up_shape_info(ProtoRoleShape* shape_info, int type = 0);
    int fetch_show_mode_id();

    virtual int schedule_move_action(Message *msg);
    virtual int transfer_to_other_scene(Message *msg);

    //由于客服端位置和服务端位置不一样，拉回当前位置
    int adjust_to_cur_coord(int error = 0);
    //通知经验更新
    int notify_experience_update(int value);
    //保存当前场景信息
    int save_cur_scene_info(int type);
    //保存特殊场景
    int save_spacial_scene_info();
    //退出时恢复上次信息
    int enter_recover_scene_info(int check = true);
    //进入时设置当前场景信息
    int enter_set_scene_info(int enter_scene);
    //校正数据库信息
    int adjust_load_db_info();
    //登录时确认是否需要传送到跨服场景
    int login_transfer_to_trvl();

    bool is_need_send_message(void);
    bool validate_festival_activity(int act_index);
    bool validate_big_act_time();

    int request_chat_login(void);
    int request_chat_logout(void);

    virtual int caculate_total_force();
    virtual int force_total_i(void);

    //打怪经验加成
    virtual double fetch_addition_exp_percent();
    double fetch_level_exp_percent();
    double fetch_vip_exp_percent();
    double fetch_team_exp_percent();

    virtual int modify_element_experience(const int value, const SerialObj &serial_obj);
	virtual int modify_blood_by_fight(const double value, const int fight_tips = FIGHT_TIPS_BLANK,
			const int64_t attackor = 0, const int skill_id = 0);
	virtual int modify_blood_by_levelup(const double value, const int fight_tips = FIGHT_TIPS_BLANK);
    virtual int modify_magic_by_notify(const double value, const int fight_tips = FIGHT_TIPS_BLANK);

    // 请求使用物品的公共接口, Message类型: Proto30400202 {{{
    virtual int request_use_item(int recogn, int serial, int item_id, int amount,
    		int auto_buy = 0, int from = 0);
    virtual int request_use_item(const int recogn, const int serial, Message *proto);
    virtual int request_insert_item(const int recogn, const int serial, const ItemObjVec& item_set);
    virtual int request_insert_item(const int recogn, const int serial, const int item_id, const int amount);
    virtual int request_insert_item(const int recogn, const int serial, Message *proto);
    virtual int respond_pack_item(Message *proto);
    // }}}

    virtual int request_relive(Message *msg);
    virtual int process_relive_after_used_item(void);

    int request_pk_state(Message *msg);
    int map_use_pack_goods(Message* msg);
    int handle_use_pack_goods(Message* msg);
    int handle_enter_scene_error(int ret);

    int notify_monster_exp(int value);
    int notify_validate_team_info();
    int notify_transfer_scene_info(const SubObj& type = SubObj());
    int notify_logic_enter_scene(int type);

	int notify_update_self_id();
	virtual int notify_update_player_info(int update_type, Int64 value = 0);
	int notify_update_fashion_add_prop(Message* msg);
	int notify_update_player_space_id(void);

    virtual int pick_up_drop_goods_begin(Message* msg);
    int pick_up_drop_goods_done(Message* msg);
    virtual int process_pick_up_suceess(AIDropPack *drop_pack);

    virtual int gather_state_begin(Message* msg);
    virtual int gather_state_end();
    virtual int gather_goods_begin(Message* msg);
    virtual int gather_goods_done(Message* msg);

    int check_is_near_npc(Message *msg);
    int sync_update_player_name(Message *msg);
    int sync_update_player_sex();

    int send_festival_icon_to_logic();
    int send_big_act_state_to_logic();

    int check_is_near_finish_npc(Message *msg);
   	int fetch_role_detail();

    int sync_add_exp(Message *msg);
    int sync_add_exp_percent(Message* msg);
    int add_quintuple_exp_percent(Message* msg);
    int refresh_buff_status(Message* msg);
    int remove_quintuple_exp_percent(Message* msg);
    int sync_direct_add_blood(Message* msg);
    int sync_direct_add_magic(Message* msg);

    int fetch_enemy_translate(Message* msg);
    int fetch_enemy_position(Message* msg);
    int fetch_nearby_player(Message* msg);

    // 收集物品时同步消息到地图模块的逻辑线程
    int sync_collect_item(const int item_id, const int num = 1);
    // 通知经验找回系统完成事件
    int sync_ext_restore_info(int event_id, int value = 0, int times = 0);
    int sync_restore_info(int event_id, int value = 0, int times = 0);

    int update_wedding_info(Int64 partner_id, const string& partner_name,
    		Int64 wedding_id, int wedding_type);
    //通知支线任务状态
    int sync_branch_task_info(int event_id, int value = 0);

    int act_serial_info_update(int type, Int64 id, Int64 value, int sub_1 = 0, int sub_2 = 0);

    // 通知地图的逻辑线程完成某个活动(用于任务监听之类)
    int sync_to_ml_activity_finish(const int activity_type, const int sub_type = 0, const int value = 1);
    // 通知逻辑进程实时更新单人副本排行
    int sync_to_logic_single_script_rank(const int rank_type, const int rank_value);

    int fight_to_takeoff_mount_tick(void);
    int timeout_takeon_mount(void);
    virtual int refresh_fight_state(const Time_Value &nowtime);
    virtual void update_fight_state(const Int64 fighter_id, const int state,
    		const Time_Value &last_tick = Time_Value(5));

    virtual void update_active_fight_state(const Int64 target_id);
    virtual int process_skill_post_launch(int skill_id, int source);

    int transfer_to_born(int command_flag = false);
    int transfer_to_main_town();	//异常返回
    int transfer_to_prev_scene();
    int transfer_to_save_scene();

    int transfer_to_point(Message *msg, bool free_transefer = false);
    int transfer_fee_deduct_finish(Message *msg);

    int refresh_random_location(void);
    int refresh_aim_location(const MoverCoord& aim_location);

    int fetch_role_panel_info(Message* msg);
    int fetch_single_player_all_detail(Message* msg);
    int make_up_single_player_detail(Message *msg);
    int fetch_ranker_detail(Message* msg);
    int process_fetch_self_ranker_info(Message *msg);

    int update_copy_role_id();
    int check_and_use_goods(Message* msg);

    int request_enter_collect_chests();
    int request_enter_answer_activity();
    int request_add_reward(int id, const SerialObj& obj);
    int request_add_game_resource(int item_id, int value, const SerialObj& obj);
    int request_add_mult_item(RewardInfo &reward_info, const SerialObj& obj, int mult = 1);
    int request_add_single_item(int item_id, int amount, int item_bind, const SerialObj& obj);

    int enter_rand_scene(Message *msg);
    int notify_quit_trvl_team(int force = false);
    int notify_quit_normal_team();
    int notify_killed_info(Int64 fighter_id);
    int notify_get_couple_buy();

public:
    //trade
    int notify_trade_to_mapplayer(Message *msg);
    bool trade_state();
    Int64 trade_des_player_id();

    int prop_item_level_up();
    int logic_refresh_fight_property(Message* msg);

    //发送登入逻辑线程，并同步信息
    int start_login_map_logic(Int64 login_tick);
    //逻辑线程完成，可以发送消息给客户端
    int finish_login_map_logic(Message* msg);

    int send_request_enter_info(int request_scene, Proto30400051& enter_info);
    int request_enter_scene_done(Proto30400052* request, int recogn);

    virtual int send_to_logic_thread(int recogn);
    virtual int send_to_logic_thread(Message& msg);
    virtual int send_to_other_scene(int scene_id, Message& msg);

    int validate_player_transfer(int jump_flag = true);
    int validate_scene_level(int scene_id);
    int start_ml_sync_transfer(int cur_scene, int target_scene);

    int sync_transfer_base(void);
    int read_transfer_base(Message* msg);

    int handle_player_relive(const MoverCoord& relive_coord, double percent = 1.0);

    virtual void set_new_role_flag(const bool flag);
    virtual bool is_new_role(void);

    int request_glide(Message *msg);

	virtual int fetch_relive_data(const int relive_mode, int &blood_percent, int &item_id, int &item_amount);
	virtual int fetch_relive_data_common(const Json::Value &relive_json,
			const int relive_mode, int &blood_percent, int &item_id, int &item_amount);

    int process_accelerate_forbit(Message *msg);

    void record_other_serial(int main_serial, int sub_serial, Int64 value,
    		Int64 ext1 = 0, Int64 ext2 = 0);

    int request_dart_forward(Message *msg);
    //特殊ai不受突进
    int special_ai_not_dart_forward( Int64 id );

    virtual double fetch_reduce_hurt_rate();
    virtual double fetch_increase_hurt_value();
    virtual double fetch_increase_hurt_rate(GameFighter* fighter);
    virtual double fetch_sub_skill_use_rate(FighterSkill* skill);	//扣除使用概率

    void refresh_free_relive_times(void);
    int left_free_relive_times(void);

    int process_read_hook_info(Message *msg);
    int push_passive_skill_when_sign(const int skill_id);

    void set_role_escort_id(const Int64 id);
    Int64 role_escort_id(void);
    GameAI *role_escort_car(void);
    void set_league_escort_id(const Int64 id);
    Int64 league_escort_id(void);
    GameAI *league_escort_car(void);

    virtual int set_pk_state(const int state);
    virtual int notify_exit_scene_cancel_info(int type, int scene_id = 0);

    int process_flaunt_info(Message *msg);
    int make_up_flaunt_position(Message *msg);
    int notify_connect_travel_chat(void);

    void init_level_property(int cur_level = -1);
    void init_skill_property();
    void init_skill_buff();

    int init_all_player_skill(void);
    void set_base_member_info(BaseMember& base);
    void set_fight_property(const FightProperty& fight_prop);

    int finish_ml_level_upgrade();
    int sync_permission_info(Message *msg);

protected:
    virtual int sync_role_info_to_chat(void);

    virtual int sync_role_info_to_logic(const int prev_scene = 0);
    virtual int finish_sync_transfer_scene(void);

    int sync_transfer_move(void);
    int sync_transfer_fight(void);
    int sync_transfer_online(void);

    virtual int validate_movable(const MoverCoord &step);
    virtual int level_upgrade(void);

    virtual int validate_relive_point(int check_type);
    virtual int validate_relive_locate(const int item_id);
    virtual int process_relive(const int relive_mode, MoverCoord &relive_coord);

    int finish_map_logic_by_login(Message *msg);
    int finish_map_logic_by_transfer(Message *msg);

    int sync_info_to_map_logic(void);
    int sync_fighter_status_to_map_logic(Int64 attacker = 0);

    int insert_protect_buff(int last = 0, int relive_mode = 0);

    virtual int check_travel_timeout(void);

protected:
    int gate_sid_;
    int client_sid_;

    bool transfer_flag_;
    Time_Value transfer_timeout_tick_;
    bool is_loading_mongo_;
    Time_Value load_mongo_tick_;

    /*玩家ID改变*/
    int copy_offline_;
    /*玩家ID没改变*/
    int start_offline_;

    int enter_error_;
    int load_first_notify_;		//加载后第一次发信息
    int offline_enter_;

    bool is_login_;
    bool is_new_role_;

    //trade
    bool is_trade_;
    Int64 responser_id;

    Int64 role_id_;
    Int64 src_role_id_;
    MapRoleDetail role_detail_;

    GameCache cache_tick_;
    CachedTimer cached_timer_;

    MapSerial serial_;
    LogicOnline online_;
    HookDetail hook_detail_;

    Int64 role_escort_car_;
    Int64 league_escort_car_;
};

#endif //_MAPPLAYER_H_
