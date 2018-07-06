/*
 * AIStruct.h
 *
 * Created on: 2013-04-02 11:21
 *     Author: lyz
 */

#ifndef _AISTRUCT_H_
#define _AISTRUCT_H_

#include "Heap.h"
#include "MapMapStruct.h"
#include "queue"

namespace GameEnum
{
	/*
	 *	AI Default Property
	 * */
	enum AI_DEFAULT_PRO
	{
		AI_RAND_MOVE_RADIUS 	= 3,
		AI_SELECT_RADIUS 		= 8,
		AI_BACK_DISTANCE 		= 6,
		AI_CHASE_BACK_DISTANCE 	= 18,
		AI_DIE_TIME				= 1,
		AI_SLEEP_TIME			= 3,
		AI_MOVE_SPAN_TIME		= 4,
		AI_DEFAULT_END
	};

	/*
	 * AI Condition Flag
	 * */
	enum AI_CONDITION_FLAG
	{
		AI_CF_NONE,
		AI_CF_NO_MOVE,				//不能移动
		AI_CF_NO_ATTACK,			//不能攻击
		AI_CF_NO_BE_ATTACKED,		//不能被攻击
		AI_CF_NO_AIM,				//不需要有目标
		AI_CF_RECYCLE_ON_ATTACKED,	//攻击后自动回收
		AI_CF_NO_PUSH,              //免疫推拉技能
		AI_CF_END
	};

    enum AI_CONFIG_TYPE
    {
        AI_CT_SCENE 		= 1,	// 读取从场景配置文件中加载
        AI_CT_MONSTER	 	= 2,	// 从怪物配置文件加载

        AI_CT_END
    };

    // 全屏技能安全区域类型
    enum AI_SAFE_AREA_TYPE
    {
        AI_SAT_DEFENDER     = 1,    // 被攻击者周围生成
        AI_SAT_ATTACK       = 2,    // 攻击者周围生成

        AI_SAT_END
    };
}

class ScriptAI;
typedef HashMap<Int64, ScriptAI *, NULL_MUTEX> ScriptAIMap;
typedef std::bitset<GameEnum::AI_CF_END> AIConditionFlag;

// (p1, p2) * (p1, p3)
extern double cross(MoverCoord &p1, MoverCoord &p2, MoverCoord &p3);
extern bool is_in_polygon(std::vector<MoverCoord> &coord_list, MoverCoord &point);

struct AIDetail
{
	//怪物种类
    int __sort;
    string __name;

    int __layout_index;	//配置Index
    int __born_span;	//出生间隔

    // 怪物的可见模型
    int __view_sort;
    int __monster_type;

    // 自动修改周围玩家血量
    int around_blood_dis_;
    int around_blood_per_;
    int around_blood_span_;
    Int64 around_use_tick_;

    //条件标识
    AIConditionFlag __condition_flag;

    //出生点坐标
    MoverCoord __birth_coord;
    //荒野奇袭一次移动的目标坐标
    MoverCoord __qixi_aim_coord;
    //产生时间
    Time_Value __born_tick;
    //自动回收时间
    Time_Value __recycle_tick;
    //自动定时无目标攻击
    std::map<std::string, Time_Value> __auto_attack_tick;
    //普通场景AI无目标时停顿标识
    int __pause_flag;

    //怪物配置读取类型, AI_CONFIG_TYPE
    int __config_type;
    string __born_type;

    int __is_back;
    int __is_remote;	//是否是远程怪
    Int64 __killed_id;

    // 采集用
    int __gather_count;
    int __touch_count;
    LongMap __toucher_map;

    //怪物召唤者ID，没有则为0
    Int64 __caller;

    //跟随目标ID 没有为0
    Int64 __leader;
    // 连招步骤
    int __combo_tag;
    // 战斗阶段
    int __fight_stage;
    std::queue<int> __random_combo_skill;
    // 定时动作时间
    Time_Value __next_interval;
    Time_Value __interval;

