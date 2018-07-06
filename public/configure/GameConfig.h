/*
 * GameConfig.h
 *
 * Created on: 2012-11-16 10:11
 *     Author: glendy
 */

#ifndef _GAMECONFIG_H_
#define _GAMECONFIG_H_

#include "ConfigStruct.h"

class GameConfig : public Json::Reader
{
public:
    enum
    {
        MAX_VERSION_CNT     = 2
    };

    typedef HashMap<int, Json::Value *, NULL_MUTEX> ConfigMap;
    typedef DoubleKeyMap<int, int, Json::Value *, NULL_MUTEX> DoubleKeyConfigMap;

    typedef std::vector<char> MptCoordList;
    typedef std::vector<ServerDetail> ServerList;

    typedef boost::unordered_map<int, MptCoordList> MptMap;
    typedef boost::unordered_map<int, MptDetail> MptDetailMap;

    typedef std::vector<GridIndexType> CoordIndexList;
    typedef boost::unordered_map<int, CoordIndexList> CoordIndexListMap;

    typedef boost::unordered_set<int64_t> CoordIndexSet;
    typedef boost::unordered_map<int, CoordIndexSet> CoordIndexSetMap;
    typedef boost::unordered_map<int, BIntSet> IntSetMap;

    typedef std::multimap<int64_t, Json::Value *> DoubleIntMultiMap;
    typedef std::pair<DoubleIntMultiMap::const_iterator, DoubleIntMultiMap::const_iterator> DoubleIntMultiMapRange;

    typedef DoubleKeyMap<int, int, int, NULL_MUTEX> SceneSceneJumpMap;

    struct BasicConfig
    {
    public:
    	BasicConfig(void);
        Json::Value &json(void);
        ConfigMap &map(void);
        DoubleKeyConfigMap &double_map(void);

        Json::Value &revert_json(void);
        ConfigMap &revert_map(void);
        DoubleKeyConfigMap &revert_double_map(void);

        const Json::Value& find(int key);
        const Json::Value& find_limit_max(int key);
        Json::Value& json_name(const char* item_name);

        int current_version(void);
        int prev_version(void);
        int validate(int key);

        void update_version(void);
        void revert_version(void);
        void convert_json_to_map(int key_flag = false);
        void load_combine_file(const string& path);

        string version_no();

    private:
        Json::Value  __json[MAX_VERSION_CNT];
        ConfigMap __map[MAX_VERSION_CNT];
        DoubleKeyConfigMap __double_map[MAX_VERSION_CNT];
        int __cur_version;
        std::string __version_no[MAX_VERSION_CNT];
    };

    struct ServerConfig
    {
    	Json::Value __global;
    	Json::Value __update_conf;	//更新配置内容
    	BasicConfig __version;		//版本信息

        ServerList __server_list;
        IntSetMap __scene_convert_to_map;
        BIntMap __convert_scene_to_map;

        bool __is_combine_server;	// 是否合服
        CreateDaysInfo __open_sever;
        CreateDaysInfo __combine_server;
        BIntSet __null_scene_set;
        ServerDetail __null_server_detail;
    };

    struct SceneConfig
    {
        BasicConfig __scene_list;
        BasicConfig __map_block;
        BasicConfig __scene;
        BasicConfig __npc;
        MptDetailMap __mpt[MAX_VERSION_CNT];
        MptMap __mpt_coord_map[MAX_VERSION_CNT];
        CoordIndexListMap __move_coord_map[MAX_VERSION_CNT];
        CoordIndexSetMap __border_coord_map[MAX_VERSION_CNT];

        MapBlockDetail __map_block_detail[MAX_VERSION_CNT];
        SceneSceneJumpMap __scene_scene_jump_map[MAX_VERSION_CNT];
        
        MptDetail __null_mpt_detail;

        MapBlockDetail &revert_map_block_detail(void);
        MptDetailMap &revert_mpt_detail_map(void);
        MptMap &revert_mpt_coord_map(void);
        CoordIndexListMap &revert_move_coord_map(void);
        CoordIndexSetMap &revert_border_coord_map(void);
        SceneSceneJumpMap &revert_scene_scene_jump_map(void);

        MapBlockDetail &map_block_detail(void);
        MptDetailMap &mpt_detail_map(void);
        MptMap &mpt_coord_map(void);
        CoordIndexListMap &move_coord_map(void);
        CoordIndexSetMap &border_coord_map(void);
        SceneSceneJumpMap &scene_scene_jump_map(void);
        void update_version(void);
        void revert_version(void);
    };

