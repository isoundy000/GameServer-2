/*
 * File Name: TrvlBattleMonitor.h
 * 
 * Created on: 2017-04-17 21:13:57
 * Author: glendy
 * 
 * Last Modified: 2017-05-15 15:06:00
 * Description: 
 */

#ifndef _TRVLBATTLEMONITOR_H_
#define _TRVLBATTLEMONITOR_H_

#include "Singleton.h"
#include "GameHeader.h"
#include "ObjectPoolEx.h"
#include "PubStruct.h"

template<class Key, class Value, class HSMUTEX> class HashMap;

class Proto30400051;
class ProtoTrvlBaseRole;
class ProtoTrvlBattleRank;
class TrvlBattleScene;
class BattleGroundActor;

struct TrvlBaseRole
{
    Int64 __role_id;
    std::string __role_name;    // 玩家名
    std::string __prev;     // 区服
    std::string __server_flag;  // 服务器标识
    
    int __sex;
    int __career;
    int __level;
    int __force;
    int __weapon;       // 武器
    int __clothes;      // 衣服
    int __wing_level;       // 翅膀等级
    int __solider_level;    // 神兵等级
    int __vip_type;     // vip等级
    int __mount_sort;   // 战骑
    int __sword_pool;   // 剑池等级
    int __tian_gang;    // 天罡
    int __fashion_id;   // 正在装备的时装id
    int __fashion_color;    // 正在使用的时装颜色

    void reset(void);
    void serialize(ProtoTrvlBaseRole *msg);
    void unserialize(const ProtoTrvlBaseRole &msg);
};


struct TrvlBattleRanker : public TrvlBaseRole
{
    int __rank;         // 排名
    int __score;        // 积分
    int __rank_score;   // 排行看到的积分
    int __total_kill_amount;    // 总击杀数
    Int64 __tick;       // 上榜时间

    void reset(void);
    void serialize(ProtoTrvlBattleRank *msg);
};

bool trvl_battle_ranker_cmp(const TrvlBattleRanker *left, const TrvlBattleRanker *right);

struct TrvlBattleRole : public BaseServerInfo
{
    Int64 __role_id;
    Int64 __league_id;
    std::string __role_name;
    int __max_floor;            // 最大层数

    int __cur_floor;			// 当前层数
    int __cur_floor_kill_amount; // 当前层击杀数
    int __last_reward_score;    // 上一个奖励的积分值
    int __next_reward_score;    // 下一个奖励的积分值
    int __next_score_reward_id; // 下一个奖励的ID
    int __even_kill_amount;     // 连斩数

    void reset(void);
    int next_award_score(void);
};

class TrvlBattleMonitor
{
public:
    friend  class MMOTrvlBattle;

    typedef ObjectPoolEx<TrvlBattleRanker> TrvlBattleRankerPool;
    typedef HashMap<Int64, TrvlBattleRanker *, NULL_MUTEX> TrvlBattleRankerMap;
    typedef std::vector<TrvlBattleRanker *> TrvlBattleRankList;

    typedef ObjectPoolEx<TrvlBattleRole> TrvlBattleRolePool;
    typedef HashMap<Int64, TrvlBattleRole *, NULL_MUTEX> TrvlBattleRoleMap;

    typedef ObjectPoolEx<TrvlBattleScene> TrvlBattleScenePool;
    typedef HashMap<int, TrvlBattleScene *, NULL_MUTEX> TrvlBattleSceneMap;

    class BattleStageTimer : public GameTimer
    {
    public:
        virtual int type(void);
        virtual int handle_timeout(const Time_Value &tv);
    };
    
    class BattleSecTimer : public GameTimer
    {
    public: 
        virtual int type(void);
        virtual int handle_timeout(const Time_Value &tv);
    };
    
public:
    TrvlBattleMonitor(void);
    ~TrvlBattleMonitor(void);

    int start(void);
    int stop(void);

    int scene_id(void);
    int activity_id(void);
    MoverCoord fetch_enter_pos(void);

    int handle_battle_stage_timeout(const Time_Value &nowtime);
    int handle_battle_stage_change(const int prev_stage, const int cur_stage);
    int handle_timeout(const Time_Value &nowtime);

    bool is_started_battle_activity(void);
    int left_actvity_sec(void);

    void ahead_event(void);
    void start_event(void);
    void stop_event(void);
   
    void recycle_scene(TrvlBattleScene *scene);
    TrvlBattleScene *trvl_battle_scene_by_floor(const int floor);

    void update_info_by_kill_player(const Int64 killer_role_id, const Int64 die_role_id, TrvlBattleScene *scene);
    void update_info_by_kill_monster(const Int64 killer_role_id, const Int64 ai_id, const int ai_sort, TrvlBattleScene *scene);

    int request_enter_battle(const int sid, Proto30400051 *request);

    void update_tb_role_of_kill(TrvlBattleScene *scene, TrvlBattleRole *killer_tb_role, MapPlayerEx *killer_player);
    void update_tb_role_of_die(TrvlBattleScene *scene, TrvlBattleRole *die_tb_role, MapPlayerEx *die_player);

