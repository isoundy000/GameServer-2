/*
 * MapLogicPlayer.h
 *
 * Created on: 2013-04-23 10:45
 *     Author: lyz
 */

#ifndef _MAPLOGICPLAYER_H_
#define _MAPLOGICPLAYER_H_

#include "GameCache.h"
#include "MapLogicSerial.h"
#include "MapLogicStruct.h"
#include "LogicOnline.h"
#include "MLTinyer.h"
#include "MapMailPlayer.h"
#include "MapLogicIllustration.h"
#include "MapShoper.h"
#include "MapLogicTeamer.h"
#include "MLMounter.h"
#include "MLLeaguer.h"
#include "MapLogicTasker.h"
#include "MapEquipment.h"
#include "MLVipPlayer.h"
#include "MapLogicLabel.h"
#include "MLAchievement.h"
#include "MLOnlineRewards.h"
#include "MLOfflineRewards.h"
#include "MLCheckIn.h"
#include "MLExpRestore.h"
#include "MLTreasures.h"
#include "MLScriptClean.h"
#include "PlayerAssist.h"
#include "MLMediaGiftPlayer.h"
#include "MLOnceRewards.h"
#include "MLRecharger.h"
#include "MLDailyRecharge.h"
#include "MLRebateRecharge.h"
#include "MLInvestRecharge.h"
#include "MLScriptCompact.h"
#include "MLTotalRecharge.h"
#include "MLTrade.h"
#include "MLGoder.h"
#include "MLMagicWeapon.h"
#include "MLCollectChests.h"
#include "MLSwordPool.h"
#include "MLHiddenTreasure.h"
#include "MLFashion.h"
#include "LogicOfflineHookPlayer.h"
#include "MLTransfer.h"