    struct FightConfig
    {
        BasicConfig __skill;
        BasicConfig __role_skill;
        BasicConfig __monster_skill;
        BasicConfig __skill_detail;
    };

    struct RoleConfig
    {
        BasicConfig __birth;
        BasicConfig __player_level;
        BasicConfig __vip;
        BasicConfig __blood_cont;
        BasicConfig __copy_player;
        BasicConfig __random_name;
        BasicConfig __replacement;
        BasicConfig __test_goods;
        BasicConfig __seven_day;
        BasicConfig __cumulative_login;
        BasicConfig __fight_power;

        IntMap sub_level_exp_;
        IntMap add_level_exp_;
    };

    struct LimitConfig
    {
        BasicConfig __exp_limit;
        BasicConfig __item_limit;
        BasicConfig __money_limit;
        BasicConfig __php_center_limit;
        BasicConfig __scene_line_limit;
        BasicConfig __ip_mac_limit;
        BasicConfig __correct_agent;

        BStrIntMap __const_map;
        BStrIntMap __fun_task_map;	//任务
        BStrIntMap __fun_level_map;	//等级

        BasicConfig __const_set;		//常量设置
        BasicConfig __scene_set;		//场景设置
        BasicConfig __function_set;		//功能开放设置
        BasicConfig __red_tips;			//红点提示
        BasicConfig __serial_set;		//流水设置
    };

    struct AIBTConfig
    {
    	BasicConfig bt_factory_;
    	BasicConfig monster_set_;

    	BasicConfig gather_;
    	BasicConfig prop_monster_;

    	std::map<std::string, BasicConfig*> tree_map_;
    };

    struct ArenaConfig
    {
    	BasicConfig arena_config_;
    	BasicConfig athletics_base_config_;
    	BasicConfig athletics_rank_config_;
    	BasicConfig escort_config_;
    };

    struct EquipConfig
    {
    	BasicConfig strengthen_[GameEnum::EQUIP_MAX_INDEX];		//强化
    	BasicConfig red_uprising[GameEnum::EQUIP_MAX_INDEX];	//红装升阶
    	BasicConfig prop_suit_;		//套装属性
    	BasicConfig refine_;		//精炼
    	BasicConfig compose_;		//装备合成
    	BasicConfig jewel_upgrade_;	//宝石升级
    	BasicConfig jewel_sublime_;	//宝石升华
    	BasicConfig smelt_level_;	//熔炼等级
    	BasicConfig smelt_exp_;		//熔炼装备经验
    	BasicConfig red_clothes_exchange_;
    	BasicConfig secret_exchange_;
    	BasicConfig legend_exchange_;
    };

    struct ItemConfig
    {
    	BasicConfig item_;
    	BasicConfig buff_;

    	BasicConfig shop_;
    	BasicConfig reward_;

        BasicConfig magic_weapon_;
        BasicConfig rama_list_;
        BasicConfig rama_open_;
    };

    struct TinyConfig
    {
    	BasicConfig font_;
    	BasicConfig tiny_;

    	BasicConfig team_;
    	BasicConfig agent_;

    	BasicConfig channel_ext_;
    	BasicConfig opinion_record_;
    };

    struct MarketConfig
    {
    	BasicConfig market_;
    	BasicConfig market_const_;
    };

    struct LeagueConfig
    {
    	BasicConfig league_;
    	BasicConfig league_log_;
    	BasicConfig league_war_;
    	BasicConfig league_fb_;
        BasicConfig boss_info_;
        BasicConfig league_boss_;
        BasicConfig league_flag_;
        BasicConfig league_skill_;
        BasicConfig league_pos_;
        BasicConfig lfb_wave_reward_;
        BasicConfig lfb_base_;
        BasicConfig lfb_cheer_attr_;
        BasicConfig lrf_weapon_;
        BasicConfig league_region_;
    };

    struct FashionConfig
    {
    	BasicConfig fashion_;
    	BasicConfig fashion_level_;
    	BasicConfig fashion_num_add_;
    	BasicConfig fashion_const_;
    	BasicConfig fashion_send_;

    	BStrIntMap __const_map;
    };

    struct SwordPoolConfig
    {
    	BasicConfig sword_pool_;
    	BasicConfig sword_pool_set_up_;
    	BasicConfig sword_pool_task_info_;
    };

    struct TransferConfig
    {
    	BasicConfig spirit_level_;
    	BasicConfig spirit_stage_;
    	BasicConfig transfer_base_;
    	BasicConfig transfer_total_;
    };