    TrvlBattleRole *find_tb_role(const Int64 role_id);
    TrvlBattleRanker *find_tb_ranker(const Int64 role_id);
    TrvlBattleRanker *find_tb_ranker_by_rank(const int rank);

    int max_floor(void);
    Int64 first_top_id(void);
    std::string &first_top_name(void);
    void set_top_player(BattleGroundActor *player);

    Int64 treasure_owner_id(void);
    std::string &treasure_owner_name(void);
    int treasure_left_sec(void);
    void set_treasure_info(const Int64 role_id, const std::string &name, const Time_Value &check_tick);

    int process_fetch_every_list(const int sid, const Int64 role_id, Message *msg);
    int process_fetch_view_player_info(const int sid, const Int64 role_id, Message *msg);
    int process_test_activity(const int sid, const Int64 role_id, Message *msg);
    void send_first_enter_floor_reward(TrvlBattleRole *tb_role, MapPlayerEx *player);

    int process_fetch_main_pannel(const int sid, const Int64 role_id, Message *msg);
    void inc_score_only(BattleGroundActor *player, const int inc_score);
    void send_treasure_timeout_reward(BattleGroundActor *player);
    void send_practice_reward_mail(const Int64 role_id, TrvlBattleRanker *tb_ranker);

protected:
    void generate_all_trvl_battle_scene(void);
    void recycle_all_trvl_battle_scene(void);

    TrvlBattleRole *fetch_tbattle_role(const Int64 role_id);
    TrvlBattleRanker *fetch_tbattle_ranker(const Int64 role_id);

    void reinit_tb_role_base_info_by_player(TrvlBattleRole *tb_role, MapPlayerEx *player);
    void reinit_tb_ranker_base_info_by_player(TrvlBattleRanker *tb_ranker, MapPlayerEx *player);
    void calc_event_kill_score(TrvlBattleRanker *killer_tb_ranker, TrvlBattleRole *killer_tb_role);
    void calc_kill_score(TrvlBattleRanker *killer_tb_ranker, TrvlBattleRole *killer_tb_role);
    void calc_bekill_score(TrvlBattleRanker *die_tb_ranker, TrvlBattleRole *killer_tb_role);

    void check_and_send_score_reward(TrvlBattleRanker *tb_ranker, TrvlBattleRole *tb_role, MapPlayerEx *player);

    void handle_sort_rank(void);

    int make_up_last_rank_list(Message *msg, const Int64 self_role, const int refresh = 0);
    int make_up_history_top_list(Message *msg, const Int64 self_role, const int refresh = 0);
    int make_up_cur_rank_list(Message *msg, const Int64 self_role, const int refresh = 0);
    int make_up_indiana_list(Message *msg, const Int64 self_role, const int refresh = 0);

    void copy_cur_rank_to_latest_rank(void);
    void send_all_tbattle_res_reward(void);
    void kick_all_online_player(void);

protected:
    int activity_id_;
    int enter_level_;
    int max_floor_;

    Time_Value activity_end_tick_;

    BattleStageTimer battle_stage_timer_;
    BattleSecTimer battle_sec_timer_;
    ActivityTimeInfo time_info_;    // 根据common_activity.json计算

    TrvlBattleRankerPool *trvl_battle_ranker_pool_;
    TrvlBattleRankerMap *cur_ranker_map_;           // 当前活动的战场排行索引, key: role_id
    TrvlBattleRankList *cur_ranker_list_;           // save 当前活动的战场排行

    TrvlBattleRolePool *trvl_battle_role_pool_;
    TrvlBattleRoleMap *battle_role_map_;            // save 当前活动层数奖励记录, key: role_id

    TrvlBattleRankerMap *last_ranker_map_;          // 上一次战场排行日志信息索引,key: role_id
    TrvlBattleRankList *last_ranker_list_;          // save 上一次战场排行日志

    TrvlBattleRankList *history_hall_of_fame_list_; // save 历史名人堂

    TrvlBattleRankList *indiana_list_;              // 夺宝记录

    TrvlBattleScenePool *trvl_battle_scene_pool_;

    bool is_created_scene_;
    TrvlBattleSceneMap *scene_map_; // key: floor

    bool is_sort_rank_;
    Time_Value sort_rank_tick_;

    Int64 first_top_id_;            // 首位登顶ID
    std::string first_top_name_;    // 首位登顶名字

    Int64 treasure_owner_;          // 秘宝拥有者
    std::string treasure_owner_name_;
    Time_Value treasure_timeout_tick_;

    BLongMap last_rank_list_page_;      // key: role_id, value: page number
    BLongMap history_top_list_page_;    // key: role_id, value: page number
    BLongMap cur_rank_list_page_;       // key: role_id, value: page number
    BLongMap indiana_list_page_;        // key: role_id, value: page number
};

typedef Singleton<TrvlBattleMonitor> TrvlBattleMonitorSingle;
#define TRVL_BATTLE_MONITOR     (TrvlBattleMonitorSingle::instance())

#endif //TRVLBATTLEMONITOR_H_