    // 用于处理追捕玩家时生成包围效果
    int __chase_index;
    int __chase_distance;
    int __chase_x_start;
    int __chase_y_start;

    // 指定巡逻路径的索引点
    int __patrol_index;

    // 安全区域中心点集合
    typedef std::vector<MoverCoord> SafePointList;
    SafePointList __safe_points;
    LongVec	__safe_point_ids;

    // 安全区域半径
    int __safe_radius;  // 单位格子
    int __safe_radius_pixel;    // 单位像素

    // 怪物没有攻击对象时固定一定时间才走回出生点
    int __extend_back_distance;
    Time_Value __idle_back_tick;

    // 用于任务统计，只要对怪物造成过伤害都会记录
    typedef boost::unordered_map<Int64, int> PlayerHurtMap;
    PlayerHurtMap __hurt_map;
    Int64 __max_hurt_id;

    ThreeObjVec left_blood_skill_;
    IntMap __left_blood_skill_use;	//key: skill_id, value:times
    IntSet __avoid_buff_set;

    // 移到目标点(moveto_action的目标)
    MoverCoord __moveto_action_coord;

    //镖车仙盟
    Int64 league_index_;
    string league_name_;

    //修改血量模式
    int modify_blood_mode_;
    int hurt_count_;
    int reward_exp_mode_;

    int __volume;			//体积
    int __full_screen_chase;	//全屏追击
    int __force_hurt;
    double __attack_interval;
    double __hit_stiff_last;    // 受击僵直持续时间

    bool __last_is_move;	// 行为树上一次是移动
    bool __is_interval_attack;	// 是否间隔攻击

    // 怪物掉落归属玩家集合
    LongSet __drop_owner_set;

    void reset(void);
};

struct AIAreaIndexInfo
{
	int ai_sort_;
	LongMap move_map_;
	CoordVec move_set_;
};

struct AIDropDetail
{
	enum
	{
		TYPE_MONEY 		= 0,		//金钱
		TYPE_GOODS 		= 1,		//物品
		MAX_DROP_TIME 	= 60,		//掉落最长时间
		MAX_BORN_RANGE	= 3,		//掉落范围
        MAX_DELAY_ALL_TIME = 60,    //转为全服可拾取的时间
        MAX_SCIRPT_ALL_TIME = 300,	//副本最长时间
		END
	};

	Int64 drop_id_;

	Int64 ai_id_;
	LongMap player_map_;
	LongMap	no_view_;

	int ai_sort_;
	int drop_type_;
	int team_share_;
	int pick_up_prep_;

	IntMap money_map_;
	PackageItem item_obj_;

	int no_auto_pickup_;	// 1 通知客户端不要自动拾取

    Time_Value drop_tick_;  // 掉落保护时间
    Time_Value recycle_tick_;   // 自动回收间隔

	void reset();
	int goods_type();
};

struct SceneAIRecord : public HeapNode
{
    int __config_index;
    int __record_index;

    double __left_fresh_sec;
    Time_Value __fresh_tick;
    int __wave_id;

    ScriptAIMap __script_ai_map;    // key: ai_id;

    struct BirthRecord
    {
        MoverCoord __birth_coord;
        int __level_index;
        int __chase_index;

        BirthRecord(void);
        void reset(void);
    };
    typedef std::list<BirthRecord> BirthRecordList;
    BirthRecordList __birth_record_list;
    
    void reset(void);
};

class AIRecordCmp
{
public:
    bool operator() (SceneAIRecord *&left, SceneAIRecord *&right);
};

struct ScriptAIDetail
{
    int __scene_config_index;
    int __record_index;
    int __level_index;
    int __wave_id;

    int __script_sort;

    void reset(void);
};

struct AIHurtSort
{
    Int64 __role_id;
    int __value;

    AIHurtSort(void);
};
class AIHurtSortCmp
{
public:
    bool operator() (const AIHurtSort &left, const AIHurtSort &right);
};

#endif //_AISTRUCT_H_