    struct MountConfig
    {
    	BasicConfig mount_[GameEnum::FUN_TOTAL_MOUNT_TYPE];
    	BasicConfig mount_level_[GameEnum::FUN_TOTAL_MOUNT_TYPE];

        BasicConfig mount_equip_;
        BasicConfig mount_set_;
        BasicConfig prop_unit_;
        BasicConfig change_goder_;
    };

    struct BeastConfig
    {
    };

    struct TaskConfig
    {
    	BasicConfig __task_setting;
    	BasicConfig __total_task;
    	BasicConfig __main;
    	BasicConfig __branch;
        BasicConfig __new_routine;
        BasicConfig __offer_routine;
        BasicConfig __league_routine;
        IntSetMap __npc_task_map[MAX_VERSION_CNT];
    };

    struct RelaxPlayConfig
    {
    	BasicConfig __collect_chests;
    	BasicConfig __chests_table;
    	BasicConfig __answer_activity;
    	BasicConfig __hotspring_activity;
    	BasicConfig __topic_bank;
    	BasicConfig __convoy;
    	BasicConfig __treasures_base;
    	BasicConfig __treasures_grid;
    };

    struct AchieveConfig
    {
    	BasicConfig __label;
    	BasicConfig __illus;
    	BasicConfig __illus_class;
    	BasicConfig __illus_group;
    	BasicConfig __achievement;
    	BasicConfig __new_achieve;
    	BasicConfig __achieve_list;
    	BasicConfig __achieve_level;
    	BasicConfig __rank_pannel;
    };

    struct BrocastConfig
    {
    	BasicConfig __brocast;
    	BasicConfig __tips_msg;
    };

    struct ActivityConfig
    {
    	BasicConfig __common_activity;	//活动大厅
    	BasicConfig __open_activity;	//开服活动
    	BasicConfig __return_activity;	//返利活动
    	BasicConfig __festival_activity;//节日活动
    	BasicConfig __combine_activity;	//合服活动
    	BasicConfig __combine_return_activity;//合服返利活动
    	BasicConfig __no_send_act;		//不传的活动
    	BasicConfig __may_activity;		//五一活动
    	BasicConfig __daily_run_item;	//天天跑酷物品配置
    	BasicConfig __daily_run_extra;	//跑酷直线跳跃物品
    };

    struct Welfare
    {
    	BasicConfig __check_model;
    	BasicConfig __day_check;
    	BasicConfig __total_check;
    	BasicConfig __online_check;
    	BasicConfig __restore;

    	BasicConfig __recharge;
    	BasicConfig __daily_recharge;
    	BasicConfig __rebate_recharge;
    	BasicConfig __invest_recharge;
    	BasicConfig __hidden_treasure;	// 藏宝阁
    	BasicConfig __operate_activity;	// 后台运营活动
    	BasicConfig __score_exchange; 	// 物品/积分兑换
    	BasicConfig __time_limit_reward;// 限时秒杀次数奖励
    	BasicConfig __word_reward;		// 真言奖励
    	BasicConfig __egg_reward;		// 幸运砸蛋次数奖励
    	BasicConfig __open_gift;		// 开服豪礼
    	BasicConfig __daily_recharge_rank;// 每日冲榜
    	BasicConfig __daily_activity;	// 后台每日活动
    	BasicConfig __total_double;		// 全民双倍
    };

    struct ScriptConfig
    {
        BasicConfig __script;
        BasicConfig __strong_robot;
        BasicConfig __clean_out;
        BasicConfig __monster_tower;
        BasicConfig __sword_skill;
    };


    struct WeddingConfig
    {
        BasicConfig __wedding;
        BasicConfig __wedding_item;
        BasicConfig __wedding_flower;
        BasicConfig __wedding_base;
        BasicConfig __wedding_property;
        BasicConfig __wedding_treasures;
        BasicConfig __wedding_label;
    };

    struct TravelConfig
    {
        bool __is_updated_travel;
        StrIntMap __flag_index_map;	//key: server flag, value: configure index
        BasicConfig __servers_list;
        BasicConfig __tarena_stage;
        BasicConfig __tarena_fight;
        BasicConfig __tmarena_rank;
        BasicConfig __tbattle_buff;
        BasicConfig __tbattle_reward;
        BasicConfig __peak_base;
        BasicConfig __peak_bet;
        std::map<int, int> __tbattle_amount_map[MAX_VERSION_CNT];
    };