class MapLogicPlayer :
		public MLTinyer,
		public MapShoper,
		public MapLogicTeamer,
		public MLMounter,
		public MLLeaguer,
		public MapMailPlayer,
		public MapLogicTasker,
		public MapEquipment,
		public MLVipPlayer,
		public MapLogicLabel,
		public MapLogicIllustration,
		public MLAchievement,
		public MLOnlineRewards,
		public MLOfflineRewards,
		public MLCollectChests,
		public MLCheckIn,
		public MLGoder,
		public MLExpRestore,
		public MLTreasures,
        public MLScriptClean,
        public PlayerAssist,
        public MLMediaGiftPlayer,
        public MLOnceRewards,
        public MLRecharger,
        public MLDailyRecharge,
        public MLRebateRecharge,
        public MLInvestRecharge,
        public MLScriptCompact,                
        public MLTotalRecharge,
        public MLTrade,
        public MLMagicWeapon,
        public MLSwordPool,
        public MLHiddenTreasure,
        public MLFashion,
        public LogicOfflineHookPlayer,
        public MLTransfer
{
public:

    enum {  // 需要定时保存到数据库的数据
        CACHE_PACKAGE			= 1,
        CACHE_MAIL				= 2,
        CACHE_ROLE				= 3,
        CACHE_INVEST			= 4,
        CACHE_VIP				= 7,
        CACHE_LABEL				= 8,
        CACHE_MOUNT				= 9,
        CACHE_ONLINE_REWARDS	= 10,
        CACHE_WELFARE			= 11,
        CACHE_EXP_RESTORE		= 13,
        CACHE_SCRIPT_CLEAN		= 14,
        CACHE_MEDIA_GIFT		= 15,
        CACHE_OPEN_ACTIVITY		= 16,
        CACHE_TASK				= 18,
        CACHE_RECHARGE_REWARDS	= 17,
        CACHE_LEVEL_REWARDS		= 19,
        CACHE_MAGIC_WEAPON      = 23,
		CACHE_ILLUSTRATION		= 28,
		CACHE_COLLECT_CHESTS	= 29,
		CACHE_OFFLINE_REWARDS 	= 30,
		CACHE_EQUIP_SMELT 		= 31,
		CACHE_SWORD_POOL 		= 32,
		CACHE_TREASURES_INFO 	= 33,
		CACHE_FASHION_INFO 		= 34,
		CACHE_TRANSFER_INFO		= 35,
        CACHE_END
    };
    enum {  // 玩家内部定时器: 在默认构造函数里设置定时间隔
        TIMEUP_SAVE      	 = 1,
        TIMEUP_LOAD_MAIL	 = 2,
        TIMEUP_CHECK_FASHION = 3,
        TIMEUP_CHECK_LABEL   = 4,
        TIMEUP_CHECK_ONE_SECOND = 5,
        TIMEUP_CHECK_FRONTER = 6,               //境界
        TIMEUP_CHECK_RECHARGE= 7,               //充值
        TIMEUP_CHECK_TASK    = 8,               //定时进行任务状态判断
        TIMEUP_CHECK_SCRIPT_COMPACT = 9,        //定时检查副本契约状态
        TUMEUP_CHECK_TRANSFER = 10,				//变身超时
        TIMEUP_END
    };

    struct CachedTimer : public GameTimer
    {
        CachedTimer(void);
        virtual ~CachedTimer(void);

        virtual int type(void);
        virtual int handle_timeout(const Time_Value &nowtime);

        MapLogicPlayer *player_;
    };

    using EntityCommunicate::respond_to_client;

public:
    MapLogicPlayer(void);
    virtual ~MapLogicPlayer(void);

    void reset(void);

    int add_pack_type(int pack_type, int pack_size);
    virtual GamePackage* find_package(int pack_type = GameEnum::INDEX_PACKAGE);

    MapMonitor *monitor(void);
    void set_role_id(const int64_t role_id);
    virtual int64_t entity_id(void);

    void set_gate_sid(const int gate_sid);
    virtual int gate_sid(void);
    virtual bool is_active(void);
    bool is_enter_scene(void);
    virtual bool is_death(void);
    virtual bool is_fight_state(void);
    virtual bool is_fight_active(void);
    virtual bool is_fight_passive(void);
    bool is_need_save_mongo(void);
    void set_is_loading_mongo(const bool flag);
    bool is_loading_mongo(void);

    void set_load_mongo_tick(const Time_Value &tick);
    Time_Value &load_mongo_tick(void);

    virtual int respond_to_client_error(const int recogn, const int error, const Message *msg_proto = 0);
    virtual int respond_to_client(Block_Buffer *buff);

    int request_load_data(const int gate_sid, const int64_t role_id);
    int sign_in(const int type = ENTER_SCENE_TRANSFER);
    int sign_out(const int type = EXIT_SCENE_TRANSFER, const bool is_save = true);
    void login_sign_in();
    void transfer_sign_in();
    void reset_every_day(void);
    virtual int request_save_player(const int recogn = TRANS_SAVE_MAP_LOGIC_PLAYER);
    int process_travel_save(const int recogn, MongoDataMap *data_map);

    int transfer_scene(Message* msg);
    int finish_transfer_scene(Message* msg);

    virtual int serialize_base(Proto31400103 *request);
    virtual int unserialize_base(Proto31400103 *request);
    virtual int serialize_online(Proto31400105 *request);
    virtual int unserialize_online(Proto31400105 *request);

    int serialize_package(ProtoPackageSet* msg_proto);
    int unserialize_package(Message* msg);

    virtual int time_up(const Time_Value &nowtime);
    virtual GameCache &cache_tick(void);

    virtual MapLogicRoleDetail &role_detail(void);
    virtual RoleExDetail &role_ex_detail(void);
    virtual MapLogicSerial &serial_record(void);
    virtual HookDetail &hook_detail(void);
    virtual PackageDetail& pack_detail(void);
    virtual LogicOnline &online(void);

    virtual int scene_id(void);
    virtual int agent_code(void);
    virtual int market_code(void);

    virtual Int64 role_id(void);
	virtual int role_career(void);

	virtual const char* name();
	virtual string &role_name(void);
    virtual int role_level(void);

    int kill_monster(Message *msg);
    int collect_item(Message *msg);
    int task_collect_monster(Message *msg);
    int team_monster_info_update(Message* msg);

    virtual int sync_death_state(Message *msg);
    virtual int sync_fight_state(Message *msg);
    virtual int sync_attend_activity(Message *msg);

    int sync_festival_icon(Message *msg);
    bool logic_validate_festival_activity(int act_index);

    int sync_big_act_state(Message *msg);
    bool logic_validate_big_act_time();

    int request_task_rename(Message *msg);
    int after_update_player_name(Transaction *transaction);

    int set_after_role_name(const string& name);
    int handle_after_role_rename();

    int set_after_role_sex();
    int handle_after_role_sex();

    int fetch_random_rename(Message *msg);
    int after_fetch_random_rename(Transaction *transaction);

    int count_of_zhuling();

public:
    int sync_info_from_map(Message* msg_proto);
    int level_upgrade(const int prev_level, const int level);
    int process_novice_level_up(Message *msg_proto);

    int notify_client_popup_info();
    int logic_test_command(Message* msg_proto);

    int test_midnight_refresh();
    // 玩家进入场景前,同步逻辑线程信息
    int sync_info_before_enter_scene(int finish_type);
    // 玩家获取周围完成，同步逻辑线程信息
    int map_obtain_area_info_done(void);
    // 通知经验找回系统完成事件
    int sync_exp_restore_event_done(int event_id, Time_Value time_v);

    int check_out_time_item();
	int open_package(void);
    int use_pack_goods(Message* msg);
    int after_check_use_pack_goods(Message* msg);
    int map_use_pack_goods_done(Message* msg);

    int request_add_blood_container(Message* msg);
    int request_use_blood_goods(Message* msg);
    int auto_use_blood_goods(Message* msg);
    int auto_buy_blood_goods(Message* msg);

    int validate_prop_usage(const Json::Value& prop_config,
    		PackageItem* pack_item);
    int dispatch_prop_status(const Json::Value& effect,
    		PackageItem* pack_item, int& use_num);
    int dispatch_beast_prop(const Json::Value& effect,
    		PackageItem* pack_item, int use_num);

    int	logic_request_add_money(Message* msg);
    int	enemy_transalye_sub_money(Message* msg);
    int logic_request_add_goods(Message* msg);
    int logic_request_remove_goods(Message* msg);
    int logic_request_add_batch_goods(Message* msg);

    int map_request_pickup_goods(Message* msg);
    int map_request_gather_goods(Message* msg);
    int fetch_other_player_info(Message* msg);
    int fetch_single_player_all_detail(Message* msg);
    int fetch_ranker_detail(Message* msg);
    int process_fetch_self_ranker_info(Message *msg);

    int check_and_set_all_brighten();
    int transfer_fee_deduct(Message* msg);

    int sync_fight_property_to_map(const int enter_type = 0);
    int notify_logic_sync_fight_property(int type = 1);
    int sync_hook_info_to_logic(void); // 同步到逻辑进程

    int request_force_exit_system(int scene_id);
    int request_enter_main_town(Message *msg);

    int request_function_need_gold(Message *msg);
    int mount_illusion_add_exp(int item_id,int item_amount,int exp_value);

    int add_act_fish_score(int value);

    int return_activity_cost_item(Message *msg);
    int lucky_wheel_activity_cost(Message *msg);
    int lucky_wheel_activity_reset(Message *msg);
    int may_activity_buy(Message *msg);
    int fetch_item_amount_info(Message *msg);
    int goddess_bless_reward(Message *msg);
    int check_daily_run_pa_event_cancle();
    int may_act_buy_many_item(Message *msg);
    int special_box_cost(Message *msg);

    int refresh_wedding_prop(int enter_type = 0);
	int update_player_wedding_info(Message *msg);
    int wedding_update_work(Message *msg);
    int wedding_buy_treasures(Message *msg);

    int process_back_act_reward_item(Message *msg);
    int process_check_money_create_trav_team(Message *msg);

public:
    virtual int pack_bill_sell(const int item_id, const int amount, Money &money);
    virtual int request_pack_item(Message *msg_proto);

    int request_sync_map_skill(int skill_id, int skill_level = 1);
    int process_skill_level_up(Message *msg);

    static int notify_player_welfare_status( EntityCommunicate *player);
    int fetch_display_welfare_elements(Message *msg, Message *respond, int test_lvl=0);

    virtual int callback_after_exchange_copper(const int recogn);
    virtual void set_repeat_req(const int recogn, const Message *req);

    int process_wedding_check_pack(Message *msg);
    int process_divorce_check_pack(Message *msg);
    int process_keepsake_upgrade_check_pack(Message *msg);
    int process_intimacy_award_check(Message *msg);
    int process_divorce_clear_label(void);
    int process_send_flower(Message *msg);

    int sync_permission_info(const Json::Value &effect);
    MongoDataMap *mongo_datamap();
//    int act_end_and_save_date();

public:
    int request_tbattle_main_pannel(Message *msg);
    int process_trvl_reward(Message *msg);


protected:
    virtual int sync_transfer_base(const int scene_id);
    virtual int sync_transfer_package(const int scene_id);
    virtual int sync_transfer_online(const int scene_id);
    virtual int sync_transfer_task(const int scene_id);
    virtual int finish_sync_transfer_scene(int scene_id, int prev_scene);

    int remove_wedding_label_by_divorce(void);

protected:
    bool is_active_;
    bool is_synced_logic_info_;
    bool is_synced_map_info_;
    bool is_obtain_area_info_;

    bool is_loading_mongo_;
    Time_Value load_mongo_tick_;

    int gate_sid_;
    int64_t role_id_;

    PackageDetail pack_detail_;
    MapLogicRoleDetail role_detail_;
    RoleExDetail role_ex_detail_;

    MapMonitor *monitor_;

    CachedTimer cached_timer_;
    GameCache cache_tick_;
    MapLogicSerial serial_;
    LogicOnline online_;
    HookDetail hook_detail_;

    int repeat_req_recogn_;
    std::auto_ptr<Message> repeat_req_;

    int count_of_zhuling_; //注灵开启条件的数目

};

#endif //_MAPLOGICPLAYER_H_