    struct WorldBossConfig
    {
    	BasicConfig __world_boss;
    	BasicConfig __trvl_wboss;
    };

    struct CornucopiaConfig
    {
    	BasicConfig __cornucopia_info;
    	BasicConfig __cornucopia_reward;
    };

    struct FashionActConfig
    {
    	BasicConfig __faasion_act_info;
    };

    struct LabourActConfig
    {
    	BasicConfig __labour_act_config;
    };

    struct OfflineVipConfig
    {
    	BasicConfig __offline_vip_config;
    };

    struct MoldingSpiritConfig
    {
    	BasicConfig __attack;
    	BasicConfig __defense;
    	BasicConfig __blood;
    	BasicConfig __all_nature;
    	BasicConfig __equip_nature;
    };

    struct FishTypeConfig
    {
    	BasicConfig __fish_score_reward;
    };

    struct GoddessBlessConfig
    {
    	BasicConfig __bless_reward_item;
    	BasicConfig __bless_reward_blessing;
    	BasicConfig __bless_exchange_item;
    	BasicConfig __blessing_reward;
    };

    struct SpecialBoxConfig
    {
    	BasicConfig __special_box_item;
    	BasicConfig __special_box_change_item;
    };

public:
    GameConfig(void);
    ~GameConfig(void);

    int init(const char *server_name);
    const Json::Value &global();
    void load_server_ip();
    void load_version_config();

    string server_ip();
    string machine_address();
    string machine_address(const string& machine, string *domain = 0, int travel_type = 1);
    string full_role_name(int server_id, const string& name);

    ServerInfo* server_info();
    const ServerDetail& cur_map_server();
    const Json::Value& server(int index);
    const Json::Value& cur_server();
    const Json::Value& cur_servers_list();

    string main_version();
    string date_version();
    bool is_same_main_version(const string& main_version);

    int travel_server_port(const string& machine);
    int travel_server_port_channel(const string& machine);
    int convert_server_type(const string &service_name);
    ServerList &server_list(void);
    const ServerDetail &server_list(uint index);

    bool is_convert_scene(const int scene);
    int convert_scene_to(const int convert_scene);
    bool is_gate_scene(int scene_id);
    const BIntSet &convert_scene_set(const int scene);

    Int64 open_day_tick(int days = 0);
    Int64 open_server_date();   		//开服时间戳
    int open_server_days();				//开服天数，从1开始
    int open_server_hours();			//开服小时数
    int left_open_activity_time(int adjust_time = 0);		//剩余时间
    int during_open_activity(int day);	//是否是第几天的开服活动

    int main_activity_type();
    int client_open_days();
    int fetch_server_index_list(const int scene_id, IntVec &index_list);
    int test_client_set_open_day(int test_day);
    int test_set_server_open_day(int test_day);

    bool do_combine_server(void);		// 当前服是否合服
    Int64 combine_day_tick(int days = 0);	// 合服后几天的时间戳
    Int64 combine_server_date();		// 合服时间
    int combine_server_days();			// 合服天数，从1开始
    int combine_server_hours();			// 合服小时数
    int left_combine_activity_time(int adjust_time = 0);	// 剩余合服活动时间
    bool is_during_combine_activity(int day);	//是否是第几天的合服活动
    void test_set_server_combine_day(int test_day);

    const Json::Value &map_block(void);
    int max_move_step(void);
    const double &monster_step_tick(void);
    int script_stop_times(void);
    int script_monster_max(void);
    MapBlockDetail &map_block_detail(void);
    const Json::Value &scene(int scene_id);
    int scene_set_type(int scene_id);
    const MptDetail &mpt(const int mpt_id);
    const CoordIndexList &move_coord_list(const int mpt_id);
    int coord_mpt_value(const int mpt_id, const int coord_x, const int coord_y);
    int fetch_coord_by_mpt_value(CoordVec& coord_set, int mpt_id, int mpt_value);
    bool is_no_camp_scene(const int scene_id);
    bool is_border_coord_for_rand(const int scene_id, const int32_t pos_x, const int32_t pos_y);
    bool is_border_coord(const int scene_id, const int32_t pos_x, const int32_t pos_y);

    const Json::Value &npc(const int scene_id, const int npc_id);
    const Json::Value &script(const int script_sort);
    const GameConfig::ConfigMap &script_map(void);
    const Json::Value &script_clean_out(int script_sort);
    const Json::Value &script_clean_order(void);
    int script_clean_fast_gold(void);
    const Json::Value &script_monster_tower(void);
    const Json::Value &script_sword_skill(int id);

    const Json::Value &skill(const int skill_id);
    const GameConfig::ConfigMap &role_skill_map(void);
    const Json::Value &skill_detail(int skill_id, int skill_level = 1);

    const Json::Value &role(int sex);
    const Json::Value &seven_day(const int day);
    const Json::Value &cumulative_login(const int day);
    const Json::Value &role_level(int career, int level);
    const Json::Value &blood_cont(const char* item_name = NULL);
    const Json::Value &copy_player(const char* item_name = NULL);
    const Json::Value &replacement_cont(const char* item_name = NULL);

    void test_goods(IntMap& goods_map);
    void role_fightConfig(const int fightnum, float &power_rate,const int level);

    int top_level(void);
    double pk_tick(void);
    int base_role_speed(void);
    double sub_level_exp(int diff);
    double add_level_exp(int diff);

    const Json::Value &vip(int vip_type_id);
    const Json::Value &vip(void);
    const Json::Value &first_name(void);
    const Json::Value &man_second(void);
    const Json::Value &man_third(void);
    const Json::Value &woman_second(void);
    const Json::Value &woman_third(void);

    const Json::Value &exp_limit(const int serial_type);
    const Json::Value &item_limit(const int serial_type);
    const int64_t item_num_limit(const int serial_type, const int item_id);
    const Json::Value &money_limit(const int serial_type);
    const Json::Value &php_center_limit();
    const Json::Value &scene_line(const int scene_id);
    const Json::Value &agent_of_ip_mac();
    const Json::Value &correct_agent(const string &agent);
    const Json::Value& function_set(const string& key);
    const Json::Value& red_tips(int id);

    int serial_no_tips(int id);
    int serial_exp_tips(int id);
    int validate_red_tips(int id);
    int const_set(const string& key);
    int arrive_fun_open_level(const string& key, int level);
    int arrive_fun_open_task(const string& key, int task);

    BStrIntMap& task_fun_open_map();
    const Json::Value& const_set_conf(const string& key);
    const Json::Value& scene_set(int scene_id);

    int load_daemon_config(Json::Value &json);
    int load_update_config(Json::Value &json);
    int update_config(const std::string &folder);

    bool check_update_sub_value(int value);

    const Json::Value &bt_factory();
    const Json::Value &monster(int monster_sort);
    const Json::Value &prop_monster(int monster_sort);
    const Json::Value &gather(int monster_sort);
    const Json::Value &behavior(const std::string& tree_name);

    const Json::Value &item(int item_id);
    const Json::Value &prop(int item_id);
    const Json::Value &equip_strengthen(int index, int lvl);
    const Json::Value &red_uprising(int index, int color_index);
    const Json::Value &prop_suit(int suit);
    const Json::Value &equip_refine(int color);
    const Json::Value &equip_compse(int id);
    const Json::Value &jewel_upgrade(int id);
    const Json::Value &jewel_sublime(int lvl);
    GameConfig::ConfigMap &jewel_sublime_map(void);
    const Json::Value &equip_smelt_level(int id);
    const Json::Value &red_clothes_exchange(int exchange_id);
    const Json::Value &secret_exchange(int exchange_id);
    const Json::Value &legend_exchange(int exchange_id);
    const Json::Value &item_smelt_exp(int id);
    const Json::Value &shop();

    const Json::Value &reward(int id);
    const Json::Value &magic_weapon(const int id);
    const Json::Value &magic_weapon();

    const Json::Value &rama_list(const int id);
    const Json::Value &rama_list();
    const Json::Value &rama_open(const int id);
    const Json::Value &rama_open();

    const Json::Value &normal_team();
    const Json::Value &normal_team(int id);

    int font_serial(int index);
    FontPair font(int index);
    const Json::Value &tiny(const char* item_name = NULL);

    const Json::Value &opinion_record();
    const Json::Value &opinion_record(const int id);

    string fetch_shop_verison();
    const Json::Value &killing(int color_id);

    int server_id();
    string server_flag();
    int agent_code(const std::string &agent_str);
    const Json::Value &channel_ext(const std::string &agent_str);

    const Json::Value &league_log(int log_type);
    const Json::Value &league(const char* item_name = NULL);
    const Json::Value &league_war(const char* item_name = NULL);
    const Json::Value &boss_info(const int boss_index);
    const Json::Value &league_boss(const char* item_name = NULL);
    int boss_attr(const int boss_index, const string name);
    const Json::Value &league_flag(const int flag_lvl);
    const Json::Value &league_skill();
    const Json::Value &league_skill_info(int id);
    string league_pos_name(int pos);
    const Json::Value &lfb_wave_reward(int wave);
    GameConfig::ConfigMap &lfb_wave_reward_map();
    const Json::Value &lfb_cheer_attr(int num);
    const Json::Value &lfb_base();
    int lfb_base_value(const string& key);

    const Json::Value &lrf_weapon(int hickty_id, int level);
    const Json::Value &league_region(int id);

    const Json::Value &fashion(int fashion_id);
    const Json::Value &fashion_level(int level);
    const Json::Value &fashion_num_add(int amount);
    GameConfig::ConfigMap &fashion_num_add();
    int fashion_const(const string& key);

    const Json::Value &sword_pool(const char* item_name);
    const Json::Value &sword_pool_set_up(int level);
    const Json::Value &sword_pool_task(int task_id);
    const Json::Value &sword_pool_total_task();

    const Json::Value &spirit_level(int level);
    const Json::Value &spirit_stage(int stage);
    GameConfig::ConfigMap &spirit_stage_map();
    const Json::Value &transfer_base();
    const Json::Value &transfer_total(int id);

    const Json::Value &mount(int type);
    const Json::Value &mount(int type, int mount_id);
    const Json::Value &mount_level(int type, int role_level);
    const Json::Value &mount_set(int type);
    const Json::Value &prop_unit(int id);
    const Json::Value &prop_unit(const char* item_name);
    const Json::Value &change_goder(int grade);

    const Json::Value &arena(const char* item_name = NULL);
    const Json::Value &athletics_base(void);
    const Json::Value &athletics_rank(void);
    const Json::Value &athletics_rank_by_id(const int id);
    const Json::Value &athletics_rank(const int rank_num);
    const Json::Value &collect_chests(const char* item_name = NULL);
    const Json::Value &answer_activity(const char* item_name = NULL);

    void init_server_info();
    void init_combine_server_flag();
    void load_servers_list();

    const Json::Value &market(int id);
    const Json::Value &market_const(const char* name);

    const Json::Value &task(void);
    const Json::Value &task_setting(void);
    const Json::Value &find_task(int task_id);
    const GameConfig::ConfigMap &all_task_map(void);
    const GameConfig::ConfigMap &main_task_map(void);
    const GameConfig::ConfigMap &branch_task_map(void);
    const BIntSet &npc_task_list(const int npc_id);
    const GameConfig::ConfigMap &new_routine_task_map(void);
    const GameConfig::ConfigMap &offer_routine_task_map(void);
    const GameConfig::ConfigMap &league_routine_task_map(void);

    const Json::Value &label(int label_id);
    const Json::Value &label_json(void);

    const Json::Value &illus(int illus_id);
    const Json::Value &illus_json(void);
    const Json::Value &illus_group(int illus_group_id);
    const Json::Value &illus_group_json(void);
    const Json::Value &illus_class(int illus_class_id);
    const Json::Value &illus_class_json(void);

    const Json::Value &achievement(int achieve_type);
    const Json::Value &achievement_json(void);

    const Json::Value &achieve_json(void);
    const Json::Value &achieve_list_json(void);
    const Json::Value &achieve_level(int level);
    ConfigMap& achieve_map(void);
    ConfigMap& achieve_list_map(void);

    const Json::Value &rank_pannel(int rank_type);
    const Json::Value &rank_json(void);

    const Json::Value &common_activity_json(void);
    const Json::Value &common_activity(int activity_id);

    const Json::Value &open_activity_json(void);
    ConfigMap& open_activity_map(void);
    ConfigMap& return_activity_map(void);
    ConfigMap& festival_activity_map(void);
    ConfigMap& combine_activity_map(void);
    ConfigMap& combine_return_activity_map(void);
    ConfigMap& may_activity_map(void);

    const Json::Value &daily_run_item(int id);
    ConfigMap& daily_run_item_map(void);
    const Json::Value &daily_run_extra(int id);
    const Json::Value &no_send_act(void);

    const Json::Value &brocast(int id);
    const Json::Value &tips_msg_format(int msg_id);
    const Json::Value &buff(int id);
    bool validate_buff(int id);

    const Json::Value &welfare_elements_json(void);
    const Json::Value &pick_online_rewards(int config_id);

    const Json::Value &treasures_base_json();
    const Json::Value &treasures_base_json(int id);
    const Json::Value &treasures_grid_json();
    const Json::Value &treasures_grid_json(int id);

    const Json::Value &convoy_json();
    const Json::Value &convoy_json(int id);
    const Json::Value &answer_activity_json();
    const Json::Value &answer_activity_json(int id);
    const Json::Value &hotspring_activity_json();
    const Json::Value &hotspring_activity_json(int id);
    const Json::Value &chests_table_json();
    const Json::Value &chests_table_json(int id);
    const Json::Value &collect_chests_json(int id);
    const Json::Value &collect_chests_json();
    const Json::Value &topic_bank_json(int id);
    const Json::Value &topic_bank_json();

    const Json::Value &daycheck_json(int id);
    const Json::Value &checkmodel_json(const int id);
    const Json::Value &onlinecheck_json(int id);
    const Json::Value &onlinecheck_json();
    const Json::Value &totalcheck_json(int id);
    const Json::Value &restore_json(void);
    const Json::Value &restore_json(int activity_id);
    const Json::Value &restore_json_by_act_id(int act_id);
    const Json::Value &exp_restore_json(void);
    const Json::Value &exp_restore_json(int activity_id);

    int fetch_recharge_id(int gold, int type);
    void recharge_id_map(IntMap& imap, int type);

    const Json::Value& recharge_json(int id);
    const Json::Value &daily_recharge_json();
    const Json::Value &daily_recharge_json(int id);
    const Json::Value &daily_recharge_by_weeknum(int weeknum);
    const Json::Value &rebate_recharge_json();
    const Json::Value &rebate_recharge_json(int id);
    const Json::Value &invest_recharge_json();
    const Json::Value &invest_recharge_json(int id);
    const Json::Value &hidden_treasure_json();
    const Json::Value &hidden_treasure_json(int day);
    const Json::Value &operate_activity(int activity_id);
    GameConfig::ConfigMap &operate_activity_map();
    const Json::Value &score_exchange(int id);
    GameConfig::ConfigMap &score_exchange_map();
    const Json::Value &time_limit_reward(int id);
    const Json::Value &word_reward(int id);
    const Json::Value &egg_reward(int id);
    GameConfig::ConfigMap &egg_reward_map();
    const Json::Value &open_gift(int location);
    GameConfig::ConfigMap &open_gift_map();
    const Json::Value &daily_recharge_rank(int type);

    GameConfig::ConfigMap &daily_activity_map();
    const Json::Value &total_double(int id);

    const Json::Value &total_recharge(void);
    const Json::Value &wedding(void);
    const Json::Value &wedding_item(const int item_id);
    const Json::Value &wedding_flower(const int item_id);
    const Json::Value &wedding_base();
    const Json::Value &wedding_property(int type = 1, int level = 0);
    const Json::Value &wedding_treasures(int id);
    const Json::Value &wedding_label(int id);

    const Json::Value &tarena_stage(int index);
    const Json::Value &tarena_fight(int self, int rivial);
    int tarena_stage_size();
    string tarena_state_name(int index);

    GameConfig::ConfigMap &tmarena_rank_map();
    bool is_updated_travel_list(void);
    void reset_updated_travel_list(void);

    const Json::Value &tbattle_buff(const int amount);
    const Json::Value &tbattle_reward(void);
    int tbattle_player_amount_to_buff_idx(const int amount);

    const Json::Value &travel_peak_base(void);
    const Json::Value &travel_peak_bet(void);
    const Json::Value &travel_peak_bet(const int promot_turns);

    const Json::Value &world_boss(const char* item_name = NULL);
    const Json::Value &trvl_wboss(const char* item_name = NULL);
    const Json::Value& cornucopia_info(int task_id);
    GameConfig::ConfigMap& cornucopia_info();
    const Json::Value& cornucopia_reward(int index);
    GameConfig::ConfigMap& cornucopia_reward();

    const Json::Value& fashion_act_info(int index);
    GameConfig::ConfigMap& fashion_act_info();

    const Json::Value& labour_act_info(int index);
    GameConfig::ConfigMap& labour_act_info();

    const Json::Value& offline_vip_info(int index);
    GameConfig::ConfigMap& offline_vip_info();

    const Json::Value& molding_spirit_info(int type, int level, int red_grade = -1);
    const Json::Value& molding_spirit_all_nature(int level, int red_grade);
    const Json::Value& molding_spirit_equip_nature(int type);
    int molding_spirit_return_sys_nature_id(int type);
    string molding_spirit_return_sys_nature_name(int type);

    const Json::Value& fish_score_reward_info(int index);
    GameConfig::ConfigMap& fish_score_reward_info();
    const Json::Value* fish_score_reward_by_day(int index, int day);
    int fish_type_max_count(int day);

    GameConfig::ConfigMap& goddess_bless_reward_item(void);
    GameConfig::ConfigMap& goddess_bless_reward_blessing(void);
    GameConfig::ConfigMap& goddess_bless_exchange_item(void);
    GameConfig::ConfigMap& goddess_bless_blessing_reward(void);

    GameConfig::ConfigMap& special_box_item();
    GameConfig::ConfigMap& special_box_change_item();
protected:
    int load_json_config(const char *doc, Json::Value &conf);

    void update_open_server_date(void);
    void update_combine_server_date(void);

    int load_server_config(void);
    int load_map_config(void);
    int load_scene_config(const int scene_id, const char *path = "map");
    int calc_scene_jump(const int start_scene, const int mid_scene, const int target_scene);
    int load_mpt_config(const char *doc, const int mpt_id, SceneConfig &scene_conf);
    int load_map_block_config(void);
    int convert_to_border_coord(const int32_t x_1, const int32_t y_1, const int32_t x_2, const int32_t y_2, CoordIndexSet &border_set); 

    int load_script_config(void);
    int load_script_map(void);
    int load_fight_config(void);
    int load_item_config(void);
    int load_equip_config(void);

    int load_tiny_config(void);
    int load_league_config(void);
    int load_fashion_config(void);
    int load_sword_pool_config(void);
    int load_transfer_config(void);
    int load_mount_config(void);
    int load_beast_config(void);
    int load_arena_config(void);

    int load_monster_config(void);
    int update_monster_config(void);

    int load_role_config(void);
    int load_limit_config(void);
    int load_market_config(void);

    int insert_total_task(ConfigMap &task_map, IntSetMap &npc_task_map);
    int load_task_config(void);

    int load_achieve_config(void);
    int load_activity_config(void);
    int load_welfare_config(void);
    int load_brocast_config(void);
    int load_relax_play_config(void);
    int load_wedding_config(void);

    void update_game_ram(const std::string &folder);
    void combine_json_file_a(Json::Value& src_json, const string& path_file);
    void combine_json_file_b(Json::Value& src_json, const string& path_file);

    int load_travel_config(void);
    int load_world_boss_config(void);

    int load_cornucopia_config();

    int load_fashion_act_config();

    int load_labour_act_config();

    int load_offline_vip_config();

    int load_molding_spirit_config();

    int load_fish_type_config();

    int load_goddess_bless_config(void);

    int load_special_box_config();
private:
    bool is_map_server_;
    string server_ip_;	//外网IP

    ServerInfo* server_info_;
    ServerConfig server_config_;

    SceneConfig scene_config_;
    ScriptConfig script_config_;

    ItemConfig item_config_;
    EquipConfig equip_config_;
    FightConfig fight_config_;

    RoleConfig role_config_;
    LimitConfig limit_config_;
    AIBTConfig ai_bt_config_;
    TinyConfig tiny_config_;

    LeagueConfig league_config_;
    FashionConfig fashion_config_;
    SwordPoolConfig sword_pool_config_;
    TransferConfig transfer_config_;
    MountConfig mount_config_;
    BeastConfig beast_config_;

    MarketConfig market_cfg_;
    TaskConfig task_cfg_;
    ArenaConfig arena_cfg_;

    RelaxPlayConfig relax_play_config_;
    AchieveConfig achieve_config_;
    ActivityConfig activity_config_;
    Welfare welfare_config_;
    BrocastConfig brocast_config_;
    WeddingConfig wedding_config_;
    TravelConfig travel_config_;
    WorldBossConfig world_boss_config_;
    CornucopiaConfig cornucopia_config_;

    Time_Value update_tick_;
    BIntSet null_int_set_;

    FashionActConfig fashion_act_config_;
    LabourActConfig labour_act_config_;

    OfflineVipConfig offline_vip_config_;
    MoldingSpiritConfig molding_spirit_config_;

    FishTypeConfig fish_score_reward_config_;
    GoddessBlessConfig goddess_bless_config_;

    SpecialBoxConfig special_box_config_;
};

typedef Singleton<GameConfig> GameConfigSingle;
#define CONFIG_INSTANCE GameConfigSingle::instance()

#endif //_GAMECONFIG_H_
